// XPLProjectionSimulation.cpp

#if defined(_MSC_VER)  // if MS Visual Studio, explicitly
#include "Windows.h"
#else                  // everyone else, including linux, mac, and MinGW / cygwin on Windows
#include <sys/stat.h>
#include <unistd.h>
#endif

// Grassroots Dicom headers
#include "gdcmReader.h"
#include "gdcmWriter.h"
#include "gdcmAttribute.h"
#include "gdcmItem.h"
#include "gdcmImage.h"
#include "gdcmImageReader.h"
#include "gdcmImageWriter.h"
#include "gdcmFile.h"
#include "gdcmTag.h"
#include "gdcmPrivateTag.h"

/* Linked Grassroots DICOM Libraries:
    gdcmcharls.lib    // Grassroots DICOM Library
    gdcmCommon.lib
    gdcmDICT.lib
    gdcmDSED.lib
    gdcmexpat.lib
    gdcmgetopt.lib
    gdcmIOD.lib
    gdcmjpeg8.lib
    gdcmjpeg12.lib
    gdcmjpeg16.lib
    gdcmMEXD.lib
    gdcmMSFF.lib
    gdcmopenjpeg.lib
    gdcmzlib.lib
    socketxx.lib      // Network & I/O library
    ws2_32.lib
    rpcrt4.lib        // Required Windows library
    XPLRayTracing.lib // Local lib (not a DICOM lib)
*/

#include "ReadPhantom.h" // RayTracer to VCT interface class
#include "VCT_Logger.h"
#include "VCT_CommonLibrary.h"
#include "ProgressBar.h"

#include "XPLRayTracing.h"
#include "XPLAcquisition.h"
#include "pugixml.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <cmath>


ProgressBar progressBar;

// Forward Declarations (implementations found at bottom of file)
int  OrganizeArgs(int argc, char *argv[]);
void Usage(std::string prog_name);
bool GetMyHostName(char *name, int len);
std::string FindDicomTemplate(const std::string &xml_file);
bool WriteVectorToDicom(gdcm::DataSet &ds, vct::Logger &log, uint16_t d1, uint16_t d2, std::string msg, int places, double v1, double v2, double v3) ;//throw(std::string);
std::string SetField(gdcm::DataSet &ds, uint16_t d1, uint16_t d2, gdcm::VR vr, std::string value);


