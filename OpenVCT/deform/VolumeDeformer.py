import OpenGL.GL as gl
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GL.shaders import compileProgram, compileShader
import numpy as np
from tifffile import imwrite
import zipfile as zip
import glfw
import xml.etree.ElementTree as ET
import time
import logging as log
import ctypes

class VolumeDeformer:
    def __init__(self):
        # Initialize variables similar to the constructor in C++
        self.hRC2 = None

        #undeformed
        self.vxlCntIn_x = 0.0
        self.vxlCntIn_y = 0.0
        self.vxlCntIn_z = 0.0
        self.voxelSize_x = 0.0
        self.voxelSize_y = 0.0
        self.voxelSize_z = 0.0

        self.nodes_feb = None
        self.elements_feb = None

        self.def_mode = 'DEFORM_ML'
        self.hadValidRC = False
        self.textureID = None
        self.deformedTextureID = None
        self.frameBuffer = None

        #deformed
        self.fxDim = 0.0
        self.fyDim = 0.0
        self.fzDim = 0.0
        self.def_ratio_x = 1.0
        self.def_ratio_y = 1.0
        self.def_ratio_z = 1.0
        self.shader_program = None

    def set_phantom_voxel_counts(self, vxlCntX, vxlCntY, vxlCntZ):
        # Set voxel counts from phantom data
        self.vxlCntIn_x = float(vxlCntX)
        self.vxlCntIn_y = float(vxlCntY)
        self.vxlCntIn_z = float(vxlCntZ)

    def set_phantom_voxel_sizes(self, vsizeX, vsizeY, vSizeZ):
        # Set voxel sizes in meters
        self.voxelSize_x = vsizeX * 0.001  # Convert from mm to meters
        self.voxelSize_y = vsizeY * 0.001
        self.voxelSize_z = vSizeZ * 0.001

    def set_deformation_ratios(self, ratio_x, ratio_y, ratio_z):
        # Set deformation ratios explicitly if needed
        self.def_ratio_x = ratio_x
        self.def_ratio_y = ratio_y
        self.def_ratio_z = ratio_z

    def process(self, pdlog, original_mesh, deformed_mesh):
        # Set up OpenGL and process the mesh data
        pdlog.info("Setting up OpenGL context.")
        
        OpenGLVersion = glGetIntegerv(GL_MAJOR_VERSION), glGetIntegerv(GL_MINOR_VERSION)
        GLSLVersion = glGetString(GL_SHADING_LANGUAGE_VERSION).decode('utf-8')
        
        pdlog.info(f"* OpenGL version {OpenGLVersion[0]}.{OpenGLVersion[1]}, GLSL version: {GLSLVersion} *")

        # Framebuffer setup
        self.frameBuffer = glGenFramebuffers(1)
        glBindFramebuffer(GL_FRAMEBUFFER, self.frameBuffer)
        pdlog.info(f"Framebuffer created with ID: {self.frameBuffer}")

        # Compile shaders
        self.build_glsl_program(pdlog)

        # Read the mesh files
        self.nodes_feb, self.elements_feb = self.read_source_mesh_febio(pdlog, original_mesh)
        self.read_def_nodes_febio(pdlog, deformed_mesh)

        # Create and bind buffers
        vbo32 = glGenBuffers(2)
        pdlog.info(f"Generated VBOs with IDs: {vbo32}")
        
        glBindBuffer(GL_ARRAY_BUFFER, vbo32[0])
        glBufferData(GL_ARRAY_BUFFER, len(self.nodes_feb) * 4, (GLfloat * len(self.nodes_feb))(*self.nodes_feb), GL_STATIC_DRAW)
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo32[1])
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, len(self.elements_feb) * 4, (GLuint * len(self.elements_feb))(*self.elements_feb), GL_STATIC_DRAW)

        # Set vertex attributes (positions and UVs)
        vertex_stride = 6 * 4  # 6 floats (3 for position, 3 for UV), 4 bytes per float
        position_location = glGetAttribLocation(self.shader_program, "myVertex")
        uv_location = glGetAttribLocation(self.shader_program, "myUV")
        
        glEnableVertexAttribArray(position_location)
        glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, vertex_stride, ctypes.c_void_p(0))
        
        glEnableVertexAttribArray(uv_location)
        glVertexAttribPointer(uv_location, 3, GL_FLOAT, GL_FALSE, vertex_stride, ctypes.c_void_p(12))

        pdlog.info("OpenGL setup and VBO creation complete.")
        return True

    def build_glsl_program(self, log):
        # Translate shader compilation from C++
        log.info(f"Building shaders program...")

        # Fragment Shader source (as defined in C++ code)
        frag_shader_source = """
        #version 420
        uniform usampler3D sampler3d;
        smooth in vec3 triTexture;
        out uint outColor;
        void main (void) {
            uint texel = texture(sampler3d, triTexture).x;
            outColor = texel;
            if (outColor == 0) discard;
        }
        """

        # Vertex Shader source
        vert_shader_source = """
        #version 420
        in vec3 myVertex;
        in vec3 myUV;
        out vec3 tetraTexture;
        void main(void) {
            gl_Position = vec4(myVertex, 1.0);
            tetraTexture = myUV;
        }
        """

        # Geometry Shader source (optional)
        geo_shader_source = """
        #version 420
        layout (lines_adjacency) in;
        layout (triangle_strip, max_vertices = 4) out;
        in vec3 tetraTexture[4];
        smooth out vec3 triTexture;
        uniform vec4 normalPlane;
        uniform vec4 pointPlane;
        uniform vec4 offset;
        void main(void) {
            int count = 0;
            int EndPtAdded = 0;
            for (int i = 0; i < 4; i++) {
                for (int j = i + 1; j < 4; j++) {
                    vec4 v1 = gl_in[i].gl_Position;
                    vec4 v2 = gl_in[j].gl_Position;
                    float dotp = dot(normalPlane, v2 - v1);
                    if (dotp != 0.0) {
                        int AddEndPt = 0;
                        float u = dot(normalPlane, pointPlane - v1) / dotp;
                        if ((EndPtAdded == 0) && ((u == 1.0) || (u == 0.0))) {
                            AddEndPt = 1;
                            EndPtAdded = 1;
                        }
                        if ((AddEndPt == 1) || ((u < 1.0) && (u > 0.0))) {
                            triTexture = u * tetraTexture[j] + (1.0 - u) * tetraTexture[i];
                            gl_Position = v1 + u * (v2 - v1);
                            gl_Position.x = 2.0 * (gl_Position.x - offset.x);
                            gl_Position.y = 2.0 * (gl_Position.y - offset.y);
                            triTexture.z = 1.0 * (triTexture.z - offset.z);
                            EmitVertex();
                            count++;
                        }
                    }
                }
            }
            if (count > 0) EndPrimitive();
        }
        """

        # Compile the shaders and assign the program to self.shader_program
        self.shader_program = compileProgram(
            compileShader(vert_shader_source, GL_VERTEX_SHADER),
            compileShader(frag_shader_source, GL_FRAGMENT_SHADER),
            compileShader(geo_shader_source, GL_GEOMETRY_SHADER)
        )
        log.info(f"Deformer Vertex Shader compiled.")
        log.info(f"Deformer Fragment Shader compiled.")
        log.info(f"Deformer Geometry Shader compiled.")

        # Use the shader program
        glUseProgram(self.shader_program)
        log.info(f"Shaders linked.")

    def load_volume(self, pdlog, volume):
        # Load the volume as a 3D texture on the GPU
        self.input_texture = volume
        self.input_texture_size = self.vxlCntIn_x * self.vxlCntIn_y * self.vxlCntIn_z
        pdlog.info(f"Uncompressed texture size is {self.input_texture_size}")

        if self.textureID:
            glDeleteTextures(1, [self.textureID])
        self.textureID = glGenTextures(1)
        glBindTexture(GL_TEXTURE_3D, self.textureID)

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1)

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI,
                     int(self.vxlCntIn_x), int(self.vxlCntIn_y), int(self.vxlCntIn_z),
                     0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, self.input_texture)
        pdlog.info(f"Texture ID {self.textureID} created.")
        return self.textureID

    def set_volume(self, pdlog):
        # Setup the deformed volume
        self.fxDim = self.vxlCntIn_x * self.def_ratio_x
        self.fyDim = self.vxlCntIn_y * self.def_ratio_y
        self.fzDim = self.vxlCntIn_z * self.def_ratio_z
        pdlog.info(f"Output phantom sizes: {self.fxDim} x {self.fyDim} x {self.fzDim}")

        if self.deformedTextureID:
            glDeleteTextures(1, [self.deformedTextureID])
        
        self.deformedTextureID = glGenTextures(1)
        glBindTexture(GL_TEXTURE_3D, self.deformedTextureID)

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER)
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1)

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI,
                     self.fxDim, self.fyDim, self.fzDim,
                     0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, None)
        
        pdlog.info(f"Deformed texture ID {self.deformedTextureID} created.")

        #cast dimensions to int here
        self.fxDim = (int)(self.fxDim)
        self.fyDim = (int)(self.fyDim)
        self.fzDim = (int)(self.fzDim)
        
        return self.deformedTextureID

    def read_source_mesh_febio(self, log, file_path):

        nodes, elements = [], []

        # Start timer for performance tracking
        start_time = time.time()
        
        log.info("Reading original nodes")
        
        try:
            # Parse XML document
            tree = ET.parse(file_path)
            root = tree.getroot()
            
            geometry_node = root.find("./Geometry/Nodes")
            
            if geometry_node is not None:
                log.info(f"Found geometry nodes in {file_path}")
                
                min_d, min_c, min_b = float('inf'), float('inf'), float('inf')
                max_d, max_c, max_b = float('-inf'), float('-inf'), float('-inf')
                
                for tool in geometry_node:
                    line = tool.text.strip()
                    b, c, d = map(float, line.split(','))

                    # Push dummy entries for deformed mesh (to be filled later)
                    nodes.extend([0.0, 0.0, 0.0])

                    # Push undeformed mesh in meters (d, c, b correspond to x, y, z)
                    nodes.extend([d, c, b])

                    # Update min/max values
                    min_d, max_d = min(min_d, d), max(max_d, d)
                    min_c, max_c = min(min_c, c), max(max_c, c)
                    min_b, max_b = min(min_b, b), max(max_b, b)
                
                log.info(f"Range of Node locations - undeformed (in mm):")
                log.info(f"\tx ranges from {min_d} to {max_d}")
                log.info(f"\ty ranges from {min_c} to {max_c}")
                log.info(f"\tz ranges from {min_b} to {max_b}")
            
            # Read the material section to identify rigid bodies
            material_node = root.find("./Material")
            material_map = {}
            
            for tool in material_node:
                material_type = tool.get('type')
                material_id = int(tool.get('id'))
                is_rigid = "rigid" in material_type.lower()
                material_map[material_id] = is_rigid

            for material_id, is_rigid in material_map.items():
                log.info(f"MaterialMap item: {material_id}, {'rigid' if is_rigid else 'non-rigid'}")

            # Read the elements and filter out the rigid body elements
            elements_node = root.find("./Geometry/Elements")
            if elements_node is not None:
                element_count = 0

                min_d, min_c, min_b = float('inf'), float('inf'), float('inf')
                max_d, max_c, max_b = float('-inf'), float('-inf'), float('-inf')
                
                for tool in elements_node:
                    line = tool.text.strip()
                    c, b, e, d = map(int, line.split(','))

                    mat = int(tool.get('mat'))
                    if not material_map[mat]:  # If not rigid, process the element
                        # Push element vertices
                        elements.extend([b - 1, c - 1, d - 1, e - 1])
                        element_count += 1

                        # Update min/max for non-rigid nodes
                        for idx in [b, c, d, e]:
                            dval, cval, bval = nodes[(idx - 1) * 6 + 3: (idx - 1) * 6 + 6] # 5 in c++
                            min_d, max_d = min(min_d, dval), max(max_d, dval)
                            min_c, max_c = min(min_c, cval), max(max_c, cval)
                            min_b, max_b = min(min_b, bval), max(max_b, bval)
                
                log.info(f"Element count: {element_count*4}") #from b to e
                log.info(f"Range of Node locations - undeformed without compression plates (in mm):")
                log.info(f"\tx ranges from {min_d} to {max_d}")
                log.info(f"\ty ranges from {min_c} to {max_c}")
                log.info(f"\tz ranges from {min_b} to {max_b}")

        except ET.ParseError as e:
            log.error(f"Can't load the .feb file \"{file_path}\": {e}")
            raise SystemExit(f"Error parsing FEBio XML file: {e}")
        
        # Log the time taken to execute the function
        log.info(f"Entire read_source_mesh_febio() function took {int((time.time() - start_time) * 1000)} ms.")
        return nodes, elements

    def read_def_nodes_febio(self, log, file_path):
        # Start timer for performance tracking
        start_time = time.time()
        
        log.info(f"Reading deforming nodes from {file_path}")

        try:
            with open(file_path, 'r') as node_file:
                found = 0
                lines = node_file.readlines()

                # Find the position of the last occurrence of "Data = ux;uy;uz"
                for i, line in enumerate(lines):
                    if "Data = ux;uy;uz" in line:
                        found = i

                log.info("Have advanced to final iteration of 'Data' in the log file... will now read it.")

                # Initialize min/max values for deformed and undeformed nodes
                mindnew, mincnew, minbnew = float('inf'), float('inf'), float('inf')
                maxdnew, maxcnew, maxbnew = float('-inf'), float('-inf'), float('-inf')
                
                if found > 0:
                    index = 0
                    
                    log.info(f"Voxel size: {self.voxelSize_x}, voxel counts: {self.vxlCntIn_x} {self.vxlCntIn_y} {self.vxlCntIn_z}")

                    # Start reading the deformation data from the position of the last "Data = ux;uy;uz"
                    for line in lines[found+1:]:
                        line = line.strip()
                        if line:
                            try:
                                a, b, c, d = map(float, line.split())

                                # Add deformation to undeformed node (node positions in meters)
                                newd = self.nodes_feb[index*6+3] + d
                                newc = self.nodes_feb[index*6+4] + c
                                newb = self.nodes_feb[index*6+5] + b

                                # Update the deformed node array based on the deformation mode
                                if self.def_mode == 'DEFORM_ML':
                                    self.nodes_feb[index*6] = newd
                                    self.nodes_feb[index*6+1] = newc
                                    self.nodes_feb[index*6+2] = newb
                                
                                elif self.def_mode == 'DEFORM_CC':
                                    self.nodes_feb[index*6] = self.nodes_feb[index*6+3] + d
                                    self.nodes_feb[index*6+1] = self.nodes_feb[index*6+4] + b
                                    self.nodes_feb[index*6+2] = self.nodes_feb[index*6+5] - c

                                # Update min/max values for deformed nodes
                                mindnew, maxdnew = min(mindnew, newd), max(maxdnew, newd)
                                mincnew, maxcnew = min(mincnew, newc), max(maxcnew, newc)
                                minbnew, maxbnew = min(minbnew, newb), max(maxbnew, newb)

                                index += 1
                            except ValueError:
                                break
                    
                    log.info(f"Range of Deformation with compression plates (in mm):")
                    log.info(f"\tx ranges from {mindnew} to {maxdnew}")
                    log.info(f"\ty ranges from {mincnew} to {maxcnew}")
                    log.info(f"\tz ranges from {minbnew} to {maxbnew}")
                    log.info(f"Number of deformed nodes: {index}")

                    # Normalize the node positions between 0 and 1
                    min_vals = [float('inf')] * 6
                    max_vals = [float('-inf')] * 6

                    log.info(f"Reading deforming nodes. Node Size: " + str(len(self.nodes_feb)))
                    log.info(f"Reading deforming nodes. Element Size: " + str(len(self.elements_feb)))

                    for i in range(0, len(self.elements_feb)):
                        for j in range(0, 6):
                            min_vals[j] = min(min_vals[j], self.nodes_feb[self.elements_feb[i]*6 + j])
                            max_vals[j] = max(max_vals[j], self.nodes_feb[self.elements_feb[i]*6 + j])

                    # Calculate deformation ratios based on min/max values
                    self.set_deformation_ratios((max_vals[0] - min_vals[0]) / (max_vals[3] - min_vals[3]),
                                                (max_vals[1] - min_vals[1]) / (max_vals[4] - min_vals[4]),
                                                (max_vals[2] - min_vals[2]) / (max_vals[5] - min_vals[5]))
                    
                    log.info(f"Deformation ratios: def_ratio_x = {self.def_ratio_x}, def_ratio_y = {self.def_ratio_y}, def_ratio_z = {self.def_ratio_z}")
                    
                    for i in range(0, len(self.nodes_feb), 6):
                        for j in range(3):  # Normalize both deformed (first 3) and undeformed (last 3) nodes
                            self.nodes_feb[i + j] = (self.nodes_feb[i + j] - min_vals[j]) / (max_vals[j] - min_vals[j])
                            self.nodes_feb[i + j + 3] = (self.nodes_feb[i + j + 3] - min_vals[j + 3]) / (max_vals[j + 3] - min_vals[j + 3])

                    
                    log.info(f"Normalization complete. Min/max values normalized. This information is only used to debug the code.")
                    for j in range(6):
                        log.info(f"Min["+str(j)+"] = "+str(min_vals[j]) + ", Max["+str(j)+"] = "+str(max_vals[j]) + ", Delta["+str(j)+"] = "+str(max_vals[j]-min_vals[j]))

        except FileNotFoundError:
            log.error(f"Can't load the log file {file_path}. Exiting.")
            raise SystemExit(f"Can't load the log file {file_path}. Exiting.")
        
        # Log the total execution time
        log.info(f"Entire read_def_nodes_febio() function took {int((time.time() - start_time) * 1000)} ms.")

    def draw(self, pdlog):
        if self.frameBuffer is None:
            pdlog.error("Framebuffer is NULL")
            raise SystemExit("Framebuffer is NULL")

        pdlog.info("Start Deforming")
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA)

        glBindFramebuffer(GL_FRAMEBUFFER, self.frameBuffer)

        # Use the shader program
        glUseProgram(self.shader_program)

        # Bind the input 3D texture (original phantom)
        glBindTexture(GL_TEXTURE_3D, self.textureID)

        glEnable(GL_BLEND)
        glDisable(GL_DEPTH_TEST)
        glDepthFunc(GL_ALWAYS)
        glBlendFunc(GL_ONE, GL_ONE)

        # Set the sampler3D uniform to use the first texture unit (unit 0)
        glUniform1i(glGetUniformLocation(self.shader_program, "sampler3d"), 0)

        # Get the location of uniform variables in the shader
        iNormalPlane = glGetUniformLocation(self.shader_program, "normalPlane")
        iPointPlane = glGetUniformLocation(self.shader_program, "pointPlane")
        iOffset = glGetUniformLocation(self.shader_program, "offset")

        # Set constant uniform values for normalPlane and offset (done only once)
        glUniform4f(iNormalPlane, 0.0, 0.0, 1.0, 0.0)  # Fixed normal to the plane
        glUniform4f(iOffset, 0.5, 0.5, 0.0, 0.0)  # Offset to center the texture in x and y

        # Buffer to hold each output slice (one slice at a time)
        pdlog.info(f"Output phantom sizes: {self.fxDim} x {self.fyDim} x {self.fzDim}")
        output_volume = np.zeros((self.fxDim, self.fyDim, self.fzDim), dtype=np.uint8)
        print(output_volume.shape)

        # Render the phantom slice by slice
        for i in range(output_volume.shape[0]):
            # Bind the deformed texture as the framebuffer attachment for this slice
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, self.deformedTextureID, 0, i)

            # Set the draw buffers
            glDrawBuffers(1, [GL_COLOR_ATTACHMENT0])

            # Set the point plane for this slice
            glUniform4f(iPointPlane, 0.0, 0.0, (i + 0.5) / self.fzDim, 1.0)

            # Clear the framebuffer before rendering the slice
            glViewport(0, 0, self.fxDim, self.fyDim)
            glClearColor(0.0, 0.0, 0.0, 0.0)
            glClear(GL_COLOR_BUFFER_BIT)

            # Bind the original 3D texture and draw elements (deforming process)
            glBindTexture(GL_TEXTURE_3D, self.textureID)
            glDrawElements(GL_LINES_ADJACENCY, len(self.elements_feb), GL_UNSIGNED_INT, None)

            # Read pixels from the framebuffer into the output buffer for this slice
            glReadPixels(0, 0, self.fxDim, self.fyDim, GL_RED_INTEGER, GL_UNSIGNED_BYTE, output_volume[i])

            # Flush OpenGL commands to make sure the framebuffer is updated
            glFlush()

            # Log progress every 30 slices
            if i % 30 == 0:
                pdlog.info(f"Slice {i} out of {self.fzDim}")

        # Save the full 3D volume using tifffile's imwrite
        #output_volume = np.transpose(output_volume, (2, 1, 0)) #Permute X and Z to match dimensions of c++ code
        imwrite('deformed_phantom.tif', output_volume, dtype=np.uint8)
        output_volume.astype('uint8').tofile("./Phantom.dat")
        pdlog.info("Deformed phantom successfully written to Phantom.dat")

        # Unbind the VBO and framebuffer to clean up
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

        pdlog.info("Deforming process completed.")

