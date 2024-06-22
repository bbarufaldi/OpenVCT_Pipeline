import cupy as cp
import numpy as np
import os
import pathlib
import pydicom
import tqdm as tqdm
import tifffile

class ConeBeamCTBackProjection:
    def __init__(self, input_folder, output_folder, size, acquisition_geometry, pixel_size=0.1, slice_thickness=1.0, object_thickness=36.9,
                 dso = 700, dsd = 700):
        """
        Initialize the ConeBeamCTBackProjection class.

        Parameters:
        projections (ndarray): 3D array where each slice is a projection (shape: num_projections x height x width).
        acquisition_geometry (list): List of dictionaries containing acquisition parameters.
        pixel_size (float): Size of the pixels in the reconstructed image (in mm).
        slice_thickness (float): Thickness of each reconstructed slice (in mm).
        object_thickness (float): Thickness of the object being reconstructed (in mm).
        """
        self.input = input_folder
        self.output = output_folder
        self.size = size
        self.projections = self.read_projections(input_folder, size)
        self.projections = cp.array(self.projections, dtype=cp.float32)
        self.acquisition_geometry = acquisition_geometry
        self.num_projections, self.projection_height, self.projection_width = self.projections.shape
        self.pixel_size = pixel_size
        self.slice_thickness = slice_thickness
        self.num_slices = int(object_thickness / slice_thickness)
        self.image_width = int(self.projection_width * 0.14 / self.pixel_size)
        self.image_height = int(self.projection_height * 0.14 / self.pixel_size)
        self.reconstructed_image = cp.zeros((self.num_slices, self.image_height, self.image_width), dtype=cp.float32)
        self.dso = dso
        self.dsd = dsd
    
    def read_projections(self, dir2Read, imgSize):      
        dcmFiles =  list(pathlib.Path(dir2Read).glob('*.IMA')) +  list(pathlib.Path(dir2Read).glob('*.dcm'))    
        dcmImg = np.empty([len(dcmFiles),imgSize[0],imgSize[1]])
        
        if not dcmFiles:    
            raise ValueError('No DICOM files found in the specified path.')

        for dcm in dcmFiles:       
            dcmH = pydicom.dcmread(str(dcm))    
            idx = int(str(dcm).split('/')[-1].split('_')[-1].split('.')[0])  
            dcmImg[idx,:,:] = dcmH.pixel_array.astype('uint16')        
        return dcmImg

    def cbct_back_projection(self):
        """
        Perform filtered back projection to reconstruct the image.
        """
        ns, nt, na = self.projections.shape[1], self.projections.shape[2], len(self.acquisition_geometry)
        wx = (self.image_width - 1) / 2.0
        wy = (self.image_height - 1) / 2.0
        wz = (self.num_slices - 1) / 2.0

        xc = (cp.arange(self.image_width) - wx) * self.pixel_size
        yc = (cp.arange(self.image_height) - wy) * self.pixel_size
        xc, yc = cp.meshgrid(xc, yc)
        xc = xc.ravel()
        yc = yc.ravel()

        zc = (cp.arange(self.num_slices) - wz) * self.slice_thickness

        ws = (ns + 1) / 2.0
        wt = (nt + 1) / 2.0

        for idx in range(self.num_slices):
            
            img_slice = cp.zeros(self.image_width * self.image_height, dtype=cp.float32)

            #print(self.acquisition_geometry["Number_Acquisitions"])
            for jdx in range(self.acquisition_geometry["Number_Acquisitions"]):
                
                acquisition = self.acquisition_geometry['Acquisitions']['ID'== jdx]
                focal_spot = cp.array(acquisition["Focal_Spot"])
                orient_s = cp.array(acquisition["Detector"]["Orient_S_mm"])
                orient_t = cp.array(acquisition["Detector"]["Orient_T_mm"])

                x_beta = xc * orient_s[0] + yc * orient_s[1]
                y_beta = self.dso - (xc * orient_t[0] + yc * orient_t[1])

                mag = self.dsd / y_beta if self.dsd != cp.inf else 1.0

                sprime = mag * x_beta
                tprime = mag * (zc[idx] - focal_spot[2])

                bs = sprime / self.pixel_size + ws
                bt = tprime / self.slice_thickness + wt

                bs = cp.clip(bs, 1, ns)
                bt = cp.clip(bt, 1, nt)

                is_ = cp.floor(bs).astype(cp.int32)
                it = cp.floor(bt).astype(cp.int32)

                wr = bs - is_
                wl = 1.0 - wr
                wu = bt - it
                wd = 1.0 - wu

                is_ = cp.clip(is_, 1, ns)
                it = cp.clip(it, 1, nt - 1)

                p1 = wl * self.projections[jdx, is_ - 1, it - 1] + wr * self.projections[jdx, is_, it - 1]
                p2 = wl * self.projections[jdx, is_ - 1, it] + wr * self.projections[jdx, is_, it]
                p0 = wd * p1 + wu * p2

                if focal_spot[2] == 0: #dfs
                    p0 *= (self.dsd ** 2) / ((x_beta ** 2) + (y_beta ** 2))

                img_slice += p0

            self.reconstructed_image[:, :, idx] = cp.reshape(img_slice, (self.image_height, self.image_width))

        #return cp.asnumpy(self.reconstructed_image)

    def write_slices(self):

        if not os.path.exists(self.output):
            os.mkdir(self.output)

        #ds = pydicom.dcmread(self.input + '/_00.dcm') #template
        for i in tqdm(range(self.num_slices)):    
            # ds.Rows = self.image_size_width
            # ds.Columns = self.image_size_height
            # ds.BitsAllocated= 32
            # ds.BitsStored = 32
            # ds.HighBit = 31
            # ds.PixelData = cp.asnumpy(self.reconstructed_image[i]).astype(np.float32)
            # pydicom.write_file(self.output + '/_{:02d}.dcm'.format(i), ds)
            tifffile.imwrite(self.output + '/_{:02d}.tif'.format(i), cp.asnumpy(self.reconstructed_image[i]).astype(np.float32))

        #return cp.asnumpy(self.reconstructed_image)
        # Directory to save the original images with only the corrected intensity
