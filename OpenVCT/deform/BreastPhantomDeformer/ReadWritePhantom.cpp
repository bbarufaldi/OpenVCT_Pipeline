// ReadWritePhantom.h

#include "ReadWritePhantom.h"

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


ReadWritePhantom::ReadWritePhantom()
        : verbose(false),
          input_from_xml(false),
          it(nullptr),
          view(vct::UNK_DEFORMATION_MODE),
          phantom_name_in(),
          input_phantom_filename(),
          phantom_name_out(),
          output_phantom_filename(),
          mesh_in(), mesh_out(),
          full_def_phantom_path(),
          deformed_stream(),
          dim_x(0), dim_y(0), dim_z(0),
          vsiz_x(0.0f), vsiz_y(0.0f), vsiz_z(0.0f),
          def_x(0), def_y(0), def_z(0),
          seqnum(0), 
          x_thickness_mm(0.0f), 
          y_thickness_mm(0.0f), 
          z_thickness_mm(0.0f), 
          deformed_buf(nullptr),
          program_name("BreastPhantomDeformer"),
          sw_version("1.0"),
          index_table_uid("unknown_UID"),
          voxel_array_uid("unknown_UID"),
          org_name("unknown_org"),
          org_div("unknown_div"),
          org_dept("unknown_dept"),
          org_grp("unknown_group"),
          org_code("unknown_code"),
          org_cntry("unknown_country"),
          org_sta("unknown_state"),
          org_city("unknown_city"),
          org_zip("unknown_zip"),
          utc_offset(0)
{
    clib.start_timer();
}


ReadWritePhantom::~ReadWritePhantom()
{
    if (deformed_stream.is_open()) deformed_stream.close();
    if (deformed_buf != nullptr) delete [] deformed_buf;
}


void ReadWritePhantom::readConfigFile()
{
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


void ReadWritePhantom::setCmdLineArgs(std::string inphantom, std::string inmesh,
                                      std::string outmesh, std::string outphantom)
{
    vct::CommonLibrary clib;
    input_phantom_filename = inphantom;
    mesh_in     = inmesh;
    mesh_out    = outmesh;
    output_phantom_filename = outphantom;

    conditionPhantomFilenames();
}


bool ReadWritePhantom::readInputXML(vct::Logger &logger, std::string xmlfile)
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
            input_from_xml = true;
            retval = clib.findNodeXML("Deformer_Config");
            if (retval)
            {
                clib.getNodeValueXML("Program_Name", program_name);
                clib.getNodeValueXML("Software_Version", sw_version);

                clib.getNodeValueXML("Input_Phantom", input_phantom_filename);
                clib.getNodeValueXML("Input_Mesh", mesh_in);
                clib.getNodeValueXML("Output_Mesh", mesh_out);

                std::string view_string;
                if (clib.getNodeValueXML("View", view_string))
                {
                    if      (view_string == "CC")  view = vct::DEFORM_CC;
                    else if (view_string == "ML")  view = vct::DEFORM_ML;
                    else if (view_string == "MLO") view = vct::DEFORM_MLO;
                    
                    if (verbose) std::cout << "View mode is " << view_string << "(" << def.interpretMode(view) << ")"<< std::endl;
                    logger.info << "View type is " << def.interpretMode(view) << "\n";
                }
                else
                {
                    std::cerr << "Could not find \"View\" node" << std::endl;
                    logger.error << "Could not find \"View\" node\n";
                }

                clib.getNodeValueXML("Output_Phantom", output_phantom_filename);
                clib.concludeSectionXML();

                logger.info << "From XML input file:\n";
                logger.info << "\t Program_Name:     " << program_name             << "\n";
                logger.info << "\t Software_Version: " << sw_version               << "\n";
                logger.info << "\t Input_Phantom:    " << input_phantom_filename   << "\n";
                logger.info << "\t Input_Mesh:       " << mesh_in                  << "\n";
                logger.info << "\t Output_Mesh:      " << mesh_out                 << "\n";
                logger.info << "\t Output_Phantom:   " << output_phantom_filename  << "\n";

                // Ensure the proper file extension is included in the filenames
                conditionPhantomFilenames();
            }

            retval = clib.getNodeValueXML("Index_Table_UID", index_table_uid);
            if (retval)
            {
                logger.info << "\t Index_Table_UID: " << index_table_uid << "\n";
            }
            else logger.error << "Could not find \"Index_Table_UID\" node\n";

            retval = clib.getNodeValueXML("Voxel_Array_UID", voxel_array_uid);
            if (retval)
            {
                logger.info << "\t Voxel_Array UID: " << voxel_array_uid << "\n";
            }
            else logger.error << "Could not find \"Voxel_Array_UID\" node\n";

            // Debug
            //if (verbose) dumpParameters("After Reading Generator_Config File from Director"); 
        }
        else logger.error << "Could not find \"Breast_Phantom_Designer\" node\n";
    }
    else logger.error << "Could not read xml input file \"" << xmlfile << "\"\n";

    return retval;
}