#pragma region Documentation
/// <summary>
/// Main entry point for the XPLProjection Simulation program
/// </summary>
/// <param name="args[1]">The material attenuation xml path.</param>
/// <param name="args[2]">The simulation config xml path.</param>
/// <param name="args[3]">The phantom data path.</param>
/// <param name="args[4]">The prefix for output image files.</param>
/// <param name="args[5]">The random number seed.</param>
#pragma endregion
int main(int argc, char *argv[])
{
    const static std::string filename("$Id$ ");
    const static std::string file_revision("$Rev$");
    const static std::string build_date(", Build Date: ");
    const static std::string time_stamp(__TIMESTAMP__);
    const static std::string VERSION(filename + file_revision + build_date + time_stamp);

    std::string prog_name(argv[0]);
        
    if (argc == 2)
    {
		auto pos = prog_name.find_last_of("\\/");
		if (pos != std::string::npos) prog_name = prog_name.substr(pos+1);
        std::string s_arg(argv[1]);
        std::transform(s_arg.begin(), s_arg.end(), s_arg.begin(), tolower); // convert to lower case

        if (s_arg.find("version") != std::string::npos) // -version or --version or /version
        {
            std::cout << prog_name << ": " << VERSION << std::endl;
            return 0;  // exit program with success
        }
        else if (s_arg.find("help")  != std::string::npos
              || s_arg.find("usage") != std::string::npos
              || s_arg.find("?")     != std::string::npos)
        {
            Usage(prog_name);  // print instructions for specifying command line arguments
            return 0;          // exit program with success
        }
        else if (s_arg.find("example") != std::string::npos)
        {
            std::cout << "\nThe xml file might look like this:\n\n";          
            std::cout << "    <?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";            
            std::cout << "    <Breast_Phantom_Designer>\n\n";
            std::cout << "        <Projector_Config>\n";
            std::cout << "            <Program_Name>RayTracingProjector</Program_Name>\n";
            std::cout << "            <Software_Version>1.0</Software_Version>\n";
            std::cout << "            <Input_Phantom>ExamplePhantom.vctx</Input_Phantom>\n";
            std::cout << "            <Output_Folder>ExamplePhantom-proj</Output_Folder>\n";
            std::cout << "            <Has_Noise>false</Has_Noise>\n";
            std::cout << "            <Has_Invsq>true</Has_Invsq>\n";
            std::cout << "            <Elec_Noise>3.0</Elec_Noise>\n";
            std::cout << "            <Gain>0.2</Gain>\n";
            std::cout << "            <Prop_Const>0.0038</Prop_Const>\n";
            std::cout << "            <Random_Seed>50</Random_Seed>\n";
            std::cout << "        </Projector_Config>\n";
            std::cout << "        <Imaging_System>\n";
            std::cout << "            <Name>ExampleImagingSystem</Name>\n";
            std::cout << "        </Imaging_System>\n";
            std::cout << "        <Exposure_Settings>\n";
            std::cout << "            <Spectrum_ID>2</Spectrum_ID>\n";
            std::cout << "            <Anode_Material>TUNGSTEN</Anode_Material>\n";
            std::cout << "            <Filter>\n";
            std::cout << "                <Material>ALUMINUM</Material>\n";
            std::cout << "                <MaterialZ>13</MaterialZ>\n";
            std::cout << "                <Thickness_mm>0.7</Thickness_mm>\n";
            std::cout << "            </Filter>\n";
            std::cout << "            <KVP>33</KVP>\n";
            std::cout << "            <Exposure>65</Exposure>\n";
            std::cout << "            <Phantom_Thickness_mm>50</Phantom_Thickness_mm>\n";
            std::cout << "            <Density_Coefficient>4.0</Density_Coefficient>\n";
            std::cout << "        </Exposure_Settings>\n";
            std::cout << "        <Detector>\n";
            std::cout << "            <Element_Size_mm>\n";
            std::cout << "                <X>0.14</X>\n";
            std::cout << "                <Y>0.14</Y>\n";
            std::cout << "            </Element_Size_mm>\n";
            std::cout << "            <Element_Count>\n";
            std::cout << "                <X>2048</X>\n";
            std::cout << "                <Y>1664</Y>\n";
            std::cout << "            </Element_Count>\n";
            std::cout << "            <Thickness_mm>0.1</Thickness_mm>\n";
            std::cout << "            <Material>0</Material>\n";
            std::cout << "            <Bits_Allocated>16</Bits_Allocated>\n";
            std::cout << "            <Bits_Stored>14</Bits_Stored>\n";
            std::cout << "            <High_Bit>13</High_Bit>\n";
            std::cout << "            <Has_Volume_Offset>true</Has_Volume_Offset>\n";
            std::cout << "            <Volume_Offset_mm>\n";
            std::cout << "                <X>0.0</X>\n";
            std::cout << "                <Y>0.0</Y>\n";
            std::cout << "                <Z>0.0</Z>\n";
            std::cout << "            </Volume_Offset_mm>\n\n";
            std::cout << "        </Detector>\n\n";
            std::cout << "        <Acquisition_Geometry>\n\n";
            std::cout << "            <Number_Acquisitions>15</Number_Acquisitions>\n\n";
            std::cout << "            <Acquisition>\n";
            std::cout << "                <ID>0</ID>\n";
            std::cout << "                <Acquisition_UID>012.34567890.123456789012345</Acquisition_UID>\n";
            std::cout << "                <Focal_Spot>\n";
            std::cout << "                    <X_mm>-90.0</X_mm>\n";
            std::cout << "                    <Y_mm>0.0</Y_mm>\n";
            std::cout << "                    <Z_mm>694.027</Z_mm>\n";
            std::cout << "                </Focal_Spot>\n";
            std::cout << "                <Phantom_Offset>\n";
            std::cout << "                    <X_mm>0.0</X_mm>\n";
            std::cout << "                    <Y_mm>0.0</Y_mm>\n";
            std::cout << "                    <Z_mm>0.0</Z_mm>\n";
            std::cout << "                </Phantom_Offset>\n";
            std::cout << "                <Detector>\n";
            std::cout << "                    <X_mm>-143.36</X_mm>\n";
            std::cout << "                    <Y_mm>0.0</Y_mm>\n";
            std::cout << "                    <Z_mm>0.0</Z_mm>\n";
            std::cout << "                    <Orient_SX_mm>1.0</Orient_SX_mm>\n";
            std::cout << "                    <Orient_SY_mm>0.0</Orient_SY_mm>\n";
            std::cout << "                    <Orient_SZ_mm>0.0</Orient_SZ_mm>\n";
            std::cout << "                    <Orient_TX_mm>0.0</Orient_TX_mm>\n";
            std::cout << "                    <Orient_TY_mm>1.0</Orient_TY_mm>\n";
            std::cout << "                    <Orient_TZ_mm>0.0</Orient_TZ_mm>\n";
            std::cout << "                </Detector>\n";
            std::cout << "            </Acquisition>\n\n";
            
            std::cout << "            <!-- * * * NOTE: Acquisitions 1-13 have been omitted for brevity * * * -->\n\n";   
            
            std::cout << "            <Acquisition>\n";
            std::cout << "                <ID>14</ID>\n";
            std::cout << "                <Acquisition_UID>012.34567890.123456789012345</Acquisition_UID>\n";
            std::cout << "                <Focal_Spot>\n";
            std::cout << "                    <X_mm>-91.247</X_mm>\n";
            std::cout << "                    <Y_mm>0.0</Y_mm>\n";
            std::cout << "                    <Z_mm>694.027</Z_mm>\n";
            std::cout << "                </Focal_Spot>\n";
            std::cout << "                <Phantom_Offset>\n";
            std::cout << "                    <X_mm>0.0</X_mm>\n";
            std::cout << "                    <Y_mm>0.0</Y_mm>\n";
            std::cout << "                    <Z_mm>0.0</Z_mm>\n";
            std::cout << "                </Phantom_Offset>\n";
            std::cout << "                <Detector>\n";
            std::cout << "                    <X_mm>-143.36</X_mm>\n";
            std::cout << "                    <Y_mm>0.0</Y_mm>\n";
            std::cout << "                    <Z_mm>0.0</Z_mm>\n";
            std::cout << "                    <Orient_SX_mm>1.0</Orient_SX_mm>\n";
            std::cout << "                    <Orient_SY_mm>0.0</Orient_SY_mm>\n";
            std::cout << "                    <Orient_SZ_mm>0.0</Orient_SZ_mm>\n";
            std::cout << "                    <Orient_TX_mm>0.0</Orient_TX_mm>\n";
            std::cout << "                    <Orient_TY_mm>1.0</Orient_TY_mm>\n";
            std::cout << "                    <Orient_TZ_mm>0.0</Orient_TZ_mm>\n";
            std::cout << "                </Detector>\n";
            std::cout << "            </Acquisition>\n\n";
            std::cout << "        </Acquisition_Geometry>\n";
            std::cout << "    </Breast_Phantom_Designer>\n";

            return 0;  // exit program with success
        }
        else
        {
            std::cerr << "Incorrect set of arguments. Type \"" << argv[0] << " -help\" to see more.\n";
            exit(0);
        }
    }
    
    
    // Declare arguments by name
    std::string attenuationFile;
    std::string configGeomFile;
    std::string input_phantom;
    std::string comment;
    std::string random_seed;
    bool using_xml_input_file = false;

    // Instantiate Logger
    std::ofstream ofs_log("XPLProjectionSimulation.log");
    vct::Logger log(ofs_log);
    log.timestamp();
    log.setSeverityLevel(vct::Logger::LOG_INFO);    
    log.info << "Software version is " << VERSION << ".\n"; // log software version

    // Instantiate the (Singleton) VCT interface class and check input arguments
    ReadPhantom &rp = ReadPhantom::getInstance();

    // Read the config file to get organization data
    rp.readConfigFile(); // may be unnecessary since RayTracer doesn't create or modify the phantom.

    std::string arg1(argv[1]);

    if (arg1 == "-xml_input")
    {
        using_xml_input_file = true;
        std::string xml_input_file(argv[2]);
        bool success = rp.readInputXML(log, xml_input_file);
        std::cout << "rp.readInputXML(" << xml_input_file << ") returned " << (success?"true":"false") << std::endl;
        if (!success)
        {
            Usage(prog_name); // Print instructions for specifying command line arguments...
            log.fatal << "Couldn't read xml input file \"" << xml_input_file << "\"\n";
            ofs_log.close();
            return 0;          // ...and exit.
        }

        attenuationFile = rp.getAttenFile();
        configGeomFile  = rp.getConfigFile();
        input_phantom   = rp.getPhantomFilename();
        comment         = rp.getComment();
        random_seed     = rp.getSeed();
    }
    else
    {
        Usage(prog_name); // Print instructions for specifying command line arguments...
        log.fatal << "Incorrect arguments on command line\n";
        ofs_log.close();
        return 0;          // ...and exit.
    }


    // Check for existence of specified attenuation XML file
    if (!using_xml_input_file)
    {
        std::ifstream ifs1(attenuationFile);
        if (!ifs1)
        {
            std::cerr << "\nAttenuation XML file, " << attenuationFile << ", doesn't appear to exist.\n" << std::endl;// * EXITS HERE WITHOUT THE CRASH *
            log.fatal << "Attenuation XML file, " << attenuationFile << ", doesn't appear to exist.\n";
            return 0;
        }
        ifs1.close();

        // Check for existence of specified configuration XML file
        std::ifstream ifs2(configGeomFile);
        if (!ifs2)
        {
            std::cerr << "\nConfiguration XML file, " << configGeomFile << ", doesn't appear to exist.\n" << std::endl;// * EXITS HERE WITHOUT THE CRASH *
            log.fatal << "Configuration XML file, " << configGeomFile << ", doesn't appear to exist.\n";
            return 0;
        }
        ifs2.close();
    }

    // Check for existence of specified phantom file
    std::ifstream ifs3(input_phantom);
    if (!ifs3)
    {
        std::cerr << "\nPhantom file " << input_phantom << " doesn't appear to exist.\n" << std::endl;// * EXITS HERE WITHOUT THE CRASH *
        log.fatal << "Phantom file " << input_phantom << " doesn't appear to exist.\n";
        return 0;
    }
    ifs3.close();

    // Verify that a comment/prefix was provided
    if (!using_xml_input_file && comment.size() < 1)
    {
        std::cerr << "\nComment/Prefix not provided\n" << std::endl; // not much of a test here
        log.fatal << "Comment/Prefix not provided\n";
        return 0;
    }

    // Verify that the random number seed is a valid integer
    int seed = 0;
    std::istringstream iss(random_seed);
    iss >> seed;
    if (iss.fail())
    {
        std::cerr << "\nRandom number seed, " << random_seed << ", is not a valid integer.\n" << std::endl; // not much of a test here
        log.fatal << "Random number seed, " << random_seed << ", is not a valid integer.\n";
        return 0;
    }

    std::cout << "Instantiating XPLRayTracing" << std::endl;
    XPLRayTracing xplRayTrace;

    // Create an array of arguments only (exclude program name)
    int acnt = argc - 1;
    int i, len = 0;
    char **agv = new char *[acnt];

    for(i=0; i<acnt; ++i)
    {
        len = strlen(argv[i+1]);
        agv[i] = new char[len + 1];
        strcpy(agv[i], argv[i+1]);
    }

    int status = xplRayTrace.SimulationSetup(acnt, agv);  // Simulation and Volume are dynamically instantiated here

    // Free up temporarily allocated dynamic memory
    for(i=0; i<acnt; ++i) delete [] agv[i];
    delete [] agv;

    if (status != 0)
    {
       Usage(prog_name); // Print instructions for specifying command line arguments
       return 0;         // ...and exit.
    }


    short width = xplRayTrace.GetWidth();
    short height = xplRayTrace.GetHeight();
    log.info << "Detector size is " << width << "x" << height << '\n';

    gdcm::ImageReader reader; // Reader persists across all acquisitions

    // Open the existing Dicom file and use as a template
    std::string dicom_filename = FindDicomTemplate(configGeomFile);
    log.info << "DICOM template: " << dicom_filename << '\n';


    reader.SetFileName(dicom_filename.c_str());
    if (!reader.Read())
    {
        std::ostringstream oss;
        oss << "Could not read input Dicom file: " << dicom_filename;
        std::string errmsg(oss.str());
        std::cerr << "\n\n* * * Fatal Error: " << errmsg << " * * *\n" << std::endl;
        log.fatal << "\n\n* * * Fatal Error: " << errmsg << " * * *\n" << '\n';

        // Shut down logging
        log.timestamp();
        log.setSeverityLevel(vct::Logger::LOG_NONE);
        ofs_log.close();
        return 1;
    }

    // Build Output folder name (Output-<prefix>-<date>-<time>)
    std::string outputDir("./Output-");
    if (using_xml_input_file)
    {
        outputDir = rp.getOutputFolder();
    }
    else
    {

        std::time_t result = std::time(nullptr);
        struct tm *gmt = gmtime(&result);
        outputDir += std::string(comment);
        outputDir += std::string("_");
        std::ostringstream oss;
        oss << std::setfill('0')
            << gmt->tm_year + 1900              << '-'
            << std::setw(2) << gmt->tm_mon + 1  << '-' // month ranges 0-11
            << std::setw(2) << gmt->tm_mday     << '_'
            << std::setw(2) << gmt->tm_hour     << '-'
            << std::setw(2) << gmt->tm_min      << '-' << gmt->tm_sec;
        outputDir += oss.str();
    }

    // Output to this folder
    #if defined(_WIN32) || defined(_WIN64) // if MS Visual Studio, explicitly
    std::wstring ms_wstring(outputDir.begin(), outputDir.end());
    CreateDirectory(ms_wstring.c_str(), NULL);
    #elif defined(__MINGW32__)             // MinGW on Windows
    mkdir(outputDir.c_str());
    #else                                  // everyone else
    mode_t mode = 0774;
    mkdir(outputDir.c_str(), mode);
    #endif

    log.info << "Created folder " << outputDir << " to receive DICOM projection files\n";


    // Allocate buffer we will use to generate all acquisition images
    size_t bufferSize = width * height * sizeof(unsigned short); // 2 bytes per pixel
    unsigned char *buffer = new unsigned char[bufferSize];                         // 1 byte per pixel
    unsigned short* imageBuf = (unsigned short *)&buffer[0];     // some utils require unsigned short

    // Get Acquisition Count (comes from accumulation of acquisitions in config XML file)
    int acqCount = xplRayTrace.GetAcquisitionCount();
    
    vct::CommonLibrary clib;
    //unsigned int stopwatch_start = clock();
    clib.start_timer();

    // Iterate over all acquisitions (determined by XML config file entries)
    for (int i = 0; i < acqCount; ++i)
    {
        gdcm::ImageWriter writer; // are we dying from static deallocation of this turkey???? DH 2017-05-15 No.

        std::ostringstream acquisition;
        acquisition << " [" << (i + 1) << " of " << acqCount << "]";
        progressBar.add_msg(acquisition.str());

        // Execute Acquistion run 
        log.info << "Commencing simulation of acquisition " << i << " of " << acqCount << '\n';
        int status = xplRayTrace.SimulationRun(imageBuf, i); // show progress bar while creating raytraced image
        if (status != 0) break;
        log.info << "Acq " << i << " is complete. About to create corresponding DICOM file\n";
        
        // Store the pixel data just generated by the run
        gdcm::File &infile = reader.GetFile();
        gdcm::DataSet &ds = infile.GetDataSet();
        
        // There's no discernable difference in choosing either of these: - DH 2016-06-07
        gdcm::Image &image = reader.GetImage(); // borrow image from template file
        
        bool overlaysInImage = image.AreOverlaysInPixelData();
        unsigned long buffer_length = image.GetBufferLength();

        double vector[3];

        // Declare private tag we need to find:
        gdcm::Tag tagUpennId( 0x6d0e, 0x000e); //, "UPENN" );
        gdcm::Tag XraySrcPos( 0x6d0e, 0x0e01); //, "X-RAY SOURCE POSITION" );
        gdcm::Tag XrayDetPos( 0x6d0e, 0x0e02); //, "DETECTOR POSITION" );
        gdcm::Tag DetNormalZ( 0x6d0e, 0x0e03); //, "DETECTOR NORMAL Z VECTOR" );
        gdcm::Tag DetNormalX( 0x6d0e, 0x0e04); //, "DETECTOR NORMAL X VECTOR" );
        gdcm::Tag DetNormalY( 0x6d0e, 0x0e05); //, "DETECTOR NORMAL Y VECTOR" );

        if (ds.FindDataElement(tagUpennId))
        {
            const char content[] = "UPENN";
            gdcm::DataElement de_tagUpennId = ds.GetDataElement(tagUpennId);
            de_tagUpennId.SetByteValue(content, (uint32_t)strlen(content));
            ds.Replace(de_tagUpennId);
        }
        else
        {
            std::cerr << "Could not find UPENN tag" << std::endl;
            log.error << "Could not find UPENN tag\n";
        }

        if (ds.FindDataElement(XraySrcPos))
        {
            gdcm::DataElement de_XraySrcPos = ds.GetDataElement(XraySrcPos);
        }
        else
        {
            std::cerr << "Could not find XraySrcPos tag" << std::endl;
            log.error << "Could not find XraySrcPos tag\n";
        }
        
        XPLAcquisition acq = xplRayTrace.GetAcquisition(i);
        float volumeOffsetZ = xplRayTrace.sim->machine->getDetector()->GetOffsetZ();

        // Determine and set the focal point position (source) for RTT purposes:  -Y, X, Z
        WriteVectorToDicom(ds, log, 0x6d0e, 0x0e01, "Source position", 6,
                          -acq.SourceY * 10.0,
                           acq.SourceX * 10.0,
                           acq.SourceZ * 10.0 - volumeOffsetZ * 10.0);       //BB and TV 05/09/18 + acq.volumeOffsetZ * 10

        log.info << "Writing source position (0x6d0e, 0x0e01) as "
                 << -acq.SourceY * 10.0 << " / " <<  acq.SourceX * 10.0 << " / " <<  acq.SourceZ * 10.0 - volumeOffsetZ * 10.0 << "\n";

        // Determine and set the detector position (location) for RTT purposes:  -Y, X, Z
        // * PLUG IN EXPLICIT -250.88 - TEMPORARY TO MAKE RTT RECONSTRUCTION WORK!!!!!! DH 2015-06-16 *
        //                      |
        //                      |
        //                      V
        WriteVectorToDicom(ds, log, 0x6d0e, 0x0e02, "Detector position", 3,
                           -height * xplRayTrace.GetDetectorSizeX() * 10.0,  //BB and TV 05/02/18 -height * xplRayTrace.GetDetectorSizeX() * 10.0
                           acq.DetectorLocationX * 10.0,
                           acq.DetectorLocationZ * 10.0);

        log.info << "Writing Detector position (0x6d0e, 0x0e02) as "
                 << -height * xplRayTrace.GetDetectorSizeX() * 10.0 
                 << " / " << acq.DetectorLocationX * 10.0 << " / " 
                 << acq.DetectorLocationZ * 10.0 << "\n";
        
        // Determine and set the detector Z vector (orientation) for RTT purposes
        WriteVectorToDicom(ds, log, 0x6d0e, 0x0e03, "Detector normal Z vector", 5,
                           acq.DetectorRX,
                           acq.DetectorRY,
                           acq.DetectorRZ);

        log.info << "Writing Detector normal Z vector (0x6d0e, 0x0e03) as "
                 << acq.DetectorRX << " / " << acq.DetectorRY << " / " << acq.DetectorRZ << "\n";
        
        // Determine and set the detector X vector (orientation) for RTT purposes
        WriteVectorToDicom(ds, log, 0x6d0e, 0x0e04, "Detector normal X vector", 5,
                            acq.DetectorSX,
                            acq.DetectorSY,
                            acq.DetectorSZ);

        log.info << "Writing Detector normal X vector (0x6d0e, 0x0e04) as "
                 << acq.DetectorSX << " / " << acq.DetectorSY << " / " << acq.DetectorSZ << "\n";
        
        // Determine and set the detector Y vector (orientation) for RTT purposes
        WriteVectorToDicom(ds, log, 0x6d0e, 0x0e05, "Detector normal Y vector", 5,
                           acq.DetectorTX,
                           acq.DetectorTY,
                           acq.DetectorTZ);

        log.info << "Writing Detector normal Y vector (0x6d0e, 0x0e05) as "
                 << acq.DetectorTX << " / " << acq.DetectorTY << " / " << acq.DetectorTZ << "\n";
        
        #if defined(REMOVE_UNUSED_TAG)
        // Remove an unused tag from the template Dicom dateset
        gdcm::Tag tagUnused(0x6d0e,0x0e06);
        if (ds.FindDataElement(tagUnused))
        {
            ds.Remove(tagUnused);
        }
        #endif
        
        gdcm::Attribute<0x0020,0x0012> atAcqNum;      // "Acquisition Number"
        atAcqNum.SetValue(i);
        ds.Replace(atAcqNum.GetAsDataElement());

        gdcm::Attribute<0x0020,0x0013> atImageNum;    // "Image Number"
        atImageNum.SetValue(i);
        ds.Replace(atImageNum.GetAsDataElement());

        gdcm::Attribute<0x0020,0x0062> laterality;    // "Laterality"
        laterality.SetValue("R");
        ds.Replace(laterality.GetAsDataElement());

        gdcm::Attribute<0x0020,0x1002> atImagesInAcq; // "Images in Acquisition"
        atImagesInAcq.SetValue(acqCount);
        ds.Replace(atImagesInAcq.GetAsDataElement());

        gdcm::Attribute<0x0028,0x0010> atRows;        // "Rows"
        atRows.SetValue(width);
        ds.Replace(atRows.GetAsDataElement());

        gdcm::Attribute<0x0028,0x0011> atColumns;     // "Columns"
        atColumns.SetValue(height);
        ds.Replace(atColumns.GetAsDataElement());

        gdcm::Attribute<0x0018, 0x0015> atBodyPartExamined; //" Body Part Examined"
        atBodyPartExamined.SetValue("BREAST");
        ds.Replace(atBodyPartExamined.GetAsDataElement());

        gdcm::Attribute<0x0028, 0x0030> atPixelSpacing;     // "Pixel Spacing"    //BB 04/12/2018
        atPixelSpacing.SetValue(xplRayTrace.GetDetectorSizeX()*10, 0);
        atPixelSpacing.SetValue(xplRayTrace.GetDetectorSizeY()*10, 1);
        ds.Replace(atPixelSpacing.GetAsDataElement());

        (void) SetField(ds, 0x0018, 0x1020, gdcm::VR::CS, VERSION);

        gdcm::Attribute<0x0018, 0x1110> atDistSrcToDet;    // "Distance Source to Detector"
        atDistSrcToDet.SetValue(double(acq.DistanceSrcToDet) * 10.0); // CM to mm
        ds.Replace(atDistSrcToDet.GetAsDataElement());
        
        // Address the image
        unsigned short bitsAllocated = xplRayTrace.GetBitsAllocated();
        unsigned short bitsStored    = xplRayTrace.GetBitsStored();
        unsigned short highBit       = xplRayTrace.GetHighBit();

        gdcm::PixelFormat pf = gdcm::PixelFormat::UINT16;
        pf.SetScalarType(gdcm::PixelFormat::UINT16); // Set this before setting bitsAllocated, bitsStored and highBit (it overwrites them) - DH 2016-06-07
        pf.SetBitsAllocated(bitsAllocated);
        pf.SetBitsStored(bitsStored);
        pf.SetHighBit(highBit);
        pf.SetPixelRepresentation(0);
        pf.SetSamplesPerPixel(1);

        image.SetPixelFormat(pf);
        image.SetNumberOfDimensions(2);
        image.SetDimension(0, (unsigned int)height);
        image.SetDimension(1, (unsigned int)width);
        image.SetPhotometricInterpretation(gdcm::PhotometricInterpretation::MONOCHROME1);
        image.SetRows(width);
        image.SetColumns(height);
        image.SetTransferSyntax(gdcm::TransferSyntax::ExplicitVRLittleEndian);
        image.SetSpacing(0, xplRayTrace.GetPixelSizeY());
        image.SetSpacing(1, xplRayTrace.GetPixelSizeX());
        
        writer.SetImage(image);

        gdcm::DataElement pixeldata(gdcm::Tag(0x7fe0, 0x0010));
        pixeldata.SetByteValue(reinterpret_cast<char *>(buffer), (uint32_t)bufferSize);   // This is where the new image data is applied
        image.SetDataElement(pixeldata);
        
        // Set Bits Allocated, Bits Stored and High BIt
        gdcm::Attribute<0x0028,0x0100> atBitsAllocated;   // "Bits Allocated"
        atBitsAllocated.SetValue(bitsAllocated);
        ds.Replace(atBitsAllocated.GetAsDataElement());

        gdcm::Attribute<0x0028,0x0010> atBitsStored;      // "Bits Stored"
        atBitsStored.SetValue(bitsStored);
        ds.Replace(atBitsStored.GetAsDataElement());

        gdcm::Attribute<0x0028,0x0011> atHighBit;         // "High Bit"
        atHighBit.SetValue(highBit);
        ds.Replace(atHighBit.GetAsDataElement());

        // Write the Dicom file out for this acquisition
        // Note: writer destructor removes file; writer must therefore persist w/reader
        // ----  .. since it uses reader's file - DH 2014-11-10
        std::ostringstream outfilename;
        outfilename << outputDir << '/' << comment << '_' << i << ".dcm";

        writer.SetFile(infile);
        writer.SetFileName(outfilename.str().c_str());
        writer.SetCheckFileMetaInformation(true); // both values have no discernable effect - DH 2016-06-07


        // Attempt to Remove extra fields (GDCM ignores these attempts) - DH 2016-06-07
        ds.Remove(gdcm::Tag(0x08, 0x020)); // Time
        ds.Remove(gdcm::Tag(0x08, 0x030)); // Date
        ds.Remove(gdcm::Tag(0x08, 0x050)); // Accession ID
        ds.Remove(gdcm::Tag(0x08, 0x090)); // Referring Physician

        ds.Remove(gdcm::Tag(0x10, 0x030)); // Patient birth date
        ds.Remove(gdcm::Tag(0x10, 0x040)); // Patient Gender

        ds.Remove(gdcm::Tag(0x20, 0x010)); // Study ID
        ds.Remove(gdcm::Tag(0x20, 0x011)); // Series #

        if (!writer.Write())
        {
            std::string errmsg("Failure to Write dicom file indicated");
            std::cerr << "\n\n* * * Fatal Error: " << errmsg << " * * *\n" << std::endl;
            log.fatal << "\n\n* * * Fatal Error: " << errmsg << " * * *\n" << '\n';

            // Shut down logging
            log.timestamp();
            log.setSeverityLevel(vct::Logger::LOG_NONE);
            ofs_log.close();
            return 1;
        }
        else if (using_xml_input_file)
        {
            // Read Date and Time from DICOM file
            vct::Date date;
            vct::Time time;
            gdcm::Tag dateTag(0x8, 0x20);
            gdcm::Tag timeTag(0x8, 0x30);
            const gdcm::DataElement &date_de = ds.GetDataElement(dateTag);
            const gdcm::DataElement &time_de = ds.GetDataElement(timeTag);

            std::ostringstream date_oss;
            date_oss << date_de.GetValue();
            std::ostringstream time_oss;
            time_oss << time_de.GetValue();

            // eg: Date Value is: "20170717", Time Value is: "185651.957996
            rp.addDicomDate(date_oss.str());
            rp.addDicomTime(time_oss.str());
        }

        log.timestamp();
        
    } // next acq

    delete [] buffer;


    // Output summary of processing
    if (using_xml_input_file) rp.generateReport();

    const int namelen = 64;
    char name[namelen];
    std::string machineName("unknown computer");

    if (GetMyHostName(name, len)) machineName = std::string(name);
    time_t rawtime;
    time(&rawtime);
    struct tm *ptm = gmtime(&rawtime);
    unsigned int stopwatch_stop = clock();

    std::ostringstream oss_summary;

    oss_summary << "Completed on " << machineName << " at UTC " << asctime(ptm)
                << "Elapsed Time: " << clib.get_elapsed_time() << " msec.";

    std::cout << "\n\n" << oss_summary.str() << std::endl;
    log.info << oss_summary.str() << '\n';
    ofs_log.flush();

    // Shut down logging
    log.timestamp();
    log.setSeverityLevel(vct::Logger::LOG_NONE);
    ofs_log.close();

    //std::cout << "About to end program" << std::endl;
    clib.sleep(1000);
    return 0; // GDCM buffer overrun check disable via "/GS-" under Configuration Properties->C/C++->Code Generation->Security Check
}


