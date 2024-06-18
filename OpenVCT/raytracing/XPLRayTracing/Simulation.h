// Simulation.h

#pragma once

#include "Machine.h"
#include "Volume.h"
#include "pugixml.hpp"
#include "AttenuationTable.h"
#include "Image.h"
#include <functional>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include "Ray.h"
#include "RayTracer.h"
#include "BooneMaterialTable.h"
#include <random>
#include <thread>
#include <fstream>

class Simulation
{
public:

	 Volume<unsigned char>* volume;
	 Machine* machine;	

    virtual bool simulate(signed short acqID, unsigned short* image);

    Simulation();
	Simulation(std::string xmlPathName, std::string attentuationPathName, std::string filePath);
	Simulation(std::string xmlPathName, std::string attentuationPathName, Volume<unsigned char> vol);
	virtual ~Simulation();

	void setUseNoise(bool val)        { useNoise = val; }
	void setElecNoise(double val)     { elecNoise = val; }
	void setGain(double val)          { gain = val; }
	void setNormAirKerma(double val)  { normAirKerma = val; }
    void setNoInverseSq(bool val)     { useInverseSq = !val; }

    void setmAs(double val)             { mAs = val; }      
    void setPropConst(double val)       { propConst = val; }            
    void setUseScaling(bool val)        { setUseScalingFlag = true; useScaling = val; }
    void setMinGray(unsigned short val) { setMinGrayValueFlag = true; minGrayscaleValue = val; }

	std::function<int(int x, int y, int w, int h)> image_func; // lambda function

protected:

	 AttenuationTable attenuationTable;
	 virtual bool process(signed short acqID, int id, Image<double> *outImage)=0;
	 bool   deleteVolume;
	 double volume_offsetX;  // uses volume and machine dimensions (or xml override)  DH 08/26/2014
	 double volume_offsetY;  // uses volume and machine dimensions (or xml override)  DH 08/26/2014
	 double volume_offsetZ;  // uses volume and machine dimensions (or xml override)  DH 09/18/2014
     bool   disable_noise;   // if true, no (poisson) noise is added to the images
	 bool   view_slice;      // if true, only process volume between specified z (vertical) bounds
	 double upper_bound,     // when view_slice is true only voxels between these bounds are processed
            lower_bound;

	 bool   useNoise;
	 double elecNoise;
	 double gain;
	 double normAirKerma;

     double mAs;
	 double propConst;  // Proportionality constant to adjust mean DV for a given mAs
    
     bool   setUseScalingFlag;           // flag: override default Image behavior?  DH Apr2015
     bool   useScaling;                  // whether to use scaling  DH Apr2015
     bool   useInverseSq;                // whether to use the inverse square law

     bool           setMinGrayValueFlag; // flag override default Image;s min gray? DH Apr2015
     unsigned short minGrayscaleValue;   // minimum grayscale value DH Apr2015  *DH: 2016-03-11 This got left out of the final implemnntation. It is unused. *
};