void ReadWritePhantom::conditionPhantomFilenames()
{
    phantom_name_in = input_phantom_filename;
    phantom_name_out = output_phantom_filename;

    // Strip path away from input phantom name if it exists
    std::string::size_type pos = phantom_name_in.find_last_of("/\\");
    if (pos != std::string::npos && pos < phantom_name_in.length()) phantom_name_in = phantom_name_in.substr(pos+1);
    
    // Strip file extension away from input phantom name if it exists
    pos = phantom_name_in.find_last_of(".");
    if (pos != std::string::npos && pos < phantom_name_in.length()) phantom_name_in = phantom_name_in.substr(0, pos);

    // Add file extension to input phantom filename if it isn't present
    pos = input_phantom_filename.find(vct::EXTENSION);
    if (pos == std::string::npos) input_phantom_filename += vct::EXTENSION;    

    // Strip path away from output phantom name if it exists
    pos = phantom_name_out.find_last_of("/\\");
    if (pos != std::string::npos && pos < phantom_name_out.length()) phantom_name_out = phantom_name_out.substr(pos+1);
    
    // Strip file extension away from output phantom name if it exists
    pos = phantom_name_out.find_last_of(".");
    if (pos != std::string::npos && pos < phantom_name_out.length()) phantom_name_out = phantom_name_out.substr(0, pos);

    // Add file extension to output phantom filename if it isn't present
    pos = output_phantom_filename.find(vct::EXTENSION);
    if (pos == std::string::npos) output_phantom_filename += vct::EXTENSION;

    if (verbose)
    {
        std::cout << __FUNCTION__ 
                  << ": phantom_name_out is \"" << phantom_name_out << "\"" 
                  << ": output_phantom_filename is \"" << output_phantom_filename << "\""
                  << std::endl;
    }
}


unsigned char *ReadWritePhantom::getPhantom(vct::Logger &logger, size_t &x_dim, size_t &y_dim, size_t &z_dim,
                                            float &vsizex_mm, float &vsizey_mm, float &vsizez_mm)
{
    unsigned char *voxels = nullptr;
    std::cout << "input_phantom_filename = " << input_phantom_filename << std::endl;
    logger.info << "input_phantom_filename = " << input_phantom_filename << "\n";

    bool status = vctx.readVctx(input_phantom_filename);
    logger.info << "vctx.readVctx(" << input_phantom_filename << ") returned " << (status?"True":"False") << "\n";

    vct::Phantom *ph = vctx.getPhantom();
    if (ph != nullptr)
    {
        vct::VoxelArray *va = ph->getVoxelArray();
        if (va)
        {
            long long x, y, z;
            va->getVoxelNum(x, y, z);

            x_dim = size_t(x);
            y_dim = size_t(y);
            z_dim = size_t(z);

            // Find Voxel sizes in millimeters
            va->getVoxelSize_mm(vsiz_x, vsiz_y, vsiz_z);

            vsizex_mm = vsiz_x;
            vsizey_mm = vsiz_y;
            vsizez_mm = vsiz_z;

            if (verbose) std::cout << __FUNCTION__ << ": vsiz_x=" << vsiz_x << ", voxel size (mm) = " << vsizex_mm << std::endl;
            logger.info << __FUNCTION__ << ": vsiz_x=" << vsiz_x << ", voxel size (mm) = " << vsizex_mm << "\n";

            voxels = (unsigned char *)va->getVoxels();
            logger.info << "voxels = " << (int *) voxels << "\n";

        }
        else
        {
            std::cerr << "Couldn't find voxelArray" << std::endl;    
            logger.fatal << "Couldn't find voxelArray! Holy Cow!" << "\n";
        }
    }
    else 
    {
        std::cerr << "Couldn't get phantom pointer" << std::endl;
        logger.fatal << "Couldn't get phantom pointer"  << "\n";
    }

    return voxels;
}


