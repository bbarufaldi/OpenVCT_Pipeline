'''
This code manages and conducts the steps of the Open Virtual Clinical Trial (VCT) pipeline.
Developed by Bruno Barufaldi (bruno.barufaldi@pennmedicine.upenn.edu) on 06/18/2024, this pipeline aims to streamline 
the workflow of virtual clinical trials through advanced computational techniques.

Special thanks to all members of the X-ray Physics Lab (XPL) at the University of Pennsylvania for their 
support and contributions to this project.
'''

import os

# Import Constants
import constants.breast_constants as breast
import constants.deform_constants as deform
import constants.system_constants as system

# Import Helpers
import writers.xml.breast_generator as generation
import writers.xml.deformer as deformation
import writers.xml.inserter as insertion
import writers.xml.xray_tracing as projection

# Import Additional Models
import OpenVCT.noise.NoiseModel as noise

# 1) Change path to Generation
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/anatomy')

pipeline = generation.XMLWriter(config=breast.BreastConfig.CUP_C, 
                                phantom_name="PhantomC",
                                xml_file = "./xml/PhantomC.xml")

pipeline.generate_phantom()

# 2) Change path to Deformation
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/deform')

pipeline = deformation.XMLWriter(config=deform.DeformerConfig.CUPC_CC, 
                                 in_phantom = "../anatomy/vctx/PhantomC.vctx",
                                 out_phantom = "./vctx/PhantomC.vctx",
                                 xml_file = "./xml/PhantomC.xml")

pipeline.compress_phantom()

# 3) Change path to Lesion Inserter
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/inserter')

# Additional (optional) parameters available. Check constructor for details
pipeline = insertion.XMLWriter(in_phantom = "../deform/vctx/PhantomC.vctx",
                               out_phantom = "./vctx/PhantomC.vctx",
                               xml_file = "./xml/PhantomC.xml",
                               num_lesions = 5,
                               size_mm = [(25, 25, 25)],
                               db_dir='db/mass',
                               weight=0.3,
                               method=1) # 0 for voxel replacement, 1 for voxel additive (partial volume)

pipeline.insert_lesions()

# 4.1) Change path to Ray Tracing (Noiseless Model)
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/raytracing')

pipeline = projection.XMLWriter(config=system.SystemConfig.HOLOGIC, 
                                phantom_name="../inserter/vctx/PhantomC.vctx", # change path to deform to project phantom without lesions
                                folder_name="proj/PhantomC-proj",
                                xml_file = "./xml/PhantomC.xml")

pipeline.project()

# 4.2) Change path to Noise Model (Add Noise)
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT')

noise = noise.NoiseModel(config=system.SystemConfig.HOLOGIC, 
                        input_folder="./raytracing/proj/PhantomC-proj",
                        output_folder="./noise/proj/PhantomC-proj")
noise.add_noise()
