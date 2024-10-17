import xml.etree.ElementTree as ET
from xml.dom import minidom

class Inserter:
    def __init__(self, xml_file=None):
        self.xml_file = xml_file
        self.Software_Name = None
        self.Software_Version = None
        self.Software_ID = None
        self.Input_Phantom = None
        self.Output_Phantom = None
        self.VOIs = []
        self.Lesions = []
        
        # If an XML file is provided, read the XML data
        if xml_file:
            self.read_xml(xml_file)

    def read_xml(self, xml_file):
        tree = ET.parse(xml_file)
        root = tree.getroot()

        self.Software_Name = root.find('Software_Name').text
        self.Software_Version = root.find('Software_Version').text
        self.Software_ID = root.find('Software_ID').text
        self.Input_Phantom = root.find('Input_Phantom').text
        self.Output_Phantom = root.find('Output_Phantom').text

        # Reading VOIs
        vois = root.find('VOIs')
        self.VOIs = []
        for voi in vois.findall('VOI'):
            self.VOIs.append({
                'Center_X': int(voi.find('Center_X').text),
                'Center_Y': int(voi.find('Center_Y').text),
                'Center_Z': int(voi.find('Center_Z').text),
                'Height': int(voi.find('Height').text),
                'Width': int(voi.find('Width').text),
                'Depth': int(voi.find('Depth').text),
                'Has_Lesion': voi.find('Has_Lesion').text == 'true'
            })

        # Reading Lesions
        lesions = root.find('Lesions')
        self.Lesions = []
        for lesion in lesions.findall('Lesion'):
            self.Lesions.append({
                'LesionName': lesion.find('LesionName').text,
                'LesionType': int(lesion.find('LesionType').text),
                'Center_X': int(lesion.find('Center_X').text),
                'Center_Y': int(lesion.find('Center_Y').text),
                'Center_Z': int(lesion.find('Center_Z').text),
                'Height': int(lesion.find('Height').text),
                'Width': int(lesion.find('Width').text),
                'Depth': int(lesion.find('Depth').text)
            })
