// ReadPhantom.cpp


#include "ReadPhantom.h"
#include "Acquisition.h"
#include "Machine.h"
#include "LabelMap.h"

#include "XPL_BreastPhantomDeformer.h"
#include "VCT_PrivateData.h"
#include "VCT_Deformation.h"
#include "VCT_CommonLibrary.h"
#include "VCT_Time.h"
#include "VCT_Date.h"
#include "VCT_Organization.h"
#include "VCT_VoxelArray.h"
#include "VCT_IndexTable.h"
#include "VCT_Subject.h"
#include "VCT_Trial.h"
#include "VCT_Phantom.h"
#include "VCT_Vctx.h"

#include <string>
#include <fstream>
#include <map>
#include <ctime>



bool AcqGeom::readInputXML(vct::CommonLibrary &clib, vct::Logger &logger)
{
    bool retval = false;
    
    clib.getNodeValueXML("ID", id);
    clib.getNodeValueXML("Acquisition_UID", uid);

    // Read Focal Spot
    retval = clib.findNodeXML("Focal_Spot");
    if (retval)
    {
        clib.getNodeValueXML("X_mm", focal_spot_x_mm);
        clib.getNodeValueXML("Y_mm", focal_spot_y_mm);
        clib.getNodeValueXML("Z_mm", focal_spot_z_mm);
        clib.concludeSectionXML();
    }
                    
    // Read Phantom Offset                                          //BB and TV 04/10/2018
    retval = clib.findNodeXML("Phantom_Offset");
    if (retval)
    {
        clib.getNodeValueXML("X_mm", ph_offset_x_mm);
        clib.getNodeValueXML("Y_mm", ph_offset_y_mm);
        clib.getNodeValueXML("Z_mm", ph_offset_z_mm);
        clib.concludeSectionXML();
    }
                    
    // Read Detector
    retval = clib.findNodeXML("Detector");
    if (retval)
    {
        clib.getNodeValueXML("X_mm", det_pos_x_mm);
        clib.getNodeValueXML("Y_mm", det_pos_y_mm);
        clib.getNodeValueXML("Z_mm", det_pos_z_mm);

        clib.getNodeValueXML("Orient_SX_mm", Svec_ori_x);
        clib.getNodeValueXML("Orient_SY_mm", Svec_ori_y);
        clib.getNodeValueXML("Orient_SZ_mm", Svec_ori_z);

        clib.getNodeValueXML("Orient_TX_mm", Tvec_ori_x);
        clib.getNodeValueXML("Orient_TY_mm", Tvec_ori_y);
        clib.getNodeValueXML("Orient_TZ_mm", Tvec_ori_z);
    }

    logger.info << "\t     Acquisition :\n";
    logger.info << "\t         ID:           " << id << "\n";
    logger.info << "\t         Acq UID:      " << uid << "\n";
    logger.info << "\t         Focal Spot x: " << focal_spot_x_mm << "\n";
    logger.info << "\t         Focal Spot y: " << focal_spot_y_mm << "\n";
    logger.info << "\t         Focal Spot z: " << focal_spot_z_mm << "\n";
    /*
    logger.info << "\t         Ph  Offset x: " << ph_offset_x_mm << "\n";
    logger.info << "\t         Ph  Offset y: " << ph_offset_y_mm << "\n";
    logger.info << "\t         Ph  Offset z: " << ph_offset_z_mm << "\n";
    */
    logger.info << "\t         Det Origin x: " << det_pos_x_mm << "\n";
    logger.info << "\t         Det Origin y: " << det_pos_y_mm << "\n";
    logger.info << "\t         Det Origin z: " << det_pos_z_mm << "\n";
    logger.info << "\t         Det SVectorX: " << Svec_ori_x << "\n";
    logger.info << "\t         Det SVectorY: " << Svec_ori_y << "\n";
    logger.info << "\t         Det SVectorZ: " << Svec_ori_z << "\n";
    logger.info << "\t         Det TVectorX: " << Tvec_ori_x << "\n";
    logger.info << "\t         Det TVectorY: " << Tvec_ori_y << "\n";
    logger.info << "\t         Det TVectorZ: " << Tvec_ori_z << "\n";

    return retval;
}





