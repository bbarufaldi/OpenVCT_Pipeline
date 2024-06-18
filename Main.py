import pyfastnoisesimd as fns
import numpy as np
import os, random, io
from helpers.noise_functions import generate_tissues, generate_thresholded_tissues, rescale, pad_mask_to_match_shape, calculate_frequencies
import tifffile
import skimage
import json
import subprocess
import pydicom as dcm

import zipfile
import gzip
import xml.etree.ElementTree as ET, xml.etree.ElementInclude as EI
import shutil
from time import time

#Parse dic
def jsonKeys2int(x):
    if isinstance(x, dict):
        return {int(k) if k.isnumeric() else k: tuple(v) if isinstance(v, list) else v for k,v in x.items()}
    return x

# Iterate directory
dir_path = r'D:/Users/bbruno/Pittsburg/Phantoms_FDA/Fatty'

#flag for noise
flag = False
vxl_mm = 0.1 #Targeted resolution
fac = np.single(0.1/vxl_mm) #Convertion factor (original resolution = 0.1 mm)

print("Hello world")

# raws = []
# dims = []

# for path in os.listdir(dir_path):
#     for path2 in os.listdir(dir_path+"/"+path):
#         if path2.startswith('pc') and path2.endswith('_crop.raw.gz') and not path2.startswith('pcl'):
#             raws.append(dir_path+"/"+path+"/"+path2)
#             dims.append(dir_path+"/"+path+"/"+path2.replace('_crop.raw.gz', '_crop.mhd'))

# count=0    
# for file in raws:
#     start = time()
#     print("Starting volume gen")

#     #Decompress mask
#     with gzip.GzipFile(filename=file, mode='rb') as gz:
#         phantom = gz.read()
    
#     meta = open(dims[count], 'r')
#     lines = meta.readlines()
#     vxl_spc = lines[8].strip().split(" = ")[1].split(" ")
#     size = lines[9].strip().split(" = ")[1].split(" ")

#     vol = np.frombuffer(bytearray(phantom), dtype=np.uint8).reshape(int(size[2]), int(size[1]), int(size[0]))

#     #Create json
#     in_file = os.path.splitext(os.path.basename(file))[0].replace(".raw", ".tif")

#     if flag == True:
#         out_file = in_file.replace("pc","noise")
#     else:
#         out_file = in_file
    
#     #resize the mask and calculate frequencies
#     template_mask = vol
#     template_mask = skimage.transform.resize(template_mask,(np.ushort(template_mask.shape[0]*fac), np.ushort(template_mask.shape[1]*fac), np.ushort(template_mask.shape[2]*fac)),
#                                                 mode='edge', anti_aliasing=False, anti_aliasing_sigma=None, preserve_range=True, order=0)
    
#     freq = calculate_frequencies(template_mask.shape, 1.5) #lacunarity

#     #get noise parameters randomly
#     random_file = random.choice(os.listdir("n_fat"))
#     print(random_file)
#     encoding = io.text_encoding("n_fat/"+random_file)
#     with open(encoding, "r") as f:
#        lines = f.readlines()

#     dic = {
#             "template_mask_path": in_file,
#             "num_volumes": {"1": int(lines[0].split(sep=" ")[0]), 
#                             "3": int(lines[0].split(sep=" ")[1])},
#             "noise_type": "Simplex",
#             "octave_thresholds": {"1": [int(lines[1].split(sep=" ")[0]),  len(freq.keys())-int(lines[1].split(sep=" ")[1])], 
#                                   "3": [int(lines[1].split(sep=" ")[2]),  len(freq.keys())-int(lines[1].split(sep=" ")[3])]},
#             "lacunarity": 1.5,
#             "persistence": {"1": float(lines[2].split(sep=" ")[0]), 
#                             "3": float(lines[2].split(sep=" ")[1])},
#             "threads": 20,
#             "seed": None,
#             "min_values": {"1": [float(lines[3].split(sep=" ")[0]), float(lines[3].split(sep=" ")[1])], 
#                            "3": [float(lines[3].split(sep=" ")[2]), float(lines[3].split(sep=" ")[3])]},
#             "max_values": {"1": [float(lines[4].split(sep=" ")[0]), float(lines[4].split(sep=" ")[1])], 
#                            "3": [float(lines[4].split(sep=" ")[2]), float(lines[4].split(sep=" ")[3])]},
#             "layers": {"1": 1, "3": 1},
#             "output_file": out_file
#     }
    
#     # Serializing json
#     json_object = json.dumps(dic, indent=2)
    
