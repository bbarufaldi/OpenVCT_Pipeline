'''
This code manages and conducts the steps of the Open Virtual Clinical Trial (VCT) pipeline.
Developed by Bruno Barufaldi (bruno.barufaldi@pennmedicine.upenn.edu) on 06/18/2024, this pipeline aims to streamline 
the workflow of virtual clinical trials through advanced computational techniques.

Special thanks to all members of the X-ray Physics Lab (XPL) at the University of Pennsylvania for their 
support and contributions to this project.
'''

# Import Constants
import constants.breast_constants as breast
import constants.deform_constants as deform
import constants.system_constants as system

# Import Helpers
import writers.xml.breast_generator as gen
import writers.xml.deformer as defo
import writers.xml.inserter as ins
import writers.xml.xray_tracing as proj

# Import Models
import OpenVCT.noise.NoiseModel as noise

# Import Others
import subprocess
import os

# 1) Change path to Generation
os.chdir('/app/OpenVCT/anatomy')

xml = gen.XMLWriter(config=breast.BreastConfig.CUP_C, 
                    phantom_name="PhantomC",
                    xml_file = "./xml/PhantomC.xml")

subprocess.call(["./BreastPhantomGenerator_docker", "-xml_input", xml.xml_file])

# 2) Change path to Deformation
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/deform')

xml = defo.XMLWriter(config=deform.DeformerConfig.CUPC_CC, 
                     in_phantom = "../anatomy/vctx/PhantomC.vctx",
                     out_phantom = "./vctx/PhantomC.vctx",
                     xml_file = "./xml/PhantomC.xml")

subprocess.call(["xvfb-run", "-s", "-screen 0 800x600x24", "python3", "VolumeDeformer.py", xml.xml_file])

# 3) Change path to Lesion Inserter
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/inserter')

# Additional (optional) parameters available. Check constructor for details
xml = ins.Inserter(in_phantom = "../deform/vctx/PhantomC.vctx",
                   out_phantom = "./vctx/PhantomC.vctx",
                   xml_file = "./xml/PhantomC.xml",
                   num_lesions = 5,
                   size_mm = [(25, 25, 25)],
                   db_dir='db/mass'
                   ) 

subprocess.call(["python3", "./LesionInserter.py", xml.xml_file, '0.3']) # optional argument for weight of lesion

# 4.1) Change path to Ray Tracing (Noiseless Model)
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/raytracing')

xml = proj.XMLWriter(config=system.SystemConfig.HOLOGIC, 
                     phantom_name="../inserter/vctx/PhantomC.vctx", # change path to deform to project phantom without lesions
                     folder_name="proj/PhantomC-proj",
                     xml_file = "./xml/PhantomC.xml"
                     )

subprocess.call(["./XPLProjectionSim_GPU_docker", "-xml_input", xml.xml_file])

# 4.2) Change path to Noise Model (Add Noise)
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT')

noise = noise.NoiseModel(config=system.SystemConfig.HOLOGIC, 
                        input_folder="raytracing/proj/phantomC-proj",
                        output_folder="noise/proj/phantomC-proj")
noise.add_noise()
