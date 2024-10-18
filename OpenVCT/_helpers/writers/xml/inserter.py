import xml.etree.ElementTree as ET
from xml.dom import minidom

from tifffile import imwrite
import numpy as np
import zipfile
import os
import logging as log
from scipy.ndimage import zoom

from readers.xml import Phantom as ph

class Inserter:
    def __init__(self, in_phantom, out_phantom, xml_file, 
                 num_lesions=1, size_mm=[(7, 7, 3)],
                 lesion_names=None, lesion_types=None, centers=None, bounding_boxes=None, 
                 db_dir='db/mass'):
        
        self.Software_Name = "LesionInserter"
        self.Software_Version = "2.0"
        self.Software_ID = "1.0"
        self.Input_Phantom = in_phantom
        self.Output_Phantom = out_phantom
        self.VOIs = []
        self.Lesions = []

        self.num_lesions = num_lesions
        self.size_mm = size_mm
        self.lesion_names = lesion_names
        self.lesion_types = lesion_types
        self.lesion_centers = centers
        self.bounding_boxes = bounding_boxes
        self.db_dir = db_dir

        self.Phantom = ph.Phantom(self.Input_Phantom)
        
        # Create a log for insertion
        log.basicConfig(filename='LesionInsertion.log', level=log.DEBUG,
                        format='%(asctime)s - %(levelname)s - %(message)s')
        self.log = log.getLogger()

        self.xml_file = xml_file

        # Check lesions for insertion and 
        self.select_lesions()
    
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
    
    def select_lesions(self):
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

        if len(self.size_mm) == 1:
            self.size_mm = np.repeat(self.size_mm[0], self.num_lesions) # Use the same size for list of lesions

        # Load all available zip files from the directory
        lesion_files = [f for f in os.listdir(self.db_dir) if f.endswith('.zip')]

        if not lesion_files:
            raise Exception(f"No lesion zip files found in the directory: {self.db_dir}")

        vol = (self.Phantom.voxel_data).astype(bool) # binary phantom
        
        for i in range(self.num_lesions):
            center = None
            shape = None

            # If lesion details are not provided, randomly select a zip file and extract XML data
            if (self.lesion_names is None or i >= len(self.lesion_names)):

                attempts = 0  # Initialize attempt counter
                
                while attempts < max_attempts:
                    attempts += 1
                    self.log.info(f"Attempt {attempts} for lesion {i}")

                    # Randomly select a lesion zip file and extract XML data
                    lesion_file = np.random.choice(lesion_files)
                    self.log.info(f"Lesion selected is {lesion_file}.")
                    
                    zip = os.path.join(self.db_dir, lesion_file)
                    map = self.read_lesion(zip)
                    map = map.astype(bool)
                    vxl = self.Phantom.get_voxel_mm()

                    # Try to insert the lesion at a random position
                    center, shape, vol = self.attempt_insertion(vol, vxl, map, self.size_mm[i])
                    
                    if center is not None:  # Check if insertion was successful
                        self.log.info(f"Lesion successfully inserted at center {center}")
                        break  # Exit the while loop if insertion is successful

                if center is None:
                    self.log.warning(f"Failed to insert lesion {i} after {max_attempts} attempts")
                    continue  # Skip to the next lesion if insertion failed
           
            else:
                # Use the provided lesion data if available
                lesion_data = {
                    'LesionName': self.lesion_names[i] if self.lesion_names and i < len(self.lesion_names) else None,
                    'LesionType': self.lesion_types[i] if self.lesion_types and i < len(self.lesion_types) else None,
                    'Center_X': self.centers[i][0] if self.centers and i < len(self.centers) else None,
                    'Center_Y': self.centers[i][1] if self.centers and i < len(self.centers) else None,
                    'Center_Z': self.centers[i][2] if self.centers and i < len(self.centers) else None,
                    'Height': self.bounding_boxes[i][0] if self.bounding_boxes and i < len(self.bounding_boxes) else None,
                    'Width': self.bounding_boxes[i][1] if self.bounding_boxes and i < len(self.bounding_boxes) else None,
                    'Depth': self.bounding_boxes[i][2] if self.bounding_boxes and i < len(self.bounding_boxes) else None,
                }
                center = (lesion_data['Center_X'], lesion_data['Center_Y'], lesion_data['Center_Z'])
                shape = (lesion_data['Height'], lesion_data['Width'], lesion_data['Depth'])

            # Insert lesion into the Lesions list
            self.Lesions.append({
            'LesionName': zip if self.lesion_names is None else self.lesion_names[i],
            'LesionType': 1 if self.lesion_types is None else self.lesion_types[i],
            'Center_X': center[0],
            'Center_Y': center[1],
            'Center_Z': center[2],
            'Height': shape[0],
            'Width': shape[1],
            'Depth': shape[2]
            })

        self.write_xml(self.xml_file)
        imwrite('binary_mask.tif', vol)

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