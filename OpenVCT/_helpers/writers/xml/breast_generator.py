import xml.etree.ElementTree as ET
from xml.dom import minidom

class XMLWriter:
    def __init__(self, config, phantom_name, xml_file):
        self.config = config
        self.phantom_name = phantom_name
        self.xml_file = xml_file
        self.write_xml(xml_file)

    def prettify(self, elem):
        """Return a pretty-printed XML string for the Element."""
        rough_string = ET.tostring(elem, 'utf-8')
        reparsed = minidom.parseString(rough_string)
        return reparsed.toprettyxml(indent="  ")

    def write_xml(self, file_path):
        root = ET.Element("Breast_Phantom_Designer")

        # Subject Section
        subject = ET.SubElement(root, "Subject")
        subject_config = self.config["Subject"]
        for key, value in subject_config.items():
            ET.SubElement(subject, key).text = str(value)

        # Phantom Section
        phantom = ET.SubElement(root, "Phantom")
        phantom_config = self.config["Phantom"]
        for key, value in phantom_config.items():
            ET.SubElement(phantom, key).text = str(value)

        # Generator Config Section
        generator_config = ET.SubElement(root, "Generator_Config")
        gen_config = self.config["Generator_Config"]
        for key, value in gen_config.items():
            ET.SubElement(generator_config, key).text = str(value)
        ET.SubElement(generator_config, "Phantom_Name").text = self.phantom_name
        ET.SubElement(generator_config, "Phantom_Filename").text = "vctx/" + self.phantom_name + ".vctx"

        # # Index Table Section
        # index_table = ET.SubElement(root, "Index_Table")
        # index_config = self.config["Index_Table"]
        # for index in index_config["Indices"]:
        #     index_element = ET.SubElement(index_table, "Index")
        #     for key, value in index.items():
        #         if key == "Materials":
        #             materials = ET.SubElement(index_element, "Materials")
        #             for material in value:
        #                 material_element = ET.SubElement(materials, "Material")
        #                 for mat_key, mat_value in material.items():
        #                     ET.SubElement(material_element, mat_key).text = str(mat_value)
        #         else:
        #             ET.SubElement(index_element, key).text = str(value)

        ET.SubElement(generator_config, "Leave_In_Temp_Folder").text = False

        # Write the tree to a file
        tree = ET.ElementTree(root)
        tree.write(file_path, encoding='utf-8', xml_declaration=True)

        # Pretty-print the XML
        pretty_xml = self.prettify(root)
        with open(file_path, "w") as f:
            f.write(pretty_xml)
