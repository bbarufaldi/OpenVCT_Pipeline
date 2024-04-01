// XPL_ProjectionSimulation.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_PrivateData.h"
#include <string>
#include <vector>

namespace vct
{

class XPL_ProjectionSimulation : public PrivateData
{
public:
   
    // Manipulators (used to set component values)

    void setProgramName(std::string name)      { program_name = name; }
    void setSWVersion(std::string vers)        { sw_version = vers; }
    void setUniqueId(std::string id)           { unique_id = id; }
    void setInputPhantomFile(std::string fn)   { phantom_in = fn; }
    void setInputMeshFile(std::string msh)     { mesh_in = msh; }
    void setOutputMeshFile(std::string msh)    { mesh_out = msh; }
    void setOutputPhantomFile(std::string &fn) { phantom_out = fn; }

    // Accessors (retrieve data)

    std::string getProgramName()       { return program_name; }
    std::string getSWVersion()         { return sw_version; }
    std::string getUniqueId()          { return unique_id; }
    std::string getInputPhantomFile()  { return phantom_in; }
    std::string getInputMeshFile()     { return mesh_in; }
    std::string getOutputMeshFile()    { return mesh_out; }
    std::string getOutputPhantomFile() { return phantom_out; }

	void writeXML()
	{
        clib->writeNodeValueXML("Program_Name", program_name);
        clib->writeNodeValueXML("Software_Version", sw_version);
        clib->writeNodeValueXML("Software_ID", unique_id);
        clib->writeNodeValueXML("Input_Phantom", phantom_in);
        clib->writeNodeValueXML("Input_Mesh", mesh_in);
        clib->writeNodeValueXML("Output_Mesh", mesh_out);
        clib->writeNodeValueXML("Output_Phantom", phantom_out);
    }

    void readXML()
	{
        clib->getNodeValueXML("Program_Name", program_name);
        clib->getNodeValueXML("Software_Version", sw_version);
        clib->getNodeValueXML("Software_ID", unique_id);
        clib->getNodeValueXML("Input_Phantom", phantom_in);
        clib->getNodeValueXML("Input_Mesh", mesh_in);
        clib->getNodeValueXML("Output_Mesh", mesh_out);
        clib->getNodeValueXML("Output_Phantom", phantom_out);
    }


    // Constructor(s)

    XPL_ProjectionSimulation(std::string version, std::string id, 
                              std::string input_phantom, std::string input_mesh,
                              std::string output_mesh, std::string output_phantom)
        : program_name("BreastPhantomGenerator"),
          sw_version(version),
          unique_id(id),
		  phantom_in(input_phantom),
          mesh_in(input_mesh),
          mesh_out(output_mesh),
          phantom_out(output_phantom)
    {}
    
	XPL_ProjectionSimulation()
        : program_name("BreastPhantomGenerator"),
          sw_version("0"),
          unique_id("0")
    {}


private:

    std::string program_name;
    std::string sw_version;
    std::string unique_id;
    std::string phantom_in;
    std::string attenuationTableFile;
    std::string configFile;
    std::string outputPrefix;
    std::string seed;
    
    std::string opt_no_noise;
    std::string opt_elec_noise;
    std::string opt_gain;
    std::string opt_mAs;
    std::string opt_prop_constant;
    std::string opt_min_gray;
    std::string opt_use_scaling;
    std::string opt_rand_quad;
    std::string opt_no_inv_sq;
    
    bool        use_noise;
    double      elec_noise;
    double      gain;
    double      mAs;
    double      prop_const;
    unsigned short min_gray;    
};

}