#     # Writing to data.json
#     with open("data.json", "w") as outfile:
#         outfile.write(json_object)

#     with open("data.json", "r") as f:
#         generator_parameters = json.load(f, object_hook=jsonKeys2int)

#     print(in_file)

#     tifffile.imwrite(in_file, template_mask)   
    
#     #determine octaves
#     #freq = calculate_frequencies(template_mask.shape, generator_parameters["lacunarity"])
#     #generator_parameters["octave_thresholds"].update({1:[5, len(freq.keys())] , 3:[5, len(freq.keys())-1]})

#     #0 air, 1 adipose, 2 skin
#     template_mask = np.where(template_mask == 29, 3, template_mask) #"glandular"
#     template_mask = np.where(template_mask == 33, 4, template_mask) #"nipple"
#     template_mask = np.where(template_mask == 40, 5, template_mask) #"muscle"
#     template_mask = np.where(template_mask == 50, 6, template_mask) #"paddle"
#     template_mask = np.where(template_mask == 65, 7, template_mask) #"antiscatter_grid"
#     template_mask = np.where(template_mask == 66, 8, template_mask) #"detector"
#     template_mask = np.where(template_mask == 88, 9, template_mask) #"ligament"
#     template_mask = np.where(template_mask == 95, 10, template_mask) #"TDLU"
#     template_mask = np.where(template_mask == 125, 11, template_mask) #"duct"
#     template_mask = np.where(template_mask == 150, 12, template_mask) #"artery"
#     template_mask = np.where(template_mask == 225, 13, template_mask) #"vein"
#     template_mask = np.where(template_mask == 200, 14, template_mask) #"mass"
#     tifffile.imwrite('test.tif', template_mask)

#     if flag == True:
#         volume_size = int(max(template_mask.shape))
#         print(volume_size)
        
#         if generator_parameters["min_values"] == None or generator_parameters["max_values"] == None or generator_parameters["layers"] == None:
#             noise_tissues = generate_tissues(
#                     n_volumes=generator_parameters["num_volumes"],
#                     noise_type=fns.NoiseType.Simplex if generator_parameters["noise_type"] == "Simplex" else fns.NoiseType.Perlin,
#                     shape=[volume_size, volume_size, volume_size],
#                     octave_thresholds=generator_parameters["octave_thresholds"],
#                     lacunarity=generator_parameters["lacunarity"],
#                     persistence=generator_parameters["persistence"],
#                     threads=generator_parameters["threads"],
#                     seed=generator_parameters["seed"]
#             )

#         else:
#             noise_tissues = generate_thresholded_tissues(
#                         n_volumes=generator_parameters["num_volumes"],
#                         noise_type=fns.NoiseType.Simplex if generator_parameters["noise_type"] == "Simplex" else fns.NoiseType.Perlin,
#                         shape=[volume_size, volume_size, volume_size],
#                         octave_thresholds=generator_parameters["octave_thresholds"],
#                         lacunarity=generator_parameters["lacunarity"],
#                         persistence=generator_parameters["persistence"],
#                         threads=generator_parameters["threads"],
#                         seed=generator_parameters["seed"],
#                         min_values= generator_parameters["min_values"],
#                         max_values= generator_parameters["max_values"],
#                         layers= generator_parameters["layers"],
#             )
            
#         print('Noise done!')

#         for label in noise_tissues.keys():
#                 noise_tissues[label] = np.ushort(noise_tissues[label])
                
#                 if label == 1: #adipose
#                     noise_tissues[label] = np.where(noise_tissues[label] == 1, 16, 15) #change to 16, 15; or 3, 1
#                 if label == 3: #gland
#                     noise_tissues[label] = np.where(noise_tissues[label] == 1, 16, 15) #change to 16, 15; or 3, 1

#         #Crete Result Mask
#         result_volume = np.array(np.zeros_like(template_mask, dtype=np.uint8))

#         result_volume = template_mask
#         for label in noise_tissues.keys():
#                 label_mask = pad_mask_to_match_shape(template_mask, noise_tissues[label].shape)
#                 label_mask = np.where(label_mask == label, 1, 0)
#                 label_mask *= noise_tissues[label]

#                 #replace materials
#                 result_volume = np.where(result_volume == label, 
#                                         np.uint8(label_mask[:template_mask.shape[0], :template_mask.shape[1], :template_mask.shape[2]]), 
#                                         result_volume)

