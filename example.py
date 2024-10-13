'''
This code manages and conducts the steps of the Open Virtual Clinical Trial (VCT) pipeline.
Developed by Bruno Barufaldi (bruno.barufaldi@pennmedicine.upenn.edu) on 06/18/2024, this pipeline aims to streamline 
the workflow of virtual clinical trials through advanced computational techniques.

Special thanks to all members of the X-ray Physics Lab (XPL) at the University of Pennsylvania for their 
support and contributions to this project.
'''

# Import Constants
import constants.breast_constants as breast
import constants.system_constants as sys

# Import Helpers
import writers.xml.breast_generator as gen
import writers.xml.xray_generator as proj

# Import Models
import OpenVCT.noise.NoiseModel as noise

# Import Others
import subprocess
import os

# # 1) Change path to Generation
os.chdir('/app/OpenVCT/anatomy')

breast_xml = gen.XMLBreastGenerator(config=breast.BreastConfig.CUP_C, 
                                    phantom_name="PhantomC")

breast_xml.write_xml("./xml/phantomC.xml")
subprocess.call(["./BreastPhantomGenerator_docker", "-xml_input", "./xml/phantomC.xml"])

# 2) Change path to Deformation
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/deform')
subprocess.call(["xvfb-run", "-s", "-screen 0 800x600x24", "python3", "VolumeDeformer.py"])

# 3.1) Change path to Ray Tracing (Noiseless Model)
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/raytracing')
writeProjXML = proj.XMLProjector(config=sys.SystemConfig.HOLOGIC, 
                                 phantom_name="./../anatomy/vctx/phantomC.vctx", 
                                 folder_name="proj/phantomC-proj")

writeProjXML.write_xml("./xml/phantomC.xml")
subprocess.call(["./XPLProjectionSim_GPU_docker", "-xml_input", "./xml/phantomC.xml", "-v"])

# 3.2) Change path to Noise Model (Add Noise)
os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT')
noise = noise.NoiseModel(config=sys.SystemConfig.HOLOGIC, 
                        input_folder="raytracing/proj/phantomC-proj",
                        output_folder="noise/proj/phantomC-proj")
noise.add_noise()
