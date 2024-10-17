// XPLRayTracing.cpp : Defines the exported functions for the DLL application.

#include "XPLRayTracing.h"

#include "RayTracer.h"
#include <iostream>
#include <string>
#include "Detector.h"
#include "BooneMaterialTable.h"
#include "AttenuationTable.h"
#include "Image.h"
#include <random>
#include "pugixml.hpp"
#include "Simulation.h"

#include "ReadPhantom.h"

#if !defined(NO_CPU)
#include "SimulationCPU.h"
#endif

#if !defined(NO_OPENCL)
#include "SimulationOCL.h"
#endif

#include "ProgressBar.h"
extern ProgressBar progressBar;




XPLRayTracing::XPLRayTracing()
    : simtype(CPU),
      sim(nullptr), acq(0),
      height(0), width(0),
      sizeX(0.0f), sizeY(0.0f),
      bitsAllocated(0),
      bitsStored(0),
      highBit(0),
      rp(nullptr)
{}


XPLRayTracing::~XPLRayTracing()
{
    if (sim != nullptr) delete sim;
}


int XPLRayTracing::SimulationSetup(int argc, char* argv[])
{
    // Required Arguments
    std::string atten_file;
    std::string config_file;
    std::string phantom_in;
    std::string comment_prefix;
    std::string seed;

    bool simTypeNotSpecified = true; // For testing if /cpu and /gpu BOTH specified

    bool use_noise = true;           // Default is to use noise
    bool set_elec_noise = false;     // Default is not to override electric noise
    bool set_gain = false;           // Default is not to override gain
    bool set_minGray = false;        // Default is not to override minimum gray scale value
    bool set_prop_const = false;     // Default is not to override proportional constant
    bool set_useScaling = false;     // Default is not to use scaling
    bool no_inverse_sq = false;      // Default is to apply inverse square law

    bool   set_mAs = false;          // Default is to not override mAs - 2015_05_22 DH 
    double mAsValue = 0.0;           // Used when overriding deflt mAs - 2015_05_22 DH 

    double gainValue = 0.0;
    double elecValue = 0.0;
    double propConstValue = 0.0;

    unsigned short minGrayscale = 50u;

    ReadPhantom &rp = ReadPhantom::getInstance();

    if (rp.getXmlInputUsed())
    {
        atten_file     = rp.getAttenFile();
        config_file    = rp.getConfigFile();
        phantom_in     = rp.getPhantomFilename();
        comment_prefix = rp.getComment();

        use_noise      = rp.getUseNoise();
        no_inverse_sq  = !rp.getUseInvSq();
        elecValue      = rp.getElecNoiseValue();

        gainValue      = rp.getGainValue();
        mAsValue       = rp.getMasValue();
        propConstValue = rp.getPropConstValue();        
        seed           = rp.getSeed();
    }
    else
    {
        atten_file     = argv[0];
        config_file    = argv[1];
        phantom_in     = argv[2];
        comment_prefix = argv[3];
        seed           = argv[4];

        std::cout << "Attentuation file: " << std::string(atten_file)     << '\n'
                  << "Config file: "       << std::string(config_file)    << '\n'
                  << "Input volume: "      << std::string(phantom_in)     << '\n' 
                  << "Output prefix: "     << std::string(comment_prefix) << '\n'
                  << "Seed: "              << std::string(seed)           << '\n';

        rp.setCmdLineArgs(atten_file, config_file, phantom_in, comment_prefix, seed);

        // Validate Optional Arguments
        for(int ndx = 5; ndx < argc; ++ndx)
        {
            std::string optArg(argv[ndx]);

            // Examine optional command line arguments
            if (optArg == "/nonoise" || optArg == "-nonoise")
            {
                if (set_elec_noise)
                {
                    std::cout << "WARNING: Inconsistency found. Disabling noise after specifying noise coefficient" << std::endl;
                }
                use_noise = false;
                rp.setNoise(optArg, use_noise);
            }
            else if (optArg.substr(0,5) == "/elec" || optArg.substr(0,5) == "-elec") // dflt=3.0 (see Simulation.h constructor)
            {
                if (!use_noise)
                {
                    std::cout << "WARNING: Inconsistency found. Elec noise value specified while disabling noise" << std::endl;
                }
                set_elec_noise = true;
                std::string::size_type n = optArg.find('=');
                if (n != std::string::npos)
                {
                   std::istringstream iss(optArg.substr(n+1));
                   iss >> elecValue; 
                   rp.setElec(optArg, elecValue);
                }
            }
            else if (optArg.substr(0,5) == "/gain" || optArg.substr(0,5) == "-gain") // dflt=0.2 ..is this gain? or 1/gain?
            {
                set_gain = true;
                std::string::size_type n = optArg.find('=');
                if (n != std::string::npos)
                {
                   std::istringstream iss(optArg.substr(n+1));
                   iss >> gainValue;
                   rp.setGain(optArg, gainValue);
                }
            }
            else if (optArg.substr(0,4) == "/mAs" || optArg.substr(0,4) == "-mAs") // dflt=12.92
            {
                set_mAs = true;
                std::string::size_type n = optArg.find('=');
                if (n != std::string::npos)
                {
                   std::istringstream iss(optArg.substr(n+1));
                   iss >> mAsValue;
                   rp.setMas(optArg, mAsValue);
                }
            }
            else if (optArg.substr(0,10) == "/propconst" || optArg.substr(0,10) == "-propconst" )
            {
                set_prop_const = true;
                std::string::size_type n = optArg.find('=');
                if (n != std::string::npos)
                {
                   std::istringstream iss(optArg.substr(n+1));
                   iss >> propConstValue;
                   rp.setPropConst(optArg, propConstValue);
                }
            }
            else if (optArg.substr(0,11) == "/mingrayval" || optArg.substr(0,11) == "-mingrayval")
            {
                set_minGray = true;
                std::string::size_type n = optArg.find('=');
                if (n != std::string::npos)
                {
                   std::istringstream iss(optArg.substr(n+1));
                   iss >> minGrayscale;
                   rp.setMinGray(optArg, minGrayscale);
                }
            }
            else if (optArg.substr(0, 11) == "/usescaling" || optArg.substr(0, 11) == "-usescaling" || optArg.substr(0, 11) == "/useScaling" || optArg.substr(0, 11) == "-useScaling")
            {
                set_useScaling = true;
            }
            else if (optArg.substr(0, 11) == "/randquad" || optArg.substr(0, 11) == "-randquad")
            {
                Acquisition::randomize_element_quad = true;
            }
            else if (optArg.substr(0, 8) == "/noinvsq" || optArg.substr(0, 8) == "-noinvsq")
            {
                no_inverse_sq = true;
            }
            else
            {
                std::cerr << "Unrecognized argument \"" << optArg << "\"" << std::endl;
                return -1;
            }
        } // end of argument validation

    } // end if (rp != nullptr && rp.getXmlInputUsed())


    // Create simulation type corresponding with active project (CPU or GPU (OpenCL))
    // ...ignoring incomplete TomoSimulation and CPUPVSimulation implementations for now

    #if !defined(NO_OPENCL)

        // OpenCL
        simtype = GPU;
        if (rp.getXmlInputUsed())
        {
            // Arguments and details were specified in a XML Input file (specified vis -xml_input <filename>)
            sim = new SimulationOCL(seed);
        }
        else
        {
            // Arguments were explicitly specified on the command line
            sim = new SimulationOCL(config_file, atten_file, phantom_in, seed);
        }

        // Override default image function (specified in Simulation constructor, which is invoked by the SimulationOCL constructor)
        sim->image_func = [](int x, int y, int w, int h){ return x * h + h - y - 1; };

    #else

        // CPU ONLY
        simtype = CPU;
        if (rp.getXmlInputUsed())
        {
            // Arguments and details were specified in a XML Input file (specified vis -xml_input <filename>)
            sim = new SimulationCPU(seed);
        }
        else
        {
            // Arguments were explicitly specified on the command line
            sim = new SimulationCPU(config_file, atten_file, phantom_in, seed);
        }

        // Override default image function (specified in Simulation constructor, which is invoked by the SimulationCPU constructor)
        sim->image_func = [](int x, int y, int w, int h){ return x * h + h - y - 1; };

    #endif

    if (sim == NULL)
    {
        std::cerr << "Unable to create simulation object" << std::endl;
        return -1;
    }

    // Set value in sim now that sim type has been established

    if (rp.getXmlInputUsed())
    {
        use_noise      = rp.getUseNoise();
        set_elec_noise = rp.getElecNoiseSet();
        set_gain       = rp.getGainSet();
        set_mAs        = rp.getMasSet();
        set_prop_const = rp.getPropConstSet();
        set_minGray    = rp.getMinGraySet();
        set_useScaling = rp.getUseScaling();
        no_inverse_sq  = rp.getUseInvSq();
    }

    if (!use_noise)     sim->setUseNoise(false);
    if (set_elec_noise) sim->setElecNoise(elecValue);
    if (set_gain)       sim->setGain(gainValue);
    if (set_mAs)        sim->setmAs(mAsValue);
    if (set_prop_const) sim->setPropConst(propConstValue);
    if (set_minGray)    sim->setMinGray(minGrayscale);
    if (set_useScaling) sim->setUseScaling(true);
    if (no_inverse_sq)  sim->setNoInverseSq(true);
        
    width = sim->machine->getDetector()->elementCountX;
    height = sim->machine->getDetector()->elementCountY;

    progressBar.print(0);

    return 0; // indicate success
}


