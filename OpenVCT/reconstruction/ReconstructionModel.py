import numpy as np
import logging as log
import argparse
import os

from readers.xml import Phantom as ph
from readers.xml import Projection as proj

import pydicom
from pydicom.dataset import Dataset, FileDataset
from pydicom.uid import generate_uid, ExplicitVRLittleEndian

from pathlib import Path

from OpenVCT.reconstruction.pyDBT.pydbt.parameters.parameterSettings import geometry_settings
from OpenVCT.reconstruction.pyDBT.pydbt.functions.FBP import FDK as FBP
from OpenVCT.reconstruction.pyDBT.pydbt.functions.SIRT import SIRT
from OpenVCT.reconstruction.pyDBT.pydbt.functions.SART import SART

class ReconstructionModel:
    def __init__(self, xml_file,  
                 input_folder, output_folder,
                 recon_mm,
                 filterType='BP',
                 cutoff=0.75,
                 nIter=2
                 ):

        self.reader = proj.Projection(xml_file)

        self.recon_mm = recon_mm
        self.filterType = filterType
        self.cutoff = cutoff
        self.nIter = nIter

        self.input_folder = input_folder
        self.output_folder = output_folder

        log.basicConfig(filename='reconstruction/Reconstruction.log', level=log.DEBUG,
                        format='%(asctime)s - %(levelname)s - %(message)s')
        self.log = log.getLogger()

    def readProjections(self):
        dcmFiles = [str(item) for item in Path(self.input_folder).glob("*.dcm")]
        
        # Test if list is empty
        if not dcmFiles:    
            raise ValueError('No DICOM files found in the specified path.')
        
        proj = [None] * len(dcmFiles)
        proj_header = [None] * len(dcmFiles)
        
        for f in dcmFiles:
            nProj = int(f.split('/')[-1].split('.')[0].replace('_', ''))
            proj_header[nProj] = pydicom.dcmread(f)
            proj[nProj]  = proj_header[nProj].pixel_array
        
        proj = np.stack(proj, axis=-1).astype(np.float64)
        return proj, proj_header
    
    def write_reconstructions(self, vol):

        os.makedirs(self.output_folder, exist_ok=True)
        base_filename = "image"
        
        vol = np.transpose(vol, (2, 1, 0)) 

        # Convert float64 to uint16 by scaling
        array_min, array_max = vol.min(), vol.max()
        scaled_array = ((vol - array_min) / (array_max - array_min) * 65535).astype(np.uint16)
        
        frames, rows, cols = scaled_array.shape

        for i in range(frames):
            # Create a new DICOM file for each frame
            ds = FileDataset(f"{self.output_folder}/{base_filename}_{i:04d}.dcm", {}, 
                            file_meta=pydicom.Dataset(), preamble=b"\0" * 128)
            
            # Set required DICOM file meta information
            ds.file_meta.TransferSyntaxUID = ExplicitVRLittleEndian
            ds.file_meta.MediaStorageSOPClassUID = pydicom.uid.MultiFrameSingleBitSecondaryCaptureImageStorage
            ds.file_meta.MediaStorageSOPInstanceUID = generate_uid()
            ds.file_meta.ImplementationClassUID = generate_uid()
            
            # Set dataset attributes
            ds.SOPClassUID = ds.file_meta.MediaStorageSOPClassUID
            ds.SOPInstanceUID = ds.file_meta.MediaStorageSOPInstanceUID
            ds.StudyInstanceUID = generate_uid()
            ds.SeriesInstanceUID = generate_uid()
            ds.FrameOfReferenceUID = generate_uid()
            #ds.is_little_endian = True
            #ds.is_implicit_VR = False

            # Set essential imaging properties
            ds.Modality = "MG"
            ds.PatientName = self.output_folder.split('/')[-1]
            ds.PatientID = "123456"
            ds.StudyID = "1"
            ds.SeriesNumber = "1"
            ds.InstanceNumber = str(i + 1)

            # Set pixel data properties based on converted array type
            ds.Rows = rows
            ds.Columns = cols
            ds.SamplesPerPixel = 1 

            ds.BitsAllocated = 16
            ds.BitsStored = 16
            ds.HighBit = 15
            ds.PixelRepresentation = 0  # Unsigned
            ds.PhotometricInterpretation = "MONOCHROME1"
            ds.PixelSpacing = [1.0, 1.0]  

            # Set PixelData for the current frame
            ds.PixelData = scaled_array[i].tobytes()

            # Save individual DICOM file
            dicom_filename = os.path.join(self.output_folder, f"{base_filename}_{i:04d}.dcm")
            ds.save_as(dicom_filename)
            self.log.info(f"Saved frame {i+1}/{frames} as {dicom_filename}")

    def reconstruct(self):

        libDir = 'reconstruction/pyDBT/build/lib.linux-x86_64-3.10'
        libFiles = [(libDir, str(item).split('/')[-1]) for item in Path(libDir).glob("*.so")]
        self.log.info(f"LibFiles located at: {libDir}")
        self.log.info(f"List of libs: {libFiles}")
        
        config = self.reader.get_data()
        vctx_file = (config['Projector_Config']['Input_Phantom']).replace("../", "") # This can be and issue
        self.log.info(f"Phantom Location: {vctx_file}")
        
        # Phantom Info
        phantom = ph.Phantom(vctx_file)
        ph_num = phantom.voxel_data.shape
        ph_mm = phantom.get_voxel_mm()
        ph_size = ph_num*ph_mm # Phantom size in mm
      
        # Acquisition Info
        self.log.info(f"Number of Acquisitions: {len(config['Acquisition_Geometry'])}")
        self.log.info(f"Central Acquisition: {config['Acquisition_Geometry'][int(len(config['Acquisition_Geometry'])/2)]}")
        self.log.info(f"SID: {config['Acquisition_Geometry'][int(len(config['Acquisition_Geometry'])/2)]['Focal_Spot']['Z_mm']}")

        geo = geometry_settings(voxels=[round(ph_size[0]/self.recon_mm[0]), round(ph_size[1]/self.recon_mm[1]), round(ph_size[2]/self.recon_mm[2])],  
                                detector_el=[config['Detector']['Element_Count']['Y'], config['Detector']['Element_Count']['X']],
                                voxels_size=self.recon_mm,
                                detector_size=[config['Detector']['Element_Size_mm']['Y'], config['Detector']['Element_Size_mm']['X']],
                                source_dist=config['Acquisition_Geometry'][int(len(config['Acquisition_Geometry'])/2)]['Focal_Spot']['Z_mm'],
                                gap = config['Detector']['Volume_Offset_mm']['Z'],
                                n_proj = len(config['Acquisition_Geometry']),
                                tube_angle = len(config['Acquisition_Geometry']),
                                detector_angle = 0,
                                offset = [0, 0])
        
        # Debug Settings
        self.log.info(f"Reconstructions Settings...")
        self.log.info(f"Number of Voxels: {geo.nx}, {geo.ny}, {geo.nz}")
        self.log.info(f"Detector Panel: {geo.nu}, {geo.nv}")
        self.log.info(f"Voxel Size: {geo.dx}, {geo.dy}, {geo.dz}")
        self.log.info(f"Del size: {geo.du}, {geo.dv}")
        self.log.info(f"DSD: {geo.DSD}")
        self.log.info(f"DSO: {geo.DSO}")
        self.log.info(f"DDR: {geo.DDR}")
        self.log.info(f"DSR: {geo.DSR}")
        self.log.info(f"DAG: {geo.DAG}")
        self.log.info(f"nProj: {geo.nProj}")
        self.log.info(f"Tube Angle: {geo.tubeAngle}")
        self.log.info(f"Detector Angle: {geo.detAngle}")
        self.log.info(f"Offset: {geo.x_offset}, {geo.y_offset}")

        self.log.info(f"Reading Projections...")
        projs, _= self.readProjections()
        self.log.info(f"Shape Projections: {projs.shape}")

        self.log.info(f"Reconstructiong Images...")
        self.log.info(f"Reconstruction method: {self.filterType}")

        if self.filterType == 'FBP' or self.filterType == 'BP':
            vol = FBP(projs, geo, self.filterType, self.cutoff, libFiles)
            self.write_reconstructions(vol)

        elif self.filterType == 'SART':
            vol = SART(projs, geo, [self.nIter], libFiles)
            self.write_reconstructions(vol)

        elif self.filterType == 'SIRT':
            vol = SIRT(projs, geo, [self.nIter], libFiles)
            self.write_reconstructions(vol)

        else:
            self.log.error('Reconstruction method not recognized.')




        