void ReadWritePhantom::initVoxelArray()
{
    vct::Phantom *ph = vctx.getPhantom();
    if (ph != nullptr)
    {
        vct::VoxelArray *va = ph->getVoxelArray();
        if (va != nullptr)
        {
            va->setVoxelNum(0LL, 0LL, 0LL);
            va->replaceVoxels(nullptr);
        }
    }
}


std::ofstream &ReadWritePhantom::prepareOutputPhantom(int xdim, int ydim, int zdim)
{
    def_x = (long long)(xdim);
    def_y = (long long)(ydim);
    def_z = (long long)(zdim);

    if (verbose)
    {
        std::cout << __FUNCTION__ 
                  << ": phantom_name_out is \"" << phantom_name_out << "\"" 
                  << ": output_phantom_filename is \"" << output_phantom_filename << "\""
                  << std::endl;
    }

    std::string temp_dir  = vctx.getTempDir();
    full_def_phantom_path = temp_dir + phantom_name_out;
    std::cout << "phantom_name_out is \"" << phantom_name_out << ".vctx" << "\"" << std::endl;
    if (verbose) std::cout << "full_def_phantom_path is \"" << full_def_phantom_path << "\"" << std::endl;

    vct::CommonLibrary clib;
    bool isAPath;
    bool exists = clib.testPath(full_def_phantom_path, isAPath);
    if (exists && !isAPath)
    {
        // There's a regular file there with this name. Get rid of it.
        remove(full_def_phantom_path.c_str());
    }
    
    // Now create the path
    exists = clib.createPath(full_def_phantom_path);    
    
    //std::cout << "\n* Press x and return to continue: "; char cx; std::cin >> cx;

    // Now open the stream in order for VolumeDeformer to iterative write consecutive slices
    std::string full_def_phantom_dat = full_def_phantom_path + "/Phantom.dat";    
    deformed_stream.open(full_def_phantom_dat, std::fstream::binary);    
    if (verbose) std::cout << "Prepared output stream is \"" << full_def_phantom_dat << "\"" << std::endl;

    return deformed_stream;
}


