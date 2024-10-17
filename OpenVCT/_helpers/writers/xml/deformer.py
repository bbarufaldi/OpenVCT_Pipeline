import xml.etree.ElementTree as ET
from xml.dom import minidom

class XMLWriter:
    def __init__(self, config, in_phantom, out_phantom, xml_file):
        self.config = config
        self.in_phantom = in_phantom
        self.out_phantom = out_phantom
        self.xml_file = xml_file
        self.write_xml(xml_file)

    def prettify(self, elem):
        """Return a pretty-printed XML string for the Element."""
        rough_string = ET.tostring(elem, 'utf-8')
        reparsed = minidom.parseString(rough_string)
        return reparsed.toprettyxml(indent="  ")

    def write_xml(self, file_path):
        #root = ET.Element("Breast_Phantom_Designer")

        # Deformer_Config
        root = ET.Element("name")
        ET.SubElement(root, "Program_Name").text = "BreastPhantomDeformer"
        ET.SubElement(root, "Software_Version").text = "2.0"
        ET.SubElement(root, "Input_Phantom").text = self.in_phantom
        ET.SubElement(root, "Input_Mesh").text = self.config["Deformer_Config"]["Input_Mesh"]
        ET.SubElement(root, "Output_Mesh").text = self.config["Deformer_Config"]["Output_Mesh"]
        ET.SubElement(root, "View").text = self.config["Deformer_Config"]["View"]
        ET.SubElement(root, "Output_Phantom").text = self.out_phantom

        # Index and Voxel Array UIDs
        ET.SubElement(root, "Index_Table_UID").text = "006.685169686552.20171031231800720"
        ET.SubElement(root, "Voxel_Array_UID").text = "007.685169686552.20171031231800770"

        # Write the tree to a file
        tree = ET.ElementTree(root)
        tree.write(file_path, encoding='utf-8', xml_declaration=True)

        # Pretty-print the XML
        pretty_xml = self.prettify(root)
        with open(file_path, "w") as f:
            f.write(pretty_xml)