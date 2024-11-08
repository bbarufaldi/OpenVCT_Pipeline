import xml.etree.ElementTree as ET

class Projection:
    def __init__(self, xml_file):
        self.tree = ET.parse(xml_file)
        self.root = self.tree.getroot()
        self.projector_config = {}
        self.imaging_system = {}
        self.exposure_settings = {}
        self.detector = {}
        self.acquisition_geometry = []
        self.parse_xml()

    def parse_xml(self):
        # Parse Projector_Config section
        projector_config = self.root.find('Projector_Config')
        if projector_config is not None:
            self.projector_config = {
                'Program_Name': projector_config.findtext('Program_Name'),
                'Software_Version': projector_config.findtext('Software_Version'),
                'Input_Phantom': projector_config.findtext('Input_Phantom'),
                'Output_Folder': projector_config.findtext('Output_Folder'),
                'Has_Noise': projector_config.findtext('Has_Noise') == 'True',
                'Has_Invsq': projector_config.findtext('Has_Invsq') == 'True',
                'Elec_Noise': float(projector_config.findtext('Elec_Noise')),
                'Gain': float(projector_config.findtext('Gain')),
                'Prop_Const': float(projector_config.findtext('Prop_Const')),
                'Random_Seed': int(projector_config.findtext('Random_Seed'))
            }

        # Parse Imaging_System section
        imaging_system = self.root.find('Imaging_System')
        if imaging_system is not None:
            self.imaging_system = {
                'Name': imaging_system.findtext('Name')
            }

        # Parse Exposure_Settings section
        exposure_settings = self.root.find('Exposure_Settings')
        if exposure_settings is not None:
            self.exposure_settings = {
                'Spectrum_ID': int(exposure_settings.findtext('Spectrum_ID')),
                'Anode_Material': exposure_settings.findtext('Anode_Material'),
                'Filter': {
                    'Material': exposure_settings.find('Filter/Material').text,
                    'MaterialZ': int(exposure_settings.find('Filter/MaterialZ').text),
                    'Thickness_mm': float(exposure_settings.find('Filter/Thickness_mm').text)
                },
                'KVP': int(exposure_settings.findtext('KVP')),
                'Exposure': int(exposure_settings.findtext('Exposure')),
                'Phantom_Thickness_mm': float(exposure_settings.findtext('Phantom_Thickness_mm')),
                'Density_Coefficient': int(exposure_settings.findtext('Density_Coefficient'))
            }

        # Parse Detector section
        detector = self.root.find('Detector')
        if detector is not None:
            self.detector = {
                'Element_Size_mm': {
                    'X': float(detector.find('Element_Size_mm/X').text),
                    'Y': float(detector.find('Element_Size_mm/Y').text)
                },
                'Element_Count': {
                    'X': int(detector.find('Element_Count/X').text),
                    'Y': int(detector.find('Element_Count/Y').text)
                },
                'Thickness_mm': float(detector.findtext('Thickness_mm')),
                'Material': detector.findtext('Material'),
                'Bits_Allocated': int(detector.findtext('Bits_Allocated')),
                'Bits_Stored': int(detector.findtext('Bits_Stored')),
                'High_Bit': int(detector.findtext('High_Bit')),
                'Has_Volume_Offset': detector.findtext('Has_Volume_Offset') == 'True',
                'Volume_Offset_mm': {
                    'X': float(detector.find('Volume_Offset_mm/X').text),
                    'Y': float(detector.find('Volume_Offset_mm/Y').text),
                    'Z': float(detector.find('Volume_Offset_mm/Z').text)
                }
            }

        # Parse Acquisition_Geometry section
        acquisition_geometry = self.root.find('Acquisition_Geometry')
        if acquisition_geometry is not None:
            for acquisition in acquisition_geometry.findall('Acquisition'):
                acq_data = {
                    'ID': int(acquisition.findtext('ID')),
                    'Acquisition_UID': acquisition.findtext('Acquisition_UID'),
                    'Focal_Spot': {
                        'X_mm': float(acquisition.find('Focal_Spot/X_mm').text),
                        'Y_mm': float(acquisition.find('Focal_Spot/Y_mm').text),
                        'Z_mm': float(acquisition.find('Focal_Spot/Z_mm').text)
                    },
                    'Detector': {
                        'X_mm': float(acquisition.find('Detector/X_mm').text),
                        'Y_mm': float(acquisition.find('Detector/Y_mm').text),
                        'Z_mm': float(acquisition.find('Detector/Z_mm').text),
                        'Orient_SX_mm': float(acquisition.find('Detector/Orient_SX_mm').text),
                        'Orient_SY_mm': float(acquisition.find('Detector/Orient_SY_mm').text),
                        'Orient_SZ_mm': float(acquisition.find('Detector/Orient_SZ_mm').text),
                        'Orient_TX_mm': float(acquisition.find('Detector/Orient_TX_mm').text),
                        'Orient_TY_mm': float(acquisition.find('Detector/Orient_TY_mm').text),
                        'Orient_TZ_mm': float(acquisition.find('Detector/Orient_TZ_mm').text)
                    }
                }
                self.acquisition_geometry.append(acq_data)

    def get_data(self):
        return {
            'Projector_Config': self.projector_config,
            'Imaging_System': self.imaging_system,
            'Exposure_Settings': self.exposure_settings,
            'Detector': self.detector,
            'Acquisition_Geometry': self.acquisition_geometry
        }
