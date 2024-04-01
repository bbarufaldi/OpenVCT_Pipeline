// Machine.cpp

#include "Machine.h"
#include "Image.h"
#include "ReadPhantom.h"
#include <ctime>   // for logging
#include <fstream> // for logging


Machine::Machine()
    : name("detector"),
      detector(nullptr),
      xRayTube(nullptr)
{

    detector = new Detector;
    xRayTube = new XRayTube;

    ReadPhantom &rph = ReadPhantom::getInstance();

    // TODO: fill in stuff here from ReadPhantom

}


Machine::Machine(pugi::xml_node &node)
{
	bool reverse_focal_found = false;

	pugi::xml_node detector_node = node.child("Detector");
    detector = new Detector(detector_node);

	pugi::xml_node xRayTube_node = node.child("XRayTube");
	xRayTube = new XRayTube(xRayTube_node);
	pugi::xml_node acquisitionGeometryNode = node.child("AcquisitionGeometry");

    // Look for optional node which requests reversing the sign of acquisition positions
    pugi::xml_node childNode = acquisitionGeometryNode.child("ReverseFocalLocationX");
	if (childNode) 
    {
		reverse_focal_found = true;
		std::stringstream(childNode.first_child().value()) >> std::boolalpha >> Acquisition::reverseFocalLocationX;
	}

    childNode = acquisitionGeometryNode.child("ReverseFocalLocationY");	
    if (childNode) 
    {
		reverse_focal_found = true;
		std::stringstream(childNode.first_child().value()) >> std::boolalpha >> Acquisition::reverseFocalLocationY;
	}
    
    childNode = acquisitionGeometryNode.child("ReverseFocalLocationZ");
	if (childNode) 
    {
		reverse_focal_found = true;
		std::stringstream(childNode.first_child().value()) >> std::boolalpha >> Acquisition::reverseFocalLocationZ;
	}

	#if defined(LOG_DIMENSIONS)
	if (reverse_focal_found)
    {
		std::time_t time_result = std::time(nullptr);
		std::ofstream log("XPLProjectionSimulation_Dimensions.log", std::ofstream::app);
		if (log)
		{
			log << std::asctime(std::localtime(&time_result)) << __FUNCTION__
                << ": detector ReverseFocalLocation X/Y/Z (from config file) is: " 
				<< (Acquisition::reverseFocalLocationX ? "true" : "false") << ", " 
				<< (Acquisition::reverseFocalLocationY ? "true" : "false") << ", "
				<< (Acquisition::reverseFocalLocationZ ? "true" : "false") << '\n';
			log.close();
		}
	}
	#endif

    std::string name;
	for (pugi::xml_node acqNode = acquisitionGeometryNode.first_child(); acqNode; acqNode = acqNode.next_sibling())
	{
        if (acqNode) 
        {    
		   std::stringstream(acqNode.first_child().name()) >> name;
           if (name == "FocalPosition")  acquisitionGeometry.push_back(Acquisition(acqNode));
        }
	}
}




/// <summary>
/// Finalizes an instance of the <see cref="Machine"/> class.
/// </summary>
Machine::~Machine(void)
{
	delete detector;
	delete xRayTube; 
}
