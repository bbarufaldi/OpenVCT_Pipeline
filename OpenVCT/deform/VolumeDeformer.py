from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GL.shaders import compileProgram, compileShader
import glfw

import numpy as np
import logging as log
import ctypes

from readers.xml import Phantom as ph
from readers.xml import Meshes as me

class VolumeDeformer:
    def __init__(self): 
        # Initialize variables similar to the constructor in C++

        #Phantom
        self.phantom = None
        
        #OGL variables
        self.hRC2 = None
        self.hadValidRC = False
        self.textureID = None
        self.deformedTextureID = None
        self.frameBuffer = None
        self.shader_program = None

        #original phantom
        self.vxlCntIn_x = 0.0
        self.vxlCntIn_y = 0.0
        self.vxlCntIn_z = 0.0
        self.voxelSize_x = 0.0
        self.voxelSize_y = 0.0
        self.voxelSize_z = 0.0
        
        #deformed phantom
        self.fxDim = 0.0
        self.fyDim = 0.0
        self.fzDim = 0.0
        self.def_ratio_x = 1.0
        self.def_ratio_y = 1.0
        self.def_ratio_z = 1.0

        #meshes
        self.nodes_feb = None 
        self.elements_feb = None
        self.def_mode = None

    def set_phantom(self, phantom):
        self.phantom = phantom

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

    def process(self, pdlog, original_mesh, deformed_mesh, phantom):

        # Set phantom
        self.phantom = phantom

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
        mesh = me.Meshes(original_mesh, deformed_mesh, phantom, pdlog)
        self.nodes_feb, self.elements_feb, self.def_mode = mesh.read_source_mesh_febio()
        self.def_ratio_x, self.def_ratio_y, self.def_ratio_z = mesh.read_def_nodes_febio()

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
        pdlog.info(f"Output phantom sizes: {self.fzDim} x {self.fxDim} x {self.fyDim}")
        output_volume = np.zeros((self.fzDim, self.fxDim, self.fyDim), dtype=np.uint8)
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

        # Unbind the VBO and framebuffer to clean up
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

        pdlog.info("Deforming process completed.")
        return output_volume

# Main function remains largely the same but includes some refinements.
if __name__ == "__main__":

    phantom = ph.Phantom('vctx/PhantomC.vctx')
    deformer = VolumeDeformer()
    
    voxel_data = phantom.voxel_data
    #voxel_data = np.transpose(voxel_data, (2, 1, 0)) #Permute X and Z to match dimensions of c++ code (JUST to DEBUG and match logs)
    #print(voxel_data.shape)
    #imwrite('Phantom.tif', voxel_data)
    
    deformer.set_phantom_voxel_counts(voxel_data.shape[2], voxel_data.shape[1], voxel_data.shape[0]) #Permute X with Z
    vxl = phantom.get_voxel_mm()
    deformer.set_phantom_voxel_sizes(vxl[2], vxl[1], vxl[0]) #Permute X with Z

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
        deformer.process(logger, "./meshes/cupC_200ML.feb", "./meshes/cupC_200CC.log", phantom)
        texture_id = deformer.load_volume(logger, voxel_data)
        logger.info(f"Loaded volume with texture ID: {texture_id}")
        deformed_texture_id = deformer.set_volume(logger)
        logger.info(f"Deformed texture generated with texture ID: {deformed_texture_id}")
        voxel_data = deformer.draw(logger)
        phantom.write_vctx(voxel_data, phantom.get_file_name()+"2", deformer.def_mode)
        logger.info("Deformed phantom successfully written to Phantom.dat")
    
    finally:
        glfw.destroy_window(window)
        glfw.terminate()
        logger.info("Processing completed successfully.")