import xml.etree.ElementTree as ET
import random
import zipfile
import os
from xml.dom import minidom

class XMLWriter:
    def __init__(self, in_phantom, out_phantom):
        self.Software_Name = "LesionInserter"
        self.Software_Version = "2.0"
        self.Software_ID = "1.0"
        self.Input_Phantom = in_phantom
        self.Output_Phantom = out_phantom
        self.VOIs = []
        self.Lesions = []
    
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
    
    def select_lesions(self, num_lesions, lesion_names=None, lesion_types=None, centers=None, bounding_boxes=None, db_dir='db/mass'):
        """
        Insert lesions into the Inserter class.

        Arguments:
        num_lesions -- number of lesions to insert
        Optional arguments:
        lesion_names -- list of lesion names
        lesion_types -- list of lesion types
        centers -- list of tuples for lesion centers (X, Y, Z)
        bounding_boxes -- list of tuples for bounding boxes (Height, Width, Depth)
        db_dir -- path to the directory containing lesion zip files
        """

        # Load all available zip files from the directory
        lesion_files = [f for f in os.listdir(db_dir) if f.endswith('.zip')]

        if not lesion_files:
            raise Exception(f"No lesion zip files found in the directory: {db_dir}")

        for i in range(num_lesions):
            # If lesion details are not provided, randomly select a zip file and extract XML data
            if lesion_names is None or i >= len(lesion_names):
                lesion_file = random.choice(lesion_files)
                lesion_data = self.read_lesion(os.path.join(db_dir, lesion_file))
           
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

            # Insert lesion into the Lesions list
            self.Lesions.append({
                'LesionName': lesion_data['LesionName'],
                'LesionType': lesion_data['LesionType'],
                'Center_X': lesion_data['Center_X'] if centers and i < len(centers) else random.uniform(0.0, 100.0),
                'Center_Y': lesion_data['Center_Y'] if centers and i < len(centers) else random.uniform(0.0, 100.0),
                'Center_Z': lesion_data['Center_Z'] if centers and i < len(centers) else random.uniform(0.0, 100.0),
                'Height': lesion_data['Height'],
                'Width': lesion_data['Width'],
                'Depth': lesion_data['Depth']
            })

    def read_lesion(self, zip_file):
        """       
        Arguments:
        zip_file -- path to the lesion zip file
        
        Returns:
        lesion_data -- dictionary with lesion details (name, type, size, etc.)
        """
        with zipfile.ZipFile(zip_file, 'r') as zip_ref:

            file = [f for f in zip_ref.namelist() if f.endswith('.raw')][0]
            size = str(file).replace('.raw', '').split('_')[-1].split('x')

            lesion_data = {
                    'LesionName': zip_file,
                    'LesionType': 1,
                    'Center_X': float(size[0])/2,
                    'Center_Y': float(size[1])/2,
                    'Center_Z': float(size[2])/2,
                    'Height': int(size[0]),
                    'Width': int(size[1]),
                    'Depth': int(size[2])
            }
            # with zip_ref.open(file) as f:
            #     voxel_data = f.read()
            #     voxel_data = np.frombuffer(bytearray(voxel_data), dtype=np.uint8).reshape(int(size[0]), int(size[1]), int(size[2]))
                
        return lesion_data