#                 tifffile.imwrite("tissue_"+str(label)+'.tif', np.uint8(label_mask[:template_mask.shape[0], :template_mask.shape[1], :template_mask.shape[2]]))
   
#     else:
#          result_volume = template_mask #no noise
    
#     #Save
#     path_dir = out_file.replace(".tif", "")
#     if os.path.exists(path_dir):
#             shutil.rmtree(path_dir)
#     os.mkdir(path_dir)

#     # result_volume = np.flip(result_volume, 2) #flip horizontally
#     tifffile.imwrite('result_volume.tif', result_volume)
#     result_volume.astype('uint8').tofile(path_dir+"/Phantom.dat")
        
#     #     #ij.IJ.saveAs(imp, "Raw Data", path_dir+"/Phantom.dat")
#     #     #result_volume = ij.py.from_java(imp)
#     #     #ij.IJ.run("Close All")

#     #Parse XML
#     tree = ET.parse('../temp/Phantom_FDA_FAT.xml')
#     root = tree.getroot()

#     values, counts = np.unique(result_volume, return_counts=True)

#     #Write XML
#     for child in root:
#             if child.tag == 'Phantom_Name':
#                 child.text = path_dir

#             elif child.tag == 'Total_Non_Air_Voxels':
#                 child.text = str(counts[0]+counts[1])

#             elif child.tag == 'Glandular_Count':
#                 child.text =str(counts[1])
            
#             elif child.tag == 'Simplex_Param':
#                 child.text = random_file

#             elif child.tag == 'Thickness_mm':
#                 child.find('X').text = str(template_mask.shape[2]*vxl_mm) 
#                 child.find('Y').text = str(template_mask.shape[1]*vxl_mm) 
#                 child.find('Z').text = str(template_mask.shape[0]*vxl_mm) 

#             elif child.tag == 'Voxel_Array':
#                 child.find('VOXEL_NUM').find('X').text = str(template_mask.shape[2])
#                 child.find('VOXEL_NUM').find('Y').text = str(template_mask.shape[1])
#                 child.find('VOXEL_NUM').find('Z').text = str(template_mask.shape[0])
#                 child.find('VOXEL_SIZE_MM').find('X').text = str(vxl_mm) 
#                 child.find('VOXEL_SIZE_MM').find('Y').text = str(vxl_mm) 
#                 child.find('VOXEL_SIZE_MM').find('Z').text = str(vxl_mm) 
                
#             elif child.tag == 'Deformation':
#                 child.find('Deformation_Mode').text = 'DEFORM_CC'
            
#     tree.write(path_dir+'/Phantom.xml')

#     #Copy additional files
#     path_subdir = path_dir+"/Private"
#     os.mkdir(path_subdir)
#     shutil.copy2('../temp/Private/XPL_AttenuationTable.xml', path_subdir+"/XPL_AttenuationTable.xml")

#     #zip
#     zf = zipfile.ZipFile(path_dir+".vctx", "w")
#     for dirname, subdirs, files in os.walk(path_dir):
#             zf.write(dirname)
#             for filename in files:
#                 zf.write(os.path.join(dirname, filename), compress_type=zipfile.ZIP_DEFLATED)
#     zf.close()

#     #delete temp files and dirs
#     shutil.rmtree(path_dir)
#     os.remove(in_file)

#     #Generate Projections
#     tree = ET.parse('../temp/Projections_FAT.xml')
#     root = tree.getroot()

#     dir_img = out_file.replace('.tif', '-proj')
#     for child in root:
#         if child.tag == 'Projector_Config':
#             child.find('Input_Phantom').text = out_file.replace('.tif', '.vctx')
#             child.find('Output_Folder').text = dir_img
    
#     tree.write('Projection.xml')
#     while os.path.isfile(dir_img+'/_0.dcm') == False: #ensure projector runs
#         subprocess.call(['XPLProjectionSim_GPU.exe', '-xml_input', "Projection.xml"])

#     #Generate Reconstructions
#     tree = ET.parse('../temp/Reconstruction.xml')
#     root = tree.getroot()

#     for child in root:
#         if child.tag == 'Reconstruction':
#             child.find('minDepth').text = str(0)
#             child.find('maxDepth').text = str(template_mask.shape[0]*vxl_mm)
#             child.find('incDepth').text = str(1)

#     tree.write('Reconstruction.xml')
#     subprocess.call(['XPLBrionaStandardScript.exe', 'f='+dir_img, '/e', '/r', '/l', '/xml=Reconstruction.xml'])

#     end = time()
#     print("Finished volume gen in: ", end-start)
#     count += 1

