import xml.etree.ElementTree as ET
from xml.dom import minidom

class XMLBreastGenerator:
    def __init__(self, config, phantom_name):
        self.config = config
        self.phantom_name = phantom_name

    def prettify(self, elem):
        """Return a pretty-printed XML string for the Element."""
        rough_string = ET.tostring(elem, 'utf-8')
        reparsed = minidom.parseString(rough_string)
        return reparsed.toprettyxml(indent="  ")

    def write_xml(self, file_path):
        root = ET.Element("Breast_Phantom_Designer")

        # Subject
        subject = ET.SubElement(root, "Subject")
        ET.SubElement(subject, "Subject_UID").text = self.config["Subject"]["Subject_UID"]
        ET.SubElement(subject, "Subject_Type").text = self.config["Subject"]["Subject_Type"]
        ET.SubElement(subject, "Gender").text = self.config["Subject"]["Gender"]
        ET.SubElement(subject, "Pediatric").text = str(self.config["Subject"]["Pediatric"]).lower()
        ET.SubElement(subject, "Age").text = str(self.config["Subject"]["Age"])
        ET.SubElement(subject, "Date_Of_Birth").text = self.config["Subject"]["Date_Of_Birth"]
        ET.SubElement(subject, "Race").text = self.config["Subject"]["Race"]
        ET.SubElement(subject, "Ethnicity").text = self.config["Subject"]["Ethnicity"]
        ET.SubElement(subject, "Weight").text = str(self.config["Subject"]["Weight"])
        ET.SubElement(subject, "Height").text = str(self.config["Subject"]["Height"])
        ET.SubElement(subject, "BMI").text = str(self.config["Subject"]["BMI"])

        # Phantom
        phantom = ET.SubElement(root, "Phantom")
        ET.SubElement(phantom, "Phantom_UID").text = self.config["Phantom"]["Phantom_UID"]
        ET.SubElement(phantom, "Phantom_Source").text = self.config["Phantom"]["Phantom_Source"]
        ET.SubElement(phantom, "Phantom_Type").text = self.config["Phantom"]["Phantom_Type"]
        ET.SubElement(phantom, "Data_Model").text = self.config["Phantom"]["Data_Model"]
        ET.SubElement(phantom, "Body_Part").text = self.config["Phantom"]["Body_Part"]
        ET.SubElement(phantom, "Laterality").text = self.config["Phantom"]["Laterality"]
        ET.SubElement(phantom, "Volume").text = str(self.config["Phantom"]["Volume"])

        # Index and Voxel Array
        ET.SubElement(root, "Index_Table_UID").text = self.config["Index_Table_UID"]
        ET.SubElement(root, "Voxel_Array_UID").text = self.config["Voxel_Array_UID"]

        # Leave in Temp Folder
        ET.SubElement(root, "Leave_In_Temp_Folder").text = str(self.config["Leave_In_Temp_Folder"]).lower()

        # Generator Config
        generator_config = ET.SubElement(root, "Generator_Config")
        ET.SubElement(generator_config, "ProgramName").text = self.config["Generator_Config"]["ProgramName"]
        ET.SubElement(generator_config, "XPLPhantom_UID").text = self.config["Generator_Config"]["XPLPhantom_UID"]
        ET.SubElement(generator_config, "Phantom_Name").text = self.phantom_name
        ET.SubElement(generator_config, "Phantom_Filename").text = "vctx/"+self.phantom_name+".vctx" #TODO: Include UID
        ET.SubElement(generator_config, "Phantom_Shape").text = self.config["Generator_Config"]["Phantom_Shape"]
        ET.SubElement(generator_config, "Max_Ligament_Thickness").text = str(self.config["Generator_Config"]["Max_Ligament_Thickness"])
        ET.SubElement(generator_config, "Min_Ligament_Thickness").text = str(self.config["Generator_Config"]["Min_Ligament_Thickness"])
        ET.SubElement(generator_config, "Voxel_Size").text = str(self.config["Generator_Config"]["Voxel_Size"])
        ET.SubElement(generator_config, "Size_X").text = str(self.config["Generator_Config"]["Size_X"])
        ET.SubElement(generator_config, "Size_Y").text = str(self.config["Generator_Config"]["Size_Y"])
        ET.SubElement(generator_config, "Size_CPrime").text = str(self.config["Generator_Config"]["Size_CPrime"])
        ET.SubElement(generator_config, "Size_CSecondary").text = str(self.config["Generator_Config"]["Size_CSecondary"])
        ET.SubElement(generator_config, "Random_Seed").text = str(self.config["Generator_Config"]["Random_Seed"])
        ET.SubElement(generator_config, "Percent_Dense").text = str(self.config["Generator_Config"]["Percent_Dense"])
        ET.SubElement(generator_config, "Skin_Thickness").text = str(self.config["Generator_Config"]["Skin_Thickness"])
        ET.SubElement(generator_config, "Ki").text = str(self.config["Generator_Config"]["Ki"])
        ET.SubElement(generator_config, "Kp").text = str(self.config["Generator_Config"]["Kp"])
        ET.SubElement(generator_config, "Ks").text = str(self.config["Generator_Config"]["Ks"])
        ET.SubElement(generator_config, "Num_Compartments").text = str(self.config["Generator_Config"]["Num_Compartments"])
        ET.SubElement(generator_config, "Compartment_Thickness").text = str(self.config["Generator_Config"]["Compartment_Thickness"])
        ET.SubElement(generator_config, "Min_Seed_Distance").text = str(self.config["Generator_Config"]["Min_Seed_Distance"])
        ET.SubElement(generator_config, "Minimum_Speed").text = str(self.config["Generator_Config"]["Minimum_Speed"])
        ET.SubElement(generator_config, "Maximum_Speed").text = str(self.config["Generator_Config"]["Maximum_Speed"])
        ET.SubElement(generator_config, "Minimum_Ratio").text = str(self.config["Generator_Config"]["Minimum_Ratio"])
        ET.SubElement(generator_config, "Maximum_Ratio").text = str(self.config["Generator_Config"]["Maximum_Ratio"])
        ET.SubElement(generator_config, "Beta_p1").text = str(self.config["Generator_Config"]["Beta_p1"])
        ET.SubElement(generator_config, "Beta_p2").text = str(self.config["Generator_Config"]["Beta_p2"])
        ET.SubElement(generator_config, "Beta_q1").text = str(self.config["Generator_Config"]["Beta_q1"])
        ET.SubElement(generator_config, "Beta_q2").text = str(self.config["Generator_Config"]["Beta_q2"])

        # Write the tree to a file
        tree = ET.ElementTree(root)
        tree.write(file_path, encoding='utf-8', xml_declaration=True)

        # Pretty-print the XML
        pretty_xml = self.prettify(root)
        with open(file_path, "w") as f:
            f.write(pretty_xml)
