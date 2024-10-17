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
        
        # Phantom Fields
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
        self.thickness_mm = {
            'X': None,
            'Y': None,
            'Z': None
        }
        self.glandular_count = None
        self.date_created = {
            'Year': None,
            'Month': None,
            'Day': None
        }
        self.time_created = {
            'Hour': None,
            'Minute': None,
            'Seconds': None,
            'Fraction': None,
            'UTC_Offset': None
        }
        self.subject = {
            'UID': None,
            'Type': None,
            'Gender': None,
            'Pediatric': None,
            'Age': None,
            'Date_Of_Birth': {
                'Year': None,
                'Month': None,
                'Day': None
            },
            'Race': None,
            'Ethnicity': None,
            'Height': None,
            'Weight': None,
            'BMI': None
        }
        self.trial = {
            'VCT_Version': None
        }
        self.organization = {
            'Name': None,
            'Division': None,
            'Department': None,
            'Group': None,
            'Institution_Code_Sequence': None,
            'Country': None,
            'State': None,
            'City': None,
            'Zip': None
        }
        self.generation_software = {
            'Name': None,
            'Version': None,
            'Repository': None,
            'Build Date': {
                'Year': None,
                'Month': None,
                'Day': None
            },
            'Build Time': {
                'Hour': None,
                'Minute': None,
                'Seconds': None,
                'Fraction': None,
                'UTC_Offset': None
            }
        }
        self.generation_station = {
            'Name': None,
            'OS': None,
            'Architecture': None
        }
        self.voxel_array = {
            'UID': None,
            'Voxel_Num': {
                'X': None,
                'Y': None,
                'Z': None
            },
            'Voxel_Size_MM': {
                'X': None,
                'Y': None,
                'Z': None
            },
            'Voxel_Array_Order': None,
            'Voxel_Type': None,
            'Number_Bits': None,
            'Endian': None,
            'Direction_Cosines': []
        }
        self.index_table = {
            'Index_Table_UID': None,
            'Maximum_Index': None,
            'Number_of_Indicies': None,
            'Indices': []
        }
        self.deformation = {
            'Sequence_Number': None,
            'Deformation_Type': None,
            'Deformation_Mode': None,
            'Deformation_Thickness': None,
            'Date_Deformed': {
                'Year': None,
                'Month': None,
                'Day': None
            },
            'Time_Deformed': {
                'Hour': None,
                'Minute': None,
                'Seconds': None,
                'Fraction': None,
                'UTC_Offset': None
            },
            'Deforming_Organization': {
                'Name': None,
                'Division': None,
                'Department': None,
                'Group': None,
                'Institution_Code_Sequence': None,
                'Country': None,
                'State': None,
                'City': None,
                'Zip': None
            },
            'Deforming_Software': {
                'Name': None,
                'Version': None,
                'Repository': None,
                'Build_Date': {
                    'Year': None,
                    'Month': None,
                    'Day': None
                },
                'Build_Time': {
                    'Hour': None,
                    'Minute': None,
                    'Seconds': None,
                    'Fraction': None,
                    'UTC_Offset': None
                }
            },
            'Deforming_Station': {
                'Name': None,
                'OS': None,
                'Architecture': None
            }
        }

        self.VOIs = []
        self.Lesions = []

        self.def_mode = None
        self.set_phantom()

    def set_phantom(self):
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
        self.thickness_mm = {
            'X': float(thickness_mm_element.find("X").text),
            'Y': float(thickness_mm_element.find("Y").text),
            'Z': float(thickness_mm_element.find("Z").text)
        }

        # Glandular Count
        self.glandular_count = int(root.find("Glandular_Count").text)

        # Parsing Date_Created
        date_created_element = root.find("Date_Created")
        self.date_created = {
            'Year': int(date_created_element.find("Year").text),
            'Month': int(date_created_element.find("Month").text),
            'Day': int(date_created_element.find("Day").text)
        }

        # Parsing Time_Created
        time_created_element = root.find("Time_Created")
        self.time_created = {
            'Hour': int(time_created_element.find("Hour").text),
            'Minute': int(time_created_element.find("Minute").text),
            'Seconds': int(time_created_element.find("Seconds").text),
            'Fraction': int(time_created_element.find("Fraction").text),
            'UTC_Offset': int(time_created_element.find("UTC_Offset").text)
        }

        # Parsing Subject
        subject_element = root.find("Subject")
        dob_element = subject_element.find("Date_Of_Birth")
        self.subject = {
            'UID': subject_element.find("Subject_UID").text,
            'Type': subject_element.find("Subject_Type").text,
            'Gender': subject_element.find("Gender").text,
            'Pediatric': int(subject_element.find("Pediatric").text),
            'Age': int(subject_element.find("Subject_Age").text),
            'Date_Of_Birth': {
                'Year': int(dob_element.find("Year").text),
                'Month': int(dob_element.find("Month").text),
                'Day': int(dob_element.find("Day").text)
            },
            'Race': subject_element.find("Race").text,
            'Ethnicity': subject_element.find("Ethnicity").text,
            'Height': float(subject_element.find("Height").text),
            'Weight': float(subject_element.find("Weight").text),
            'BMI': float(subject_element.find("BMI").text)
        }

        # Parsing Trial
        self.trial = {
            'VCT_Version': float(root.find("Trial/VCT_Version").text)
        }

        # Parsing Organization
        organization_element = root.find("Organization")
        self.organization = {
            'Name': organization_element.find("Name").text,
            'Division': organization_element.find("Division").text,
            'Department': organization_element.find("Department").text,
            'Group': organization_element.find("Group").text,
            'Institution_Code_Sequence': organization_element.find("Institution_Code_Sequence").text,
            'Country': organization_element.find("Country").text,
            'State': organization_element.find("State").text,
            'City': organization_element.find("City").text,
            'Zip': organization_element.find("Zip").text
        }

        # Parsing Generation_Software
        generation_software_element = root.find("Generation_Software")
        self.generation_software = {
            'Name': generation_software_element.find("Name").text,
            'Version': generation_software_element.find("Version").text,
            'Repository': generation_software_element.find("Repository").text,
            'Build_Date': {
                'Year': None,
                'Month': None,
                'Day': None
            },
            'Build_Time': {
                'Hour': None,
                'Minute': None,
                'Seconds': None,
                'Fraction': None,
                'UTC_Offset': None
            },
        }

        # Parsing Generation_Station
        generation_station_element = root.find("Generation_Station")
        self.generation_station = {
            'Name': generation_station_element.find("Name").text,
            'OS': generation_station_element.find("OS").text,
            'Architecture': generation_station_element.find("Architecture").text
        }

        # Parsing Voxel_Array
        voxel_array_element = root.find("Voxel_Array")
        voxel_num_element = voxel_array_element.find("VOXEL_NUM")
        voxel_size_element = voxel_array_element.find("VOXEL_SIZE_MM")
        direction_cosines_element = voxel_array_element.find("Direction_Cosines")

        self.voxel_array = {
            'UID': voxel_array_element.find("Voxel_Array_UID").text,
            'Voxel_Num': {
                'X': int(voxel_num_element.find("X").text),
                'Y': int(voxel_num_element.find("Y").text),
                'Z': int(voxel_num_element.find("Z").text)
            },
            'Voxel_Size_MM': {
                'X': float(voxel_size_element.find("X").text),
                'Y': float(voxel_size_element.find("Y").text),
                'Z': float(voxel_size_element.find("Z").text)
            },
            'Voxel_Array_Order': voxel_array_element.find("VOXEL_ARRAY_ORDER").text,
            'Voxel_Type': voxel_array_element.find("VOXEL_TYPE").text,
            'Number_Bits': int(voxel_array_element.find("Number_Bits").text),
            'Endian': voxel_array_element.find("Endian").text,
            'Direction_Cosines': [float(value.text) for value in direction_cosines_element.findall("Value")]
        }

        # Parsing Index_Table
        index_table_element = root.find("Index_Table")
        self.index_table = {
            'Index_Table_UID': index_table_element.find("Index_Table_UID").text,
            'Maximum_Index': int(index_table_element.find("Maximum_Index").text),
            'Number_of_Indicies': int(index_table_element.find("Number_of_Indicies").text),
            'Indices': []
        }

        # Parsing each Index
        for index in index_table_element.findall("Index"):
            index_data = {
                "Index_ID": int(index.find("Index_ID").text),
                "Number_Of_Materials": int(index.find("Number_Of_Materials").text),
                "Materials": []
            }

            # Parsing materials within each index
            materials = index.find("Materials")
            for material in materials.findall("Material"):
                material_data = {
                    "Material_Name": material.find("Material_Name").text,
                    "Material_Weight": float(material.find("Material_Weight").text),
                    "Material_Density": float(material.find("Material_Density").text),
                    "Material_MaterialZ": int(material.find("Material_MaterialZ").text)
                }
                index_data["Materials"].append(material_data)

            self.index_table['Indices'].append(index_data)

        # Parsing Deformation block
        deformation_element = root.find("Deformation")
        if deformation_element is not None:
            self.deformation = {
                'Sequence_Number': int(deformation_element.find("Sequence_Number").text),
                'Deformation_Type': deformation_element.find("Deformation_Type").text,
                'Deformation_Mode': deformation_element.find("Deformation_Mode").text,
                'Deformation_Thickness': deformation_element.find("Deformation_Thickness").text,
                'Date_Deformed': {
                    'Year': int(deformation_element.find("Date_Deformed/Year").text),
                    'Month': int(deformation_element.find("Date_Deformed/Month").text),
                    'Day': int(deformation_element.find("Date_Deformed/Day").text)
                },
                'Time_Deformed': {
                    'Hour': int(deformation_element.find("Time_Deformed/Hour").text),
                    'Minute': int(deformation_element.find("Time_Deformed/Minute").text),
                    'Seconds': int(deformation_element.find("Time_Deformed/Seconds").text),
                    'Fraction': int(deformation_element.find("Time_Deformed/Fraction").text),
                    'UTC_Offset': int(deformation_element.find("Time_Deformed/UTC_Offset").text)
                },
                'Deforming_Organization': {
                    'Name': deformation_element.find("Deforming_Organization/Name").text,
                    'Division': deformation_element.find("Deforming_Organization/Division").text,
                    'Department': deformation_element.find("Deforming_Organization/Department").text,
                    'Group': deformation_element.find("Deforming_Organization/Group").text,
                    'Institution_Code_Sequence': deformation_element.find("Deforming_Organization/Institution_Code_Sequence").text,
                    'Country': deformation_element.find("Deforming_Organization/Country").text,
                    'State': deformation_element.find("Deforming_Organization/State").text,
                    'City': deformation_element.find("Deforming_Organization/City").text,
                    'Zip': deformation_element.find("Deforming_Organization/Zip").text
                },
                'Deforming_Software': {
                    'Name': deformation_element.find("Deforming_Software/Name").text,
                    'Version': deformation_element.find("Deforming_Software/Version").text,
                    'Repository': deformation_element.find("Deforming_Software/Repository").text,
                    'Build_Date': {
                        'Year': int(deformation_element.find("Deforming_Software/Build_Date/Year").text),
                        'Month': int(deformation_element.find("Deforming_Software/Build_Date/Month").text),
                        'Day': int(deformation_element.find("Deforming_Software/Build_Date/Day").text)
                    },
                    'Build_Time': {
                        'Hour': int(deformation_element.find("Deforming_Software/Build_Time/Hour").text),
                        'Minute': int(deformation_element.find("Deforming_Software/Build_Time/Minute").text),
                        'Seconds': int(deformation_element.find("Deforming_Software/Build_Time/Seconds").text),
                        'Fraction': int(deformation_element.find("Deforming_Software/Build_Time/Fraction").text),
                        'UTC_Offset': int(deformation_element.find("Deforming_Software/Build_Time/UTC_Offset").text)
                    }
                },
                'Deforming_Station': {
                    'Name': deformation_element.find("Deforming_Station/Name").text,
                    'OS': deformation_element.find("Deforming_Station/OS").text,
                    'Architecture': deformation_element.find("Deforming_Station/Architecture").text
                }
            }

        # Reading VOIs
        vois = root.find('VOIs')
        if vois is not None:
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
        if lesions is not None:
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

    def write_xml(self, out_phantom, out_name, output_file, xml=None):
        """Writes the stored information back to an XML file."""

        root = ET.Element("VCT_Phantom")

        # Phantom general information
        ET.SubElement(root, "Phantom_Name").text = out_name
        ET.SubElement(root, "Phantom_Shape").text = "ANTHROPOMORPHIC"
        ET.SubElement(root, "Max_Ligament_Thickness").text = str(self.max_ligament_thickness)
        ET.SubElement(root, "Min_Ligament_Thickness").text = str(self.min_ligament_thickness)
        ET.SubElement(root, "Phantom_UID").text = self.phantom_uid
        ET.SubElement(root, "Phantom_Source").text = self.phantom_source
        ET.SubElement(root, "Phantom_Type").text = self.phantom_type
        ET.SubElement(root, "Data_Model").text = self.data_model
        ET.SubElement(root, "Body_Part").text = self.body_part
        ET.SubElement(root, "Laterality").text = self.laterality
        ET.SubElement(root, "Total_Non_Air_Voxels").text = str(self.total_non_air_voxels)

        # Thickness_mm information
        thickness_mm = ET.SubElement(root, "Thickness_mm")
        ET.SubElement(thickness_mm, "X").text = str(out_phantom.shape[2]*self.get_voxel_mm()[2]) if xml == None else str(out_phantom.shape[1]*self.get_voxel_mm()[1])
        ET.SubElement(thickness_mm, "Y").text = str(out_phantom.shape[1]*self.get_voxel_mm()[1]) if xml == None else str(out_phantom.shape[2]*self.get_voxel_mm()[2])
        ET.SubElement(thickness_mm, "Z").text = str(out_phantom.shape[0]*self.get_voxel_mm()[0])

        # Glandular Count TODO: UPDATE THIS FIELD
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

        # Subject information
        subject = ET.SubElement(root, "Subject")
        ET.SubElement(subject, "Subject_UID").text = self.subject['UID']
        ET.SubElement(subject, "Subject_Type").text = self.subject['Type']
        ET.SubElement(subject, "Gender").text = self.subject['Gender']
        ET.SubElement(subject, "Pediatric").text = str(self.subject['Pediatric'])
        ET.SubElement(subject, "Subject_Age").text = str(self.subject['Age'])
        dob = ET.SubElement(subject, "Date_Of_Birth")
        ET.SubElement(dob, "Year").text = str(self.subject['Date_Of_Birth']['Year'])
        ET.SubElement(dob, "Month").text = str(self.subject['Date_Of_Birth']['Month'])
        ET.SubElement(dob, "Day").text = str(self.subject['Date_Of_Birth']['Day'])
        ET.SubElement(subject, "Race").text = self.subject['Race']
        ET.SubElement(subject, "Ethnicity").text = self.subject['Ethnicity']
        ET.SubElement(subject, "Height").text = str(self.subject['Height'])
        ET.SubElement(subject, "Weight").text = str(self.subject['Weight'])
        ET.SubElement(subject, "BMI").text = str(self.subject['BMI'])

        # Trial information
        trial = ET.SubElement(root, "Trial")
        ET.SubElement(trial, "VCT_Version").text = str(self.trial['VCT_Version'])

        # Organization information
        organization = ET.SubElement(root, "Organization")
        ET.SubElement(organization, "Name").text = self.organization['Name']
        ET.SubElement(organization, "Division").text = self.organization['Division']
        ET.SubElement(organization, "Department").text = self.organization['Department']
        ET.SubElement(organization, "Group").text = self.organization['Group']
        ET.SubElement(organization, "Institution_Code_Sequence").text = self.organization['Institution_Code_Sequence']
        ET.SubElement(organization, "Country").text = self.organization['Country']
        ET.SubElement(organization, "State").text = self.organization['State']
        ET.SubElement(organization, "City").text = self.organization['City']
        ET.SubElement(organization, "Zip").text = self.organization['Zip']

        # Generation Software information
        generation_software = ET.SubElement(root, "Generation_Software")
        ET.SubElement(generation_software, "Name").text = self.generation_software['Name']
        ET.SubElement(generation_software, "Version").text = self.generation_software['Version']
        ET.SubElement(generation_software, "Repository").text = self.generation_software['Repository']

        # Get current date and time
        current_time = datetime.now()
        
        build_date = ET.SubElement(generation_software, "Build_Date")
        ET.SubElement(build_date, "Year").text = str(current_time.year)
        ET.SubElement(build_date, "Month").text = f"{current_time.month:02d}"  # Zero-padded month
        ET.SubElement(build_date, "Day").text = f"{current_time.day:02d}"  # Zero-padded day

        build_time = ET.SubElement(generation_software, "Build_Time")
        ET.SubElement(build_time, "Hour").text = f"{current_time.hour:02d}"  # Zero-padded hour
        ET.SubElement(build_time, "Minute").text = f"{current_time.minute:02d}"  # Zero-padded minute
        ET.SubElement(build_time, "Seconds").text = f"{current_time.second:02d}"  # Zero-padded seconds
        ET.SubElement(build_time, "Fraction").text = "0000"  # Keeping the fraction static
        ET.SubElement(build_time, "UTC_Offset").text = "-5"  # Adjust for your timezone

        # Generation Station information
        generation_station = ET.SubElement(root, "Generation_Station")
        ET.SubElement(generation_station, "Name").text = self.generation_station['Name']
        ET.SubElement(generation_station, "OS").text = self.generation_station['OS']
        ET.SubElement(generation_station, "Architecture").text = self.generation_station['Architecture']

        # Voxel_Array information
        voxel_array = ET.SubElement(root, "Voxel_Array")
        ET.SubElement(voxel_array, "Voxel_Array_UID").text = self.voxel_array['UID']
        voxel_num = ET.SubElement(voxel_array, "VOXEL_NUM")
        ET.SubElement(voxel_num, "X").text = str(out_phantom.shape[2]) if xml == None else str(out_phantom.shape[1])
        ET.SubElement(voxel_num, "Y").text = str(out_phantom.shape[1]) if xml == None else str(out_phantom.shape[2])
        ET.SubElement(voxel_num, "Z").text = str(out_phantom.shape[0])
        
        voxel_size_mm = ET.SubElement(voxel_array, "VOXEL_SIZE_MM")
        ET.SubElement(voxel_size_mm, "X").text = str(self.get_voxel_mm()[2])
        ET.SubElement(voxel_size_mm, "Y").text = str(self.get_voxel_mm()[1])
        ET.SubElement(voxel_size_mm, "Z").text = str(self.get_voxel_mm()[0])
        ET.SubElement(voxel_array, "VOXEL_ARRAY_ORDER").text = self.voxel_array['Voxel_Array_Order']
        ET.SubElement(voxel_array, "VOXEL_TYPE").text = self.voxel_array['Voxel_Type']
        ET.SubElement(voxel_array, "Number_Bits").text = str(self.voxel_array['Number_Bits'])
        ET.SubElement(voxel_array, "Endian").text = self.voxel_array['Endian']
        
        # Add Direction Cosines
        direction_cosines = ET.SubElement(voxel_array, "Direction_Cosines")
        for value in self.voxel_array['Direction_Cosines']:
            ET.SubElement(direction_cosines, "Value").text = str(value)

        # Index_Table information
        index_table = ET.SubElement(root, "Index_Table")
        ET.SubElement(index_table, "Index_Table_UID").text = self.index_table['Index_Table_UID']
        ET.SubElement(index_table, "Maximum_Index").text = str(int(self.index_table['Maximum_Index'])*2) # BUG: Fix this doubling the number of Indices to increase buffer size
        ET.SubElement(index_table, "Number_of_Indicies").text = str(int(self.index_table['Number_of_Indicies'])*2-1) # BUG: Fix this doubling the number of Indices to increase buffer size

        for index in self.index_table['Indices']:
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
        
        # BUG: Fix this doubling the number of Indices to increase buffer size
        for i in range(int(self.index_table['Maximum_Index'])+1, int(self.index_table['Maximum_Index']*2+1)):
            index_element = ET.SubElement(index_table, "Index")
            ET.SubElement(index_element, "Index_ID").text = str(i)
            ET.SubElement(index_element, "Number_Of_Materials").text = str(1)
            materials = ET.SubElement(index_element, "Materials")
            material_element = ET.SubElement(materials, "Material")
            ET.SubElement(material_element, "Material_Name").text = "Air"
            ET.SubElement(material_element, "Material_Weight").text = "1.0"
            ET.SubElement(material_element, "Material_Density").text = "0.0012"
            ET.SubElement(material_element, "Material_MaterialZ").text = "201"          

        # Add Deformation block
        deformation = ET.SubElement(root, "Deformation")
        ET.SubElement(deformation, "Sequence_Number").text = str(self.deformation['Sequence_Number']) if None else "0"
        ET.SubElement(deformation, "Deformation_Type").text = self.deformation['Deformation_Type'] if None else "FINITE_ELEMENT"
        ET.SubElement(deformation, "Deformation_Mode").text = self.deformation['Deformation_Mode'] if xml==None else "DEFORM_"+ xml.View
        ET.SubElement(deformation, "Deformation_Thickness").text = "TODO"

        # Add Date_Deformed
        date_deformed = ET.SubElement(deformation, "Date_Deformed")
        ET.SubElement(date_deformed, "Year").text = str(self.deformation['Date_Deformed']['Year']) if None else str(current_time.year)
        ET.SubElement(date_deformed, "Month").text = f"{self.deformation['Date_Deformed']['Month']:02d}" if None else f"{current_time.month:02d}"  # Zero-padded month
        ET.SubElement(date_deformed, "Day").text = f"{self.deformation['Date_Deformed']['Day']:02d}" if None else f"{current_time.day:02d}"  # Zero-padded day

        # Add Time_Deformed
        time_deformed = ET.SubElement(deformation, "Time_Deformed")
        ET.SubElement(time_deformed, "Hour").text = f"{self.deformation['Time_Deformed']['Hour']:02d}" if None else f"{current_time.hour:02d}"  # Zero-padded hour
        ET.SubElement(time_deformed, "Minute").text = f"{self.deformation['Time_Deformed']['Minute']:02d}" if None else f"{current_time.minute:02d}"  # Zero-padded minute
        ET.SubElement(time_deformed, "Seconds").text = f"{self.deformation['Time_Deformed']['Seconds']:02d}" if None else f"{current_time.second:02d}"  # Zero-padded seconds
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

        # # Add VOIs block
        # vois_element = ET.SubElement(root, "VOIs")
        # for voi in self.VOIs:
        #     voi_element = ET.SubElement(vois_element, "VOI")
        #     ET.SubElement(voi_element, "Center_X").text = str(voi.get('Center_X', 'N/A'))
        #     ET.SubElement(voi_element, "Center_Y").text = str(voi.get('Center_Y', 'N/A'))
        #     ET.SubElement(voi_element, "Center_Z").text = str(voi.get('Center_Z', 'N/A'))
        #     ET.SubElement(voi_element, "Height").text = str(voi.get('Height', 'N/A'))
        #     ET.SubElement(voi_element, "Width").text = str(voi.get('Width', 'N/A'))
        #     ET.SubElement(voi_element, "Depth").text = str(voi.get('Depth', 'N/A'))
        #     ET.SubElement(voi_element, "Has_Lesion").text = str(voi.get('Has_Lesion', 'true')).lower()

        # # Write Lesions
        # lesions_element = ET.SubElement(root, "Lesions")
        # for lesion in self.Lesions:
        #     lesion_element = ET.SubElement(lesions_element, "Lesion")
        #     ET.SubElement(lesion_element, "LesionName").text = str(lesion.get('LesionName', 'Unknown'))
        #     ET.SubElement(lesion_element, "LesionType").text = str(lesion.get('LesionType', 'Unknown'))
        #     ET.SubElement(lesion_element, "Center_X").text = str(lesion.get('Center_X', 'N/A'))
        #     ET.SubElement(lesion_element, "Center_Y").text = str(lesion.get('Center_Y', 'N/A'))
        #     ET.SubElement(lesion_element, "Center_Z").text = str(lesion.get('Center_Z', 'N/A'))
        #     ET.SubElement(lesion_element, "Height").text = str(lesion.get('Height', 'N/A'))
        #     ET.SubElement(lesion_element, "Width").text = str(lesion.get('Width', 'N/A'))
        #     ET.SubElement(lesion_element, "Depth").text = str(lesion.get('Depth', 'N/A'))

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
    
    def write_vctx(self, out_phantom, xml): 
        # Temporary directory to extract zip contents
        out_dir = xml.Output_Phantom
        temp_dir = (out_dir.split('/')[-1]).replace('.vctx', '')
    
        # Create a temporary directory to work in
        if not os.path.exists(temp_dir):
            os.makedirs(temp_dir+"/Private")

        flag_def = False

        # TODO: Fix this. Only works for phantoms with the same name.
        with zip.ZipFile(self.file_path, mode='r') as filezip:
            file_list = filezip.namelist()  # Get list of files in zip      
            
            breast_phantom_gen_path = self.file_name + "/Private/BreastPhantomGenerator.xml"
            attenuation_table_path = self.file_name + "/Private/XPL_AttenuationTable.xml"
            deformation_path = self.file_name + "/Private/BreastPhantomDeformer.xml"

            if breast_phantom_gen_path in file_list:
                filezip.extract(breast_phantom_gen_path, './')
                filezip.extract(attenuation_table_path, './')
                
            if deformation_path in file_list:
                filezip.extract(deformation_path, './')
                flag_def = True
        
        # Replace files
        out_phantom.astype('uint8').tofile(temp_dir+"/Phantom.dat")
        
        if not flag_def: # Deformation
            self.write_xml(out_phantom, temp_dir, temp_dir+"/Phantom.xml", xml)
            shutil.copy(xml.xml_file, temp_dir+"/Private/BreastPhantomDeformer.xml")

        else: # Insertion
            self.write_xml(out_phantom, temp_dir, temp_dir+"/Phantom.xml")
            shutil.copy(xml.xml_file, temp_dir+"/Private/BreastPhantomInserter.xml")

        # Compress files
        filezip = zip.ZipFile(out_dir, "w")
        for dirname, subdirs, files in os.walk(temp_dir):
            filezip.write(dirname)
            for filename in files:
                filezip.write(os.path.join(dirname, filename), compress_type=zip.ZIP_DEFLATED)
        filezip.close()
            
        #Clean up the temporary directory
        #shutil.rmtree(temp_dir)
