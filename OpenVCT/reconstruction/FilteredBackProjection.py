import numpy as np
import cupy as cp
import os
import pathlib
import pydicom
from tqdm import tqdm
import tifffile

class FilteredBackProjection:
        
    def __init__(self, input_folder, output_folder, size, pixel_size, slice_thickness, object_thickness, acquisition_geometry):
        """
        Initialize the FilteredBackProjection class.

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
        self.projections = cp.array(self.projections)
        self.acquisition_geometry = acquisition_geometry
        self.num_projections, self.projection_height, self.projection_width = self.projections.shape #inverted
        
        self.pixel_size = pixel_size
        self.slice_thickness = slice_thickness
        self.num_slices = int(object_thickness / slice_thickness)
        self.image_size_width, self.image_size_height = [int(self.projection_width * 0.14 / self.pixel_size), int(self.projection_height * 0.14 / self.pixel_size)]
        self.reconstructed_image = cp.zeros((self.num_slices, self.image_size_width, self.image_size_height), dtype=cp.float32)
        self.filter = self._ramp_filter()

    def read_projections(self, dir2Read, imgSize):      
        """
        Read dicom files (projections).

        Parameters:
        dir2Read: directory to read dcm images (shape: height x width).
        imgSize: projection size.
        """

        dcmFiles =  list(pathlib.Path(dir2Read).glob('*.IMA')) +  list(pathlib.Path(dir2Read).glob('*.dcm'))    
        dcmImg = np.empty([len(dcmFiles),imgSize[0],imgSize[1]])
        
        if not dcmFiles:    
            raise ValueError('No DICOM files found in the specified path.')

        for dcm in dcmFiles:       
            dcmH = pydicom.dcmread(str(dcm))    
            ind = int(str(dcm).split('/')[-1].split('_')[-1].split('.')[0])  
            dcmImg[ind,:,:] = dcmH.pixel_array.astype('uint16')        
        return dcmImg

    def _ramp_filter(self):
        """
        Create a ramp filter for the filtered back projection.
        """
        n = cp.fft.rfftfreq(self.projection_width).astype(cp.float64)
        filter = n * (2.0 * cp.pi)
        return filter

    def _apply_filter(self, projection):
        """
        Apply the ramp filter to a projection using FFT.
        """
        projection_fft = cp.fft.rfft(projection, axis=-1)
        projection_filtered = cp.fft.irfft(projection_fft * self.filter, axis=-1)
        return projection_filtered
    
    def fdk_weight(self, projection, dso, dsd, offset_s, offset_t):
        """
        Apply Feldkamp-Davis-Kress (FDK) weighting to the projection data.
        """
        ns, nt = projection.shape
        ss = cp.arange(ns) - (ns / 2.0) + offset_s
        tt = cp.arange(nt) - (nt / 2.0) + offset_t
        ss, tt = cp.meshgrid(ss, tt)
        ss = ss.astype(cp.float32)
        tt = tt.astype(cp.float32)

        ww = (dso / cp.sqrt(dsd**2 + ss**2 + tt**2)).astype(cp.float32)
        projection *= ww

    def back_project(self):
        """
        Perform filtered back projection to reconstruct the image.
        """
        x_center = self.image_size_width // 2
        y_center = self.image_size_height // 2

        z_positions = cp.arange(self.num_slices) * self.slice_thickness

        for i in tqdm(range(self.num_projections)):
            
            acquisition = self.acquisition_geometry['Acquisitions']['ID'== i]
            projection = self.projections[i]

            # Apply the FDK weighting
            #self.fdk_weight(projection, dso, dsd, acquisition['offset_s'], acquisition['offset_t'])

            # Apply the filter to the projection
            #filtered_projection = self._apply_filter(projection)
            filtered_projection = projection
            #tifffile.imwrite('projection.tiff', cp.asnumpy(projection))
            
            focal_spot = cp.array(acquisition['Focal_Spot'])
            detector_position = cp.array(acquisition['Detector']['Position_mm'])
            #detector_position = cp.array([0, 0, 0])
            orient_s = cp.array(acquisition['Detector']['Orient_S_mm'])
            orient_t = cp.array(acquisition['Detector']['Orient_T_mm'])
            
            # Create coordinate grids
            x = cp.arange(self.projection_width) - (self.projection_width // 2)
            y = cp.arange(self.projection_height) - (self.projection_height // 2)
            x_grid, y_grid = cp.meshgrid(x, y)
            
            # Calculate pixel positions
            pixel_position = detector_position[:, None, None] + x_grid * orient_s[:, None, None] + y_grid * orient_t[:, None, None]
            relative_position = pixel_position - focal_spot[:, None, None]
            distance = cp.linalg.norm(relative_position, axis=0)
            scale_factor = self.projection_width * 0.14 / distance  # using detector element size

            x_rot = (relative_position[0] * scale_factor / self.pixel_size).astype(int) + x_center
            y_rot = (relative_position[1] * scale_factor / self.pixel_size).astype(int) + y_center
            
            # Mask to filter out-of-bounds indices
            mask = (x_rot >= 0) & (x_rot < self.image_size_height) & (y_rot >= 0) & (y_rot < self.image_size_width)

            #print("mask.shape: ", mask.shape)
            #print("reconstructed_image.shape: ", self.reconstructed_image.shape)
            #print("projection.shape: ", projection.shape)
            for z_idx, z_pos in enumerate(z_positions):
                z_rot = ((relative_position[2] + z_pos) * scale_factor / self.pixel_size).astype(int)
                slice_mask = mask & (z_rot >= 0) & (z_rot < self.image_size_height)
                self.reconstructed_image[z_idx, y_rot[slice_mask], x_rot[slice_mask]] += filtered_projection[slice_mask]

        return cp.asnumpy(self.reconstructed_image)
    
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
        