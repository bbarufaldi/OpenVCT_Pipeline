#pragma once
#include "vector3.h"
#include "pugixml.hpp"
#include  "Detector.h"

class Acquisition
{
public:

    Acquisition();
	Acquisition(pugi::xml_node &node);
	~Acquisition(void);
    
    rowVector getDetectorElementLocation(Detector &detector, double i, double j);

	rowVector focalPosition;
    rowVector phantomPosition;			//BB and TV 04/10/2018
	rowVector detectorPosition;
	rowVector detectorDirectionR;
	rowVector detectorDirectionS;
	rowVector detectorDirectionT;
	double    distanceSrcToDet;	

    static bool reverseFocalLocationX;
    static bool reverseFocalLocationY;
    static bool reverseFocalLocationZ;
    static bool randomize_element_quad;
};