ReadPhantom &ReadPhantom::getInstance()
{
    // This is a (Scott) Meyers Singleton
    static ReadPhantom instance;
    return instance;
}


ReadPhantom::ReadPhantom()
        : phantom_name_in(),
          input_phantom_filename(),
          full_def_phantom_path(),
          voxels(nullptr),
          dim_x(0), dim_y(0), dim_z(0),
          vsiz_x(0.0f), vsiz_y(0.0f), vsiz_z(0.0f),
          seqnum(0), xml_input_used(false),
          program_name("XPLProjectionSim_GPU"),
          sw_version("1.0"),
          index_table_uid("unknown_UID"),
          voxel_array_uid("unknown_UID"),
          imaging_system_name("hologic"),
          org_name("unknown_org"),
          org_div("unknown_div"),
          org_dept("unknown_dept"),
          org_grp("unknown_group"),
          org_code("unknown_code"),
          org_cntry("unknown_country"),
          org_sta("unknown_state"),
          org_city("unknown_city"),
          org_zip("unknown_zip"),
          utc_offset(0),

          use_noise(true),
          use_scaling(false),
          use_rand_quad(false),
          use_inv_sq(true),

          elec_set(false),
          gain_set(false),
          mas_set(false),
          prop_set(false),
          mingray_set(false),

          elec_noise_value(0.0),
          gain_value(0.0),
          mas_value(0.0),
          prop_const(0.0),
          mingray_value(0u),

          // XRayTube values
          spectrumID(2u),
          kVP (32u),
          anode_material_name(""),
          filter_material_name(""),
          filterMaterialZ(42u),
          filterThickness_mm(0.05),
          phantom_thickness_mm(0.0),
          density_coefficient(0.0),

          // Detector values (init to hologic)
          elementCountX(2048u),
          elementCountY(1792u),
          elementsizeX_mm(0.14), // pixel size in mm
          elementsizeY_mm(0.14), // pixel size in mm
          det_thickness_mm(0.1),
          det_material(0),
          allocatedbits(16),
          storedbits(14),
          highbit(13),
          useXYVolumeOffsets(false), // pertains to X and Y only
          volumeOffsetX_mm(0.0), 
          volumeOffsetY_mm(0.0),
          volumeOffsetZ_mm(-0.25),

          // Acquisitions
          number_of_acquisitions(0)
{
    // Start portable timer
    _starttime = std::clock(); /////DH 2020-07-20  ftime(&_starttime);
}


ReadPhantom::~ReadPhantom()
{
}


void ReadPhantom::readConfigFile()
{
    vct::CommonLibrary clib;
    bool status = clib.loadXML("vct_config.xml");
    if (status)
    {
        status = clib.findNodeXML("vct_config");
        if (status)
        {
            status = clib.findNodeXML("Organization");
            if (status)
            {
                clib.getNodeValueXML("Organization_Name", org_name);
                clib.getNodeValueXML("Division",          org_div);
                clib.getNodeValueXML("Department",        org_dept);
                clib.getNodeValueXML("Group",             org_grp);
                clib.getNodeValueXML("Institution_Code_Sequence", org_code);
                clib.getNodeValueXML("Country",           org_cntry);
                clib.getNodeValueXML("State",             org_sta);
                clib.getNodeValueXML("City",              org_city);
                clib.getNodeValueXML("ZipCode",           org_zip);
                clib.getNodeValueXML("UTC_Offset",        utc_offset);
                clib.concludeSectionXML();
            }
        }
    }
}


void ReadPhantom::setCmdLineArgs(std::string atten, std::string config, std::string ph_in,
                                 std::string comment, std::string seed)
{

    attenuationFile = atten;
    configGeomFile = config;
    input_phantom_filename = ph_in;
    commentString = comment;
    rand_seed = seed;    

    conditionPhantomFilename();
}


