'''
This code manages and conducts the steps of the Open Virtual Clinical Trial (VCT) pipeline.
Developed by Bruno Barufaldi (bruno.barufaldi@pennmedicine.upenn.edu) on 06/18/2024, this pipeline aims to streamline 
the workflow of virtual clinical trials through advanced computational techniques.

Special thanks to all members of the X-ray Physics Lab (XPL) at the University of Pennsylvania for their 
support and contributions to this project.
'''
import helpers.VctToolkit.writers.xml.breast_generator as gen
import helpers.VctToolkit.constants.breast_constants as breast

import helpers.VctToolkit.writers.xml.xray_generator as proj
import helpers.VctToolkit.constants.system_constants as sys

import OpenVCT.noise.NoiseModel as noise

import subprocess
import os

#Change path and create results folders
#os.chdir('/app/OpenVCT/anatomy') #change pwd

if not os.path.exists('xml'):
    os.makedirs('xml')
if not os.path.exists('vctx'):
    os.makedirs('vctx')

# breast_xml = gen.XMLBreastGenerator(config=breast.BreastConfig.CUP_C, 
#                                     phantom_name="phantomC")

# breast_xml.write_xml("./xml/phantomC.xml")
# subprocess.call(["./BreastPhantomGenerator_docker", "-xml_input", "./xml/phantomC.xml"])

#os.chdir(os.environ['HOME'])
os.chdir('/app/OpenVCT/deform') #change pwd
#subprocess.call(["./BreastPhantomDeformer_docker", "-v"])
subprocess.call(["./BreastPhantomDeformer_docker", "-xml_input", "./xml/phantomC.xml"])

# os.chdir(os.environ['HOME'])
# os.chdir('/app/OpenVCT/raytracing') #change pwd
# writeProjXML = proj.XMLProjector(config=sys.SystemConfig.HOLOGIC, 
#                                  phantom_name="./../anatomy/vctx/phantomC.vctx", 
#                                  folder_name="proj/phantomC-proj")

# writeProjXML.write_xml("./xml/phantomC.xml")
# subprocess.call(["./XPLProjectionSim_GPU_docker", "-xml_input", "./xml/phantomC.xml"])

# os.chdir('../') #change pwd
# noise = noise.NoiseModel(config=sys.SystemConfig.HOLOGIC, 
#                         input_folder="raytracing/proj/phantomC-proj",
#                         output_folder="noise/proj/phantomC-proj")
# noise.add_noise()


# fbp = fbp.FilteredBackProjection(input_folder="noise/proj/noise_1867251184_crop-proj-60mAs-rlz1", 
#                                  output_folder="reconstruction/rec/noise_1867251184_crop-proj-60mAs-rlz1", 
#                                  size=[2048, 1664],
#                                  pixel_size=0.1, slice_thickness=1.0, object_thickness=36.9,
#                                  acquisition_geometry=config.SystemConfig.HOLOGIC["Acquisition_Geometry"])

# fbp.back_project()
# fbp.write_slices()

# cbp = cbp.ConeBeamCTBackProjection(input_folder="noise/proj/noise_1867251184_crop-proj-60mAs-rlz1", 
#                                    output_folder="reconstruction/rec/noise_1867251184_crop-proj-60mAs-rlz1", 
#                                    size=[2048, 1664],
#                                    acquisition_geometry=config.SystemConfig.HOLOGIC["Acquisition_Geometry"])
# cbp.cbct_back_projection()
# cbp.write_slices()