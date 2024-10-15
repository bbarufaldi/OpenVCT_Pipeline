import xml.etree.ElementTree as ET
import random
import zipfile
import os
import numpy as np

from xml.dom import minidom
from scipy.ndimage import zoom
from tifffile import imwrite
import logging as log

from readers.xml import Phantom as ph

class XMLWriter:
    def __init__(self, in_phantom, out_phantom, xml_file):
        self.Software_Name = "LesionInserter"
        self.Software_Version = "2.0"
        self.Software_ID = "1.0"
        self.Input_Phantom = in_phantom
        self.Output_Phantom = out_phantom
        self.VOIs = []
        self.Lesions = []

        # Get phantom for lesion insertion
        self.Phantom = ph.Phantom(in_phantom)
        self.Phantom.set_phantom()

        self.xml = xml_file

        # Create a log for insertion
        log.basicConfig(filename='LesionInsertion.log', level=log.DEBUG,
                        format='%(asctime)s - %(levelname)s - %(message)s')
        self.log = log.getLogger()
    
    def write_xml(self, output_file):
        root = ET.Element("name")

        # Write software information
        ET.SubElement(root, "Software_Name").text = self.Software_Name
        ET.SubElement(root, "Software_Version").text = self.Software_Version
        ET.SubElement(root, "Software_ID").text = self.Software_ID
        ET.SubElement(root, "Input_Phantom").text = self.Input_Phantom
        ET.SubElement(root, "Output_Phantom").text = self.Output_Phantom

        # Write VOIs
        vois_element = ET.SubElement(root, "VOIs")
        for lesion in self.Lesions:
            voi_element = ET.SubElement(vois_element, "VOI")
            ET.SubElement(voi_element, "Center_X").text = str(lesion.get('Center_X', 'N/A'))
            ET.SubElement(voi_element, "Center_Y").text = str(lesion.get('Center_Y', 'N/A'))
            ET.SubElement(voi_element, "Center_Z").text = str(lesion.get('Center_Z', 'N/A'))
            ET.SubElement(voi_element, "Height").text = str(lesion.get('Height', 'N/A'))
            ET.SubElement(voi_element, "Width").text = str(lesion.get('Width', 'N/A'))
            ET.SubElement(voi_element, "Depth").text = str(lesion.get('Depth', 'N/A'))
            ET.SubElement(voi_element, "Has_Lesion").text = str(lesion.get('Has_Lesion', 'true')).lower()

        # Write Lesions
        lesions_element = ET.SubElement(root, "Lesions")
        for lesion in self.Lesions:
            lesion_element = ET.SubElement(lesions_element, "Lesion")
            ET.SubElement(lesion_element, "LesionName").text = str(lesion.get('LesionName', 'Unknown'))
            ET.SubElement(lesion_element, "LesionType").text = str(lesion.get('LesionType', 'Unknown'))
            ET.SubElement(lesion_element, "Center_X").text = str(lesion.get('Center_X', 'N/A'))
            ET.SubElement(lesion_element, "Center_Y").text = str(lesion.get('Center_Y', 'N/A'))
            ET.SubElement(lesion_element, "Center_Z").text = str(lesion.get('Center_Z', 'N/A'))
            ET.SubElement(lesion_element, "Height").text = str(lesion.get('Height', 'N/A'))
            ET.SubElement(lesion_element, "Width").text = str(lesion.get('Width', 'N/A'))
            ET.SubElement(lesion_element, "Depth").text = str(lesion.get('Depth', 'N/A'))

        # Write the XML tree to a file
        tree = ET.ElementTree(root)
        tree.write(output_file, encoding='utf-8', xml_declaration=True)

        # Pretty-print the XML
        pretty_xml = self.prettify(root)
        with open(output_file, "w") as f:
            f.write(pretty_xml)

    def prettify(self, elem):
        """Return a pretty-printed XML string for the Element."""
        rough_string = ET.tostring(elem, 'utf-8')
        reparsed = minidom.parseString(rough_string)
        return reparsed.toprettyxml(indent="  ")
    
    def select_lesions(self, num_lesions, size_mm, 
                       lesion_names=None, lesion_types=None, centers=None, bounding_boxes=None, db_dir='db/mass'):
        """
        Insert lesions into the Inserter class.

        Arguments:
        num_lesions -- number of lesions to insert
        size_mm -- list of desired sizes per lesion (targeted lesion in mm)
        Optional arguments:
        lesion_names -- list of lesion names
        lesion_types -- list of lesion types
        centers -- list of tuples for lesion centers (X, Y, Z)
        bounding_boxes -- list of tuples for bounding boxes (Height, Width, Depth)
        db_dir -- path to the directory containing lesion zip files
        """

        max_attempts = 10 # insertion will fail if the number of attempts are over max_attemps

        if len(size_mm) == 1:
            size_mm = np.repeat(size_mm[0], num_lesions) # Use the same size for list of lesions

        # Load all available zip files from the directory
        lesion_files = [f for f in os.listdir(db_dir) if f.endswith('.zip')]

        if not lesion_files:
            raise Exception(f"No lesion zip files found in the directory: {db_dir}")

        vol = (self.Phantom.voxel_data).astype(bool) # binary phantom
        
        for i in range(num_lesions):
            center = None
            shape = None

            # If lesion details are not provided, randomly select a zip file and extract XML data
            if lesion_names is None or i >= len(lesion_names):

                attempts = 0  # Initialize attempt counter
                
                while attempts < max_attempts:
                    attempts += 1
                    self.log.info(f"Attempt {attempts} for lesion {i}")

                    # Randomly select a lesion zip file and extract XML data
                    lesion_file = random.choice(lesion_files)
                    self.log.info(f"Lesion selected is {lesion_file}.")
                    
                    zip = os.path.join(db_dir, lesion_file)
                    map = self.read_lesion(zip)
                    map = map.astype(bool)
                    vxl = self.Phantom.get_voxel_mm()

                    # Try to insert the lesion at a random position
                    center, shape, vol = self.attempt_insertion(vol, vxl, map, size_mm[i])
                    
                    if center is not None:  # Check if insertion was successful
                        self.log.info(f"Lesion successfully inserted at center {center}")
                        break  # Exit the while loop if insertion is successful

                if center is None:
                    self.log.warning(f"Failed to insert lesion {i} after {max_attempts} attempts")
                    continue  # Skip to the next lesion if insertion failed
           
            else:
                # Use the provided lesion data if available
                lesion_data = {
                    'LesionName': lesion_names[i] if lesion_names and i < len(lesion_names) else None,
                    'LesionType': lesion_types[i] if lesion_types and i < len(lesion_types) else None,
                    'Center_X': centers[i][0] if centers and i < len(centers) else None,
                    'Center_Y': centers[i][1] if centers and i < len(centers) else None,
                    'Center_Z': centers[i][2] if centers and i < len(centers) else None,
                    'Height': bounding_boxes[i][0] if bounding_boxes and i < len(bounding_boxes) else None,
                    'Width': bounding_boxes[i][1] if bounding_boxes and i < len(bounding_boxes) else None,
                    'Depth': bounding_boxes[i][2] if bounding_boxes and i < len(bounding_boxes) else None,
                }
                center = (lesion_data['Center_X'], lesion_data['Center_Y'], lesion_data['Center_Z'])
                shape = (lesion_data['Height'], lesion_data['Width'], lesion_data['Depth'])

            # Insert lesion into the Lesions list
            self.Lesions.append({
            'LesionName': zip if lesion_names is None else lesion_names[i],
            'LesionType': 1 if lesion_types is None else lesion_types[i],
            'Center_X': center[0],
            'Center_Y': center[1],
            'Center_Z': center[2],
            'Height': shape[0],
            'Width': shape[1],
            'Depth': shape[2]
            })

            self.write_xml(self.xml)
            imwrite('binary_vol.tif', vol) # for debug

    def read_lesion(self, zip_file):
        """       
        Arguments:
        zip_file -- path to the lesion zip file
        size_mm -- target size for lesion
        
        Returns:
        map -- lesion mask
        """
        with zipfile.ZipFile(zip_file, 'r') as zip_ref:

            file = [f for f in zip_ref.namelist() if f.endswith('.raw')][0]
            size = str(file).replace('.raw', '').split('_')[-1].split('x')

            with zip_ref.open(file) as f:
                map = f.read()
                map = np.frombuffer(bytearray(map), dtype=np.uint8).reshape(int(size[0]), int(size[1]), int(size[2]))
        
        return map
    
    def attempt_insertion(self, vol, vxl, map, target_size):
        """       
        Arguments:
        vol -- binary 3D phantom volume (bool type)
        vxl -- voxel size in mm (X, Y, Z)
        map -- binary map of lesion (bool type)
        target_size -- targeted lesion size in mm (X, Y, Z)
        
        Returns:
        center -- boolean variable (flag); True if lesion is successfully inserted, False otherwise
        vol -- binary volume with lesion bounding boxes (set to air/zeros)
        """
        center = None

        # Convert target_size from mm to voxel space
        target = np.array(target_size) / np.array(vxl)
        target = np.round(target).astype(int) 

        # Resize the lesion mask to match the target size in voxels
        lesion_shape = np.array(map.shape)
        scaling_factors = target / lesion_shape  # scaling per dimension
        resized_lesion = zoom(map.astype(float), scaling_factors, order=1) > 0.5  # resize lesion to target voxel size
        
        #print(f"Target size in voxels: {target}")
        #print(f"Scaling factors: {scaling_factors}")
        #print(f"Resized lesion shape: {resized_lesion.shape}")

        # Select a random candidate position for the center of the lesion in the phantom volume
        lesion_shape = resized_lesion.shape
        phantom_shape = vol.shape

        # Ensure the lesion fits within the phantom (lesion bounding box should not exceed phantom bounds)
        max_bounds = np.array(phantom_shape) - np.array(lesion_shape)
        if np.any(max_bounds < 0):
            return center, vol  # The lesion cannot fit in the phantom

        # Generate random coordinates for the top-left corner of the lesion within the phantom bounds
        random_position = np.random.randint(0, max_bounds + 1)

        # Extract the region in the phantom volume where the lesion will be inserted
        sub_volume = vol[random_position[0]:random_position[0] + lesion_shape[0],
                        random_position[1]:random_position[1] + lesion_shape[1],
                        random_position[2]:random_position[2] + lesion_shape[2]]

        # Check if all voxels in the selected region of the phantom are available (True)
        if np.all(sub_volume):
            # If all voxels are True, set them to False (map the lesion region to zeros in the phantom)
            vol[random_position[0]:random_position[0] + lesion_shape[0],
                random_position[1]:random_position[1] + lesion_shape[1],
                random_position[2]:random_position[2] + lesion_shape[2]] = ~resized_lesion
            
            lesion_center = random_position + np.array(lesion_shape) // 2
            center = lesion_center

        #print('random_position', random_position)
        #print('center', center)
        return center, lesion_shape, vol

    def insertion_replacement(self):
        """
        Insert lesions into the Phantom by updating only the voxels of the resized lesion mask.
        
        For each lesion in `self.Lesions`, this method will:
        - Read the binary mask of the lesion.
        - Resize the mask based on the bounding box described in `self.Lesions`.
        - Replace the corresponding phantom voxels with the new index.
        """

        # Get the current max index from the phantom's index table
        max_index = self.Phantom.index_table['Maximum_Index']

        # Loop over each lesion in the self.Lesions list
        for lesion in self.Lesions:
            # Increment the max index to create a new index for the lesion
            new_index = max_index + 1
            max_index = new_index  # Update the max index

            # Define the new materials for the lesion (as per your example)
            new_materials = [
                {
                    "Material_Name": "Adipose",
                    "Material_Weight": 0.1,
                    "Material_Density": 0.93,
                    "Material_MaterialZ": 204
                },
                {
                    "Material_Name": "Glandular",
                    "Material_Weight": 0.9,
                    "Material_Density": 1.02,
                    "Material_MaterialZ": 205
                }
            ]

            # Create the new index entry
            new_index_entry = {
                "Index_ID": new_index,
                "Number_Of_Materials": len(new_materials),
                "Materials": new_materials
            }

            # Append the new index to the index table
            self.Phantom.index_table['Indices'].append(new_index_entry)
            self.Phantom.index_table['Maximum_Index'] = max_index

            # Get the lesion's center and size (bounding box)
            center = np.array([lesion['Center_X'], lesion['Center_Y'], lesion['Center_Z']])
            size = np.array([lesion['Height'], lesion['Width'], lesion['Depth']])

            # Calculate the bounds for the lesion within the phantom
            min_bounds = np.floor(center - size / 2).astype(int)  # Starting voxel position
            max_bounds = np.ceil(center + size / 2).astype(int)  # Ending voxel position

            # Read the lesion mask from the zip file (assuming lesion['LesionName'] is the zip path)
            lesion_mask = self.read_lesion(lesion['LesionName'])

            # Resize the lesion mask to match the bounding box in the phantom
            lesion_shape = lesion_mask.shape
            target_shape = (max_bounds[2] - min_bounds[2], max_bounds[1] - min_bounds[1], max_bounds[0] - min_bounds[0])
            scaling_factors = np.array(target_shape) / np.array(lesion_shape)
            resized_lesion_mask = zoom(lesion_mask.astype(float), scaling_factors, order=1) > 0.5  # Resizing the lesion mask

            # Update only the voxels of the phantom that correspond to the resized lesion mask
            phantom_region = self.Phantom.voxel_data[min_bounds[0]:max_bounds[0],
                                                    min_bounds[1]:max_bounds[1],
                                                    min_bounds[2]:max_bounds[2]]

            # Replace voxels in the phantom where the resized lesion mask is True
            phantom_region[resized_lesion_mask] = new_index

            # Update the phantom's voxel data with the modified region
            self.Phantom.voxel_data[min_bounds[0]:max_bounds[0],
                                    min_bounds[1]:max_bounds[1],
                                    min_bounds[2]:max_bounds[2]] = phantom_region

        imwrite('phantom.tif', self.Phantom.voxel_data)
        # Log or return a message indicating completion of the insertion
        self.log.info("Lesion insertion and index table update completed.")

    def insertion_additive(self):
        """
        Insert lesions into the Phantom by updating voxels with a new index that combines
        current materials and the Glandular material, while maintaining the sum of weights = 1.0.
        
        This method ensures that indices are not repeated: if an index with the same material composition
        already exists, it will reuse that index instead of creating a new one.
        """
        current_max_index = self.Phantom.index_table['Maximum_Index']

        # Method to compare material compositions
        def find_existing_index(materials):
            for index_entry in self.Phantom.index_table['Indices']:
                existing_materials = index_entry['Materials']
                if len(existing_materials) == len(materials):
                    match = all(
                        any(
                            existing['Material_Name'] == new['Material_Name'] and
                            abs(existing['Material_Weight'] - new['Material_Weight']) < 1e-6  # Floating-point tolerance
                            for existing in existing_materials
                        ) for new in materials
                    )
                    if match:
                        return index_entry['Index_ID']
            return None

        # Loop over each lesion in the self.Lesions list
        for lesion in self.Lesions:
            # Get the lesion's center and size (bounding box)
            center = np.array([lesion['Center_X'], lesion['Center_Y'], lesion['Center_Z']])
            size = np.array([lesion['Height'], lesion['Width'], lesion['Depth']])

            # Calculate the bounds for the lesion within the phantom
            min_bounds = np.floor(center - size / 2).astype(int)  # Starting voxel position
            max_bounds = np.ceil(center + size / 2).astype(int)  # Ending voxel position

            # Read the lesion mask from the zip file
            lesion_mask = self.read_lesion(lesion['LesionName'])

            # Resize the lesion mask to match the bounding box in the phantom
            lesion_shape = lesion_mask.shape
            target_shape = (max_bounds[0] - min_bounds[0], max_bounds[1] - min_bounds[1], max_bounds[2] - min_bounds[2])
            scaling_factors = np.array(target_shape) / np.array(lesion_shape)
            resized_lesion_mask = zoom(lesion_mask.astype(float), scaling_factors, order=1) > 0.5  # Resizing the lesion mask

            # Get the affected region in the phantom
            phantom_region = self.Phantom.voxel_data[min_bounds[0]:max_bounds[0],
                                                    min_bounds[1]:max_bounds[1],
                                                    min_bounds[2]:max_bounds[2]]

            # Find the indices where the lesion will affect the phantom
            affected_voxels = np.where(resized_lesion_mask)

            # Iterate over affected voxels directly
            for idx in range(len(affected_voxels[0])):
                x = affected_voxels[0][idx]
                y = affected_voxels[1][idx]
                z = affected_voxels[2][idx]

                # Get the current index at the voxel (x, y, z)
                current_index = phantom_region[x, y, z]
                current_materials = self.Phantom.index_table['Indices'][current_index]['Materials']

                # Reduce the weight of the current materials by 0.1 (while keeping total weight = 1.0)
                updated_materials = []
                total_weight_reduction = 0.1

                for material in current_materials:
                    reduced_weight = material['Material_Weight'] - total_weight_reduction / len(current_materials)
                    updated_materials.append({
                        "Material_Name": material['Material_Name'],
                        "Material_Weight": max(0, reduced_weight),  # Ensure weight doesn't go below 0
                        "Material_Density": material['Material_Density'],
                        "Material_MaterialZ": material['Material_MaterialZ']
                    })

                # Add Glandular material with a weight of 0.1
                updated_materials.append({
                    "Material_Name": "Glandular",
                    "Material_Weight": 0.1,
                    "Material_Density": 1.02,
                    "Material_MaterialZ": 205
                })

                # Check if this material composition already exists
                existing_index = find_existing_index(updated_materials)
                if existing_index is not None:
                    new_index = existing_index  # Reuse the existing index
                else:
                    # Create a new index for this material composition
                    new_index = current_max_index + 1
                    current_max_index = new_index

                    # Create the new index entry
                    new_index_entry = {
                        "Index_ID": new_index,
                        "Number_Of_Materials": len(updated_materials),
                        "Materials": updated_materials
                    }

                    # Append the new index to the index table
                    self.Phantom.index_table['Indices'].append(new_index_entry)
                    self.Phantom.index_table['Maximum_Index'] = current_max_index

                # Update the phantom voxel data at this position with the new index
                phantom_region[x, y, z] = new_index

                # Update the phantom's voxel data with the modified region
                self.Phantom.voxel_data[min_bounds[0]:max_bounds[0],
                                        min_bounds[1]:max_bounds[1],
                                        min_bounds[2]:max_bounds[2]] = phantom_region

        # Log or return a message indicating completion of the insertion
        self.log.info("Additive lesion insertion with index reuse completed.")
        # print(self.Phantom.index_table)
        # imwrite('phantom.tif', self.Phantom.voxel_data)