#pragma region Documentation
/// <summary>
/// Ensure optional command line arguments are last while preserving the order of required arguments
/// </summary>
/// <param name="arg">The command line arguments specified.</param>
/// <return>The number of required arguments found.</return>
#pragma endregion
int OrganizeArgs(int argc, char *argv[])
{
    int numreqd = 0;
    int  foundx = 0;
    bool foundr = false;

    // Not efficient, but effective
    for(int i = 1; i < argc; ++i)
    {
        if (*argv[i] == '/')   // Note: on other OS this will be "-" or "--"
        {
            std::string option(argv[i]);     // save current option

            // shift remainder of options forward
            for (int j = i; j < argc - 1; ++j) strcpy(argv[j], argv[j + 1]);
            strcpy(argv[argc - 1], option.c_str());  // place this option at the end

            // retest at this index to get an accurate count
            if (!((*argv[i]) == '/')) numreqd++;
        }
        else
        {
            numreqd++;
        }
    }

    return numreqd;
}


#pragma region Documentation
/// <summary>
/// Print usage summary
/// </summary>
/// <param name="prog_name">The name of the program.</param>
#pragma endregion
void Usage(std::string prog_name)
{
    std::cout << "\nUsage:   " << prog_name << " -xml_input <xml_input_file>.xml\n\n";
    std::cout << "   or:   "   << prog_name << " -version   ..to check program version info. \n\n";
    std::cout << "   or:   "   << prog_name << " -example   ..to see an example xml_input file.\n\n";

    return;
}


