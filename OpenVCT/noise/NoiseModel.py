"""
This code snippet demonstrates how to add a noise model from clinical mammography to the noise.Model class. 
The noise model is provided by LAVI (Laboratory of Computer Vision) and is designed to be integrated into 
clinical systems for enhanced image analysis. 

Steps:
1. Import necessary libraries and dependencies.
2. Define the noise.Model class, including its attributes and methods.
3. Implement a method to load noise models from clinical data.
4. Integrate the noise model into the image analysis workflow.

Key Functions:
- addNoise: Applies the loaded noise model to the images for correction.

Edited by Bruno Barufaldi (bruno.barufaldi@pennmedicine.upenn.edu).
Credits to Arthur Costa, Rennan Brandao, Rodrigo Vimieiro, Lucas Borges, and Marcelo Costa from LAVI 
for providing the scatter models.
"""

import numpy as np
import pathlib
import pydicom
import os
from scipy.io import loadmat
from skimage.filters import threshold_otsu
from scipy.signal import fftconvolve
from tqdm import tqdm

class NoiseModel:
    def __init__(self, config, input_folder, output_folder):
        self.config = config
        self.input_folder = input_folder
        self.output_folder = output_folder
    
    def read_dcm_image(self, dir2Read, imgSize, index):
        file =  dir2Read+"/_"+str(index)+".dcm" 

        if not os.path.exists(file):
            raise ValueError(file + ' does not exist.')

        dcmH = pydicom.dcmread(dir2Read+"/_"+str(index)+".dcm")    
        dcmImg = dcmH.pixel_array.astype('uint16')        
        return dcmImg

    def add_noise(self):

        nrlz = 1 # Number of realizations of the noisy image
        
        kvp = self.config["Exposure_Settings"]["KVP"]
        fullDosemAs = self.config["Exposure_Settings"]["Exposure"]
        
        dose2gen = 100      # % of the full-dose to be used as the base dose for the generated noise and intensity correction
        dose2genprc = dose2gen / 100

        # Load noise parameters
        parameters = np.load(self.config["Noise_Config"]["Parameters"])
        fullDose = np.load(self.config["Noise_Config"]["Reference"]+"_"+str(kvp)+"_"+str(fullDosemAs)+".npz")

        sigma_e = parameters['sigma_e']
        tau = parameters['tau']
        gama = parameters['gama']

        # Reference
        fullDoseMin = fullDose['min']
        fullDoseMax = fullDose['max']
        fullDoseMean = fullDose['mean']

        nProjs = self.config["Acquisition_Geometry"]["Number_Acquisitions"]
        nRows = self.config["Detector"]["Element_Count"][0]
        nCols = self.config["Detector"]["Element_Count"][1]

        folder_name = self.input_folder         
        
        for n in range(1,nrlz+1):           
            # Directory to save the noisy images with the corrected intensity
            folder_name_noisy = self.output_folder + '-{}mAs-rlz{}'.format(int(fullDosemAs*dose2genprc), n)
            if not os.path.exists(folder_name_noisy):
                os.mkdir(folder_name_noisy)
            
            # Directory to save the original images with only the corrected intensity
            folder_name_gt = self.output_folder  + '-CM'
            if not os.path.exists(folder_name_gt):
                os.mkdir(folder_name_gt)
            
            for z in tqdm(range(nProjs)):

                vct_img = self.read_dcm_image(folder_name, (nRows, nCols), z) - tau # Subtracting tau              
                vct_mask = vct_img < threshold_otsu(vct_img) #6000
                
                # Normalizing the projection based on the fullDose
                vct_img_min = vct_img[vct_mask].min()
                vct_img_max = vct_img[vct_mask].max()
                vct_img_mean = vct_img[vct_mask].mean()
                temp = (vct_img-vct_img_mean)/(vct_img_max-vct_img_min)
            
                # Normalizing based on the overall average min and max of the projections
                vct_img = (fullDoseMax[:,z] - fullDoseMin[:,z])*temp + fullDoseMean[:,z]
                
                # Correcting signal
                vct_img = vct_img * dose2genprc

                #lambda_e was calculated for R Breast. To deal with L breasts use: lambda_e = np.flip(lambda_e,1)
                ke = np.load(self.config["Noise_Config"]["Kernel"]+"_"+str(z)+".npz")["arr"]
                lambda_e = np.load(self.config["Noise_Config"]["Lambda"]+"_"+str(z)+".npz")["arr"] 
                
                # Poisson noise
                poissonNoise = np.sqrt(lambda_e * vct_img) * fftconvolve(np.random.normal(size=(nRows,nCols)), ke, mode='same')
                
                # Electronic noise
                electronicNoise = np.sqrt(sigma_e**2) * np.random.normal(size=(nRows,nCols))
                
                # Structural noise
                structuralNoise = np.sqrt(gama**2 * vct_img**2) * np.random.normal(size=(nRows,nCols))
                
                vct_gen = vct_img + electronicNoise + poissonNoise + structuralNoise + tau # Adding tau after performing noise addition
                
                vct_gen = vct_gen.astype(np.uint16)
            
                ds = pydicom.dcmread(folder_name + '/_{}.dcm'.format(z))
                ds.PixelData = vct_gen.tobytes()
                ds.SeriesNumber = np.random.randint(1000000)
                ds.InstanceNumber = z
            
                pydicom.dcmwrite(folder_name_noisy + '/_{:02d}.dcm'.format(z), ds)
                
                # Save noiseless version with corrected mean value
                if n == 1 and dose2gen == 100:
                    
                    vct_gen = (vct_img/ dose2genprc) + tau
                    
                    vct_gen = vct_gen.astype(np.uint16)
            
                    ds.PixelData = vct_gen.tobytes() 
                    ds.SeriesNumber = np.random.randint(1000000)
                
                    pydicom.dcmwrite(folder_name_gt + '/_{:02d}.dcm'.format(z), ds)
        