bool ReadPhantom::readInputXML(vct::Logger &logger, std::string xmlfile)
{
    bool retval = false;

    logger.info << "Input XML File is " << xmlfile << "\n";

    // Attempt to open the file with pugi
    vct::CommonLibrary clib;
    retval = clib.loadXML(xmlfile);
    if (retval)
    {
        retval = clib.findNodeXML("Breast_Phantom_Designer");
        if (retval)
        {
            // Read Projector Config Section
            retval = clib.findNodeXML("Projector_Config");
            if (retval)
            {
                clib.getNodeValueXML("Program_Name", program_name);
                clib.getNodeValueXML("Software_Version", sw_version);
                clib.getNodeValueXML("Input_Phantom", input_phantom_filename);                
                clib.getNodeValueXML("Output_Folder", output_folder);

                if (clib.getNodeValueXML("Has_Noise", opt_noise))
                {
                    // This test will set use_noise to boolean true or false
                    use_noise = (opt_noise == "True" || opt_noise == "true");
                }

                if (clib.getNodeValueXML("Has_Invsq", opt_invsq))
                {
                    // This test will set use_inv_sq to boolean true or false
                    use_inv_sq = (opt_invsq == "True" || opt_invsq == "true");
                }

                if (clib.getNodeValueXML("Elec_Noise", elec_noise_value))
                {
                    elec_set = true;
                    opt_elec = std::to_string(elec_noise_value);
                }

                if (clib.getNodeValueXML("Gain", gain_value))
                {
                    gain_set = true;
                    opt_gain = std::to_string(gain_value);
                }

                if (clib.getNodeValueXML("Prop_Const", prop_const))
                {
                    prop_set = true;
                    opt_propconst = std::to_string(prop_const);
                }
                
                clib.getNodeValueXML("Random_Seed", rand_seed);
                
                clib.concludeSectionXML();

                logger.info << "From XML input file:\n";
                logger.info << "Projector_Config\n";
                logger.info << "\t Program_Name:         " << program_name << "\n";
                logger.info << "\t Software_Version:     " << sw_version << "\n";
                logger.info << "\t Input_Phantom:        " << input_phantom_filename << "\n";
                logger.info << "\t Output_Folder:        " << output_folder << "\n";
                logger.info << "\t Has_Noise:            " << (use_noise?"true":"false") << "\n";
                logger.info << "\t Has_Invsq:            " << (use_inv_sq?"true":"false") << "\n";
                logger.info << "\t Elec_Noise:           " << elec_noise_value << "\n";                
                logger.info << "\t Gain:                 " << gain_value << "\n";
                logger.info << "\t Prop_Const:           " << prop_const << "\n";
                logger.info << "\t Random_Seed:          " << rand_seed       << "\n";
            }
            else logger.error << "Could not find node \"Projector_Config\"\n";


            // Read Imaging System Section
            retval = clib.findNodeXML("Imaging_System");
            if (retval)
            {
                clib.getNodeValueXML("Name", imaging_system_name);

                clib.concludeSectionXML();
            }
            else logger.error << "Could not find node \"Imaging_System\"\n";
                
            // Read Exposure Settings Section
            retval = clib.findNodeXML("Exposure_Settings");
            if (retval)
            {
                clib.getNodeValueXML("Spectrum_ID", spectrumID);
                clib.getNodeValueXML("Anode_Material", anode_material_name);

                // Read Filter Section
                retval = clib.findNodeXML("Filter");
                if (retval)
                {
                    clib.getNodeValueXML("Material", filter_material_name);
                    clib.getNodeValueXML("MaterialZ", filterMaterialZ);                 
                    clib.getNodeValueXML("Thickness_mm", filterThickness_mm);
                    clib.concludeSectionXML();
                }
                else logger.error << "Could not find node \"Filter\"\n";

                clib.getNodeValueXML("KVP", kVP);

                if (clib.getNodeValueXML("Exposure", mas_value))
                {
                    mas_set = true;
                    opt_mAs = std::to_string(mas_value);
                }
                else logger.error << "Could not find node \"Exposure\"\n";

                clib.getNodeValueXML("Phantom_Thickness_mm", phantom_thickness_mm);
                clib.getNodeValueXML("Density_Coefficient", density_coefficient);
                clib.concludeSectionXML();

                logger.info << "Exposure_Settings\n";
                logger.info << "\t Spectrum_ID:          " << spectrumID << "\n";
                logger.info << "\t Anode_Material:       " << anode_material_name << "\n";
                logger.info << "\t Filter:\n";
                logger.info << "\t     Material:         " << filter_material_name << "\n";
                logger.info << "\t     MaterialZ:        " << filterMaterialZ << "\n";
                logger.info << "\t     Thickness_mm:     " << filterThickness_mm << "\n";
                logger.info << "\t KVP:                  " << kVP << "\n";
                logger.info << "\t Exposure:             " << mas_value << "\n";
                logger.info << "\t Phantom_Thickness_mm: " << phantom_thickness_mm << "\n";
                logger.info << "\t Density_Coefficient:  " << density_coefficient << "\n";
            }
            else logger.error << "Could not find node \"Exposure_Settings\"\n";
                
            // Read Detector Section
            retval = clib.findNodeXML("Detector");
            if (retval)
            {
                if (clib.findNodeXML("Element_Size_mm"))
                {
                    clib.getNodeValueXML("X", elementsizeX_mm);
                    clib.getNodeValueXML("Y", elementsizeY_mm);
                    clib.concludeSectionXML();
                }
                else logger.error << "Could not find node \"Element_Size_mm\"\n";
                
                if (clib.findNodeXML("Element_Count"))
                {
                    clib.getNodeValueXML("X", elementCountX);
                    clib.getNodeValueXML("Y", elementCountY);
                    clib.concludeSectionXML();
                }
                else logger.error << "Could not find node \"Element_Count\"\n";

                clib.getNodeValueXML("Thickness_mm", det_thickness_mm);

                clib.getNodeValueXML("Material", det_material);
                clib.getNodeValueXML("Bits_Allocated", allocatedbits);
                clib.getNodeValueXML("Bits_Stored", storedbits);
                clib.getNodeValueXML("High_Bit", highbit);
                clib.getNodeValueXML("Has_Volume_Offset", useXYVolumeOffsets);
                
                if (clib.findNodeXML("Volume_Offset_mm"))
                {
                    clib.getNodeValueXML("X", volumeOffsetX_mm); // optional offset override
                    clib.getNodeValueXML("Y", volumeOffsetY_mm); // optional offset override
                    clib.getNodeValueXML("Z", volumeOffsetZ_mm); // absolute offset
                    clib.concludeSectionXML();
                }
                else logger.error << "Could not find node \"Volume_Offset_mm\"\n";
                
                clib.concludeSectionXML();

                logger.info << "Detector\n";
                logger.info << "\t Element_Size_mm:\n";
                logger.info << "\t     X:                " << elementsizeX_mm << "\n";
                logger.info << "\t     Y:                " << elementsizeY_mm << "\n";
                logger.info << "\t Element_Count:\n";
                logger.info << "\t     X:                " << elementCountX << "\n";
                logger.info << "\t     Y:                " << elementCountY << "\n";
                logger.info << "\t Thickness_mm:         " << det_thickness_mm << "\n";
                logger.info << "\t Material:             " << det_material << "\n";
                logger.info << "\t Bits_Allocated:       " << allocatedbits << "\n";
                logger.info << "\t Bits_Stored:          " << storedbits << "\n";
                logger.info << "\t High_Bit:             " << highbit << "\n";
                logger.info << "\t Has_Volume_Offset:    " << (useXYVolumeOffsets?"true":"false") << "\n";
                logger.info << "\t Volume_Offset_mm:\n";
                logger.info << "\t     X:                " << volumeOffsetX_mm << "\n";
                logger.info << "\t     Y:                " << volumeOffsetY_mm << "\n";
                logger.info << "\t     Z:                " << volumeOffsetZ_mm << "\n";
            }
            else logger.error << "Could not find node \"Detector\"\n";


            // Read Acquisition Geometry              
            retval = clib.findNodeXML("Acquisition_Geometry");
            if (retval)
            {
                clib.getNodeValueXML("Number_Acquisitions", number_of_acquisitions);

                // Dump to logger
                logger.info << "Acquisitions:\n";
                logger.info << "\t Number_Acquisitions:  " << number_of_acquisitions << "\n";

                if (clib.findNodeXML("Acquisition"))
                {
                    AcqGeom geom;
                    geom.readInputXML(clib, logger);
                    geometries.push_back(geom);
                }

                while(clib.nextNodeXML("Acquisition"))
                {
                    AcqGeom geom;
                    geom.readInputXML(clib, logger);                  
                    geometries.push_back(geom);
                }
            }
            else logger.error << "Could not find node \"Acquisition_Geometry\"\n";


            // Ensure the proper file extension is included in the filenames
            conditionPhantomFilename();

            xml_input_used = true;

            // Debug
            //if (verbose) dumpParameters("After Reading Generator_Config File from Director"); 
        }
        else logger.error << "Could not find node \"Breast_Phantom_Designer\"\n";
    }
    else logger.error << "Could not read xml input file \"" << xmlfile << "\" *\n";

    return retval;
}


