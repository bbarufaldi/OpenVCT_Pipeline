import xml.etree.ElementTree as ET

class Deformer:
    def __init__(self, xml_file):
      
      self.xml_file = xml_file
      self.Program_Name = None
      self.Software_Version = None
      self.Input_Phantom = None
      self.Input_Mesh = None
      self.Output_Mesh = None
      self.View = None
      self.Output_Phantom = None
      self.Index_Table_UID = None
      self.Voxel_Array_UID = None
      
      # Initialize by reading the XML file
      self.read_xml(self.xml_file)

    def read_xml(self, xml_file):
      tree = ET.parse(xml_file)
      root = tree.getroot()

      # Extract fields directly from the root, as the XML structure doesn't seem to have another enclosing element
      self.Program_Name = root.find('Program_Name').text
      self.Software_Version = root.find('Software_Version').text
      self.Input_Phantom = root.find('Input_Phantom').text
      self.Input_Mesh = root.find('Input_Mesh').text
      self.Output_Mesh = root.find('Output_Mesh').text
      self.View = root.find('View').text
      self.Output_Phantom = root.find('Output_Phantom').text
      self.Index_Table_UID = root.find('Index_Table_UID').text
      self.Voxel_Array_UID = root.find('Voxel_Array_UID').text