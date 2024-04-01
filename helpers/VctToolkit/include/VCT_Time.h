// VCT_Time.h

#pragma once

#include "VCT_CommonLibrary.h"
#include <ctime>
#include <sstream>
#include <string>


#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

#pragma region Documentation
/// <summary>This class is responsible for managing a single instance of utc_offset. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
class UTC_Offset
{
public:
    
    #pragma region Documentation
    /// <summary>Set or get the value of UTC Offset. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>This design enables the definition of a static variable completely - and atypically - in a header file.</remarks>
    /// <parm name="offset">The UTC Offset value. </parm>
    /// <parm name="set">If true, the set the UTC offset value (otherwise retrieve its value). </parm>
    #pragma endregion
    static void setOrGetUtcOffset(short &offset, bool set)
    {
        static short utc_offset = 0; /// Static variable: current offset from UTC (a factor of timezones & DST) 
        if (set) utc_offset = offset;
        else offset = utc_offset;
    }
};


#pragma region Documentation
/// <summary>This class is responsible for managing date-related states and functions. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
class Time : public UTC_Offset
{
public:
    
    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Time() // Default Constructor
        : hour(0u),
          minute(0u),
          seconds(0u),
          frac_nsec(0u)
    {
        time_t tim = time(NULL);
        struct tm *tms = localtime(&tim);
        hour    = static_cast<unsigned int>(tms->tm_hour);
        minute  = static_cast<unsigned int>(tms->tm_min);
        seconds = static_cast<unsigned int>(tms->tm_sec);
    }
    
    
    #pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="timestring">A string representation of a time (hh:mm:ss). </parm>
    #pragma endregion
    Time(std::string timestring) // Default Constructor
    {
        char colon;
        std::istringstream iss(timestring);
        iss >> hour >> colon >> minute >> colon >> seconds;  
        frac_nsec = 0u;
    }
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Time(const Time &rhs)
        : hour(rhs.hour),
          minute(rhs.minute),
          seconds(rhs.seconds),
          frac_nsec(rhs.frac_nsec)
    {}    

    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Time()
    {}

    
    // Manipulators (used to set component values)


    #pragma region Documentation
    /// <summary>Set the hour component of the time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="h">The hour (24-hour clock - values between 0 and 24, inclusively). </parm>
    /// <return>True if value is accepted (otherwise false). </return>
    #pragma endregion
    bool setHour(unsigned int h)      { if (h <= 24)           { hour       = h; return true; } return false; }
    
    
    #pragma region Documentation
    /// <summary>Set the minute component of the time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="m">The minute (values between 0 and 60, inclusively). </parm>
    /// <return>True if value is accepted (otherwise false). </return>
    #pragma endregion
    bool setMinute(unsigned int m)    { if (m < 60)            { minute     = m; return true; } return false; }
    
    
    #pragma region Documentation
    /// <summary>Set the seconds component of the time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="s">The seconds (values between 0 and 60, inclusively). </parm>
    /// <return>True if value is accepted (otherwise false). </return>
    #pragma endregion
    bool setSeconds(unsigned int s)   { if (s < 60)            { seconds    = s; return true; } return false; }
    
    
    #pragma region Documentation
    /// <summary>Set the fractional seconds (nanoseconds) component of the time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="f">The fractional seconds (values between 0 and 9999, inclusively). </parm>
    /// <return>True if value is accepted (otherwise false). </return>
    #pragma endregion
    bool setFracNSec(unsigned int f)  { if (f < 10000000)      { frac_nsec  = f; return true; } return false; }
    
    
    #pragma region Documentation
    /// <summary>Set the Universal Coordinated Time (UTC) offset. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="u">The UTC offset (values between -24 and +24, inclusively). </parm>
    /// <return>True if value is accepted (otherwise false). </return>
    #pragma endregion
    bool setUtcOffset(short u) { if (u < 13 && u > -13) { setOrGetUtcOffset(u, true); return true; } return false; }
    
    
    // Accessors (used to retrieve component values)


    #pragma region Documentation
    /// <summary>Retrieve the hour. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The hour. </return>
    #pragma endregion
    unsigned int getHour()     { return hour; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the minute. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The minute. </return>
    #pragma endregion
    unsigned int getMinute()   { return minute; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the seconds. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The seconds. </return>
    #pragma endregion
    unsigned int getSeconds()  { return seconds; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the fractional seconds (nanoseconds). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The fractional seconds. </return>
    #pragma endregion
    unsigned int getFracNSec() { return frac_nsec; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the UTC offset. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The UTC offset. </return>
    #pragma endregion
    short getUtcOffset()       { short u; setOrGetUtcOffset(u, false); return u; }

    
    // XML Processing


    #pragma region Documentation
    /// <summary>Read a Time section from the current XML file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->getNodeValueXML("Hour", hour);
            clib->getNodeValueXML("Minute", minute);
            clib->getNodeValueXML("Seconds", seconds);
            clib->getNodeValueXML("Fraction", frac_nsec);
            short utc_offset;
            clib->getNodeValueXML("UTC_Offset", utc_offset);
            setOrGetUtcOffset(utc_offset, true);
        }
    }


    #pragma region Documentation
    /// <summary>Write a Time section to the current XML file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            short utc_offset;
            setOrGetUtcOffset(utc_offset, false);

            std::ostringstream oss;
            oss << std::setfill('0') << std::setw(2) << hour;
            std::string hour_string(oss.str());

            oss.str(""); oss.clear();
            oss << std::setfill('0') << std::setw(2) << minute;
            std::string min_string(oss.str());

            oss.str(""); oss.clear();
            oss << std::setfill('0') << std::setw(2) << seconds;
            std::string sec_string(oss.str());

            oss.str(""); oss.clear();
            oss << std::setfill('0') << std::setw(4) << frac_nsec;
            std::string frac_string(oss.str());

            oss.str(""); oss.clear();
            oss << std::setfill('0') << std::setw(2) << utc_offset;
            std::string utc_string(oss.str());

            clib->writeNodeValueXML("Hour", hour_string);
            clib->writeNodeValueXML("Minute", min_string);
            clib->writeNodeValueXML("Seconds", sec_string);
            clib->writeNodeValueXML("Fraction", frac_string);
            clib->writeNodeValueXML("UTC_Offset", utc_string);
        }
    }


    #pragma region Documentation
    /// <summary>Set this time to the current time (UTC not affected). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void setNow()
    {
        time_t tim = time(NULL);
        struct tm *tms = localtime(&tim);
        hour    = static_cast<unsigned int>(tms->tm_hour);
        minute  = static_cast<unsigned int>(tms->tm_min);
        seconds = static_cast<unsigned int>(tms->tm_sec);
    }


    #pragma region Documentation
    /// <summary>Convert this time to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>A string representing this time. </return>
    #pragma endregion
    std::string toString()
    {
        short utc_offset;
        setOrGetUtcOffset(utc_offset, false);
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(2) << hour << ":" 
            << std::setfill('0') << std::setw(2) << minute << ":" 
            << std::setfill('0') << std::setw(2) << seconds << "."
            << std::setfill('0') << std::setw(6) << frac_nsec
            << ", UTC Offset: "  << utc_offset;
            
        return oss.str();
    }


private:

    unsigned int hour;       /// 24 hours in a day
    unsigned int minute;     /// 60 mins in an hour
    unsigned int seconds;    /// 60 whole seconds in a minute
    unsigned int frac_nsec;  /// fractional part of seconds (nanoseconds)
};


} // namespace vct