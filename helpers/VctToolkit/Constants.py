class SystemConfig:
    HOLOGIC = {
        "Projector_Config": {
            "Has_Noise": False,
            "Has_Invsq": True,
            "Elec_Noise": 3.0,
            "Gain": 0.2,
            "Prop_Const": 0.0038,
            "Random_Seed": 1
        },
        "Imaging_System": {
            "Name": "HOLOGIC"
        },
        "Exposure_Settings": {
            "Spectrum_ID": 2,
            "Anode_Material": "TUNGSTEN",
            "Filter": {
                "Material": "Aluminum",
                "MaterialZ": 13,
                "Thickness_mm": 0.7
            },
            "KVP": 31,
            "Exposure": 60,
            "Phantom_Thickness_mm": 60,
            "Density_Coefficient": 10
        },
        "Detector": {
            "Element_Size_mm": [0.14, 0.14],
            "Element_Count": [2048, 1664],
            "Thickness_mm": 0.1,
            "Material": 0,
            "Bits_Allocated": 16,
            "Bits_Stored": 14,
            "High_Bit": 13,
            "Has_Volume_Offset": True,
            "Volume_Offset_mm": [0.0, 0.0, -20.0]
        },
        "Acquisition_Geometry": {
            "Number_Acquisitions": 15,
            "Acquisitions": [
                {
                    "ID": 0,
                    "Acquisition_UID": "011.685169686552.20180203161823302",
                    "Focal_Spot": [91.247, 0.0, 694.027],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 1,
                    "Acquisition_UID": "011.685169686552.20180203161823356",
                    "Focal_Spot": [78.271, 0.0, 695.61],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 2,
                    "Acquisition_UID": "011.685169686552.20180203161823404",
                    "Focal_Spot": [65.268, 0.0, 696.951],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 3,
                    "Acquisition_UID": "011.685169686552.20180203161823449",
                    "Focal_Spot": [52.241, 0.0, 698.048],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 4,
                    "Acquisition_UID": "011.685169686552.20180203161823500",
                    "Focal_Spot": [39.197, 0.0, 698.902],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 5,
                    "Acquisition_UID": "011.685169686552.20180203161823553",
                    "Focal_Spot": [26.139, 0.0, 699.512],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 6,
                    "Acquisition_UID": "011.685169686552.20180203161823607",
                    "Focal_Spot": [13.072, 0.0, 699.878],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 7,
                    "Acquisition_UID": "011.685169686552.20180203161823658",
                    "Focal_Spot": [0.0, 0.0, 700.0],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 8,
                    "Acquisition_UID": "011.685169686552.20180203161823710",
                    "Focal_Spot": [-13.072, 0.0, 699.878],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 9,
                    "Acquisition_UID": "011.685169686552.20180203161823776",
                    "Focal_Spot": [-26.139, 0.0, 699.512],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 10,
                    "Acquisition_UID": "011.685169686552.20180203161823824",
                    "Focal_Spot": [-39.197, 0.0, 698.902],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 11,
                    "Acquisition_UID": "011.685169686552.20180203161823876",
                    "Focal_Spot": [-52.241, 0.0, 698.048],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 12,
                    "Acquisition_UID": "011.685169686552.20180203161823930",
                    "Focal_Spot": [-65.268, 0.0, 696.951],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 13,
                    "Acquisition_UID": "011.685169686552.20180203161823980",
                    "Focal_Spot": [-78.271, 0.0, 695.61],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                },
                {
                    "ID": 14,
                    "Acquisition_UID": "011.685169686552.20180203161824030",
                    "Focal_Spot": [-91.247, 0.0, 694.027],
                    "Detector": {
                        "Position_mm": [-143.36, 0.0, 0.0],
                        "Orient_S_mm": [1.0, 0.0, 0.0],
                        "Orient_T_mm": [0.0, 1.0, 0.0]
                    }
                }
            ]
        },
        "Noise_Config": {
            # "Parameters": "noise/Parameters_Hologic_DBT_quadratic.mat",
            # "Kernel": "noise/Kernel_Hologic_DBT.mat",
            # "Reference": "noise/Reference_Hologic_DBT_31_60.mat",
            "Parameters": "noise/param/HOLOGIC_3D.npz",
            "Kernel": "noise/kernel/HOLOGIC_3D",
            "Reference": "noise/full/HOLOGIC_3D",
            "Lambda": "noise/lambda/HOLOGIC_3D"
        }
    }
