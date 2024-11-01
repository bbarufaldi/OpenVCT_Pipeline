// SimulationOCL.h

#pragma once

#include "Simulation.h"
#include "Image.h"
#include <string>

class SimulationOCL : public Simulation
{
public:
    SimulationOCL(std::string seed);
	SimulationOCL(std::string xmlPathName, std::string attentuationPathName, std::string filePath, std::string seed);
	~SimulationOCL();

protected:
    virtual const char *Fileopen(const char* fileName);
	virtual bool process(signed short acqID, int id, Image<double> *outImage);
	std::string mSeed;
    char *openCLProgramSource;
};
