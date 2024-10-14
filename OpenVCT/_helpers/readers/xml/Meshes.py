import time
import xml.etree.ElementTree as ET

class Meshes:
    def __init__(self, input_mesh, output_mesh, phantom, log):
        self.input_mesh = input_mesh
        self.output_mesh = output_mesh

        if 'CC' in output_mesh:
            self.def_mode = 'DEFORM_CC'
        else:
            self.def_mode = 'DEFORM_ML'
        
        self.def_ratio_x = None
        self.def_ratio_y = None
        self.def_ratio_z = None

        self.nodes = []
        self.elements = []

        self.phantom = phantom
        self.log = log
    
    def set_deformation_ratios(self, ratio_x, ratio_y, ratio_z):
        # Set deformation ratios explicitly if needed
        self.def_ratio_x = ratio_x
        self.def_ratio_y = ratio_y
        self.def_ratio_z = ratio_z

    def read_source_mesh_febio(self):

        # Start timer for performance tracking
        start_time = time.time()
        
        self.log.info("Reading original nodes")
        
        try:
            # Parse XML document
            tree = ET.parse(self.input_mesh)
            root = tree.getroot()
            
            geometry_node = root.find("./Geometry/Nodes")
            
            if geometry_node is not None:
                self.log.info(f"Found geometry nodes in {self.input_mesh}")
                
                min_d, min_c, min_b = float('inf'), float('inf'), float('inf')
                max_d, max_c, max_b = float('-inf'), float('-inf'), float('-inf')
                
                for tool in geometry_node:
                    line = tool.text.strip()
                    b, c, d = map(float, line.split(','))

                    # Push dummy entries for deformed mesh (to be filled later)
                    self.nodes.extend([0.0, 0.0, 0.0])

                    # Push undeformed mesh in meters (d, c, b correspond to x, y, z)
                    self.nodes.extend([d, c, b])

                    # Update min/max values
                    min_d, max_d = min(min_d, d), max(max_d, d)
                    min_c, max_c = min(min_c, c), max(max_c, c)
                    min_b, max_b = min(min_b, b), max(max_b, b)
                
                self.log.info(f"Range of Node locations - undeformed (in mm):")
                self.log.info(f"\tx ranges from {min_d} to {max_d}")
                self.log.info(f"\ty ranges from {min_c} to {max_c}")
                self.log.info(f"\tz ranges from {min_b} to {max_b}")
            
            # Read the material section to identify rigid bodies
            material_node = root.find("./Material")
            material_map = {}
            
            for tool in material_node:
                material_type = tool.get('type')
                material_id = int(tool.get('id'))
                is_rigid = "rigid" in material_type.lower()
                material_map[material_id] = is_rigid

            for material_id, is_rigid in material_map.items():
                self.log.info(f"MaterialMap item: {material_id}, {'rigid' if is_rigid else 'non-rigid'}")

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
                        self.elements.extend([b - 1, c - 1, d - 1, e - 1])
                        element_count += 1

                        # Update min/max for non-rigid nodes
                        for idx in [b, c, d, e]:
                            dval, cval, bval = self.nodes[(idx - 1) * 6 + 3: (idx - 1) * 6 + 6] # 5 in c++
                            min_d, max_d = min(min_d, dval), max(max_d, dval)
                            min_c, max_c = min(min_c, cval), max(max_c, cval)
                            min_b, max_b = min(min_b, bval), max(max_b, bval)
                
                self.log.info(f"Element count: {element_count*4}") #from b to e
                self.log.info(f"Range of Node locations - undeformed without compression plates (in mm):")
                self.log.info(f"\tx ranges from {min_d} to {max_d}")
                self.log.info(f"\ty ranges from {min_c} to {max_c}")
                self.log.info(f"\tz ranges from {min_b} to {max_b}")

        except ET.ParseError as e:
            self.log.error(f"Can't load the .feb file \"{self.input_mesh}\": {e}")
            raise SystemExit(f"Error parsing FEBio XML file: {e}")
        
        # Log the time taken to execute the function
        self.log.info(f"Entire read_source_mesh_febio() function took {int((time.time() - start_time) * 1000)} ms.")
        
        return self.nodes, self.elements, self.def_mode

    def read_def_nodes_febio(self):
        # Start timer for performance tracking
        start_time = time.time()
        
        self.log.info(f"Reading deforming nodes from {self.output_mesh}")

        try:
            with open(self.output_mesh, 'r') as node_file:
                found = 0
                lines = node_file.readlines()

                # Find the position of the last occurrence of "Data = ux;uy;uz"
                for i, line in enumerate(lines):
                    if "Data = ux;uy;uz" in line:
                        found = i

                self.log.info("Have advanced to final iteration of 'Data' in the log file... will now read it.")

                # Initialize min/max values for deformed and undeformed nodes
                mindnew, mincnew, minbnew = float('inf'), float('inf'), float('inf')
                maxdnew, maxcnew, maxbnew = float('-inf'), float('-inf'), float('-inf')
                
                if found > 0:
                    index = 0
                    
                    self.log.info(f"Voxel size: {self.phantom.get_voxel_mm()}, voxel counts: {self.phantom.voxel_data.shape}")

                    # Start reading the deformation data from the position of the last "Data = ux;uy;uz"
                    for line in lines[found+1:]:
                        line = line.strip()
                        if line:
                            try:
                                a, b, c, d = map(float, line.split())

                                # Add deformation to undeformed node (node positions in meters)
                                newd = self.nodes[index*6+3] + d
                                newc = self.nodes[index*6+4] + c
                                newb = self.nodes[index*6+5] + b

                                # Update the deformed node array based on the deformation mode
                                if self.def_mode == 'DEFORM_ML':
                                    self.nodes[index*6] = newd
                                    self.nodes[index*6+1] = newc
                                    self.nodes[index*6+2] = newb
                                
                                elif self.def_mode == 'DEFORM_CC':
                                    self.nodes[index*6] = self.nodes[index*6+3] + d
                                    self.nodes[index*6+1] = self.nodes[index*6+4] + b
                                    self.nodes[index*6+2] = self.nodes[index*6+5] - c

                                # Update min/max values for deformed nodes
                                mindnew, maxdnew = min(mindnew, newd), max(maxdnew, newd)
                                mincnew, maxcnew = min(mincnew, newc), max(maxcnew, newc)
                                minbnew, maxbnew = min(minbnew, newb), max(maxbnew, newb)

                                index += 1
                            except ValueError:
                                break
                    
                    self.log.info(f"Range of Deformation with compression plates (in mm):")
                    self.log.info(f"\tx ranges from {mindnew} to {maxdnew}")
                    self.log.info(f"\ty ranges from {mincnew} to {maxcnew}")
                    self.log.info(f"\tz ranges from {minbnew} to {maxbnew}")
                    self.log.info(f"Number of deformed nodes: {index}")

                    # Normalize the node positions between 0 and 1
                    min_vals = [float('inf')] * 6
                    max_vals = [float('-inf')] * 6

                    self.log.info(f"Reading deforming nodes. Node Size: " + str(len(self.nodes)))
                    self.log.info(f"Reading deforming nodes. Element Size: " + str(len(self.elements)))

                    for i in range(0, len(self.elements)):
                        for j in range(0, 6):
                            min_vals[j] = min(min_vals[j], self.nodes[self.elements[i]*6 + j])
                            max_vals[j] = max(max_vals[j], self.nodes[self.elements[i]*6 + j])

                    # Calculate deformation ratios based on min/max values
                    self.set_deformation_ratios((max_vals[0] - min_vals[0]) / (max_vals[3] - min_vals[3]),
                                                (max_vals[1] - min_vals[1]) / (max_vals[4] - min_vals[4]),
                                                (max_vals[2] - min_vals[2]) / (max_vals[5] - min_vals[5]))
                    
                    self.log.info(f"Deformation ratios: def_ratio_x = {self.def_ratio_x}, def_ratio_y = {self.def_ratio_y}, def_ratio_z = {self.def_ratio_z}")
                    
                    for i in range(0, len(self.nodes), 6):
                        for j in range(3):  # Normalize both deformed (first 3) and undeformed (last 3) nodes
                            self.nodes[i + j] = (self.nodes[i + j] - min_vals[j]) / (max_vals[j] - min_vals[j])
                            self.nodes[i + j + 3] = (self.nodes[i + j + 3] - min_vals[j + 3]) / (max_vals[j + 3] - min_vals[j + 3])

                    
                    self.log.info(f"Normalization complete. Min/max values normalized. This information is only used to debug the code.")
                    for j in range(6):
                        self.log.info(f"Min["+str(j)+"] = "+str(min_vals[j]) + ", Max["+str(j)+"] = "+str(max_vals[j]) + ", Delta["+str(j)+"] = "+str(max_vals[j]-min_vals[j]))

        except FileNotFoundError:
            self.log.error(f"Can't load the log file {self.output_mesh}. Exiting.")
            raise SystemExit(f"Can't load the log file {self.output_mesh}. Exiting.")
        
        # Log the total execution time
        self.log.info(f"Entire read_def_nodes_febio() function took {int((time.time() - start_time) * 1000)} ms.")

        return self.def_ratio_x, self.def_ratio_y, self.def_ratio_z
