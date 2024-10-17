// Simulation.cpp

#include "Simulation.h"
#include "Machine.h"
#include "Volume.h"
#include "pugixml.hpp"
#include "AttenuationTable.h"
#include "Image.h"
#include "Ray.h"
#include "RayTracer.h"
#include "BooneMaterialTable.h"
#include "ReadPhantom.h"
#include <functional>
#include <ctime>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <thread>
#include <fstream>



/// <summary>
/// Initializes a new instance of the <see cref="Simulation"/> class.
/// </summary>
Simulation::Simulation()
    : volume(nullptr), machine(nullptr), attenuationTable(), 
      useNoise(true), elecNoise(3.0), gain(0.2), normAirKerma(12.92), mAs(57.0), propConst(0.00565614), // default settings
      setUseScalingFlag(false), useScaling(false), useInverseSq(true),
      setMinGrayValueFlag(false), minGrayscaleValue(50u)
{
    // This is typically overridden by assignment stmt by derived classes, via assignment stmt
    image_func = [](int x, int y, int w, int h){ return x * h + h - y - 1; };

    volume = new Volume<unsigned char>;
    machine = new Machine;

    ReadPhantom &rph = ReadPhantom::getInstance();
    attenuationTable.initialize(rph.getAttenFile());
    rph.setAcquisitionGeometry(machine);

    volume_offsetX = machine->getDetector()->GetOffsetX();
    volume_offsetY = machine->getDetector()->GetOffsetY();
    volume_offsetZ = machine->getDetector()->GetOffsetZ(); 

    // Calculate the offsets based on volume and detector dimensions
    double detectorWidth = machine->getDetector()->elementCountX * machine->getDetector()->elementsizeX;
    double volumeLength = volume->voxelSize.v[0] * volume->voxelCount[0];

    volume_offsetX = volume_offsetX + 0.5 * volumeLength;                                               //BB and TV 05/11/2018 volume_offsetX +

    //std::cout << "volumeLength: " << volumeLength << std::endl;
    //std::cout << "volume_offsetX: " << volume_offsetX << std::endl;
    //std::cout << "volume_offsetY: " << volume_offsetY << std::endl;
    //std::cout << "volume_offsetZ: " << volume_offsetZ << std::endl;
        
}



/// <summary>
/// Initializes a new instance of the <see cref="Simulation"/> class.
/// </summary>
/// <param name="xmlPathName">Name of the XML path.</param>
/// <param name="attentuationPathName">Name of the attentuation path.</param>
/// <param name="filePath">The file path.</param>
Simulation::Simulation(std::string xmlPathName, std::string attentuationPathName, std::string filePath)
    : volume(nullptr), machine(nullptr), attenuationTable(attentuationPathName.c_str()), 
      useNoise(true), elecNoise(3.0), gain(0.2), normAirKerma(12.92), mAs(57.0), propConst(0.00565614), // default settings
      setUseScalingFlag(false), useScaling(false), useInverseSq(true),
      setMinGrayValueFlag(false), minGrayscaleValue(50u)
{
    // This is typically overridden by assignment stmt by derived classes, via assignment stmt
    image_func = [](int x, int y, int w, int h){ return x * h + h - y - 1; };

    pugi::xml_document doc;
    if (doc.load_file(xmlPathName.c_str()))
    {
        pugi::xml_node tools = doc.first_child();
        pugi::xml_node volume_node = tools.child("Volume");

        volume = new Volume<unsigned char> (volume_node, filePath);
        
        pugi::xml_node machine_node = tools.child("Machine");
        machine = new Machine(machine_node);

        // Locate the volume on the detector  DH 08/26/2014
        if (machine->getDetector()->GetUseXMLOffsets())
        {
            // Use the override offsets specified in the XML detector section
            volume_offsetX = machine->getDetector()->GetOffsetX();
            volume_offsetY = machine->getDetector()->GetOffsetY();
        }
        else
        {
            // Calculate the offsets based on volume and detector dimensions
            double detectorWidth = machine->getDetector()->elementCountX * machine->getDetector()->elementsizeX;
            double volumeLength = volume->voxelSize.v[0] * volume->voxelCount[0];

            volume_offsetX = 0.5 * volumeLength;
            volume_offsetY = 0.0;
        }
        
        volume_offsetZ = machine->getDetector()->GetOffsetZ(); // this offset isn't optional
        
        // Log offsets used - TODO: need to add an actual logging facility for this - DH 08/26/2014

    } else std::cout << "\nXML FILE PATH, " << xmlPathName << " FAILED TO LOAD\n" << std::endl;
}


/// <summary>
/// Destructor for the simulation (base) class..
/// </summary>
Simulation::~Simulation()
{
    //std::cout << "IN Simulation DESTRUCTOR" << std::endl;
    if (volume != nullptr) delete volume;
    if (machine != nullptr) delete machine;
}


/// <summary>
/// Simulates projection at the specified acquisition geometry (via index into acquisition geometry vector).
/// </summary>
/// <param name="acqID">The acquisition ID (which geometry to use).</param>
/// <param name="outFilePath">A pointer to the corresponding image instance.</param>
bool Simulation::simulate(signed short acqID, unsigned short *image)
{
    bool status = true; // return value
    
    
    Image<double> outImage(machine->getDetector()->elementCountX, machine->getDetector()->elementCountY);
    
    status = process(acqID, 0, &outImage);
    if (status)
    {
        if (setUseScalingFlag)   outImage.setUseScaledValue(useScaling);
        if (setMinGrayValueFlag) outImage.setMinGrayscaleValue(minGrayscaleValue);

        outImage.transform(image, image_func);
    }
    return status;
}

