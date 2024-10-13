// VCT_Date.h

#pragma once

#include "VCT_CommonLibrary.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

#pragma region Documentation
/// <summary>This class is responsible for managing date-related states and functions. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
class Date
{
public:
    
    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Date() // Default Constructor
    {
        time_t tim = time(NULL);
        struct tm *tms = localtime(&tim);
        curr_year = 1900 + tms->tm_year;
        year = curr_year;
        month = tms->tm_mon + 1;
        day = tms->tm_mday;
    }
    
    
    #pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="datestring">A string representation of a date (yyyy-mm-dd-).</parm>
    #pragma endregion
    Date(std::string datestring) // Default Constructor
    {
        char delin;
        std::istringstream iss(datestring);
        iss >> year >> delin >> month >> delin >> day;  
        if (year < 100) year += 1900;
    }
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Date(const Date &rhs)
        : curr_year(rhs.curr_year),
          year(rhs.year),
          month(rhs.month),
          day(rhs.day)
    {}

    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Date()
    {}


    // Manipulators (used to set component values)


    #pragma region Documentation
    /// <summary>Set the year component of the date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="y">The (four digit) year. </parm>
    /// <return>True if value is accepted (otherwise false). </return>
    #pragma endregion
    bool setYear(int y)           { if (y <= curr_year) { year  = y; return true; } return false; }


    #pragma region Documentation
    /// <summary>Set the month component of the date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="m">The (two digit) month. </parm>
    /// <return>True if value is accepted (otherwise false). </return>
    #pragma endregion
    bool setMonth(unsigned int m) { if (m > 0 && m <= 12) { month = m; return true; } return false; }
    
    
    #pragma region Documentation
    /// <summary>Set the day component of the date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="d">The (numeric) day. </parm>
    /// <return>True if value is accepted (otherwise false). </return>
    #pragma endregion
    bool setDay(unsigned int d)   { if (d > 0 && d < 32)  { day   = d; return true; } return false; }


    // Accessors (used to retrieve component values)


    #pragma region Documentation
    /// <summary>Retrieve the year. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The year. </return>
    #pragma endregion
    int getYear()           { return year; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the month. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The month. </return>
    #pragma endregion
    unsigned int getMonth() { return month; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the dat. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The day. </return>
    #pragma endregion
    unsigned int getDay()   { return day; }


    // XML file Interaction 


    #pragma region Documentation
    /// <summary>Read a Date section from the current XML file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            bool success = clib->getNodeValueXML("Year", year);
            if (success)
            {
            clib->getNodeValueXML("Month", month);
            clib->getNodeValueXML("Day", day);
        }
    }
    }


    #pragma region Documentation
    /// <summary>Write a Date section to the current XML file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            std::ostringstream oss;
            oss << std::setfill('0') << std::setw(4) << year;
            std::string year_string(oss.str());

            oss.str(""); oss.clear();
            oss << std::setfill('0') << std::setw(2) << month;
            std::string month_string(oss.str());

            oss.str(""); oss.clear();
            oss << std::setfill('0') << std::setw(2) << day;
            std::string day_string(oss.str());

            clib->writeNodeValueXML("Year", year_string);
            clib->writeNodeValueXML("Month", month_string);           
            clib->writeNodeValueXML("Day", day_string);
        }
    }


    #pragma region Documentation
    /// <summary>Set (components) of this date to the current date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void setNow()
    {
        time_t tim = time(NULL);
        struct tm *tms = localtime(&tim);
        year  = 1900 + tms->tm_year;
        month = tms->tm_mon + 1;
        day   = tms->tm_mday;
    }

    
    #pragma region Documentation
    /// <summary>Convert this date to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>A string representing this date. </return>
    #pragma endregion
    std::string toString()
    {
        std::ostringstream oss;
        oss << std::setfill('0') << std::setw(4) << year << "-" 
            << std::setfill('0') << std::setw(2) << month << "-" 
            << std::setfill('0') << std::setw(2) << day;
        return oss.str();
    }

    
    #pragma region Documentation
    /// <summary>Convert a string to (this) date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="str">A string representing a date (format: yyyy-mm-dd).</parm>
    #pragma endregion
    void fromString(std::string str)
    {
        char delin;
        std::istringstream iss(str);
        iss >> year >> delin >> month >> delin >> day;
        if (year < 100) year += 1900;
    }

    
private:

    int          curr_year; /// the current year (includes the year 1900 baseline)
    unsigned int year;      /// the year (may or may not include the year 1900 baseline)
    unsigned int month;     /// the month (ranges from 1 to 12, inclusively)
    unsigned int day;       /// the day (ranges from 1 to 31, inclusively)
};


} // namespace vct