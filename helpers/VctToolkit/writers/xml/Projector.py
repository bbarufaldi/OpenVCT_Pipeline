import xml.etree.ElementTree as ET
from xml.dom import minidom

class XMLProjector:
    def __init__(self, config, phantom_name, folder_name):
        self.config = config
        self.phantom_name = phantom_name
        self.folder_name = folder_name

    def prettify(self, elem):
        """Fix XML identation string for the Element."""
        rough_string = ET.tostring(elem, 'utf-8')
        reparsed = minidom.parseString(rough_string)
        return reparsed.toprettyxml(indent="  ")

    def write_xml(self, file_path):
        root = ET.Element("Breast_Phantom_Designer")

        # Projector_Config
        projector_config = ET.SubElement(root, "Projector_Config")
        ET.SubElement(projector_config, "Program_Name").text = str("BreastPhantomRayTracingProjector")
        ET.SubElement(projector_config, "Software_Version").text = str("2.0")
        ET.SubElement(projector_config, "Input_Phantom").text = self.phantom_name
        ET.SubElement(projector_config, "Output_Folder").text = self.folder_name

        for key, value in self.config["Projector_Config"].items():
            ET.SubElement(projector_config, key).text = str(value)
        
        # Imaging_System
        imaging_system = ET.SubElement(root, "Imaging_System")
        for key, value in self.config["Imaging_System"].items():
            ET.SubElement(imaging_system, key).text = str(value)
        
        # Exposure_Settings
        exposure_settings = ET.SubElement(root, "Exposure_Settings")
        for key, value in self.config["Exposure_Settings"].items():
            if isinstance(value, dict):
                sub_element = ET.SubElement(exposure_settings, key)
                for sub_key, sub_value in value.items():
                    ET.SubElement(sub_element, sub_key).text = str(sub_value)
            else:
                ET.SubElement(exposure_settings, key).text = str(value)
        
        # Detector
        detector = ET.SubElement(root, "Detector")
        for key, value in self.config["Detector"].items():
            if isinstance(value, list):
                sub_element = ET.SubElement(detector, key)
                ET.SubElement(sub_element, "X").text = str(value[0])
                ET.SubElement(sub_element, "Y").text = str(value[1])
                if len(value) > 2:
                    ET.SubElement(sub_element, "Z").text = str(value[2])
            else:
                ET.SubElement(detector, key).text = str(value)
        
        # Acquisition_Geometry
        acquisition_geometry = ET.SubElement(root, "Acquisition_Geometry")
        ET.SubElement(acquisition_geometry, "Number_Acquisitions").text = str(self.config["Acquisition_Geometry"]["Number_Acquisitions"])
        
        for acquisition in self.config["Acquisition_Geometry"]["Acquisitions"]:
            acquisition_element = ET.SubElement(acquisition_geometry, "Acquisition")
            for key, value in acquisition.items():
                if isinstance(value, list):
                    sub_element = ET.SubElement(acquisition_element, key)
                    ET.SubElement(sub_element, "X_mm").text = str(value[0])
                    ET.SubElement(sub_element, "Y_mm").text = str(value[1])
                    ET.SubElement(sub_element, "Z_mm").text = str(value[2])
                
                elif isinstance(value, dict):
                    sub_element = ET.SubElement(acquisition_element, key)
                    for sub_key, sub_value in value.items():
                        if isinstance(sub_value, list) and sub_key.startswith("Position_mm"):
                            ET.SubElement(sub_element, sub_key.replace("Position_mm", "X_mm")).text = str(sub_value[0])
                            ET.SubElement(sub_element, sub_key.replace("Position_mm", "Y_mm")).text = str(sub_value[1])
                            ET.SubElement(sub_element, sub_key.replace("Position_mm", "Z_mm")).text = str(sub_value[2])
                        
                        elif isinstance(sub_value, list) and sub_key.startswith("Orient"):
                            ET.SubElement(sub_element, sub_key.replace("_mm", "X_mm")).text = str(sub_value[0])
                            ET.SubElement(sub_element, sub_key.replace("_mm", "Y_mm")).text = str(sub_value[1])
                            ET.SubElement(sub_element, sub_key.replace("_mm", "Z_mm")).text = str(sub_value[2])
                else:
                    ET.SubElement(acquisition_element, key).text = str(value)
        
        tree = ET.ElementTree(root)
        tree.write(file_path, encoding='utf-8', xml_declaration=True)

        pretty_xml = self.prettify(root)
        with open(file_path, "w") as f:
            f.write(pretty_xml)