void ReadPhantom::conditionPhantomFilename()
{
    phantom_name_in = input_phantom_filename;

    // Strip path away from input phantom name if it exists
    std::string::size_type pos = phantom_name_in.find_last_of("/\\");
    if (pos != std::string::npos && pos < phantom_name_in.length()) phantom_name_in = phantom_name_in.substr(pos+1);
    
    // Strip file extension away from input phantom name if it exists
    pos = phantom_name_in.find_last_of('.');
    if (pos != std::string::npos && pos < phantom_name_in.length()) phantom_name_in = phantom_name_in.substr(0, pos);

    // Add file extension to input phantom filename if it isn't present
    pos = input_phantom_filename.find(vct::EXTENSION);
    if (pos == std::string::npos) input_phantom_filename += vct::EXTENSION;    
}

// CC-deformed (compressed) phantoms need to be roated 90 clockwise about the x axis
bool ReadPhantom::rotate90X()
{
    // X coordinates are unchanged
    // Y coordinate is former Z coordinate
    // Z coordinate is center Y coordinate minus Y coordinate

    size_t total_size = dim_x * dim_y * dim_z;

    unsigned char *voxels_p = new unsigned char[total_size];

    size_t ndx;
    size_t ndx_p;

    size_t dim_x_p = dim_x;
    size_t dim_y_p = dim_z;
    size_t dim_z_p = dim_y;
    
    size_t x_p, y_p, z_p;

    for(size_t z=0; z<dim_z; ++z)
    {
        for(size_t y=0; y<dim_y; ++y)
        {
            for(size_t x=0; x<dim_x; ++x)
            {
                ndx = ((z * dim_y) + y) * dim_x + x; // dim_x * (dim_y * z + y) + x;
                x_p = x;
                y_p = z;
                z_p = dim_y - y - 1;
                ndx_p = ((z_p * dim_y_p) + y_p) * dim_x_p + x_p; // dim_x_p * (dim_y_p * z_p + y_p) + x_p;
                voxels_p[ndx_p] = voxels[ndx];
            }
        }
    }
    
    // Copy from the old buffer and delete the new buffer
    dim_x = dim_x_p;
    dim_y = dim_y_p;
    dim_z = dim_z_p;
    memcpy(voxels, voxels_p, total_size);
    delete [] voxels_p;

    return true;
}


