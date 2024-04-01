// Detector.h

#pragma once

#include "vector3.h"
#include "pugixml.hpp"

class Detector
{
public:
    // Constructors and destructor
    Detector();	
    Detector(pugi::xml_node &node);
	~Detector(void);

    // Why are these public?
	unsigned int   elementCountX;
    unsigned int   elementCountY;
	double         elementsizeX; // pixel size in cm
    double         elementsizeY; // pixel size in cm 
	double         thickness;
	unsigned short material;

	// Accessors
	short  GetAllocatedBits();
	short  GetStoredBits();
	short  GetHighBit();
	bool   GetUseXMLOffsets();
    double GetOffsetX();
    double GetOffsetY();
    double GetOffsetZ();

	rowVector getElementLocation(int i, int j);

private:
	short  allocatedbits;
	short  storedbits;
	short  highbit;
	bool   useXMLVolumeOffsets;
	double volumeOffsetX;
	double volumeOffsetY;
	double volumeOffsetZ;
};