int XPLRayTracing::SimulationRun(unsigned short* imageBuffer, int id)
{
    int retval = -1;
    if (sim != nullptr)
    {
        if (sim->simulate(id, imageBuffer))
        {
            progressBar.print(100);
            retval = EXIT_SUCCESS; // indicate success
        }
        else retval = EXIT_FAILURE;
    }

    return retval;
}


std::string XPLRayTracing::GetSimType()
{
    std::string retval;

    switch(simtype)
    {
    case CPU: retval = "CPU"; break;
    case GPU: retval = "GPU"; break;
    }
    return retval;
}


int XPLRayTracing::GetAcquisitionCount()
{
    if (sim != nullptr) acq = static_cast<int>(sim->machine->acquisitionGeometry.size());
    return acq;
}


XPLAcquisition XPLRayTracing::GetAcquisition(int i)
{
    
    XPLAcquisition acq;    
    
    if ((i >= 0) && (i < GetAcquisitionCount()))
    {
        SimulationConfig a = SimulationGetConfig(i);
        
        acq.DistanceSrcToDet  = a.DistanceSrcToDet;
        acq.DetectorLocationX = a.DetectorLocation[0];
        acq.DetectorLocationY = a.DetectorLocation[1];
        acq.DetectorLocationZ = a.DetectorLocation[2];
        
        acq.DetectorRX = a.DetectorZDir[0];
        acq.DetectorRY = a.DetectorZDir[1];
        acq.DetectorRZ = a.DetectorZDir[2];
        
        acq.DetectorSX = a.DetectorXDir[0];
        acq.DetectorSY = a.DetectorXDir[1];
        acq.DetectorSZ = a.DetectorXDir[2];
        
        acq.DetectorTX = a.DetectorYDir[0];
        acq.DetectorTY = a.DetectorYDir[1];
        acq.DetectorTZ = a.DetectorYDir[2];
        
        acq.SourceX = a.SourceLocation[0];
        acq.SourceY = a.SourceLocation[1];
        acq.SourceZ = a.SourceLocation[2];
    }
    return acq;
}