void ReadWritePhantom::finishOutputPhantom()
{ 
    if (deformed_stream.is_open()) deformed_stream.close();   // VolumeDeformer should close this
    
    if (verbose)
    {
        std::cout << __FUNCTION__ 
                  << ": phantom_name_out is \"" << phantom_name_out << "\"" 
                  << ": output_phantom_filename is \"" << output_phantom_filename << "\""
                  << std::endl;
    }

    vct::Phantom *ph = vctx.getPhantom();
    if (verbose) std::cout << __FUNCTION__ << ": DEBUG: phantom is at " << ph << std::endl;
    if (ph != nullptr)
    {
        vct::VoxelArray *va = ph->getVoxelArray();
        if (verbose) std::cout << __FUNCTION__ << ": DEBUG: VoxelArray is at " << va << std::endl;
        if (va != nullptr)
        {
            // Read the voxels
            long long total_size = def_x * def_y * def_z * sizeof(char);
            ph->getVoxelArray()->setTotalSize(total_size);
            deformed_buf = new unsigned char[total_size];
            if (verbose) 
            {
                std::cout << __FUNCTION__ << ": DEBUG: total size is " << total_size 
                          << ", deformed_buf is at 0x" << std::hex 
					      << reinterpret_cast<unsigned long *>(deformed_buf)
					      << std::endl;
            }

            if (deformed_buf != nullptr)
            {
                // Read the deformed phantom into memory
                vct::CommonLibrary clib;
                std::string phantomData = full_def_phantom_path + "/Phantom.dat";
                if (verbose) std::cout << "Reading binary data back in from file \"" << phantomData << "\"" << std::endl;
                std::ifstream ifs(phantomData, std::fstream::binary);
                clib.inputByChunks(ifs, deformed_buf, total_size);
                deformed_stream.close();

                // Set the new voxel array information
                va->setVoxelNum(def_x, def_y, def_z);
                va->setVoxelSize_mm(vsiz_x, vsiz_y, vsiz_z);
                va->replaceVoxels(deformed_buf);

                // Update Date and Time
                vct::Date date;
                vct::Time time;
                date.setNow();
                time.setUtcOffset(utc_offset);
                time.setNow();
        
                // Populate and set the Deformer section
                def.setSequenceNumber(seqnum);
                def.setDefType(vct::FINITE_ELEMENT);

                // Figure out what mode we're using
                x_thickness_mm = def_x *  vsiz_x;
                y_thickness_mm = def_y *  vsiz_y;
                z_thickness_mm = def_z *  vsiz_z;

                vct::DEFORMATION_MODE dmode = view;
                def.setDefThickness(y_thickness_mm);

                /**
                if (mesh_out.find("CC") != std::string::npos || 
                    mesh_out.find("cc") != std::string::npos)
                {
                    dmode = vct::DEFORM_CC;
                    def.setDefThickness(z_thickness_mm);
                }
                else if (mesh_out.find("ML") != std::string::npos || 
                            mesh_out.find("ml") != std::string::npos)
                {
                    dmode = vct::DEFORM_ML;
                    def.setDefThickness(y_thickness_mm);
                }
                else if (mesh_out.find("MLO") != std::string::npos ||  // warning: this isn't yet realistically supported
                         mesh_out.find("mlo") != std::string::npos)
                {
                    dmode = vct::DEFORM_MLO;
                    def.setDefThickness(sqrt((z_thickness_mm * z_thickness_mm) + (y_thickness_mm * y_thickness_mm)));
                }
                **/

                def.setDefMode(dmode);
                def.setDefDate(date);
                def.setDefTime(time);
                
                vct::Organization org;

                org.setName(org_name);
                org.setDivision(org_div);
                org.setDepartment(org_dept);
                org.setGroup(org_grp);
                org.setInstituionCode(org_code);
                org.setCountry(org_cntry);
                org.setState(org_sta);
                org.setCity(org_city);
                org.setZip(org_zip);

                vct::Software software;

                software.setName(program_name);
                software.setVersion(sw_version);
                software.setRepository("https://xraylabsvr1.intranet.imagephysics.com/svn/VCTPipelineCode");
                software.setDate(date);
                software.setTime(time);

                vct::Station station;
                station.querySystem();
        
                def.setOrganization(&org);
                def.setSoftware(&software);
                def.setStation(&station);
    
                ph->setDeformation(&def);

                if (index_table_uid.size() > 0)
                {
                    it = ph->getIndexTable();
                    it->setUID(index_table_uid);
                }
                
                vct::VoxelArray *voxel_array = ph->getVoxelArray();

                if (voxel_array_uid.size() > 0)
                {
                    voxel_array->setVoxelArrayUID(voxel_array_uid);
                }

                if (verbose) std::cout << "DEBUG: about to try to count glandular. voxel_array is " << voxel_array << ", it is " << it << std::endl;
                size_t non_air = size_t(total_size);
                size_t glandular_voxels = static_cast<size_t>(voxel_array->countGlandular(*it, non_air));

                if (verbose)
                {
                    std::cout << "There are " << total_size << " voxels total" << std::endl;
                    std::cout << "There are " << non_air << " voxels which are not air, " << std::endl;
                    std::cout << "There are " << glandular_voxels << " glandular voxels, " << std::endl;
                }

                ph->setTotalNonAirVoxels(non_air);                
                ph->setXThickness_mm(x_thickness_mm);
                ph->setYThickness_mm(y_thickness_mm);
                ph->setZThickness_mm(z_thickness_mm);
                ph->setGlandularCount(glandular_voxels);


                // Create Private Data: BreastPhantomDeformation command line arguments
                vct::XPL_BreastPhantomDeformer *xpl_deformer = new vct::XPL_BreastPhantomDeformer;
                if (xpl_deformer != nullptr)
                {
                    xpl_deformer->setProgramName(program_name);
                    xpl_deformer->setSWVersion(sw_version);
                    xpl_deformer->setUniqueId("1.00.00.1"); // TODO: find out what this is supposed to be
                    xpl_deformer->setInputPhantomFile(input_phantom_filename);
                    xpl_deformer->setInputMeshFile(mesh_in);
                    xpl_deformer->setOutputMeshFile(mesh_out);
                    xpl_deformer->setOutputPhantomFile(output_phantom_filename);
                    vctx.addPrivateData(xpl_deformer);
                }

                // Write the new phantom out
                if (verbose) std::cout << "\n* Saving \"" << output_phantom_filename << "\" *" << std::endl;

                // Ensure that the phantom_name includes no path component
                std::string::size_type pos = phantom_name_out.find_last_of("/\\");
                if (pos != std::string::npos)
                {
                    phantom_name_out = phantom_name_out.substr(pos+1);
                }

                if (verbose) std::cout << "About to call vctx.writeVctx(" << phantom_name_out << ", " << output_phantom_filename << ")\n";
                vctx.writeVctx(phantom_name_out, output_phantom_filename); // path and archive file (vctx)

                /*
                // Remove the temporary phantom
                std::cout << "Removing temporary phantom file " << full_def_phantom_path << std::endl;
                clib.deleteFile(full_def_phantom_path);
                */


                std::string phantom_output_path;
                pos = output_phantom_filename.find_last_of("/\\");
                if (pos != std::string::npos)
                {
                    phantom_output_path = output_phantom_filename.substr(0, pos+1);
                }

                // Write out a generation report if input was from an XML file (as opposed to pure command line)
                if (input_from_xml)
                {
                    // Now write out a simple, non-compressed xml file for the database to read
                    clib.initializeXML("Phantom");

                    // Write number of non-air voxels
                    clib.writeNodeValueXML("Total_Non_Air_Voxels", non_air); 

                    // Write thickness section
                    clib.addASectionXML("Thickness_mm");
                    clib.writeNodeValueXML("X", x_thickness_mm);
                    clib.writeNodeValueXML("Y", y_thickness_mm);
                    clib.writeNodeValueXML("Z", z_thickness_mm);
                    clib.concludeSectionXML();

                    // Write glandular count
                    clib.writeNodeValueXML("Total_Non_Air_Voxels", non_air);
                    clib.writeNodeValueXML("Glandular_Count", glandular_voxels); 
    
                    clib.addASectionXML("Date_Created");
                    date.writeXML(&clib);
                    clib.concludeSectionXML();
        
                    clib.addASectionXML("Time_Created");        
                    time.writeXML(&clib);
                    clib.concludeSectionXML();

                    clib.addASectionXML("Station");
                    station.writeXML(&clib);
                    clib.concludeSectionXML();

                    clib.writeNodeValueXML("Institution_Code_Sequence", org_code);

                    unsigned int diff = clib.get_elapsed_time();
                    clib.writeNodeValueXML("Elapsed_Time_msec", diff);

                    // Build filename and save file
                    if (verbose) std::cout << "DEBUG: output_phantom_filename is \"" << output_phantom_filename << "\"" << std::endl;
                    std::string filename(output_phantom_filename);
                    std::string::size_type pos = filename.find_last_of("/\\");
                    if (pos != std::string::npos) filename = filename.substr(pos+1);
                    pos = filename.find(".vctx");
                    if (pos != std::string::npos) filename = filename.substr(0, pos);
                    filename += "_report.xml";
                    clib.saveXML(phantom_output_path + filename);
                }
            }
            else std::cerr << __FUNCTION__ << ": cannot read deformed voxels" << std::endl;
        }
    }
}
    

