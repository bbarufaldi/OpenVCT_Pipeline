// VCT_Station.h

#pragma once

#include "VCT_CommonLibrary.h"
#include <string>
#if !defined(_MSC_VER)
    #include <sys/utsname.h>
#endif

#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

#pragma region Documentation
/// <summary> Station class for Open Virtual Clinical Trials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class Station
{
public:

	#pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ws">The station's name. </parm>
    /// <parm name="os">The station's operating system. </parm>
    /// <parm name="arch">The station's architecture. </parm>
    #pragma endregion
    Station(std::string &ws, std::string &os, std::string &arch)
        : workstation(ws),
          station_os(os),
          architecture(arch)
    {}

    
    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Station()
        : workstation("Unknown_Workstation"),
          station_os("Unknown_OS"),
          architecture("Unknown_Architecture")
    {}
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Station(const Station &rhs)
        : workstation(rhs.workstation),
          station_os(rhs.station_os),
          architecture(rhs.architecture)
    {}

    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Station()
    {}

    
    // Manipulators (used to set component values)

    #pragma region Documentation
    /// <summary>Set the station's name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="station">The station's name. </parm>
    #pragma endregion
    void setStation(std::string station)   { workstation = station; }
    
    
    #pragma region Documentation
    /// <summary>Set the station's operating system. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="os">The station's operating system. </parm>
    #pragma endregion
    void setOS(std::string os)             { station_os = os; }
    
    
    #pragma region Documentation
    /// <summary>Set the station's architecture. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="arch">The station's architecture. </parm>
    #pragma endregion
    void setArchitecture(std::string arch) { architecture = arch; }

    
    // Accessors (used to retrieve component values)

    
    #pragma region Documentation
    /// <summary>Retrieve the station's name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The station's name. </return>
    #pragma endregion
    std::string getStation()               { return workstation; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the station's operating system. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The station's operating system. </return>
    #pragma endregion
    std::string getOS()                    { return station_os; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the station's architecture. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The station's architecture. </return>
    #pragma endregion
    std::string getArchitecture()          { return architecture; }

    
    // XML file Interaction 

    
    #pragma region Documentation
    /// <summary>Read the station section of the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        clib->getNodeValueXML("Name", workstation);
        clib->getNodeValueXML("OS", station_os);
        clib->getNodeValueXML("Architecture", architecture);
    }

    
    #pragma region Documentation
    /// <summary>Write this station'ssection in the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        clib->writeNodeValueXML("Name", workstation);
        clib->writeNodeValueXML("OS", station_os);
        clib->writeNodeValueXML("Architecture", architecture);
    }


    #pragma region Documentation
    /// <summary>Query the current system to find station information.</summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void querySystem()
    {
        #if defined(_MSC_VER)
        
            station_os = getenv("OS"); // DIES HERE ON LINUX
        
            #pragma warning(disable : 4996)
            
            DWORD vers = GetVersion();
            int major_version = DWORD(LOBYTE(LOWORD(vers))); // eg 6
            int minor_version = DWORD(HIBYTE(LOWORD(vers))); // eg 2

            switch(major_version)
            {
            case 10:
                station_os = "Windows 10";  break; // or windows server 2016
                break;
            case 6:
                switch(minor_version)
                {
                case 3: station_os = "Windows 8.1";   break; // or windows server 2012 r2
                case 2: station_os = "Windows 8";     break; // or windows server 2012
                case 1: station_os = "Windows 7";     break; // or windows server 2008 r2
                case 0: station_os = "Windows Vista"; break; // or windows server 2008
                };
                break;
            case 5:
                switch(minor_version)
                {
                case 2: station_os = "Windows Server 2003"; break; // or windows server 2003 r2
                case 1: station_os = "Windows XP";          break;
                case 0: station_os = "Windows 2000";        break;
                };
                break;
            };
            workstation = getenv("COMPUTERNAME");
            architecture = getenv("PROCESSOR_ARCHITECTURE");

        #else

            struct utsname uts;
            uname(&uts);
            workstation  = uts.nodename;
            architecture = uts.machine;
            station_os   = uts.sysname;
            station_os += ", release";
            station_os += uts.release;
            station_os += ", version";
            station_os += uts.version;

        #endif

    }

private:

    std::string  workstation;
    std::string  station_os;
    std::string  architecture;
};

}