float XPLRayTracing::GetDetectorSizeX()
{
    if (sim != nullptr) sizeX = static_cast<float>(sim->machine->getDetector()->elementsizeX);
    return sizeX;
}


float XPLRayTracing::GetDetectorSizeY()
{
    if (sim != nullptr) sizeY = static_cast<float>(sim->machine->getDetector()->elementsizeY);
    return sizeY;
}


short XPLRayTracing::GetBitsAllocated()
{
    if (sim != nullptr) bitsAllocated = sim->machine->getDetector()->GetAllocatedBits();
    return bitsAllocated;
}


short XPLRayTracing::GetBitsStored()
{
    if (sim != nullptr) bitsStored = sim->machine->getDetector()->GetStoredBits();
    return bitsStored;
}


short XPLRayTracing::GetHighBit()
{
    if (sim != nullptr) highBit = sim->machine->getDetector()->GetHighBit();
    return highBit;
}

short XPLRayTracing::GetWidth()
{
    return width;
}


short XPLRayTracing::GetHeight()
{
    return height;
}


float XPLRayTracing::GetPixelSizeX()
{
    return sizeX;
}


float XPLRayTracing::GetPixelSizeY()
{
    return sizeY;
}


SimulationConfig XPLRayTracing::SimulationGetConfig(int i)
{
    Acquisition a = sim->machine->acquisitionGeometry[i];
    SimulationConfig c;

    c.DetectorLocation[0] = static_cast<float>(a.detectorPosition[0]);
    c.DetectorLocation[1] = static_cast<float>(a.detectorPosition[1]);
    c.DetectorLocation[2] = static_cast<float>(a.detectorPosition[2]);

    c.DetectorXDir[0] = static_cast<float>(a.detectorDirectionS[0]);
    c.DetectorXDir[1] = static_cast<float>(a.detectorDirectionS[1]);
    c.DetectorXDir[2] = static_cast<float>(a.detectorDirectionS[2]);

    c.DetectorYDir[0] = static_cast<float>(a.detectorDirectionT[0]);
    c.DetectorYDir[1] = static_cast<float>(a.detectorDirectionT[1]);
    c.DetectorYDir[2] = static_cast<float>(a.detectorDirectionT[2]);

    rowVector z = rowVector::cross(a.detectorDirectionS, a.detectorDirectionT);

    c.DetectorZDir[0] = static_cast<float>(z[0]);
    c.DetectorZDir[1] = static_cast<float>(z[1]);
    c.DetectorZDir[2] = static_cast<float>(z[2]);

    c.SourceLocation[0] = static_cast<float>(a.focalPosition[0]);
    c.SourceLocation[1] = static_cast<float>(a.focalPosition[1]);
    c.SourceLocation[2] = static_cast<float>(a.focalPosition[2]);

    c.DistanceSrcToDet  = static_cast<float>(a.distanceSrcToDet);

    return c;
}
