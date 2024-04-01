// VCT_Trial.h

#pragma once

#include "VCT_CommonLibrary.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>


namespace vct
{

class Trial
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Trial() : vct_version("unknown_version")
    {}
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    #pragma endregion
    Trial(const Trial &rhs)
        : vct_version(rhs.vct_version)
    {}


    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Trial()
    {}


    #pragma region Documentation
    /// <summary>Set the Version field of the Trial object. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vers">A string representing the version. </parm>
    #pragma endregion
    void setVctVersion(std::string vers) { vct_version = vers; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the Version. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    std::string getVctVersion() { return vct_version; }


    // XML file Interaction 


    #pragma region Documentation
    /// <summary>Read the Trial section of the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->getNodeValueXML("VCT_Version", vct_version);
        }
    }
    
    #pragma region Documentation
    /// <summary>Write this Trial's section in the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->writeNodeValueXML("VCT_Version", vct_version);
        }
    }

private:
    std::string  vct_version;
};

}
