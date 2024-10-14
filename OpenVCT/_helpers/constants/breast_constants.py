class BreastConfig:
    CUP_C = {
        "Subject": {
            "Subject_UID": "000.685169686552.20171031174553593",
            "Subject_Type": "Anthropomorphic",
            "Gender": "Female",
            "Pediatric": False,
            "Age": 47,
            "Date_Of_Birth": "1970-01-02",
            "Race": "null",
            "Ethnicity": "White",
            "Weight": 60.0,
            "Height": 1.65,
            "BMI": 22.03856749311295
        },
        "Phantom": {
            "Phantom_UID": "001.685169686552.20171031174553593",
            "Phantom_Source": "University of Pennsylvania",
            "Phantom_Type": "Voxel Array",
            "Data_Model": "Indexed",
            "Body_Part": "Breast",
            "Laterality": "Left",
            "Volume": 700,
            "Phantom_Name": "Phantom",
            "Phantom_Filename": "Phantom.vctx",  
            "Voxel_Size": 0.02  
        },
        "Organization": {
            "Name": "University of Pennsylvania",  
            "Division": "Perlman School of Medicine", 
            "Department": "Radiology",  
            "Group": "X-Ray Physics Lab",  
            "Institution_Code_Sequence": "1.2.826.0.1.3680043.2.936",  
            "Country": "United States",  
            "State": "Pennsylvania", 
            "City": "Philadelphia",  
            "Zip": "19104" 
        },
        "Index_Table_UID": "006.685169686552.20171031174553693",
        "Voxel_Array_UID": "007.685169686552.20171031174553763",
        "Leave_In_Temp_Folder": False,
        "Index_Table": {
        "Maximum_Index": 9,
        "Number_of_Indicies": 10,
        "Indices": [
            {
                "Index_ID": 0,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Air",
                        "Material_Weight": 1.000000,
                        "Material_Density": 0.001200,
                        "Material_MaterialZ": 201
                    }
                ]
            },
            {
                "Index_ID": 1,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Adipose",
                        "Material_Weight": 0.5,
                        "Material_Density": 0.930000,
                        "Material_MaterialZ": 204
                    },
                    {
                        "Material_Name": "Glandular",
                        "Material_Weight": 0.5,
                        "Material_Density": 1.0,
                        "Material_MaterialZ": 205
                    }
                ]
            },
            {
                "Index_ID": 2,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Skin",
                        "Material_Weight": 1.000000,
                        "Material_Density": 0.930000,
                        "Material_MaterialZ": 203
                    }
                ]
            },
            {
                "Index_ID": 3,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Adipose",
                        "Material_Weight": 1.000000,
                        "Material_Density": 0.930000,
                        "Material_MaterialZ": 204
                    }
                ]
            },
            {
                "Index_ID": 4,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Glandular",
                        "Material_Weight": 1.000000,
                        "Material_Density": 1.040000,
                        "Material_MaterialZ": 205
                    }
                ]
            },
            {
                "Index_ID": 5,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Calcium",
                        "Material_Weight": 1.000000,
                        "Material_Density": 1.55,
                        "Material_MaterialZ": 20
                    }
                ]
            },
            {
                "Index_ID": 6,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Glandular",
                        "Material_Weight": 1.000000,
                        "Material_Density": 0.8775,
                        "Material_MaterialZ": 205
                    }
                ]
            },
            {
                "Index_ID": 7,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Glandular",
                        "Material_Weight": 1.000000,
                        "Material_Density": 0.855,
                        "Material_MaterialZ": 205
                    }
                ]
            },
            {
                "Index_ID": 8,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Glandular",
                        "Material_Weight": 1.000000,
                        "Material_Density": 0.837,
                        "Material_MaterialZ": 205
                    }
                ]
            },
            {
                "Index_ID": 9,
                "Number_Of_Materials": 1,
                "Materials": [
                    {
                        "Material_Name": "Glandular",
                        "Material_Weight": 1.000000,
                        "Material_Density": 0.81,
                        "Material_MaterialZ": 205
                    }
                ]
            }
        ],
        "Data_Source": "NIST",
        "Generator_Config": {
            "ProgramName": "BreastPhantomGenerator",
            "XPLPhantom_UID": "888076.0.685169686552.20171031174553593",
            "Phantom_Name": "Phantom",
            "Phantom_Filename": "Phantom.vctx",
            "Phantom_Shape": "Anthropomorphic",
            "Max_Ligament_Thickness": 0.4,
            "Min_Ligament_Thickness": 0.2,
            "Voxel_Size": 0.2,
            "Size_X": 6.270616636,
            "Size_Y": 6.270616636,
            "Size_CPrime": 10.72938336,
            "Size_CSecondary": 6.270616636,
            "Random_Seed": 473,
            "Percent_Dense": 25,
            "Skin_Thickness": 0.15,
            "Ki": 282,
            "Kp": 48,
            "Ks": 91,
            "Num_Compartments": 421,
            "Compartment_Thickness": 0.04,
            "Min_Seed_Distance": 0.15,
            "Minimum_Speed": 0.01,
            "Maximum_Speed": 1.0,
            "Minimum_Ratio": 1.0,
            "Maximum_Ratio": 4.0,
            "Beta_p1": 2.0,
            "Beta_p2": 2.0,
            "Beta_q1": 0.5,
            "Beta_q2": 3.5
        }
    }
}