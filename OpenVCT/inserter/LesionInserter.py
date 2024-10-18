import numpy as np
import random
import os
import argparse
from scipy.ndimage import zoom
import logging as log
import zipfile
from tifffile import imwrite

from readers.xml import Phantom as ph
from readers.xml import Inserter as ins

class LesionInserter:
    def __init__(self, reader):

        # Get phantom for lesion insertion
        self.Phantom = ph.Phantom(reader.Input_Phantom)
        self.VOIs = reader.VOIs
        self.Lesions = reader.Lesions
        #print(self.Lesions)

        log.basicConfig(filename='LesionInsertion.log', level=log.DEBUG,
                        format='%(asctime)s - %(levelname)s - %(message)s')
        self.log = log.getLogger()

    def read_lesion(self, zip_file):
        """       
        Arguments:
        zip_file -- path to the lesion zip file
        size_mm -- target size for lesion
        
        Returns:
        map -- lesion mask
        """
        with zipfile.ZipFile(zip_file, 'r') as zip_ref:

            file = [f for f in zip_ref.namelist() if f.endswith('.raw')][0]
            size = str(file).replace('.raw', '').split('_')[-1].split('x')

            with zip_ref.open(file) as f:
                map = f.read()
                map = np.frombuffer(bytearray(map), dtype=np.uint8).reshape(int(size[0]), int(size[1]), int(size[2]))
        
        return map

    def insertion_replacement(self, total_weight_replace = 0.99):
        """
        Insert lesions into the Phantom by updating only the voxels of the resized lesion mask.
        
        For each lesion in `self.Lesions`, this method will:
        - Read the binary mask of the lesion.
        - Resize the mask based on the bounding box described in `self.Lesions`.
        - Replace the corresponding phantom voxels with the new index.
        """

        # Get the current max index from the phantom's index table
        max_index = self.Phantom.index_table['Maximum_Index']

        # Loop over each lesion in the self.Lesions list
        for lesion in self.Lesions:
            # Increment the max index to create a new index for the lesion
            new_index = max_index + 1
            max_index = new_index  # Update the max index

            # Define the new materials for the lesion
            if ('cluster' or 'calc') in lesion['LesionName']:
                new_materials = [
                    {
                        "Material_Name": "Glandular",
                        "Material_Weight": 1 - total_weight_replace,
                        "Material_Density": 1.02,
                        "Material_MaterialZ": 205
                    },
                    {
                        "Material_Name": "Calcium",
                        "Material_Weight": total_weight_replace,
                        "Material_Density": 1.55,
                        "Material_MaterialZ": 20
                    }
                ]
            else:
                new_materials = [
                    {
                        "Material_Name": "Adipose",
                        "Material_Weight": 1 - total_weight_replace,
                        "Material_Density": 0.93,
                        "Material_MaterialZ": 204
                    },
                    {
                        "Material_Name": "Glandular",
                        "Material_Weight": total_weight_replace,
                        "Material_Density": 1.02,
                        "Material_MaterialZ": 205
                    }
                ]

            # Create the new index entry
            new_index_entry = {
                "Index_ID": new_index,
                "Number_Of_Materials": len(new_materials),
                "Materials": new_materials
            }

            # Append the new index to the index table
            self.Phantom.index_table['Indices'].append(new_index_entry)
            self.Phantom.index_table['Maximum_Index'] = max_index

            # Get the lesion's center and size (bounding box)
            center = np.array([lesion['Center_X'], lesion['Center_Y'], lesion['Center_Z']])
            size = np.array([lesion['Height'], lesion['Width'], lesion['Depth']])

            # Calculate the bounds for the lesion within the phantom
            min_bounds = np.floor(center - size / 2).astype(int)  # Starting voxel position
            max_bounds = np.ceil(center + size / 2).astype(int)  # Ending voxel position

            # Read the lesion mask from the zip file (assuming lesion['LesionName'] is the zip path)
            lesion_mask = self.read_lesion(lesion['LesionName'])

            # Resize the lesion mask to match the bounding box in the phantom
            lesion_shape = lesion_mask.shape
            target_shape = (max_bounds[2] - min_bounds[2], max_bounds[1] - min_bounds[1], max_bounds[0] - min_bounds[0])
            scaling_factors = np.array(target_shape) / np.array(lesion_shape)
            resized_lesion_mask = zoom(lesion_mask.astype(float), scaling_factors, order=1) > 0.5  # Resizing the lesion mask

            # Update only the voxels of the phantom that correspond to the resized lesion mask
            phantom_region = self.Phantom.voxel_data[min_bounds[0]:max_bounds[0],
                                                    min_bounds[1]:max_bounds[1],
                                                    min_bounds[2]:max_bounds[2]]

            # Replace voxels in the phantom where the resized lesion mask is True
            phantom_region[resized_lesion_mask] = new_index

            # Update the phantom's voxel data with the modified region
            self.Phantom.voxel_data[min_bounds[0]:max_bounds[0],
                                    min_bounds[1]:max_bounds[1],
                                    min_bounds[2]:max_bounds[2]] = phantom_region

        imwrite('phantom.tif', self.Phantom.voxel_data)
        # Log or return a message indicating completion of the insertion
        self.log.info("Lesion insertion and index table update completed.")

    def insertion_additive(self, total_weight_reduction = 0.1):
        """
        Insert lesions into the Phantom by updating voxels with a new index that combines
        current materials and the Glandular material, while maintaining the sum of weights = 1.0.
        
        This method ensures that indices are not repeated: if an index with the same material composition
        already exists, it will reuse that index instead of creating a new one.
        """
        current_max_index = self.Phantom.index_table['Maximum_Index']

        # Method to compare material compositions
        def find_existing_index(materials):
            for index_entry in self.Phantom.index_table['Indices']:
                existing_materials = index_entry['Materials']
                if len(existing_materials) == len(materials):
                    match = all(
                        any(
                            existing['Material_Name'] == new['Material_Name'] and
                            abs(existing['Material_Weight'] - new['Material_Weight']) < 1e-6  # Floating-point tolerance
                            for existing in existing_materials
                        ) for new in materials
                    )
                    if match:
                        return index_entry['Index_ID']
            return None

        # Loop over each lesion in the self.Lesions list
        for lesion in self.Lesions:
            # Get the lesion's center and size (bounding box)
            center = np.array([lesion['Center_X'], lesion['Center_Y'], lesion['Center_Z']])
            size = np.array([lesion['Height'], lesion['Width'], lesion['Depth']])

            # Calculate the bounds for the lesion within the phantom
            min_bounds = np.floor(center - size / 2).astype(int)  # Starting voxel position
            max_bounds = np.ceil(center + size / 2).astype(int)  # Ending voxel position

            # Read the lesion mask from the zip file
            lesion_mask = self.read_lesion(lesion['LesionName'])

            # Resize the lesion mask to match the bounding box in the phantom
            lesion_shape = lesion_mask.shape
            target_shape = (max_bounds[0] - min_bounds[0], max_bounds[1] - min_bounds[1], max_bounds[2] - min_bounds[2])
            scaling_factors = np.array(target_shape) / np.array(lesion_shape)
            resized_lesion_mask = zoom(lesion_mask.astype(float), scaling_factors, order=1) > 0.5  # Resizing the lesion mask

            # Get the affected region in the phantom
            phantom_region = self.Phantom.voxel_data[min_bounds[0]:max_bounds[0],
                                                    min_bounds[1]:max_bounds[1],
                                                    min_bounds[2]:max_bounds[2]]

            # Find the indices where the lesion will affect the phantom
            affected_voxels = np.where(resized_lesion_mask)

            # Iterate over affected voxels directly
            for idx in range(len(affected_voxels[0])):
                x = affected_voxels[0][idx]
                y = affected_voxels[1][idx]
                z = affected_voxels[2][idx]

                # Get the current index at the voxel (x, y, z)
                current_index = phantom_region[x, y, z]
                current_materials = self.Phantom.index_table['Indices'][current_index]['Materials']

                # Reduce the weight of the current materials by 0.1 (while keeping total weight = 1.0)
                updated_materials = []

                for material in current_materials:
                    reduced_weight = material['Material_Weight'] - total_weight_reduction / len(current_materials)
                    updated_materials.append({
                        "Material_Name": material['Material_Name'],
                        "Material_Weight": max(0, reduced_weight),  # Ensure weight doesn't go below 0
                        "Material_Density": material['Material_Density'],
                        "Material_MaterialZ": material['Material_MaterialZ']
                    })

                # Add Glandular material
                updated_materials.append({
                    "Material_Name": "Glandular",
                    "Material_Weight": total_weight_reduction,
                    "Material_Density": 1.02,
                    "Material_MaterialZ": 205
                })

                # Check if this material composition already exists
                existing_index = find_existing_index(updated_materials)
                if existing_index is not None:
                    new_index = existing_index  # Reuse the existing index
                else:
                    # Create a new index for this material composition
                    new_index = current_max_index + 1
                    current_max_index = new_index

                    # Create the new index entry
                    new_index_entry = {
                        "Index_ID": new_index,
                        "Number_Of_Materials": len(updated_materials),
                        "Materials": updated_materials
                    }

                    # Append the new index to the index table
                    self.Phantom.index_table['Indices'].append(new_index_entry)
                    self.Phantom.index_table['Maximum_Index'] = current_max_index

                # Update the phantom voxel data at this position with the new index
                phantom_region[x, y, z] = new_index

                # Update the phantom's voxel data with the modified region
                self.Phantom.voxel_data[min_bounds[0]:max_bounds[0],
                                        min_bounds[1]:max_bounds[1],
                                        min_bounds[2]:max_bounds[2]] = phantom_region

        # Log or return a message indicating completion of the insertion
        self.log.info("Additive lesion insertion with index reuse completed.")
        # print(self.Phantom.index_table)
        # imwrite('phantom.tif', self.Phantom.voxel_data)
        
if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Lesion Inserter XML Reader")
    parser.add_argument('xml_input', type=str, help='Path to the XML file to read')
    parser.add_argument('weight', type=str, help='Lesion weight')

    args = parser.parse_args()
    if args.xml_input == None:
        log.error('Missing XML file and XML creation failed.')
        exit

    else:
        weight = float(args.weight) if args.weight is not None else 0.5
        reader = ins.Inserter(args.xml_input)
        lesion_inserter = LesionInserter(reader)
        lesion_inserter.insertion_additive(total_weight_reduction = weight)

        output_phantom = lesion_inserter.Phantom.voxel_data # Get Phantom
        #output_phantom = np.transpose(lesion_inserter.Phantom.voxel_data, (0, 2, 1)) # Transpose for CC compression
        lesion_inserter.Phantom.write_vctx(output_phantom, reader)

       