unsigned char *ReadPhantom::getPhantom(size_t &x_dim, size_t &y_dim, size_t &z_dim,
                                       float &vsizex_m, float &vsizey_m, float &vsizez_m)
{
    //std::cout << "input_phantom_filename = " << input_phantom_filename << std::endl;

    vctx.readVctx(input_phantom_filename);
    vct::Phantom *ph = vctx.getPhantom();

    if (ph != nullptr)
    {
        vct::VoxelArray *va = ph->getVoxelArray();
        if (va != nullptr)
        {
            long long x, y, z;
            va->getVoxelNum(x, y, z);

            dim_x = x_dim = size_t(x);
            dim_y = y_dim = size_t(y);
            dim_z = z_dim = size_t(z);

            // Find Voxel sizes in millimeters
            va->getVoxelSize_mm(vsiz_x, vsiz_y, vsiz_z);
            vsizex_m = vsiz_x;
            vsizey_m = vsiz_y;
            vsizez_m = vsiz_z;

            voxels = (unsigned char *)va->getVoxels();
            //std::cout << "voxels = " << (int *) voxels << std::endl;         

            vct::Deformation *def = ph->getDeformation();
            if (def != nullptr)
            {
                vct::DEFORMATION_TYPE deftype = def->getDefType();
                vct::DEFORMATION_MODE defmode = def->getDefMode();
                          
                if (deftype == vct::FINITE_ELEMENT && 
                    defmode == vct::DEFORM_CC) 
                {
                    //std::cout << "Rotating phantom by 90 degrees because its mode is DEFORM_CC" << std::endl;
                    // Rotate the phantom to place it correctly between the breast support and the compression paddle
                    rotate90X();

                    //Exchange y and z thickness to reflect rotated voxel body
                    double ythick = ph->getYThickness_mm();
                    double zthick = ph->getZThickness_mm();
                    ph->setYThickness_mm(zthick);
                    ph->setXThickness_mm(ythick);

                    // Update dimensions due to rotated voxel body (y and z already exhanged)
                    va->setVoxelNum(dim_x, dim_y, dim_z);
                    x_dim = dim_x;
                    y_dim = dim_y;
                    z_dim = dim_z;
                }
            }

            // Get attenuationTable
            std::string vctx_path = vctx.getPrivateDirIn();
            //std::cout << __FUNCTION__ << ", line " << __LINE__ << ": private dir is \"" << vctx_path << "\"" << std::endl;
            std::string vctx_atten_file_path = vctx_path + "XPL_AttenuationTable.xml";
            //std::cout << "\n* JUST BUILT ATTENUATION FILE PATH \"" << vctx_atten_file_path << "\" *\n" << std::endl;
            attenuationFile = vctx_atten_file_path;
            vct::CommonLibrary clib;
            bool isPath;
            if (clib.testPath(vctx_atten_file_path, isPath))
            {
                attenuationFile = vctx_atten_file_path;
                //std::cout << "Overriding Attenuation_File: " << attenuationFile << "\n";
            }
            ///else
            ///{
            ///    std::cerr << vctx_atten_file_path << " is not a path, apparently" << std::endl;
            ///}
        } 
        else
        {
            std::cerr << "Couldn't find voxelArray" << std::endl;    
        }
    }
    else
    {
        std::cerr << "Couldn't get phantom pointer" << std::endl;
    }

    return voxels;
}


