// Volume.h

#pragma once

///#define READ_OLDER_FORMAT_PHANTOM


#include <string>
#include <vector>
#include <map>
#include "vector3.h"
#include "pugixml.hpp"
#include "LabelMap.h"
#include "ReadPhantom.h"
#include "VCT_CommonLibrary.h"
#include "VCT_VoxelArray.h"
#include "VCT_Vctx.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>

#pragma region Documentation
/// <summary>
/// This class manages a 3D voxel body
/// </summary>
#pragma endregion

template <class T> 
class Volume
{
public:
	Volume();	
	Volume(std::string filepath, rowVector vSize);
	Volume(rowVector size, rowVector count);
	Volume(const pugi::xml_node &node, std::string filePath);
	~Volume(void);

    int GetNumMaterials(); // 2018-11-13 DH *

	T getVoxel(unsigned int x, unsigned int y, unsigned int z);
	void setVoxel(unsigned int x, unsigned int y, unsigned int z, T value);
	bool   GetViewSlice();
	double GetSliceUpperBound();
	double GetSliceLowerBound();

    vct::Vctx vctx;
    vct::VoxelArray *va;
	T *voxels;
	int voxelCount[3];
	rowVector voxelSize;
    std::map<unsigned short, LabelMap> labelMap;

private:
	bool   view_slice;      // if true, only process volume between specified z (vertical) bounds
	double upper_bound, lower_bound; // when view_slice is true only voxels between these bounds are processed
};


#pragma region Documentation
/// <summary>
/// (Constructor) Initializes a new instance of the <see cref="Volume"/> class.
/// </summary>

template <class T>
Volume<T>::Volume() : view_slice(false), voxels(nullptr)
{
	voxelCount[0] = 0;
	voxelCount[1] = 0;
	voxelCount[2] = 0;

    ReadPhantom &rp = ReadPhantom::getInstance();
    size_t x_dim, y_dim, z_dim;
    float vsizex_m, vsizey_m, vsizez_m;
    voxels = (unsigned char *)rp.getPhantom(x_dim, y_dim, z_dim, vsizex_m, vsizey_m, vsizez_m);

    if (!rp.buildLabelMap(&labelMap))
    {
        std::cerr << "Volume constructor: Couldn't build labelMap" << std::endl;
    }    

	if (voxels != nullptr)
    {
        //std::cout << "Voxel address is " << (int *) voxels << std::endl;

        // Voxel counts are scrambled in this application
		voxelCount[0] = static_cast<int>(z_dim);
		voxelCount[1] = static_cast<int>(x_dim);
		voxelCount[2] = static_cast<int>(y_dim);
        //std::cout << "Voxel Counts are " << voxelCount[0] << ", " << voxelCount[1] << ", " << voxelCount[2] << std::endl;
		
        // Voxelsize used to come from config.xml; it is now stored in the phantom
        voxelSize.v[0] = static_cast<double>(vsizex_m * 0.1); // convert to CM
        voxelSize.v[1] = static_cast<double>(vsizey_m * 0.1);
        voxelSize.v[2] = static_cast<double>(vsizez_m * 0.1);
        //std::cout << "Voxel Sizes are " << voxelSize.v[0] << ", " << voxelSize.v[1] << ", " << voxelSize.v[2] << " cm."<< std::endl;
	} 
    else 
    {
        std::cerr << "COULD NOT READ THE VOXEL ARRAY DATA!" << std::endl;
        exit(-1);
    }
    //std::cout << "\n* LABEL MAP SIZE IS " << labelMap.size() << std::endl;
}


#pragma region Documentation
/// <summary>
/// (Constructor) Initializes a new instance of the <see cref="Volume"/> class.
/// </summary>
/// <param name="node">The XML node which address Volume information.</param>
/// <param name="filePath">Path to the binary file containing Volume voxels.</param>
#pragma endregion

