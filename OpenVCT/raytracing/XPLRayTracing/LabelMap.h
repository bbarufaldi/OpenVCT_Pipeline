// LabelMap.h

#pragma once

#include "pugixml.hpp"
#include <vector>
#include <map>

/// <summary>
/// A class of material information used in the calculation of attenuation of a label value
/// </summary>
class Material
{
public:

	Material(double weight, double density, unsigned short materialZ);
	double weight, density;
	unsigned short materialZ;
};

/// <summary>
/// 
/// </summary>
class LabelMap
{
public:

    LabelMap();
	LabelMap(pugi::xml_node &node);
	~LabelMap();

	const std::vector<Material> &getMaterialMap() { return materialMap; }
    void addMaterial(Material mat);

private:
	std::vector<Material> materialMap;
};

