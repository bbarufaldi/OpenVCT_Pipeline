// XPL_BreastPhantomInserter.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_Software.h"
#include "VCT_LesionMask.h"
#include "VCT_PrivateData.h"
#include <map>

namespace vct
{

class XPL_BreastPhantomInserter : public PrivateData
{
public:

    // Manipulators (used to set component values)

    void setProgramName(std::string name)           { program_name = name; }
    void setSWVersion(std::string vers)             { sw_version = vers; }
    void setUniqueId(std::string id)                { unique_id = id; }
    void setInputPhantom(std::string filename)      { phantom_filename_in = filename; }
    void setOutputPhantom(std::string filename)     { phantom_filename_out = filename; }
    void setRadius(float radius)                    { sph_radius = radius; }
    void setMaterial(int mat)                       { sph_material = mat; }
    void setBlend(int blend)                        { blend_material = blend; }
    void setInsertX(float x)                        { insert_x = x; }
    void setInsertY(float y)                        { insert_y = y; }
    void setInsertZ(float z)                        { insert_z = z; }
    
    // Accessors (used to retrieve component values)

    std::string      setProgramName()       { return program_name; }
    std::string      setSWVersion()         { return sw_version; }
    std::string      setUniqueId()          { return unique_id; }
    std::string      getInputPhantom()      { return phantom_filename_in; }
    std::string      getOutputPhantom()     { return phantom_filename_out; }
    float            setRadius()            { return sph_radius; }
    int              setMaterial()          { return sph_material; }
    int              setBlend()             { return blend_material; }
    float            setInsertX()           { return insert_x; }
    float            setInsertY()           { return insert_y; }
    float            setInsertZ()           { return insert_z; }


    // XML Interaction routines


    void writeXML()
    {  
        clib->writeNodeValueXML("Software_Name", sw_name);
        clib->writeNodeValueXML("Software_Version", sw_version);
        clib->writeNodeValueXML("Software_ID", unique_id);
        clib->writeNodeValueXML("Input_Phantom", phantom_filename_in);
        clib->writeNodeValueXML("Output_Phantom", phantom_filename_out);
        clib->writeNodeValueXML("Sphere_Radius", sph_radius);
        clib->writeNodeValueXML("Sphere_Material", sph_material);
        clib->writeNodeValueXML("Blend_Material", blend_material);
        clib->writeNodeValueXML("Insert_X", insert_x);
        clib->writeNodeValueXML("Insert_Y", insert_y);
        clib->writeNodeValueXML("Insert_Z", insert_z);
    }
    
	void readXML()
	{
        clib->getNodeValueXML("Software_Name", sw_name);
        clib->getNodeValueXML("Software_Version", sw_version);
        clib->getNodeValueXML("Software_ID", unique_id);
        clib->getNodeValueXML("Input_Phantom", phantom_filename_in);
        clib->getNodeValueXML("Output_Phantom", phantom_filename_out);
        clib->getNodeValueXML("Sphere_Radius", sph_radius);
        clib->getNodeValueXML("Sphere_Material", sph_material);
        clib->getNodeValueXML("Blend_Material", blend_material);
        clib->getNodeValueXML("Insert_X", insert_x);
        clib->getNodeValueXML("Insert_Y", insert_y);
        clib->getNodeValueXML("Insert_Z", insert_z);
    }

    // Constructor(s)
    
	XPL_BreastPhantomInserter()
        : sw_name("BreastPhantomInserter"),
          sw_version("1.0"),
          unique_id("1.0")
    {
        PrivateData::setProgramName("BreastPhantomInserter");
    }

private:

    std::string sw_name;
    std::string sw_version;
    std::string unique_id;
    std::string phantom_filename_in;
    std::string phantom_filename_out;
    float sph_radius;
    int   sph_material;
    int   blend_material;
    float insert_x;
    float insert_y;
    float insert_z;

};

}
