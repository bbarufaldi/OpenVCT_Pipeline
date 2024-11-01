// XRayTube.cpp


#include "XRayTube.h"
#include "ReadPhantom.h"
#include <sstream>
#include <string>


// Default Constructor
XRayTube::XRayTube()
    : spectrumID(2),
      kVP(32),
	  filterMaterialZ(42),
	  filterThickness_cm(0.005) 
{
    //std::cout << "IN XRAYTUBE DEFAULT CONSTRUCTOR" << std::endl;
    ReadPhantom &rph = ReadPhantom::getInstance();

    spectrumID          = rph.getSpectrumID();
    kVP                 = rph.getkVp();
    filterMaterialZ     = rph.getFilterMaterialZ();
    filterThickness_cm  = rph.getFilterThickness_mm() * 0.1; // convert to CM
}


// Parametric Constructor
XRayTube::XRayTube(pugi::xml_node &node)
{

	pugi::xml_node childNode = node.child("Anode");
	std::stringstream(childNode.child("SpectrumID").first_child().value()) >> spectrumID;
	std::stringstream(childNode.child("kVp").first_child().value()) >> kVP;
	childNode = node.child("Filter");
	std::stringstream(childNode.child("MaterialZ").first_child().value()) >> filterMaterialZ;
	std::stringstream(childNode.child("Thickness_cm").first_child().value()) >>  filterThickness_cm;
}


XRayTube::~XRayTube(void)
{}
