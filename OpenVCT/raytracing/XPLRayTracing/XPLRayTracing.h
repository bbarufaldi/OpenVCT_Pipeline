// XPLRayTracing.h

#pragma once

#include "Simulation.h"
#include "SimulationConfig.h"
#include "XPLAcquisition.h"


#if !defined(NO_CPU)
#include "SimulationCPU.h"
#elif !defined(NO_OPENCL)
#include "SimulationOCL.h"
#endif

#include "ProgressBar.h"
extern ProgressBar progressBar;

#include <string>

class XPLRayTracing
{
public:

    enum SimType {CPU, GPU};

    XPLRayTracing();
    virtual ~XPLRayTracing();

    int SimulationSetup(int argc, char* argv[]);
    int SimulationRun(unsigned short* imageBuffer, int id);

    std::string GetSimType();

    int   GetAcquisitionCount();
    XPLAcquisition GetAcquisition(int i);

    float GetDetectorSizeX();
    float GetDetectorSizeY();

    short GetBitsAllocated();
    short GetBitsStored();
    short GetHighBit();

    short GetWidth();
    short GetHeight();

    float GetPixelSizeX();
    float GetPixelSizeY();

    SimulationConfig SimulationGetConfig(int i);

    Simulation *sim;

private:
      SimType simtype;
      int     acq;
	  short   height;
      short   width;
	  float   sizeX;
      float   sizeY;
	  short   bitsAllocated;
      short   bitsStored;
      short   highBit;

      ReadPhantom *rp;
};