void ReadPhantom::setAcquisitionGeometry(Machine *machine)
{
    if (machine != nullptr)
    {
        bool have_directionR = false;

        for(auto &ac : geometries)
        {
            Acquisition acq;

            acq.focalPosition.v[0] = ac.focal_spot_x_mm * 0.1; // convert to CM
            acq.focalPosition.v[1] = ac.focal_spot_y_mm * 0.1; 
            acq.focalPosition.v[2] = ac.focal_spot_z_mm * 0.1;

            /*
            acq.phantomPosition.v[0] = ac.ph_offset_x_mm * 0.1; // convert to CM       //BB and TV 04/10/2018
            acq.phantomPosition.v[1] = ac.ph_offset_y_mm * 0.1;
            acq.phantomPosition.v[2] = ac.ph_offset_z_mm * 0.1;
            */
            acq.detectorPosition.v[0] = ac.det_pos_x_mm * 0.1; // convert to CM
            acq.detectorPosition.v[1] = ac.det_pos_y_mm * 0.1;
            acq.detectorPosition.v[2] = ac.det_pos_z_mm * 0.1;

            acq.detectorDirectionS.v[0] = ac.Svec_ori_x;
            acq.detectorDirectionS.v[1] = ac.Svec_ori_y;
            acq.detectorDirectionS.v[2] = ac.Svec_ori_z;
            /*
            std::cout << __FUNCTION__ << ", line " << __LINE__ 
                      << ": acq.detectorDirectionS.v[0] = " << acq.detectorDirectionS.v[0] 
                      << ": acq.detectorDirectionS.v[1] = " << acq.detectorDirectionS.v[1] 
                      << ": acq.detectorDirectionS.v[2] = " << acq.detectorDirectionS.v[2] << std::endl;
            */
            acq.detectorDirectionT.v[0] = ac.Tvec_ori_x;
            acq.detectorDirectionT.v[1] = ac.Tvec_ori_y;
            acq.detectorDirectionT.v[2] = ac.Tvec_ori_z;
            /*
            std::cout << __FUNCTION__ << ", line " << __LINE__ 
                << ": acq.detectorDirectionT.v[0] = " << acq.detectorDirectionT.v[0] 
                << ": acq.detectorDirectionT.v[1] = " << acq.detectorDirectionT.v[1] 
                << ": acq.detectorDirectionT.v[2] = " << acq.detectorDirectionT.v[2] << std::endl;
            */



            // Calculate the R vector if not explicitly specified in XML
            if (!have_directionR) 
            {
                acq.detectorDirectionR = rowVector::cross(acq.detectorDirectionS, acq.detectorDirectionT);
                //std::cout << __FUNCTION__ << ", line " << __LINE__ << ": Calculating detectorDirectionR\n" ;
            }

            //std::cout << __FUNCTION__ << ", line " << __LINE__ << ": DetectorDirectionR: " << acq.detectorDirectionR.v[0] << "/" << acq.detectorDirectionR.v[1] << "/" << acq.detectorDirectionR.v[2] << std::endl;

            // Ensure that the Vectors are normalized
            double mag = 1.0;
            if ((mag = acq.detectorDirectionR.length()) != 1.0)  acq.detectorDirectionR = acq.detectorDirectionR / mag;
            if ((mag = acq.detectorDirectionS.length()) != 1.0)  acq.detectorDirectionS = acq.detectorDirectionS / mag;
            if ((mag = acq.detectorDirectionT.length()) != 1.0)  acq.detectorDirectionT = acq.detectorDirectionT / mag;

            //std::cout << __FUNCTION__ << ", line " << __LINE__ << ": DetectorDirectionR: " << acq.detectorDirectionR.v[0] << "/" << acq.detectorDirectionR.v[1] << "/" << acq.detectorDirectionR.v[2] << std::endl;

            // Calculate distance source to detector DH 09/05/2014
            double dx = acq.focalPosition.v[0] - volumeOffsetX_mm * 0.1;        //BB and TV 05/09/2018  check if it's minus     
            double dy = acq.focalPosition.v[1] - volumeOffsetY_mm * 0.1;        //BB and TV 05/09/2018  check if it's minus
            double dz = acq.focalPosition.v[2] - volumeOffsetZ_mm * 0.1;        //BB and TV 05/09/2018

            acq.distanceSrcToDet = sqrt((dx * dx) + (dy * dy) + (dz * dz));

            machine->acquisitionGeometry.push_back(acq);
        }
    }
}
    