# Main function remains largely the same but includes some refinements.
if __name__ == "__main__":
    deformer = VolumeDeformer()
    
    with zip.ZipFile('vctx/PhantomC.vctx', mode='r') as filezip:
        with filezip.open('phantomC/Phantom.dat') as file:
            voxel_data = file.read()
    
    voxel_data = np.frombuffer(bytearray(voxel_data), dtype=np.uint8).reshape(851, 628, 314)
    print(voxel_data.shape)
    voxel_data = np.transpose(voxel_data, (2, 1, 0)) #Permute X and Z to match dimensions of c++ code
    print(voxel_data.shape)
    #imwrite('Phantom.tif', voxel_data)
    
    deformer.set_phantom_voxel_counts(314, 628, 851)
    deformer.set_phantom_voxel_sizes(0.2, 0.2, 0.2)

    log.basicConfig(filename='VolumeDeformer.log', level=log.DEBUG,
                    format='%(asctime)s - %(levelname)s - %(message)s')
    logger = log.getLogger()

    if not glfw.init():
        logger.error("Failed to initialize GLFW")
        raise SystemExit("Failed to initialize GLFW")

    glfw.window_hint(glfw.VISIBLE, glfw.FALSE)
    window = glfw.create_window(800, 600, "Hidden OpenGL Window", None, None)
    if not window:
        logger.error("Failed to create GLFW window")
        glfw.terminate()
        raise SystemExit("Failed to create GLFW window")

    glfw.make_context_current(window)

    try:
        deformer.process(logger, "./meshes/cupC_200ML.feb", "./meshes/cupC_200ML.log")
        texture_id = deformer.load_volume(logger, voxel_data)
        logger.info(f"Loaded volume with texture ID: {texture_id}")
        deformed_texture_id = deformer.set_volume(logger)
        logger.info(f"Deformed texture generated with texture ID: {deformed_texture_id}")
        deformer.draw(logger)
    finally:
        glfw.destroy_window(window)
        glfw.terminate()
        logger.info("Processing completed successfully.")