#pragma once
#include "pugixml.hpp"
/// <summary>
/// 
/// </summary>
class XRayTube
{
private:

	unsigned short spectrumID;
    unsigned short kVP;
	unsigned short filterMaterialZ;
	double         filterThickness_cm;

public:

	XRayTube();
	XRayTube(pugi::xml_node &node);
	~XRayTube(void);

	unsigned short getSpectrumID()      { return spectrumID; }
	unsigned short getkVP()             { return kVP; }
	unsigned short getFilterMaterialZ() { return filterMaterialZ; }
	double         getFilterThickness() { return filterThickness_cm; }

};