#pragma region Documentation
/// <summary>
/// Open the Dicom template file corresponding to the specified equipment
/// </summary>
/// <param name="args[0]">The DicomFile object to be used.</param>
#pragma endregion
std::string FindDicomTemplate(const std::string &xml_file)
{
    std::string retval("hologic.dcm");

    // Only check for DICOM template filename if xml_file provided (when NOT using xml_input)
    if (xml_file.size() > 0)
    {
        pugi::xml_document doc;

        if (doc.load_file(xml_file.c_str()))
        {
            pugi::xml_node tools = doc.first_child();
            if (tools)
            {
                pugi::xml_node node = tools.child("Machine");
                if (node)
                {
                    pugi::xml_node childNode = node.child("Name");
                    if (childNode)
                    {
                        std::stringstream(childNode.first_child().value()) >> retval;
                        retval += ".dcm";
                    } else std::cerr << "couldn't find \"Name\"" << std::endl;
                } else std::cerr << "couldn't find \"Machine\"" << std::endl;
            } else std::cerr << "couldn't find first child" << std::endl;
        } else std::cerr << "couldn't load xml file " << xml_file << std::endl;
    }

    return retval;
}


#pragma region Documentation
/// <summary>
/// Portable function to retrieve the name of the computer.
/// </summary>
/// <param name="name">A buffer to receive the computer name.</param>
/// <param name="name">The size of the computer name buffer.</param>
#pragma endregion
bool GetMyHostName(char *name, int len)
{
    bool retval = false;

    #if defined(_MSC_VER)

    unsigned long name_length = len + 1;
    wchar_t *widename = new wchar_t[len];
    retval = GetComputerName(widename, &name_length) != 0;
    for(int i=0;i<len;++i) name[i] = widename[i];
    name[len] = '\0';

    #else

    char hostname[len];
    gethostname(hostname, len);
    strncpy(name, hostname, len);
    name[len] = '\0';    
    retval = true;

    #endif

    return retval;
}


