// AttenuationTable.h

#pragma once

#include <string>
#include <map>
#include <vector>

/// <summary>
/// A class that maintains attenuations of different materials 
/// </summary>
class AttenuationTable
{
public:

    AttenuationTable();
    AttenuationTable(std::string filePath);
	~AttenuationTable(void);

    void initialize(std::string attenFilename);

	double getAttenuation(unsigned short materialIndex, unsigned short energy);
	std::vector<double> * getAttenuationTable(unsigned short materialIndex);
	double getDensity(unsigned short materialIndex);	
	size_t getMaterialCount();
	size_t getEnergyCount(unsigned short materialIndex);

private:

	std::map<unsigned short, std::vector<double> *> table;
	std::map<unsigned short, double> density;
};

