// LabelMap.cpp

#include "LabelMap.h"
#include <sstream>


Material::Material(double w, double d, unsigned short mat) : weight(w), density(d), materialZ(mat)
{}


/// <summary>
/// Initializes a new instance of the <see cref="LabelMap"/> class.
/// </summary>
LabelMap::LabelMap()
{}

/// <summary>
/// Initializes a new instance of the <see cref="LabelMap"/> class.
/// </summary>
/// <param name="node">The node.</param>
LabelMap::LabelMap(pugi::xml_node &node)
{
    for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling())
    {
        double w=0.0, d=-1.0; // OpenCL version depends on this initialization (-1.0) - DH 2016-08-04
        unsigned short mat;
        pugi::xml_node matNode = childNode;
        std::stringstream(matNode.child("Weight").first_child().value()) >> w;
      
        if (matNode.child("Density"))
        {
             std::stringstream(matNode.child("Density").first_child().value()) >> d;
        }
        std::stringstream(matNode.child("MaterialZ").first_child().value()) >> mat;

        materialMap.push_back(Material(w, d, mat));
    }
}


LabelMap::~LabelMap(void)
{
}

void LabelMap::addMaterial(Material mat)
{
    materialMap.push_back(mat);
}