#pragma region Documentation
/// <summary>
/// Write a vector of three real numbers to the specified location.
/// </summary>
/// <param name="ds">Dicom dataset to use.</param>
/// <param name="log">Reference to vct Logger.</param>
/// <param name="d1">First part of the dicom tag.</param>
/// <param name="d1">Second part of the dicom tag.</param>
/// <param name="d1">The element component of the dicom tag.</param>
/// <param name="msg">Terse description of the attribute.</param>
/// <param name="places">Number of places to round to.</param>
/// <param name="v1">First real number value to encode.</param>
/// <param name="v2">Second real number value to encode.</param>
/// <param name="v3">Third real number value to encode.</param>
/// <return>Status where success is True (otherwise false). </return>
#pragma endregion
bool WriteVectorToDicom(gdcm::DataSet &ds, vct::Logger &log, uint16_t d1, uint16_t d2, std::string msg, int places, double v1, double v2, double v3)
{
    bool retval = false;

    gdcm::Tag tagVector(d1, d2);
    if (!ds.FindDataElement(tagVector))
    { 
        std::ostringstream oss;
        oss << "Couldn't find tag 0x" << std::hex << d1 << ", 0x" << std::hex << d2 << " (" << msg << ")";
        std::string errmsg(oss.str());
        std::cerr << "\n\n* * * error: " << errmsg << " * * *\n" << std::endl;
        log.error << errmsg << "\n";
        retval = false;
    }
    else
    { 
        gdcm::DataElement deVector = ds.GetDataElement(tagVector); 

        #if defined(USE_NON_STREAM_IO_ANACHRONISM)

        char buf[32];
        sprintf_s<32>(buf, "%.2f\\%f.2\\%.2f", v1, v2, v3);
        deVector.SetByteValue((const char *)buf, strlen(buf));

        #else
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(places) << float(v1) << "\\" << float(v2) << "\\" << float(v3);
        std::string ostr(oss.str()); 
        
        // Remove trailing space if present - iff it exists
        auto itr = std::find(ostr.begin(), ostr.end(), ' ');
        if (itr != ostr.end()) // this test is required on linux
        {
           ostr.erase(itr);    // remove any space characters
        }
        deVector.SetByteValue((const char *)ostr.c_str(), (uint32_t)ostr.length());

        #endif
        
        ds.Replace(deVector);
        retval = true;
    }

    return retval;
}


