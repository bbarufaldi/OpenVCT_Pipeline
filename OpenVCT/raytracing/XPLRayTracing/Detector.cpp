// Detector.cpp

#include "Detector.h"
#include "ReadPhantom.h"
#include <sstream>  


/// <summary>
/// Initializes a new instance of the <see cref="Detector"/> class.
/// </summary>
Detector::Detector()
    : 	elementCountX(0u),
        elementCountY(0u),
        elementsizeX(0.0), // pixel size in cm
        elementsizeY(0.0), // pixel size in cm 
        thickness(0.0),
        material(0u),
        allocatedbits(0),
        storedbits(0),
        highbit(0),
        useXMLVolumeOffsets(false),
        volumeOffsetX(0.0),
        volumeOffsetY(0.0),
        volumeOffsetZ(0.0)
{
    ReadPhantom &rph = ReadPhantom::getInstance();

    elementCountX       = rph.getElementCountX();
    elementCountY       = rph.getElementCountY();
    elementsizeX        = rph.getElementSizeX_mm() * 0.1;
    elementsizeY        = rph.getElementSizeY_mm() * 0.1;
    thickness           = rph.getDetThickness_mm() * 0.1;
    material            = rph.getDetMaterial();

    allocatedbits       = rph.getAllocatedBits();
    storedbits          = rph.getStoredBits();
    highbit             = rph.getHighBit();
    useXMLVolumeOffsets = rph.getUseXYVolOffsets();
    volumeOffsetX       = rph.getOptVolOffsetX_mm() * 0.1;
    volumeOffsetY       = rph.getOptVolOffsetY_mm() * 0.1;
    volumeOffsetZ       = rph.getVolumeOffsetZ_mm() * 0.1;
}



/// <summary>
/// Initializes a new instance of the <see cref="Detector"/> class.
/// </summary>
Detector::Detector(pugi::xml_node &node) 
	: elementsizeX(0.014), elementsizeY(0.014), elementCountX(2048), elementCountY(1792), thickness(0.01), material(0),
	  allocatedbits(16), storedbits(16), highbit(15),
	  useXMLVolumeOffsets(false), volumeOffsetX(0.0), volumeOffsetY(0.0), volumeOffsetZ(0.0)
{		 
	pugi::xml_node childNode = node.child("ElementSizeX_cm");
	if (childNode) {
		std::stringstream(childNode.first_child().value()) >> elementsizeX;
	}
	childNode = node.child("ElementSizeY_cm");
	if (childNode) {
		std::stringstream(childNode.first_child().value()) >> elementsizeY;
	}

	childNode = node.child("ElementCountX");
	if (childNode) {
		std::stringstream(childNode.first_child().value()) >> elementCountX;
	}
	childNode = node.child("ElementCountY");
	if (childNode) {
		std::stringstream(childNode.first_child().value()) >> elementCountY;
	}

	childNode = node.child("Thickness_cm");
	if (childNode) {
		std::stringstream(childNode.first_child().value()) >> thickness;
	}
	childNode = node.child("Material");
	if (childNode) {
		std::stringstream(childNode.first_child().value()) >> material;
	}

	childNode = node.child("BitsAllocated");
	if (childNode) {
		std::stringstream(childNode.first_child().value()) >> allocatedbits;
	}
	childNode = node.child("BitsStored");
	if (childNode) {
		std::stringstream(childNode.first_child().value()) >> storedbits;
	}
	childNode = node.child("HighBit");
	if (childNode) {
		std::stringstream(childNode.first_child().value()) >> highbit;
	}

	childNode = node.child("UseXMLVolumeOffsets");
	if (childNode)
	{
		std::stringstream(childNode.first_child().value()) >> std::boolalpha >> useXMLVolumeOffsets;	
		childNode = node.child("VolumeOffsetX_cm");
		std::stringstream(childNode.first_child().value()) >> volumeOffsetX;
		childNode = node.child("VolumeOffsetY_cm");
		std::stringstream(childNode.first_child().value()) >> volumeOffsetY;
	}

	// The z offset is equipment dependent and is not optional (it cannot be calculated from the volume)
	childNode = node.child("VolumeOffsetZ_cm");
	if (childNode)
	{
		std::stringstream(childNode.first_child().value()) >> volumeOffsetZ;
	}
}

/// <summary>
/// Gets the element location.
/// </summary>
/// <param name="i">The i.</param>
/// <param name="j">The j.</param>
/// <returns></returns>
rowVector Detector::getElementLocation(int i, int j)
{
	return rowVector();

//	return location + elementsizeX*(i+0.5)*directionX + elementsizeY*(j+0.5)*directionY;
//	return location + elementsizeX*(i+0.5)*directionX + elementsizeY*(j+0.5)*directionY;
}
	



/// <summary>
/// Finalizes an instance of the <see cref="Detector"/> class.
/// </summary>
Detector::~Detector(void)
{
}

/// <summary>
/// Retrieves the number of allocated bits for the <see cref="Detector"/> class.
/// </summary>
short Detector::GetAllocatedBits()
{
	return allocatedbits;
}

/// <summary>
/// Retrieves the number of stored bits for the <see cref="Detector"/> class.
/// </summary>
short Detector::GetStoredBits()
{
	return storedbits;
}

/// <summary>
/// Retrieves the value of the high bit for the <see cref="Detector"/> class.
/// </summary>
short Detector::GetHighBit()
{
	return highbit;
}

/// <summary>
/// Retrieves whether to use offsets specified in XML for raytracing the phantom for the <see cref="Detector"/> class.
/// </summary>
bool Detector::GetUseXMLOffsets()
{
	return useXMLVolumeOffsets;
}

/// <summary>
/// Retrieves the offset in the x direction to begin raytracing the phantom for the <see cref="Detector"/> class.
/// </summary>
double Detector::GetOffsetX()
{
	return volumeOffsetX;
}

/// <summary>
/// Retrieves the offset in the y direction to begin raytracing the phantom for the <see cref="Detector"/> class.
/// </summary>
double Detector::GetOffsetY()
{
	return volumeOffsetY;
}

/// <summary>
/// Retrieves the offset in the z direction to begin raytracing the phantom for the <see cref="Detector"/> class.
/// </summary>
double Detector::GetOffsetZ()
{
	return volumeOffsetZ;
}
