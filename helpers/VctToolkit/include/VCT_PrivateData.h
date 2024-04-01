// VCT_PrivateData.h

#pragma once

#include "VCT_CommonLibrary.h"
#include <string>


#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

#pragma region Documentation
/// <summary> Private class for Open Virtual Clinical Trials. </summary>
/// <remarks> This class serves as a basis for inheritance only; it cannot be instantiated directly.</remarks>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class PrivateData
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    PrivateData()
        : clib(nullptr),
          program_name(""),
          abs_filename(""),
          rel_filename("")
    {}
    

    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>Copy constructor is private to prevent attempted copying (due to non-copyable, underlying streams.</remarks>
    #pragma endregion
    PrivateData(const PrivateData &rhs)
        : clib(nullptr),
          program_name(rhs.program_name),
          abs_filename(rhs.abs_filename),
          rel_filename(rhs.rel_filename)
    {
        if (rhs.clib != nullptr) clib = new CommonLibrary(*rhs.clib);
    }


    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~PrivateData()
    {
        if (clib != nullptr)
        {
            delete clib;
            clib = nullptr;
        }
    }
    
    
    #pragma region Documentation
    /// <summary>Set the address of an instance of the Common Library. </summary>
    /// <parm name="cl">A pointer to an instance of the Common Library.</parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="cl">CommonLibrary pointer (for current context). </parm>
    /// <remarks>This is a virtual function.</remarks>    
    #pragma endregion
    virtual void setCommonLib(CommonLibrary *cl) { clib = cl; }
    
    
    #pragma region Documentation
    /// <summary>Set the name of the application.</summary>
    /// <parm name="nam">The name of the program.</parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="nam">The program name. </parm>
    /// <remarks>This is a virtual function.</remarks>   
    #pragma endregion
    virtual void setProgramName(std::string nam) { program_name = nam; }

    
    #pragma region Documentation
    /// <summary>Get the name of the application.</summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The name of the program.</return>
    /// <remarks>This is a virtual function.</remarks>   
    #pragma endregion
    virtual std::string getProgramName() { return program_name; }

    
    #pragma region Documentation
    /// <summary>Write the XML data associated with this class or its descendants.</summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>This is a pure virtual function; this entire class must be subclassed.</remarks>   
    #pragma endregion
    virtual void writeXML() = 0;

protected:

    CommonLibrary *clib;      /// Pointer to Common Library instance

    std::string program_name; /// Name of the application producing this data
    std::string abs_filename; /// Absolution path and filename
    std::string rel_filename; /// Relative path and filename
};

}