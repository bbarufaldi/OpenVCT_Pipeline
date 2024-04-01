// AttenuationTable.cpp

#include "AttenuationTable.h"
#include "ReadPhantom.h"
#include "pugixml.hpp"
#include <vector>
#include <sstream>
#include <string>
#include <iostream>


typedef std::map<unsigned short, std::vector<double> *> MapType;


AttenuationTable::AttenuationTable()
{
    ReadPhantom &rph = ReadPhantom::getInstance();
    std::string attenuationFile = rph.getAttenFile();
    if (attenuationFile.size() > 0)
    {
        initialize(attenuationFile);
    }
}


AttenuationTable::AttenuationTable(std::string filePath)
{
	pugi::xml_document doc;
	if (filePath.size() > 0 && doc.load_file(filePath.c_str()))
	{
		 pugi::xml_node tools = doc.first_child();
		 for (pugi::xml_node tool = tools.first_child(); tool; tool = tool.next_sibling())
		 {
			 pugi::xml_node nameNode = tool.first_child();        // Name node    eg: <ns1:Name>NIST_1_H.txt</ns1:Name>
			 pugi::xml_node zNode = nameNode.next_sibling();      // Z node       eg: <ns1:Z>1</ns1:Z>
			 unsigned short z;
			 std::stringstream(zNode.first_child().value()) >> z; // Z value      eg: 1
			 
			 double densityValue=0.0;
			 pugi::xml_node firstAttNode;
			 if (tool.child("ns1:Density"))                       // Density node eg: <ns1:Density>0.09</ns1:Density>
			 {
				 pugi::xml_node densityNode = zNode.next_sibling(); //density			 
				 std::stringstream(densityNode.first_child().value()) >> densityValue;
				 firstAttNode = densityNode.next_sibling();
			 }
			 else
			 {
				 firstAttNode = zNode.next_sibling();
			 }
			 density.insert(std::pair<unsigned short, double> (z, densityValue));

		
			 // Misnomer alert: tempTable is not temporary - DH 2016-04-08
			 std::vector<double> * tempTable = new std::vector<double>; // We don't care about disallocate (don't deallocate until end of program - DH 2016-04-08)
			 for (pugi::xml_node attNode =firstAttNode; attNode; attNode = attNode.next_sibling())
			 {				 
	 			 const pugi::char_t * line = attNode.name(); //att
				 double attValue;
				 const pugi::char_t * value = attNode.first_child().value();
				 std::stringstream(value) >> attValue;
				 tempTable->push_back(attValue);
			 }
			 table.insert(std::pair<unsigned short, std::vector<double> *> (z, tempTable));           
		 }		 
	}
	else
	{
		std::cout << "Unable to open " << filePath.c_str() << std::endl;
	}
}


AttenuationTable::~AttenuationTable(void)
{}


void AttenuationTable::initialize(std::string attenFilename)
{
    // TODO: Replace pugixml with vct CommonLibrary XML interface
    
    //std::cout << "INITIALIZING ATTENUATION FILE \"" << attenFilename << "\"" << std::endl;
	pugi::xml_document doc;
	if (doc.load_file(attenFilename.c_str()))
	{
		 pugi::xml_node tools = doc.first_child();
		 for (pugi::xml_node tool = tools.first_child(); tool; tool = tool.next_sibling())
		 {
			 pugi::xml_node nameNode = tool.first_child();        // Name node    eg: <Name>NIST_1_H.txt</Name>
			 pugi::xml_node zNode = nameNode.next_sibling();      // Z node       eg: <Z>1</Z>
			 unsigned short z;
			 std::stringstream(zNode.first_child().value()) >> z; // Z value      eg: 1
			 
			 double densityValue=0.0;
			 pugi::xml_node firstAttNode;
			 if (tool.child("Density"))                           // Density node eg: <Density>0.09</Density>
			 {
				 pugi::xml_node densityNode = zNode.next_sibling(); //density			 
				 std::stringstream(densityNode.first_child().value()) >> densityValue;
				 firstAttNode = densityNode.next_sibling();
			 }
			 else
			 {
				 firstAttNode = zNode.next_sibling();
			 }
			 density.insert(std::pair<unsigned short, double> (z, densityValue));
		
			 // Misnomer alert: tempTable is not temporary - DH 2016-04-08
			 std::vector<double> * tempTable = new std::vector<double>; // We don't care about disallocate (don't deallocate until end of program - DH 2016-04-08)
			 for (pugi::xml_node attNode =firstAttNode; attNode; attNode = attNode.next_sibling())
			 {				 
	 			 const pugi::char_t * line = attNode.name(); //att
				 double attValue;
				 const pugi::char_t * value = attNode.first_child().value();
				 std::stringstream(value) >> attValue;
				 tempTable->push_back(attValue);
			 }
             //std::cout << "\ttempTable for material " << z << " has " << tempTable->size() << " entries." << std::endl;
			 table.insert(std::pair<unsigned short, std::vector<double> *> (z, tempTable)); 
		 }
	}
	else
	{
		std::cout << "Unable to open attenuationFile \"" << attenFilename.c_str() << "\"" << std::endl;
	}
}

/// <summary>
/// Gets the attentuation.
/// </summary>
/// <param name="materialIndex">Index of the material.</param>
/// <param name="energy">The energy.</param>
/// <returns></returns>
double AttenuationTable::getAttenuation(unsigned short materialIndex, unsigned short energy)
{
	MapType::iterator iter = table.begin();
	iter = table.find(materialIndex);
    if (iter != table.end() ) 
        return (*iter->second)[energy];
    else
    {
        //std::cout << __FUNCTION__ << ": material " << materialIndex << ", energy " << energy << " not found, resorting to 0.0" << std::endl;
        return 0.0;
    }
}


/// <summary>
/// Gets the attentuation table.
/// </summary>
/// <param name="materialIndex">Index of the material.</param>
/// <returns></returns>
std::vector<double> * AttenuationTable::getAttenuationTable(unsigned short materialIndex)
{
	MapType::iterator iter = table.begin();
	iter = table.find(materialIndex);
    if (iter != table.end() ) 
	{

		return iter->second;
	}
    else
	{
        return NULL;
	}
}

/// <summary>
/// Gets the density.
/// </summary>
/// <param name="materialIndex">Index of the material.</param>
/// <returns></returns>
double AttenuationTable::getDensity(unsigned short materialIndex)
{
	std::map<unsigned short, double>::iterator iter = density.begin();
	iter = density.find(materialIndex);
    if (iter != density.end()) 
        return iter->second;
    else
        return 0;
}


/// <summary>
/// Gets the material count.
/// </summary>
/// <returns></returns>
size_t AttenuationTable::getMaterialCount()
{
	return table.size();
}

/// <summary>
/// Gets the energy count.
/// </summary>
/// <param name="materialIndex">Index of the material.</param>
/// <returns></returns>
size_t AttenuationTable::getEnergyCount(unsigned short materialIndex)
{
	MapType::iterator iter = table.begin();
	iter = table.find(materialIndex);
    if (iter != table.end() ) 
        return iter->second->size();
    else
        return 0;
}