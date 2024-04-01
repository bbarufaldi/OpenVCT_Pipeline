// VCT_Software.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_Date.h"
#include "VCT_Time.h"
#include <string>


#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

#pragma region Documentation
/// <summary>This class is responsible for managing software-related states and functions. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
class Software
{
public:
    
    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Software()
        : sw_name(),
          sw_version(),
          repository(),
          build_date(),
          build_time()
    {}


    #pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="name">Name of the software. </parm>
    /// <parm name="vers">Version of the software. </parm>
    /// <parm name="repo">Repository where the software is controlled. </parm>
    /// <parm name="date">Date of the software build. </parm>
    /// <parm name="time">Time of the software build. </parm>
    #pragma endregion
    Software(std::string name, std::string vers, std::string repo, Date date, Time time)
        : sw_name(name),
          sw_version(vers),
          repository(repo),
          build_date(date),
          build_time(time)
    {}
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Software(const Software &rhs)
        : sw_name(rhs.sw_name),
          sw_version(rhs.sw_version),
          repository(rhs.repository),
          build_date(rhs.build_date),
          build_time(rhs.build_time)
    {}
    

    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Software()
    {}

    
    // Manipulators (used to set component values)

    #pragma region Documentation
    /// <summary>Set the name of the software. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="name">The software's name. </parm>
    #pragma endregion
    void setName(std::string name)       { sw_name = name; }


    #pragma region Documentation
    /// <summary>Set the version of the software. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vers">The software version. </parm>
    #pragma endregion
    void setVersion(std::string vers)    { sw_version = vers; }
    
    
    #pragma region Documentation
    /// <summary>Set repository where the software is controlled. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="repo">The software's repository. </parm>
    #pragma endregion
    void setRepository(std::string repo) { repository = repo; }
    
    
    #pragma region Documentation
    /// <summary>Set the date of the software build. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="date">The date of the software build. </parm>
    #pragma endregion
    void setDate(vct::Date date)         { build_date = date; }
    
    
    #pragma region Documentation
    /// <summary>Set the time of the software build. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="time">The time of the software build. </parm>
    #pragma endregion
   void setTime(vct::Time time)         { build_time = time; }
    

    // Accessors (used to retrieve component values)


    #pragma region Documentation
    /// <summary>Retrieve the software name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The software version. </return>
    #pragma endregion
    std::string getName()        { return sw_name; }


    #pragma region Documentation
    /// <summary>Retrieve the software's version. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The software's version. </return>
    #pragma endregion
    std::string getVersion()     { return sw_version; }


    #pragma region Documentation
    /// <summary>Retrieve the software repository. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The software repository. </return>
    #pragma endregion
    std::string getRepository()  { return repository; }


    #pragma region Documentation
    /// <summary>Retrieve the software's build date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The software's build date. </return>
    #pragma endregion
    vct::Date getDate()          { return build_date; }


    #pragma region Documentation
    /// <summary>Retrieve the software's build time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The software's build time. </return>
    #pragma endregion
    vct::Time getTime()          { return build_time; }


    // XML file Interaction 


    #pragma region Documentation
    /// <summary>Read a software section from the current XML file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->getNodeValueXML("Name", sw_name);
            clib->getNodeValueXML("Version", sw_version);
            clib->getNodeValueXML("Repository", repository);
            
            clib->findNodeXML("Build_Date");
            build_date.readXML(clib);
            clib->concludeSectionXML();
            
            clib->findNodeXML("Build_Time");
            build_time.readXML(clib);
            clib->concludeSectionXML();
        }
    }
    
    
    #pragma region Documentation
    /// <summary>Write a software section to the current XML file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->writeNodeValueXML("Name", sw_name);
            clib->writeNodeValueXML("Version", sw_version);
            clib->writeNodeValueXML("Repository", repository);

            clib->addASectionXML("Build_Date");
            build_date.writeXML(clib);
            clib->concludeSectionXML();

            clib->addASectionXML("Build_Time");
            build_time.writeXML(clib);
            clib->concludeSectionXML();
        }
    }


private:

    std::string  sw_name;    /// the name of the software (program/application)
    std::string  sw_version; /// the software version
    std::string  repository; /// the repository where the software is configuration managed
    Date         build_date; /// the software build date
    Time         build_time; /// the software build time
};

}
