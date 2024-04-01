// VCT_Deformation.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_Organization.h"
#include "VCT_Software.h"
#include "VCT_Station.h"
#include "VCT_Date.h"
#include "VCT_Time.h"

#include <algorithm>


#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{
    

#pragma region Documentation
/// <summary>Deformation type. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum DEFORMATION_TYPE
{
    UNK_DEFORMATION_TYPE = 0,
    NO_DEFORMATION       = 1,
    FINITE_ELEMENT       = 2
};


#pragma region Documentation
/// <summaryDeformation mode. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum DEFORMATION_MODE
{
    UNK_DEFORMATION_MODE = 0,
    DEFORM_CC            = 1,
    DEFORM_ML            = 2,
    DEFORM_MLO           = 3
};


#pragma region Documentation
/// <summary> Deformation class for Open Virtual Clinical Trials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class Deformation
{
public:

    #pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="seq">The deformation's sequence number. </parm>
    /// <parm name="dt">The deformation's type. </parm>
    /// <parm name="thick">The deformation's thickness. </parm>
    /// <parm name="date">The deformation's date. </parm>
    /// <parm name="time">The deformation's time. </parm>
    /// <parm name="org">The deformation's organization. </parm>
    /// <parm name="sw">The deformation's software. </parm>
    /// <parm name="sta">The deformation's station. </parm>
    #pragma endregion
    Deformation(unsigned int seq, vct::DEFORMATION_TYPE dt, float thick, Date date, Time time, Organization *org, Software *sw, Station *sta)
        : seq_number(seq),
          def_type(dt),
          def_thickness(thick),
          def_date(date),
          def_time(time),
          def_org(org),
          def_software(sw),
          def_station(sta)
    {}

    
    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Deformation()
        : seq_number(0),
          def_type(UNK_DEFORMATION_TYPE),
          def_thickness(0.0f),
          def_date(),
          def_time(),
          def_org(nullptr),
          def_software(nullptr),
          def_station(nullptr)
    {}

    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Deformation(const Deformation &rhs)
        : seq_number(rhs.seq_number),
          def_type(rhs.def_type),
          def_thickness(rhs.def_thickness),
          def_date(rhs.def_date),
          def_time(rhs.def_time),
          def_org(nullptr),
          def_software(nullptr),
          def_station(nullptr)
    {
        if (rhs.def_org != nullptr)      def_org = new Organization(*rhs.def_org);
        if (rhs.def_software != nullptr) def_software = new Software(*rhs.def_software);
        if (rhs.def_station != nullptr)  def_station = new Station(*rhs.def_station);
    }
    
    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Deformation()
    {}
	
	
	// Manipulators (used to set component values)

    #pragma region Documentation
    /// <summary>Set the deformation's sequence number. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="seqnum">The deformation's sequence number. </parm>
    #pragma endregion
    void setSequenceNumber(unsigned int seqnum) { seq_number = seqnum; }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's type (using enumeration). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="type">The deformation's type. </parm>
    #pragma endregion
    void setDefType(DEFORMATION_TYPE type)      { def_type = type; }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's type (using a string). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm type="type">The deformation's type. </parm>
    #pragma endregion
    void setDefType(std::string type)           { def_type = interpretType(type); }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's mode (using enumeration). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="mode">The deformation's mode. </parm>
    #pragma endregion
    void setDefMode(DEFORMATION_MODE mode)      { def_mode = mode; }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's mode (using a string). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="mode">The deformation's mode. </parm>
    #pragma endregion
    void setDefMode(std::string mode)           { def_mode =  interpretMode(mode); }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's thickness. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="thickness">The deformation'sthicknessname. </parm>
    #pragma endregion
    void setDefThickness(float thickness)       { def_thickness = thickness; }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="date">The deformation's date. </parm>
    #pragma endregion
    void setDefDate(Date date)                  { def_date = date; }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="time">The deformation's time. </parm>
    #pragma endregion
    void setDefTime(Time time)                  { def_time = time; }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's organization. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="org">The deformation's organization. </parm>
    #pragma endregion
    void setOrganization(Organization *org)     { def_org = org; }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's software. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="sw">The deformation's software. </parm>
    #pragma endregion
    void setSoftware(Software *sw)              { def_software = sw; }
    
    
    #pragma region Documentation
    /// <summary>Set the deformation's station. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="station">The deformation's station. </parm>
    #pragma endregion
    void setStation(Station *station)           { def_station = station; }

    
    // Accessors (used to retrieve component values)

    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's sequence number. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's sequence number. </return>
    #pragma endregion
    unsigned int     getSequenceNumber()        { return seq_number; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's type enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's type enumeration. </return>
    #pragma endregion
    DEFORMATION_TYPE getDefType()               { return def_type; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's type as a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's type as a string. </return>
    #pragma endregion
    std::string      getDefTypeString()         { return interpretType(def_type); }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's mode as an enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's mode as an enumeration. </return>
    #pragma endregion
    DEFORMATION_MODE getDefMode()               { return def_mode; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's mode as a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's momde as a string. </return>
    #pragma endregion
    std::string      getDefModeString()         { return interpretMode(def_mode); }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's thickness. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's thickness. </return>
    #pragma endregion
    float            getDefThickness()          { return def_thickness; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's date. </return>
    #pragma endregion
    Date             getDefDate()               { return def_date; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's time. </return>
    #pragma endregion
    Time             getTime()                  { return def_time; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's organization. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's organization. </return>
    #pragma endregion
    Organization    *getOrganization()          { return def_org; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's software. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's software. </return>
    #pragma endregion
    Software        *getSoftware()              { return def_software; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the deformation's station. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The deformation's station. </return>
    #pragma endregion
    Station         *getStation()               { return def_station; }


    // XML file Interaction 


    #pragma region Documentation
    /// <summary>Write this deformation's section to the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {        
        clib->writeNodeValueXML("Sequence_Number", seq_number);
        clib->writeNodeValueXML("Deformation_Type", interpretType(def_type));
        clib->writeNodeValueXML("Deformation_Mode", interpretMode(def_mode));

        clib->writeNodeValueXML("Deformation_Thickness", def_thickness);
        
        clib->addASectionXML("Date_Deformed");
        def_date.writeXML(clib);
        clib->concludeSectionXML();
        
        clib->addASectionXML("Time_Deformed");
        def_time.writeXML(clib);
        clib->concludeSectionXML();

        if (def_org != nullptr)
        {
            clib->addASectionXML("Deforming_Organization");

            def_org->writeXML(clib);
            clib->concludeSectionXML();
        }

        if (def_software != nullptr)
        {
            clib->addASectionXML("Deforming_Software");
            def_software->writeXML(clib);
            clib->concludeSectionXML();
        }

        if (def_station != nullptr)
        {
            clib->addASectionXML("Deforming_Station");
            def_station->writeXML(clib);
            clib->concludeSectionXML();
        }
    }

    
    #pragma region Documentation
    /// <summary>Read the deformation section from the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        clib->getNodeValueXML("Sequence_Number", seq_number);
        std::string type, mode;
        clib->getNodeValueXML("Deformation_Type", type);
        clib->getNodeValueXML("Deformation_Mode", mode);
        def_type = interpretType(type);
        def_mode = interpretMode(mode);

        clib->getNodeValueXML("Deformation_Thickness", def_thickness);
        
        if (clib->findNodeXML("Date_Deformed"))
        {
            def_date.readXML(clib);
            clib->concludeSectionXML();
        }
        
        if (clib->findNodeXML("Time_Deformed"))
        {
            def_time.readXML(clib);
            clib->concludeSectionXML();
        }

        if (clib->findNodeXML("Deforming_Organization"))
        {
            if (def_org == nullptr) def_org = new Organization;
            if (def_org != nullptr)
            {
                def_org->readXML(clib);
                clib->concludeSectionXML();
            }
        }

        if (clib->findNodeXML("Deforming_Software"))
        {
            if (def_software == nullptr) def_software = new Software;
            if (def_software != nullptr)
            {
                def_software->readXML(clib);
                clib->concludeSectionXML();
            }
        }

        if (clib->findNodeXML("Deforming_Station"))
        {
            if (def_station == nullptr) def_station = new Station;
            if (def_station != nullptr)
            {
                def_station->readXML(clib);
                clib->concludeSectionXML();
            }
        }
    }
    

    #pragma region Documentation
    /// <summary>Convert a string to the corresponding deformation type enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="type_string">A string representing a corresponding deformation type enumeration instance. </parm>
    /// <return>The corresponding deformation type enumeration instance (or unknown type if not found). </return>
    #pragma endregion  
    DEFORMATION_TYPE interpretType(std::string type_string)
    {
        std::transform(type_string.begin(), type_string.end(), type_string.begin(), toupper); // convert to upper case

        DEFORMATION_TYPE retval(UNK_DEFORMATION_TYPE);
        if      (type_string == "NO_DEFORMATION" || type_string == "NO DEFORMATION") retval = NO_DEFORMATION;
        else if (type_string == "FINITE_ELEMENT" || type_string == "FINITE ELEMENT") retval = FINITE_ELEMENT;
        return retval;
    }

    
    #pragma region Documentation
    /// <summary>Convert a deformation type enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="subj_type">Deformation type enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretType(DEFORMATION_TYPE subj_type)
    {
        std::string retval("UNK_DEFORMATION_TYPE");
		if      (subj_type == NO_DEFORMATION) retval = "NO_DEFORMATION";
		else if (subj_type == FINITE_ELEMENT) retval = "FINITE_ELEMENT";
        return retval;
    }
    

    #pragma region Documentation
    /// <summary>Convert a string to the corresponding deformation mode enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="mode_string">A string representing a corresponding deformation mode enumeration instance. </parm>
    /// <return>The corresponding deformation mode enumeration instance (or unknown mode if not found). </return>
    #pragma endregion  
    DEFORMATION_MODE interpretMode(std::string mode_string)
    {
        std::transform(mode_string.begin(), mode_string.end(), mode_string.begin(), toupper); // convert to upper case

        DEFORMATION_MODE retval(UNK_DEFORMATION_MODE);
        if      (mode_string == "DEFORM_CC"  || mode_string == "DEFORM CC")  retval = DEFORM_CC;
        else if (mode_string == "DEFORM_ML"  || mode_string == "DEFORM ML")  retval = DEFORM_ML;
        else if (mode_string == "DEFORM_MLO" || mode_string == "DEFORM MLO") retval = DEFORM_MLO;
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a deformation mode enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="subj_mode">Deformation mode enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretMode(DEFORMATION_MODE subj_mode)
    {
        std::string retval("UNK_DEFORMATION_MODE");
		if      (subj_mode == DEFORM_CC)  retval = "DEFORM_CC";
		else if (subj_mode == DEFORM_ML)  retval = "DEFORM_ML";
		else if (subj_mode == DEFORM_MLO) retval = "DEFORM_MLO";
        return retval;
    }

private:

    unsigned int     seq_number;
    DEFORMATION_TYPE def_type;
    DEFORMATION_MODE def_mode;
    float            def_thickness;
    Date             def_date;
    Time             def_time;
    Organization     *def_org;
    Software         *def_software;
    Station          *def_station;
};


}
