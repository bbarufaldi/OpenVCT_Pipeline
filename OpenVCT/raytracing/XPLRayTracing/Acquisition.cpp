// XPLRayTracing

#include "Acquisition.h"
#include "pugixml.hpp"
#include "Detector.h"
#include "vector3.h"
#include <sstream>
#include <cmath>
#include <iostream> // for Debug
#include <ctime>    // for logging
#include <fstream>  // for logging


bool Acquisition::reverseFocalLocationX  = false;
bool Acquisition::reverseFocalLocationY  = false;
bool Acquisition::reverseFocalLocationZ  = false;
bool Acquisition::randomize_element_quad = false;


Acquisition::Acquisition()
{}

Acquisition::Acquisition(pugi::xml_node &node)
{
    pugi::xml_node focalNode = node.child("FocalPosition");
    {
        pugi::xml_node childNode = focalNode.child("LocationX_cm");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> focalPosition.v[0];
        }
        childNode = focalNode.child("LocationY_cm");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> focalPosition.v[1];
        }
        childNode = focalNode.child("LocationZ_cm");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> focalPosition.v[2];
        }

        #if defined(LOG_DIMENSIONS)
        std::time_t time_result = std::time(nullptr);
        std::ofstream log("XPLProjectionSimulation_Dimensions.log", std::ofstream::app);
        if (log)
        {
            log << std::asctime(std::localtime(&time_result)) << __FUNCTION__
                << ": FocalLocation X/Y/Z (from config file) is: " 
                << focalPosition.v[0] << ", " << focalPosition.v[1] << ", " << focalPosition.v[2] << '\n';
            log.close();
        }
        #endif

        // Reverse the positions as indicated
        if (reverseFocalLocationX && focalPosition.v[0] != 0.0) { focalPosition.v[0] = -focalPosition.v[0]; }
        if (reverseFocalLocationY && focalPosition.v[1] != 0.0) { focalPosition.v[1] = -focalPosition.v[1]; }
        if (reverseFocalLocationZ && focalPosition.v[2] != 0.0) { focalPosition.v[0] = -focalPosition.v[2]; }

        #if defined(LOG_DIMENSIONS)
        if (reverseFocalLocationX || reverseFocalLocationY || reverseFocalLocationZ)
        {
            std::time_t time_result = std::time(nullptr);
            std::ofstream log("XPLProjectionSimulation_Dimensions.log", std::ofstream::app);
            if (log)
            {
                log << std::asctime(std::localtime(&time_result)) << __FUNCTION__
                    << ": FocalLocation X/Y/Z (after reversing) is: " 
                    << focalPosition.v[0] << ", " << focalPosition.v[1] << ", " << focalPosition.v[2] << '\n';
                log.close();
            }
        }
        #endif

    }

    pugi::xml_node phantomNode = node.child("PhantomPosition");
    {
        pugi::xml_node childNode = phantomNode.child("LocationX_cm");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> phantomPosition.v[0];
        }
        childNode = phantomNode.child("LocationY_cm");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> phantomPosition.v[1];
        }
        childNode = phantomNode.child("LocationZ_cm");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> phantomPosition.v[2];
        }

        #if defined(LOG_DIMENSIONS)
        std::time_t time_result = std::time(nullptr);
        std::ofstream log("XPLProjectionSimulation_Dimensions.log", std::ofstream::app);
        if (log)
        {
            log << std::asctime(std::localtime(&time_result)) << __FUNCTION__
                << ": PhantomLocation X/Y/Z (from config file) is: "
                << phantomPosition.v[0] << ", " << phantomPosition.v[1] << ", " << phantomPosition.v[2] << '\n';
            log.close();
        }
        #endif
    }


    // NO LONGER USED? (leftover from configLabel.xml)
    pugi::xml_node detectorNode = node.child("DetectorPosition");
    {
        pugi::xml_node  childNode = detectorNode.child("OriginLocationX_cm");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorPosition.v[0];
        }
        childNode = detectorNode.child("OriginLocationY_cm");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorPosition.v[1];
        }
        childNode = detectorNode.child("OriginLocationZ_cm");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorPosition.v[2];
        }
        
        // Can be calculated given others, but not for now...
        int have_directionR = 0;

        childNode = detectorNode.child("DirectionRX");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorDirectionR.v[0];  
			have_directionR++; 
        }
        childNode = detectorNode.child("DirectionRY");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorDirectionR.v[1];  
			have_directionR++; 
        }
        childNode = detectorNode.child("DirectionRZ");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorDirectionR.v[2];  
			have_directionR++; 
        }

        childNode = detectorNode.child("DirectionSX");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorDirectionS.v[0];
        }
        childNode = detectorNode.child("DirectionSY");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorDirectionS.v[1];
        }
        childNode = detectorNode.child("DirectionSZ");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorDirectionS.v[2]; 

        }

        childNode = detectorNode.child("DirectionTX");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorDirectionT.v[0];
        }
        childNode = detectorNode.child("DirectionTY");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorDirectionT.v[1];
        }
        childNode = detectorNode.child("DirectionTZ");
        if (childNode) {
            std::stringstream(childNode.first_child().value()) >> detectorDirectionT.v[2];
        }

        #if defined(LOG_DIMENSIONS)
        std::time_t time_result = std::time(nullptr);
        std::ofstream log("XPLProjectionSimulation_Dimensions.log", std::ofstream::app);
        if (log)
        {
            log << std::asctime(std::localtime(&time_result)) << __FUNCTION__
                << ": Detector Origin X/Y/Z (from config file) is: " 
                << detectorPosition.v[0] << ", " << detectorPosition.v[1] << ", " << detectorPosition.v[2] << '\n';
			if (have_directionR)
            {
                log << ": DirectionR X/Y/Z (from config file) is: " 
                    << detectorDirectionR.v[0] << ", " << detectorDirectionR.v[1] << ", " << detectorDirectionR.v[2] << '\n';
            }
            log << ": DirectionS X/Y/Z (from config file) is: " 
                    << detectorDirectionS.v[0] << ", " << detectorDirectionS.v[1] << ", " << detectorDirectionS.v[2] << '\n';
            log << ": DirectionT X/Y/Z (from config file) is: " 
                    << detectorDirectionT.v[0] << ", " << detectorDirectionT.v[1] << ", " << detectorDirectionT.v[2] << '\n';
            log.close();
        }
        #endif

        // Calculate the R vector if not explicitly specified in XML
		if (have_directionR != 3) {
            detectorDirectionR = rowVector::cross(detectorDirectionS, detectorDirectionT);
        }
    }

    // Calculate distance source to detector DH 09/05/2014
    double dx = focalPosition.v[0];
    double dy = focalPosition.v[1];
    double dz = focalPosition.v[2];
    distanceSrcToDet = sqrt((dx * dx) + (dy * dy) + (dz * dz));

    #if defined(LOG_DIMENSIONS)
    std::time_t time_result = std::time(nullptr);
    std::ofstream log("XPLProjectionSimulation_Dimensions.log", std::ofstream::app);
    if (log)
    {
        log << std::asctime(std::localtime(&time_result)) << __FUNCTION__
            << ": Distance focal pt to detector (calc'd from focal pos, assuming detector at 0/0/0) is: " << distanceSrcToDet << '\n';
        log.close();
    }
    #endif
}


/// <summary>
/// Gets the detector element location.
/// </summary>
/// <param name="detector">The detector.</param>
/// <param name="i">The i.</param>
/// <param name="j">The j.</param>
/// <returns></returns>
rowVector Acquisition::getDetectorElementLocation(Detector &detector, double i, double j) // OBSERVATION (DH): detector parameter isn't being used here
{
    rowVector retval;

    // Lambda function to return random value between 0.25 and 0.75
    auto r = [] { return ((double)rand() / ((double)RAND_MAX + 1.0)) * 0.5 + 0.25; };

    // Return a position within a quadrant of the detector element (which quadrant is specifed by i and j)
    if (randomize_element_quad)
    {
        // Add some randomness to the position - DH 2016-03-16
        retval = detectorPosition + detector.elementsizeX *(i + r())*detectorDirectionS + detector.elementsizeY*(j + r())*detectorDirectionT;
    }
    else
    {
        // Return the center of the quadrant
        retval = detectorPosition + detector.elementsizeX *(i + 0.5)*detectorDirectionS + detector.elementsizeY*(j + 0.5)*detectorDirectionT;
    }

    return retval;
}

Acquisition::~Acquisition(void)
{}
