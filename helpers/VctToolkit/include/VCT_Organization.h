// VCT_Organization.h

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
/// <summary> Organization class for Open Virtual Clinical Trials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class Organization
{
public:

    #pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="name">The organization's name. </parm>
    /// <parm name="div">The organization's division. </parm>
    /// <parm name="dept">The organization's department. </parm>
    /// <parm name="grp">The organization's group name. </parm>
    /// <parm name="uid">The organization's institution code. </parm>
    /// <parm name="country">The organization's country. </parm>
    /// <parm name="state">The organization's state. </parm>
    /// <parm name="city">The organization's city. </parm>
    /// <parm name="zip">The organization's zip code. </parm>
    #pragma endregion
    Organization(std::string name, std::string div, std::string dept, std::string grp,
                 std::string uid, std::string country, std::string state, std::string city, 
                 std::string zip)
        : org_name(name),
          division(div),
          department(dept),
          group(grp),
          instCodeSeq(uid),
          org_country(country),
          org_state(state),
          org_city(city),
          org_zip(zip)
    {}

    
    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Organization()
        : org_name(),
          division(),
          department(),
          group(),
          instCodeSeq(),
          org_country(),
          org_state(),
          org_city(),
          org_zip()
    {}
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Organization(const Organization &rhs)
        : org_name(rhs.org_name),
          division(rhs.division),
          department(rhs.department),
          group(rhs.group),
          instCodeSeq(rhs.instCodeSeq),          
          org_country(rhs.org_country),
          org_state(rhs.org_state),
          org_city(rhs.org_city),
          org_zip(rhs.org_zip)
    {}

    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Organization()
    {}
    
    
    // Manipulators (used to set component values)

    #pragma region Documentation
    /// <summary>Set the organization's name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="org">The organization's name. </parm>
    #pragma endregion
    void setName(std::string org)             { org_name = org; }
    
    
    #pragma region Documentation
    /// <summary>Set the organization's division. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="div">The organization's division. </parm>
    #pragma endregion
    void setDivision(std::string div)         { division = div; }
    
    
    #pragma region Documentation
    /// <summary>Set the organization's department name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="dept">The organization's department name. </parm>
    #pragma endregion
    void setDepartment(std::string dept)      { department = dept; }
    
    
    #pragma region Documentation
    /// <summary>Set the organization's group. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="grp">The organization's group name. </parm>
    #pragma endregion
    void setGroup(std::string grp)            { group = grp; }
    
    
    #pragma region Documentation
    /// <summary>Set the organization's institution code. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="seq">The organization's institution code. </parm>
    #pragma endregion
    void setInstituionCode(std::string seq)   { instCodeSeq = seq; }

    
    #pragma region Documentation
    /// <summary>Set the organization's country. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="country">The organization's country. </parm>
    #pragma endregion
    void setCountry(std::string country)      { org_country = country; }
    
    
    #pragma region Documentation
    /// <summary>Set the organization's state. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="state">The organization's state. </parm>
    #pragma endregion
    void setState(std::string state)          { org_state = state; }
    
    
    #pragma region Documentation
    /// <summary>Set the organization's city. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="city">The organization's city. </parm>
    #pragma endregion
    void setCity(std::string city)            { org_city = city; }
    
    
    #pragma region Documentation
    /// <summary>Set the organization's zip code. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="zip">The organization's zip code. </parm>
    #pragma endregion
    void setZip(std::string zip)              { org_zip = zip; }

    
    // Accessors (used to retrieve component values)

    
    #pragma region Documentation
    /// <summary>Retrieve the organization's name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization's name. </return>
    #pragma endregion
    std::string getName()           { return org_name; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the organization's division. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization's division. </return>
    #pragma endregion
    std::string getDivision()       { return division; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the organization's department. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization's department. </return>
    #pragma endregion
    std::string getDepartment()     { return department; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the organization's group. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization's group. </return>
    #pragma endregion
    std::string getGroup()          { return group; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the organization's institution code. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization's institution code. </return>
    #pragma endregion
    std::string getInstituionCode() { return instCodeSeq; }

    
    #pragma region Documentation
    /// <summary>Retrieve the organization's country. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization's country. </return>
    #pragma endregion
    std::string getCountry()        { return org_country; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the organization's state. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization's state. </return>
    #pragma endregion
    std::string getState()          { return org_state; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the organization's city. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization's city. </return>
    #pragma endregion
    std::string getCity()           { return org_city; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the organization's zip code. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization's zip code. </return>
    #pragma endregion
    std::string getZip()            { return org_zip; }


    // XML file Interaction 


    #pragma region Documentation
    /// <summary>Read the XML section pertaining to organization. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->getNodeValueXML("Name", org_name);
            clib->getNodeValueXML("Division", division);
            clib->getNodeValueXML("Department", department);
            clib->getNodeValueXML("Group", group);
            clib->getNodeValueXML("Institution_Code_Sequence", instCodeSeq);

            clib->getNodeValueXML("Country", org_country);
            clib->getNodeValueXML("State", org_state);
            clib->getNodeValueXML("City", org_city);
            clib->getNodeValueXML("Zip", org_zip);
        }
    } 
    
    
    #pragma region Documentation
    /// <summary>Write this organization's XML section. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->writeNodeValueXML("Name", org_name);
            clib->writeNodeValueXML("Division", division);
            clib->writeNodeValueXML("Department", department);
            clib->writeNodeValueXML("Group", group);
            clib->writeNodeValueXML("Institution_Code_Sequence", instCodeSeq);
            clib->writeNodeValueXML("Country", org_country);
            clib->writeNodeValueXML("State", org_state);
            clib->writeNodeValueXML("City", org_city);
            clib->writeNodeValueXML("Zip", org_zip);
        }
    }

    
private:

    std::string  org_name;    /// Organization name
    std::string  division;    /// Organization division name
    std::string  department;  /// Organization Department name
    std::string  group;       /// Organization group name

    std::string  instCodeSeq; /// Organization's ID (code sequence)

    std::string  org_country; /// Organization's country
    std::string  org_state;   /// Organization's state
    std::string  org_city;    /// Organization's city
    std::string  org_zip;     /// Organization's zip code
};


}