bool ReadPhantom::buildLabelMap(std::map<unsigned short, LabelMap> *lmap)
{
    bool retval = false;

    vct::Phantom *ph = vctx.getPhantom();
    if (ph != nullptr)
    {
        vct::IndexTable *it = ph->getIndexTable();
        if (it != nullptr)
        {
            unsigned short label_id;
            double         weight;
            double         density;
            unsigned short materialZ;

            int max_label = it->getLargestIndex();
            for(int ndx=0; ndx<=max_label; ++ndx)
            {
                vct::Label lbl = it->getLabel(ndx);
                if (lbl.getId() == ndx)
                {
                    retval = true;
                    int num_mats = lbl.getNumMaterials();
                    LabelMap innerMap;
                    for(int m=0; m<num_mats; ++m)
                    {
                        vct::Material vmat = lbl.getMaterial(m);

                        label_id  = ndx;
                        weight    = vmat.getWeight();
                        density   = vmat.getDensity();
                        materialZ = vmat.getMaterialZ();
                        //std::cout << "\n * * * DENSITY: " << density << ", materialZ: " << materialZ << "* * *\n" << std::endl;

                        Material xpl_mat(weight, density, materialZ);
                        innerMap.addMaterial(xpl_mat);
                    }

                    lmap->insert(std::pair<unsigned short, LabelMap>(label_id, innerMap));
                }
            }
        }
    }

    return retval;
}