#pragma region Documentation
/// <summary>
/// Set a DICOM field and its value regardless of whether it already exists.
/// </summary>
/// <param name="d1">Dicom dataset to use.</param>
/// <param name="d2">The group component of the dicom tag.</param>
/// <param name="vr">The established Value Representation for this field.</param>
/// <param name="value">A string containing the value(s) to be set in the field.</param>
#pragma endregion
std::string SetField(gdcm::DataSet &ds, uint16_t d1, uint16_t d2, gdcm::VR vr, std::string value)
{
    std::string retval;

    gdcm::Tag tag(d1, d2);
    if (tag.IsIllegal())
    {
        retval = "Tag is illegal!\n";
    }
    else
    {
        gdcm::DataElement de = ds.GetDataElement(tag);
        if (de.IsEmpty())
        {
            // Create a new data element for this tag
            gdcm::DataElement de(tag);
            de.SetVR(vr);
            de.SetByteValue(value.c_str(), (uint32_t)value.size());
            if (de.IsEmpty())
            {
                std::ostringstream oss;
                oss << "Data Element for " << std::hex << d1 << ", " << d2 << std::dec << " is still empty.\n";
                retval += oss.str();
            }
            else
            {
                ds.Insert(de);
            }
        }
        else
        {
            de.SetByteValue(value.c_str(), (uint32_t)value.size());
            ds.Replace(de);
        }
    }

    return retval;
}
