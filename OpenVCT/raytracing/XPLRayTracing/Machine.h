// Machine.h

#pragma once

#include <string>
#include <vector>
#include "XRayTube.h"
#include "Detector.h"
#include "Acquisition.h"
#include "pugixml.hpp"

class Machine
{
public:

    Machine();
    Machine(pugi::xml_node &node);
    ~Machine(void);
    
    Detector *getDetector() { return detector; }
    XRayTube *getXRayTube() { return xRayTube; }

    std::vector<Acquisition> acquisitionGeometry; // should probably be private with an accessor, but is used directly by SimulationXXX and XPLRayTracing

private:

    std::string name;
    Detector    *detector;
    XRayTube    *xRayTube;
};

