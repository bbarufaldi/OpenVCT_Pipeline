'''
This code manages and conducts the steps of the Open Virtual Clinical Trial (VCT) pipeline.
Developed by Bruno Barufaldi (bruno.barufaldi@pennmedicine.upenn.edu) on 06/18/2024, this pipeline aims to streamline 
the workflow of clinical trials through advanced computational techniques.

Special thanks to all members of the X-ray Physics Lab (XPL) at the University of Pennsylvania for their 
support and contributions to this project.
'''

import helpers.VctToolkit.writers.xml.Projector as proj
import helpers.VctToolkit.Constants as config
import subprocess
import OpenVCT.noise.NoiseModel as noise
import os

os.chdir('./OpenVCT/raytracing') #change pwd

writeProjXML = proj.XMLProjector(config.SystemConfig.HOLOGIC, 
                                 "vctx/noise_1867251184_crop.vctx", 
                                 "proj/noise_1867251184_crop-proj")

writeProjXML.write_xml("./xml/noise_1867251184.xml")
subprocess.call(["./XPLProjectionSim_GPU_docker", "-xml_input", "./xml/noise_1867251184.xml"])

os.chdir('../') #change pwd
noise = noise.NoiseModel(config.SystemConfig.HOLOGIC, 
                        "raytracing/proj/noise_1867251184_crop-proj",
                        "noise/proj/noise_1867251184_crop-proj")
noise.addNoise()