void ReadPhantom::generateReport()
{
    // Write out a generation report if input was from an XML file (as opposed to pure command line)
    if (xml_input_used)
    {
        vct::CommonLibrary clib;

        // Now write out a simple, non-compressed xml file for the database to read
        clib.initializeXML("Projection");

        vct::Date    date;
        vct::Time    time;
        vct::Station station;
        station.querySystem();

        clib.addASectionXML("Station");
        station.writeXML(&clib);
        clib.concludeSectionXML();

        clib.writeNodeValueXML("Institution_Code_Sequence", org_code);
        
        // Get Duration
        _endtime = std::clock();///////ftime(&_endtime);
        int elapsed_time_msec = static_cast<int>(100.0 * double(_endtime - _starttime) / CLOCKS_PER_SEC + 0.5);
        clib.writeNodeValueXML("Elapsed_Time_msec", elapsed_time_msec);

        clib.addASectionXML("Acquisitions");

        for(int acq=0; acq<acq_dates.size(); ++acq)
        {
            clib.addASectionXML("Acquisition");
            {
                clib.addASectionXML("Date_Projected");
                acq_dates[acq].writeXML(&clib);
                clib.concludeSectionXML();
            
                clib.addASectionXML("Time_Projected");
                acq_times[acq].writeXML(&clib);
                clib.concludeSectionXML();
            }
            clib.concludeSectionXML();
        }
        clib.concludeSectionXML();

        // Build filename and save file - need output folder path and name report file after dicom folder name
        std::string filename(output_folder);
        filename += "_report.xml";
        clib.saveXML(filename);
    }
}


// Parse DICOM strings and store date & time for a sequence of acquisitions
void ReadPhantom::addDicomDate(std::string date_string)
{
    if (date_string.size() > 7)
    {
        vct::Date date;

        int year = std::stoi(date_string.substr(0, 4));             //std::cout << "year: " << year << std::endl;
        unsigned int mon = std::stoi(date_string.substr(4, 2));     //std::cout << "mon: "  << mon  << std::endl;
        unsigned int day = std::stoi(date_string.substr(6, 2));     //std::cout << "day: "  << day  << std::endl;

        date.setYear(year);
        date.setMonth(mon);
        date.setDay(day);

        acq_dates.push_back(date);
    }
}


void ReadPhantom::addDicomTime(std::string time_string)
{
    if (time_string.size() > 12)
    {
        vct::Time time;

        unsigned int hour;
        unsigned int min;
        unsigned int sec;
        unsigned int frac;
        short utc_offset;

        vct::Time::setOrGetUtcOffset(utc_offset, false);    //std::cout << "utc_offset: " << utc_offset << std::endl;
        hour = std::stoi(time_string.substr(0, 2));         //std::cout << "hour: " << hour << std::endl;
        min  = std::stoi(time_string.substr(2, 2));         //std::cout << "min: " << min << std::endl;
        sec  = std::stoi(time_string.substr(4, 2));         //std::cout << "sec: " << sec << std::endl;
        frac = std::stoi(time_string.substr(7, 6));         //std::cout << "frac: " << frac << std::endl;

        time.setHour(hour - utc_offset);
        time.setMinute(min);
        time.setSeconds(sec);
        time.setFracNSec(frac);

        acq_times.push_back(time);
    }
}
