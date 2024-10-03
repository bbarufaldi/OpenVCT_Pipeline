// XPL_AttenuationTable.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_PrivateData.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>


namespace vct
{

// Default materials to be stored in the attenuation file
// ..these are used for projection filters
const int ALUMINUM   = 13;
const int MOLYBDENUM = 42;
const int RHODIUM    = 45;
const int TUNGSTEN   = 74;


class AttenPair
{
public:
    std::string name;
    std::string value;
};



class AttenMaterial
{
public:
    std::list<AttenPair> atten_fields;
};



class XPL_AttenuationTable : public PrivateData
{
public:


    // Manipulators (used to set component values)




    // Accessors (retrieve data)



    void addMaterial(int mat)
    {
        materials.push_back(mat);
    }



    bool buildAttenTable()
    {
        bool retval = false;

        // Add common materials that are used for Projection
        materials.push_back(ALUMINUM);
        materials.push_back(MOLYBDENUM);
        materials.push_back(RHODIUM);
        materials.push_back(TUNGSTEN);


        // Sort the material list and remove duplicates to match the order in the original - and very large - attenuationTable
        std::sort(materials.begin(), materials.end());
        std::unique(materials.begin(), materials.end());

        std::ifstream atten("attenuationTable.xml");
        if (atten)
        {
            for (auto m : materials)
            {
                findMaterial(atten, m);
            }
            atten.close();
        }
        else
        {
            std::cerr << "Could not open attenuationTable.xml" << std::endl;
        }

        return retval;
    }


    bool extractNameValuePair(std::string &line, std::string &name, std::string &value)
    {
        bool retval = false;

        std::string::size_type pos1, pos2, pos3;

        pos1 = line.find("<ns1", 0);
        if (pos1 != std::string::npos)
        {
            pos2 = line.find(">", pos1);
            if (pos2 != std::string::npos)
            {
                name = line.substr(pos1 + 5, pos2 - pos1 - 5); // skip over "ns1:" (problematic for CommonLib XML functions)

                pos3 = line.find("<", pos2);
                if (pos3 != std::string::npos)
                {
                    value = line.substr(pos2 + 1, pos3 - pos2 - 1);
                    value = clib->trim(value);
                    //std::cout << "Name: \"" << name << ", value: \"" << value << "\"" << std::endl;
                    retval = true;
                }
            }
        }

        return retval;
    }


    bool findMaterial(std::ifstream &atten, int mat)
    {
        bool retval = false;

        std::ostringstream sch;
        sch << "<ns1:Z>" << mat << "</ns1:Z>";
        std::string sch_str(sch.str());
        //std::cout << "looking for \"" << sch_str << "\"\n";

        bool capture = false;
        std::string line, prev_line;

        AttenMaterial amat;
        AttenPair     apair;

        while (!atten.eof())
        {
            getline(atten, line);
            if (line.find(sch_str) != std::string::npos)
            {
                capture = true;
            }
            if (capture)
            {
                if (extractNameValuePair(prev_line, apair.name, apair.value))
                {
                    amat.atten_fields.push_back(apair);
                }
            }
            if (line.find("</ns1:MaterialAttenuation>") != std::string::npos)
            {
                if (capture)
                {
                    break;
                }
                capture = false;
            }
            prev_line = line;
        }
        if (amat.atten_fields.size() > 0)  attentable.push_back(amat);

        return retval;
    }


    // XML file Interaction 


    void writeXML()
    {
        for (auto m : attentable)
        {
            clib->addASectionXML("MaterialAttenuation");
            //std::cout << "XPL_AttenuationTable.h writeXML(): writing material\n";

            for (auto p : m.atten_fields)
            {
                clib->writeNodeValueXML(p.name, p.value);
            }
            clib->concludeSectionXML();
        }
    }


    void readXML()
    {
        clib->getNodeValueXML("Software_Name", sw_name);
        clib->getNodeValueXML("Software_Version", sw_version);
        clib->getNodeValueXML("Software_ID", unique_id);
        clib->getNodeValueXML("Phantom_Name", phantom_name);
    }


    // Constructor(s)


    XPL_AttenuationTable()
        : sw_name("XPL_AttenuationTable"),
          sw_version("0"),
          unique_id("0"),
          phantom_name()
    {
        PrivateData::setProgramName("XPL_AttenuationTable");
    }

private:

    std::string sw_name;
    std::string sw_version;
    std::string unique_id;
    std::string phantom_name;

    std::vector<int> materials;
    std::list<AttenMaterial> attentable;
};

}