size_t ReadWritePhantom::countGlandular(size_t total, unsigned char *voxels, size_t &non_air)
{
    const int NIST_GLANDULAR = 205;

    non_air = 0; // total number of phantom voxels which are not air
    double glandular_count = 0.0;    
    size_t count_0 = 0, count_1 = 0, count_2 = 0, count_3 = 0, count_4 = 0, count_5 = 0;

    // Query the Index Table for all labels containing glandularity
    std::map<int, float> glandular_labels = it->findLabelsWithThisMaterial(NIST_GLANDULAR);

    if (verbose) std::cout << "Starting counts..." << std::endl;
    for(size_t i = 0; i<total; ++i)
    {
        int v = voxels[i];
        if (v != 0) non_air++;

        // Check against all labels containing glandualr material
        for(auto &lbl : glandular_labels)
        {
            // if this label contains glandular, increment the count by the glandular material's weight for this label
            if (v == lbl.first) glandular_count += lbl.second;
        }

        if (verbose)
        {
            // Gather counts on the first 6 labels
            if      (v == 0) count_0++;
            else if (v == 1) count_1++;
            else if (v == 2) count_2++;
            else if (v == 3) count_3++;
            else if (v == 4) count_4++;
            else if (v == 5) count_5++;
        }
    }

    if (verbose)
    {
        // Report the voxel counts for the first 6 labels
        std::cout << "Voxel counts:\n"
                  << "  0: " << count_0 << "\n"  // 50186508  (100% Air)
                  << "  1: " << count_1 << "\n"  // 0         (100% Glandular)
                  << "  2: " << count_2 << "\n"  // 39048208  (100% Skin)
                  << "  3: " << count_3 << "\n"  // 0         (100% Adipose)
                  << "  4: " << count_4 << "\n"  // 17015284  (25% Glandular, 75% Water)
                  << "  5: " << count_5          // 0         (Calcium)
                  << std::endl;
    }

    return size_t(glandular_count + 0.5); // round up
}
