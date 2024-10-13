import numpy as np
import xml.etree.ElementTree as ET
from xml.dom import minidom
import zipfile as zip
from datetime import datetime
import os
import shutil

class Phantom:
    def __init__(self, file_path):
        self.file_path = file_path
        self.file_name = (file_path.split('/')[-1]).replace('.vctx', '')

        self.voxel_data = None
        self.xml = None
        
        #Phantom Fields
        self.phantom_name = None
        self.phantom_shape = None
        self.max_ligament_thickness = None
        self.min_ligament_thickness = None
        self.phantom_uid = None
        self.phantom_source = None
        self.phantom_type = None
        self.data_model = None
        self.body_part = None
        self.laterality = None
        self.total_non_air_voxels = None
        self.thickness_mm = {}
        self.glandular_count = None
        self.date_created = {}
        self.time_created = {}
        self.subject = {}
        self.trial = {}
        self.organization = {}
        self.generation_software = {}
        self.generation_station = {}
        self.voxel_array = {}
        self.index_table = []

        self.set_phantom()
        self.def_mode = None

    def set_phantom(self):
        xml = None

        with zip.ZipFile(self.file_path, mode='r') as filezip:
            with filezip.open(self.file_name+"/Phantom.dat") as file:
                self.voxel_data = file.read()
            
            with filezip.open(self.file_name+"/Phantom.xml") as file:
                self.xml = file
                self.read_xml(self.xml)

        self.voxel_data = np.frombuffer(bytearray(self.voxel_data), dtype=np.uint8).reshape(self.voxel_array['Voxel_Num']['Z'], 
                                                                                            self.voxel_array['Voxel_Num']['Y'], 
                                                                                            self.voxel_array['Voxel_Num']['X'])

    def get_file_name(self):
        return self.file_name
    
    def get_voxel_mm(self):
        return np.asarray((self.voxel_array['Voxel_Size_MM']['Z'], 
                           self.voxel_array['Voxel_Size_MM']['Y'], 
                           self.voxel_array['Voxel_Size_MM']['X']), dtype=np.float16)
    
    def get_thickness_mm(self):
        return np.asarray((self.voxel_array['Z'], self.thickness_mm['Y'], self.thickness_mm['X']), dtype=np.float16)

    def read_xml(self, xml_file):
        tree = ET.parse(xml_file)
        root = tree.getroot()

        # Parsing fields
        self.phantom_name = root.find("Phantom_Name").text
        self.phantom_shape = root.find("Phantom_Shape").text
        self.max_ligament_thickness = float(root.find("Max_Ligament_Thickness").text)
        self.min_ligament_thickness = float(root.find("Min_Ligament_Thickness").text)
        self.phantom_uid = root.find("Phantom_UID").text
        self.phantom_source = root.find("Phantom_Source").text
        self.phantom_type = root.find("Phantom_Type").text
        self.data_model = root.find("Data_Model").text
        self.body_part = root.find("Body_Part").text
        self.laterality = root.find("Laterality").text
        self.total_non_air_voxels = int(root.find("Total_Non_Air_Voxels").text)

        # Parsing Thickness
        thickness_mm_element = root.find("Thickness_mm")
        self.thickness_mm['X'] = float(thickness_mm_element.find("X").text)
        self.thickness_mm['Y'] = float(thickness_mm_element.find("Y").text)
        self.thickness_mm['Z'] = float(thickness_mm_element.find("Z").text)

        # Glandular Count
        self.glandular_count = int(root.find("Glandular_Count").text)

        # Parsing Date_Created
        date_created_element = root.find("Date_Created")
        self.date_created['Year'] = int(date_created_element.find("Year").text)
        self.date_created['Month'] = int(date_created_element.find("Month").text)
        self.date_created['Day'] = int(date_created_element.find("Day").text)

        # Parsing Time_Created
        time_created_element = root.find("Time_Created")
        self.time_created['Hour'] = int(time_created_element.find("Hour").text)
        self.time_created['Minute'] = int(time_created_element.find("Minute").text)
        self.time_created['Seconds'] = int(time_created_element.find("Seconds").text)
        self.time_created['Fraction'] = int(time_created_element.find("Fraction").text)
        self.time_created['UTC_Offset'] = int(time_created_element.find("UTC_Offset").text)

        # Parsing Subject
        subject_element = root.find("Subject")
        self.subject['UID'] = subject_element.find("Subject_UID").text
        self.subject['Type'] = subject_element.find("Subject_Type").text
        self.subject['Gender'] = subject_element.find("Gender").text
        self.subject['Pediatric'] = int(subject_element.find("Pediatric").text)
        self.subject['Age'] = int(subject_element.find("Subject_Age").text)
        dob_element = subject_element.find("Date_Of_Birth")
        self.subject['Date_Of_Birth'] = {
            'Year': int(dob_element.find("Year").text),
            'Month': int(dob_element.find("Month").text),
            'Day': int(dob_element.find("Day").text)
        }
        self.subject['Race'] = subject_element.find("Race").text
        self.subject['Ethnicity'] = subject_element.find("Ethnicity").text
        self.subject['Height'] = float(subject_element.find("Height").text)
        self.subject['Weight'] = float(subject_element.find("Weight").text)
        self.subject['BMI'] = float(subject_element.find("BMI").text)

        # Parsing Trial
        self.trial['VCT_Version'] = float(root.find("Trial/VCT_Version").text)

        # Parsing Organization
        organization_element = root.find("Organization")
        self.organization['Name'] = organization_element.find("Name").text
        self.organization['Division'] = organization_element.find("Division").text
        self.organization['Department'] = organization_element.find("Department").text
        self.organization['Group'] = organization_element.find("Group").text
        self.organization['Institution_Code_Sequence'] = organization_element.find("Institution_Code_Sequence").text
        self.organization['Country'] = organization_element.find("Country").text
        self.organization['State'] = organization_element.find("State").text
        self.organization['City'] = organization_element.find("City").text
        self.organization['Zip'] = organization_element.find("Zip").text

        # Parsing Generation_Software
        generation_software_element = root.find("Generation_Software")
        self.generation_software['Name'] = generation_software_element.find("Name").text
        self.generation_software['Version'] = generation_software_element.find("Version").text
        self.generation_software['Repository'] = generation_software_element.find("Repository").text

        # Parsing Generation_Station
        generation_station_element = root.find("Generation_Station")
        self.generation_station['Name'] = generation_station_element.find("Name").text
        self.generation_station['OS'] = generation_station_element.find("OS").text
        self.generation_station['Architecture'] = generation_station_element.find("Architecture").text

        # Parsing Voxel_Array
        voxel_array_element = root.find("Voxel_Array")
        self.voxel_array['UID'] = voxel_array_element.find("Voxel_Array_UID").text
        voxel_num_element = voxel_array_element.find("VOXEL_NUM")
        self.voxel_array['Voxel_Num'] = {
            'X': int(voxel_num_element.find("X").text),
            'Y': int(voxel_num_element.find("Y").text),
            'Z': int(voxel_num_element.find("Z").text)
        }
        voxel_size_element = voxel_array_element.find("VOXEL_SIZE_MM")
        self.voxel_array['Voxel_Size_MM'] = {
            'X': float(voxel_size_element.find("X").text),
            'Y': float(voxel_size_element.find("Y").text),
            'Z': float(voxel_size_element.find("Z").text)
        }
        self.voxel_array['Voxel_Array_Order'] = voxel_array_element.find("VOXEL_ARRAY_ORDER").text
        self.voxel_array['Voxel_Type'] = voxel_array_element.find("VOXEL_TYPE").text
        self.voxel_array['Number_Bits'] = int(voxel_array_element.find("Number_Bits").text)
        self.voxel_array['Endian'] = voxel_array_element.find("Endian").text

        # Parsing Index_Table
        index_table_element = root.find("Index_Table")
        for index in index_table_element.findall("Index"):
            materials = []
            for material in index.find("Materials").findall("Material"):
                materials.append({
                    "Material_Name": material.find("Material_Name").text,
                    "Material_Weight": float(material.find("Material_Weight").text),
                    "Material_Density": float(material.find("Material_Density").text),
                    "Material_MaterialZ": int(material.find("Material_MaterialZ").text)
                })
            self.index_table.append({
                "Index_ID": int(index.find("Index_ID").text),
                "Number_Of_Materials": int(index.find("Number_Of_Materials").text),
                "Materials": materials
            })

    def write_xml(self, out_phantom, out_name, output_file, def_mode):
        """Method to write the stored information back to an XML file"""

        root = ET.Element("VCT_Phantom")

        # Add phantom general information
        ET.SubElement(root, "Phantom_Name").text = out_name
        ET.SubElement(root, "Phantom_Shape").text = "Anthropomorphic"
        ET.SubElement(root, "Max_Ligament_Thickness").text = str(self.max_ligament_thickness)
        ET.SubElement(root, "Min_Ligament_Thickness").text = str(self.min_ligament_thickness)
        ET.SubElement(root, "Phantom_UID").text = self.phantom_uid
        ET.SubElement(root, "Phantom_Source").text = self.phantom_source
        ET.SubElement(root, "Phantom_Type").text = self.phantom_type
        ET.SubElement(root, "Data_Model").text = self.data_model
        ET.SubElement(root, "Body_Part").text = self.body_part
        ET.SubElement(root, "Laterality").text = self.laterality
        ET.SubElement(root, "Total_Non_Air_Voxels").text = str(self.total_non_air_voxels)

        # Add Thickness_mm information
        thickness_mm = ET.SubElement(root, "Thickness_mm")
        ET.SubElement(thickness_mm, "X").text = str(out_phantom.shape[2]*self.get_voxel_mm()[2])
        ET.SubElement(thickness_mm, "Y").text = str(out_phantom.shape[1]*self.get_voxel_mm()[1])
        ET.SubElement(thickness_mm, "Z").text = str(out_phantom.shape[0]*self.get_voxel_mm()[0])

        # Add Glandular Count
        ET.SubElement(root, "Glandular_Count").text = str(self.glandular_count)

        # Add Date_Created information
        date_created = ET.SubElement(root, "Date_Created")
        ET.SubElement(date_created, "Year").text = str(self.date_created.get('Year', ''))
        ET.SubElement(date_created, "Month").text = str(self.date_created.get('Month', ''))
        ET.SubElement(date_created, "Day").text = str(self.date_created.get('Day', ''))

        # Add Time_Created information
        time_created = ET.SubElement(root, "Time_Created")
        ET.SubElement(time_created, "Hour").text = str(self.time_created.get('Hour', ''))
        ET.SubElement(time_created, "Minute").text = str(self.time_created.get('Minute', ''))
        ET.SubElement(time_created, "Seconds").text = str(self.time_created.get('Seconds', ''))
        ET.SubElement(time_created, "Fraction").text = str(self.time_created.get('Fraction', ''))
        ET.SubElement(time_created, "UTC_Offset").text = str(self.time_created.get('UTC_Offset', ''))

        # Add Subject information
        subject = ET.SubElement(root, "Subject")
        ET.SubElement(subject, "Subject_UID").text = self.subject.get('UID', '')
        ET.SubElement(subject, "Subject_Type").text = self.subject.get('Type', '')
        ET.SubElement(subject, "Gender").text = self.subject.get('Gender', '')
        ET.SubElement(subject, "Pediatric").text = str(self.subject.get('Pediatric', ''))
        ET.SubElement(subject, "Subject_Age").text = str(self.subject.get('Age', ''))
        dob = ET.SubElement(subject, "Date_Of_Birth")
        ET.SubElement(dob, "Year").text = str(self.subject.get('Date_Of_Birth', {}).get('Year', ''))
        ET.SubElement(dob, "Month").text = str(self.subject.get('Date_Of_Birth', {}).get('Month', ''))
        ET.SubElement(dob, "Day").text = str(self.subject.get('Date_Of_Birth', {}).get('Day', ''))
        ET.SubElement(subject, "Race").text = self.subject.get('Race', '')
        ET.SubElement(subject, "Ethnicity").text = self.subject.get('Ethnicity', '')
        ET.SubElement(subject, "Height").text = str(self.subject.get('Height', ''))
        ET.SubElement(subject, "Weight").text = str(self.subject.get('Weight', ''))
        ET.SubElement(subject, "BMI").text = str(self.subject.get('BMI', ''))

        # Add Trial information
        trial = ET.SubElement(root, "Trial")
        ET.SubElement(trial, "VCT_Version").text = str(self.trial.get('VCT_Version', ''))

        # Add Organization information
        organization = ET.SubElement(root, "Organization")
        ET.SubElement(organization, "Name").text = self.organization.get('Name', '')
        ET.SubElement(organization, "Division").text = self.organization.get('Division', '')
        ET.SubElement(organization, "Department").text = self.organization.get('Department', '')
        ET.SubElement(organization, "Group").text = self.organization.get('Group', '')
        ET.SubElement(organization, "Institution_Code_Sequence").text = self.organization.get('Institution_Code_Sequence', '')
        ET.SubElement(organization, "Country").text = self.organization.get('Country', '')
        ET.SubElement(organization, "State").text = self.organization.get('State', '')
        ET.SubElement(organization, "City").text = self.organization.get('City', '')
        ET.SubElement(organization, "Zip").text = self.organization.get('Zip', '')

        # Add Generation_Software information
        generation_software = ET.SubElement(root, "Generation_Software")
        ET.SubElement(generation_software, "Name").text = self.generation_software.get('Name', '')
        ET.SubElement(generation_software, "Version").text = self.generation_software.get('Version', '')
        ET.SubElement(generation_software, "Repository").text = self.generation_software.get('Repository', '')

        # Add Generation_Station information
        generation_station = ET.SubElement(root, "Generation_Station")
        ET.SubElement(generation_station, "Name").text = self.generation_station.get('Name', '')
        ET.SubElement(generation_station, "OS").text = self.generation_station.get('OS', '')
        ET.SubElement(generation_station, "Architecture").text = self.generation_station.get('Architecture', '')

        # Add Voxel_Array information
        voxel_array = ET.SubElement(root, "Voxel_Array")
        ET.SubElement(voxel_array, "Voxel_Array_UID").text = self.voxel_array.get('UID', '')
        voxel_num = ET.SubElement(voxel_array, "VOXEL_NUM")
        ET.SubElement(voxel_num, "X").text = str(out_phantom.shape[2])
        ET.SubElement(voxel_num, "Y").text = str(out_phantom.shape[1])
        ET.SubElement(voxel_num, "Z").text = str(out_phantom.shape[0])
        voxel_size_mm = ET.SubElement(voxel_array, "VOXEL_SIZE_MM")
        ET.SubElement(voxel_size_mm, "X").text = str(self.get_voxel_mm()[2])
        ET.SubElement(voxel_size_mm, "Y").text = str(self.get_voxel_mm()[1])
        ET.SubElement(voxel_size_mm, "Z").text = str(self.get_voxel_mm()[0])
        ET.SubElement(voxel_array, "VOXEL_ARRAY_ORDER").text = self.voxel_array.get('Voxel_Array_Order', '')
        ET.SubElement(voxel_array, "VOXEL_TYPE").text = self.voxel_array.get('Voxel_Type', '')
        ET.SubElement(voxel_array, "Number_Bits").text = str(self.voxel_array.get('Number_Bits', ''))
        ET.SubElement(voxel_array, "Endian").text = self.voxel_array.get('Endian', '')

        # Add Index_Table information
        index_table = ET.SubElement(root, "Index_Table")
        for index in self.index_table:
            index_element = ET.SubElement(index_table, "Index")
            ET.SubElement(index_element, "Index_ID").text = str(index['Index_ID'])
            ET.SubElement(index_element, "Number_Of_Materials").text = str(index['Number_Of_Materials'])
            materials = ET.SubElement(index_element, "Materials")
            for material in index['Materials']:
                material_element = ET.SubElement(materials, "Material")
                ET.SubElement(material_element, "Material_Name").text = material['Material_Name']
                ET.SubElement(material_element, "Material_Weight").text = str(material['Material_Weight'])
                ET.SubElement(material_element, "Material_Density").text = str(material['Material_Density'])
                ET.SubElement(material_element, "Material_MaterialZ").text = str(material['Material_MaterialZ'])
        
        # Get current date and time
        current_time = datetime.now()

        # Add Deformation block
        deformation = ET.SubElement(root, "Deformation")
        ET.SubElement(deformation, "Sequence_Number").text = "0"
        ET.SubElement(deformation, "Deformation_Type").text = "FINITE_ELEMENT"
        ET.SubElement(deformation, "Deformation_Mode").text = def_mode
        ET.SubElement(deformation, "Deformation_Thickness").text = "TODO"

        # Add current Date_Deformed
        date_deformed = ET.SubElement(deformation, "Date_Deformed")
        ET.SubElement(date_deformed, "Year").text = str(current_time.year)
        ET.SubElement(date_deformed, "Month").text = f"{current_time.month:02d}"  # Zero-padded month
        ET.SubElement(date_deformed, "Day").text = f"{current_time.day:02d}"  # Zero-padded day

        # Add current Time_Deformed
        time_deformed = ET.SubElement(deformation, "Time_Deformed")
        ET.SubElement(time_deformed, "Hour").text = f"{current_time.hour:02d}"  # Zero-padded hour
        ET.SubElement(time_deformed, "Minute").text = f"{current_time.minute:02d}"  # Zero-padded minute
        ET.SubElement(time_deformed, "Seconds").text = f"{current_time.second:02d}"  # Zero-padded seconds
        ET.SubElement(time_deformed, "Fraction").text = "0000"  # Keeping the fraction static
        ET.SubElement(time_deformed, "UTC_Offset").text = "-5"  # Adjust for your timezone

        # Add Deforming Organization block
        deforming_org = ET.SubElement(deformation, "Deforming_Organization")
        ET.SubElement(deforming_org, "Name").text = "University of Pennsylvania"
        ET.SubElement(deforming_org, "Division").text = "Perlman School of Medicine"
        ET.SubElement(deforming_org, "Department").text = "Radiology"
        ET.SubElement(deforming_org, "Group").text = "X-Ray Physics Lab"
        ET.SubElement(deforming_org, "Institution_Code_Sequence").text = "1.2.826.0.1.3680043.2.936"
        ET.SubElement(deforming_org, "Country").text = "United States"
        ET.SubElement(deforming_org, "State").text = "Pennsylvania"
        ET.SubElement(deforming_org, "City").text = "Philadelphia"
        ET.SubElement(deforming_org, "Zip").text = "19104"

        # Add Deforming Software block
        deforming_software = ET.SubElement(deformation, "Deforming_Software")
        ET.SubElement(deforming_software, "Name").text = "VolumeDeformer"
        ET.SubElement(deforming_software, "Version").text = "2.0"
        ET.SubElement(deforming_software, "Repository").text = "None"
        
        # Add Build_Date (using current date)
        build_date = ET.SubElement(deforming_software, "Build_Date")
        ET.SubElement(build_date, "Year").text = str(current_time.year)
        ET.SubElement(build_date, "Month").text = f"{current_time.month:02d}"
        ET.SubElement(build_date, "Day").text = f"{current_time.day:02d}"

        # Add Build_Time (using current time)
        build_time = ET.SubElement(deforming_software, "Build_Time")
        ET.SubElement(build_time, "Hour").text = f"{current_time.hour:02d}"
        ET.SubElement(build_time, "Minute").text = f"{current_time.minute:02d}"
        ET.SubElement(build_time, "Seconds").text = f"{current_time.second:02d}"
        ET.SubElement(build_time, "Fraction").text = "0000"
        ET.SubElement(build_time, "UTC_Offset").text = "-5"

        # Add Deforming Station block
        deforming_station = ET.SubElement(deformation, "Deforming_Station")
        ET.SubElement(deforming_station, "Name").text = "T7910-2"
        ET.SubElement(deforming_station, "OS").text = "Ubuntu 20.04"
        ET.SubElement(deforming_station, "Architecture").text = "AMD64"

        # Write the tree to a file
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
    
    def write_vctx(self, out_phantom, out_dir, def_mode):
        # Temporary directory to extract zip contents
        temp_dir = out_dir
    
        # Create a temporary directory to work in
        if not os.path.exists(temp_dir):
            os.makedirs(temp_dir+"/Private")

        with zip.ZipFile(self.file_path, mode='r') as filezip:           
            filezip.extract(self.file_name+"/Private/BreastPhantomGenerator.xml", temp_dir+"/Private/BreastPhantomGenerator.xml")
            filezip.extract(self.file_name+"/Private/XPL_AttenuationTable.xml", temp_dir+"/Private/XPL_AttenuationTable.xml")

        # Replace files
        self.write_xml(out_phantom, out_dir, temp_dir+"/Phantom.xml", def_mode)
        out_phantom.astype('uint8').tofile(temp_dir+"/Phantom.dat")

        # Compress files
        filezip = zip.ZipFile(out_dir+".vctx", "w")
        for dirname, subdirs, files in os.walk(out_dir):
            filezip.write(dirname)
            for filename in files:
                filezip.write(os.path.join(dirname, filename), compress_type=zip.ZIP_DEFLATED)
        filezip.close()
            
        #Clean up the temporary directory
        shutil.rmtree(temp_dir)