// SimulationCPU.h

#pragma once

#if !defined(NO_CPU)

#include "Simulation.h"
#include "Image.h"
#include <string>

class SimulationCPU: public Simulation
{
public:	
    SimulationCPU(std::string seed);
    SimulationCPU(std::string xmlPathName, std::string attentuationPathName, std::string filePath,  std::string seed);
	~SimulationCPU();

protected:
	virtual bool process(signed short acqID, int id, Image<double> *outImage);
    std::string mSeed;
};

#endif