template <class T>
Volume<T>::Volume(const pugi::xml_node &node, std::string filePath)
    : view_slice(false), voxels(nullptr)
{
	int sx=0, sy=0, sz=0;
    
    voxelCount[0] = 0;
    voxelCount[1] = 0;
    voxelCount[2] = 0;

	pugi::xml_node childNode = node.child("VoxelSizeX_cm");
	std::stringstream(childNode.first_child().value()) >> voxelSize.v[0];
	childNode = node.child("VoxelSizeY_cm");
	std::stringstream(childNode.first_child().value()) >> voxelSize.v[1];
	childNode = node.child("VoxelSizeZ_cm");
	std::stringstream(childNode.first_child().value()) >> voxelSize.v[2];

	childNode = node.child("VoxelCountX");
	std::stringstream(childNode.first_child().value()) >> sx;

	childNode = node.child("VoxelCountY");
	std::stringstream(childNode.first_child().value()) >> sy;

	childNode = node.child("VoxelCountZ");
	std::stringstream(childNode.first_child().value()) >> sz;

	#if defined(LOG_DIMENSIONS)
	std::time_t time_result = std::time(nullptr);
	std::ofstream log("XPLProjectionSimulation_Dimensions.log", std::ofstream::app);
	if (log)
	{
		log << std::asctime(std::localtime(&time_result)) << __FUNCTION__
			 << ": volume voxel size (from config file) is: " 
          << voxelSize.v[0] << 'x' << voxelSize.v[1] << 'x' << voxelSize.v[2]
			 << "\n\nand volume voxel count (from config file) is: " 
          << voxelCount[0] << 'x' << voxelCount[1] << 'x' << voxelCount[2] 
          << '\n';
		log.close();
	}
	#endif

	childNode = node.child("ViewVolumeSlice");
	if (childNode)
	{
		std::stringstream(childNode.first_child().value()) >> std::boolalpha >> view_slice;	
		childNode = node.child("UpperVolumeBound_vxl");
		std::stringstream(childNode.first_child().value()) >> upper_bound;
		childNode = node.child("LowerVolumeBound_vxl");
		std::stringstream(childNode.first_child().value()) >> lower_bound; 
	}

	childNode = node.child("LabelMap");
	for (pugi::xml_node labelNode = childNode.first_child(); labelNode; labelNode = labelNode.next_sibling())
	{
		unsigned short label = labelNode.first_attribute().as_uint();
		labelMap.insert(std::pair<unsigned short, LabelMap>(label, LabelMap(labelNode)));
	}

	ReadPhantom &rp = ReadPhantom::getInstance();

    size_t x_dim, y_dim, z_dim;
    float vsizex_m, vsizey_m, vsizez_m;

    std::cout << __FUNCTION__ << ": filepath is \"" << filePath << "\"" << std::endl;

    voxels = (unsigned char *)rp.getPhantom(x_dim, y_dim, z_dim, vsizex_m, vsizey_m, vsizez_m);

	if (voxels != nullptr)
    {
        //std::cout << "Voxel address is " << (int *) voxels << std::endl;

        // Voxel counts are scrambled in this application
		voxelCount[0] = static_cast<int>(z_dim);
		voxelCount[1] = static_cast<int>(x_dim);
		voxelCount[2] = static_cast<int>(y_dim);
        //std::cout << "Voxel Counts are " << voxelCount[0] << ", " << voxelCount[1] << ", " << voxelCount[2] << std::endl;
		
        // Voxelsize used to come from config.xml; it is now stored in the phantom
        voxelSize.v[0] = static_cast<double>(vsizex_m * 0.1); // convert to CM
        voxelSize.v[1] = static_cast<double>(vsizey_m * 0.1);
        voxelSize.v[2] = static_cast<double>(vsizez_m * 0.1);
        //std::cout << "Voxel Sizes are " << voxelSize.v[0] << ", " << voxelSize.v[1] << ", " << voxelSize.v[2] << " cm."<< std::endl;
	} 
    else 
    {
        std::cerr << "COULD NOT READ THE VOXEL ARRAY DATA!" << std::endl;
        exit(-1);
    }
    
    //std::cout << "\n* LABEL MAP SIZE IS " << labelMap.size() << std::endl;
}


/// <summary>
/// Initializes a new instance of the <see cref="Volume"/> class.
/// </summary>
/// <param name="size">The size.</param>
/// <param name="count">The count.</param>
template <class T>Volume<T>::Volume(rowVector size, rowVector count): voxelSize(size)
{
	voxelCount[0] = count[0];
	voxelCount[1] = count[1];
	voxelCount[2] = count[2];

	this->voxels.resize(voxelCount[0] * voxelCount[1] * voxelCount[2], 0);
}


/// <summary>
/// Returns the number of Materials represented in this volume's labelMap.
/// </summary>
/// <returns></returns>
template <class T> int Volume<T>::GetNumMaterials()
{
    int retval = 0;
    for (auto lbl : labelMap)
    {
        retval += static_cast<int>(lbl.second.getMaterialMap().size());
    }
    return retval;
}


/// <summary>
/// Gets the voxel.
/// </summary>
/// <param name="i">The i.</param>
/// <param name="j">The j.</param>
/// <param name="k">The k.</param>
/// <returns>The label value found at the specified location</returns>
template <class T> T Volume<T>::getVoxel(unsigned int i, unsigned int j, unsigned int k)
{
	return voxels[(voxelCount[0] - i - 1)*voxelCount[1]*voxelCount[2]  + (voxelCount[2] - k - 1)  *voxelCount[1]+j] ;
}


/// <summary>
/// Sets the voxel.
/// </summary>
/// <param name="i">The i.</param>
/// <param name="j">The j.</param>
/// <param name="k">The k.</param>
/// <param name="value">The value.</param>
template <class T>
void Volume<T>::setVoxel(unsigned int i, unsigned int j, unsigned int k, T value)
{
	voxels[(voxelCount[0] - i - 1)*voxelCount[1]*voxelCount[2]  + (voxelCount[2] - k-1)  *voxelCount[1]+j]  = value;
}
 

/// <summary>
/// Finalizes an instance of the <see cref="Volume"/> class.
/// </summary>
template <class T>
Volume<T>::~Volume(void)
{
    //std::cout << "IN VOLUME DESTRUCTOR" << std::endl;
    //std::cout << "Volume destructor: Not explicitly deallocating voxels" << std::endl;
}


/// <summary>
/// Retrieves whether to process only a (vertical) slice of the specified volume.
/// </summary>
template <class T> 
bool Volume<T>::GetViewSlice()
{
	return view_slice;
}


/// <summary>
/// Retrieves the upper bound of the slice to process..
/// </summary>
template <class T> 
double Volume<T>::GetSliceUpperBound()
{
	return upper_bound;
}


/// <summary>
/// Retrieves the lower bound of the slice to process.
/// </summary>
template <class T>
double Volume<T>::GetSliceLowerBound()
{
	return lower_bound;
}

