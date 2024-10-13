// VCT_Lesion.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_Date.h"
#include "VCT_Time.h"
#include "VCT_Organization.h"
#include "VCT_Software.h"
#include "VCT_Station.h"
#include "VCT_LesionMask.h"
#include <algorithm>
#include <map>


#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{


enum LESION_TYPE
{
    UNK_LESIONTYPE  = 0,
    BENIGN          = 1,
    MALIGNANT       = 2,
    DCIS            = 4,
    INVASIVE_CANCER = 8,
    MASS            = 16,
    CALC            = 32,
};


#pragma region Documentation
/// <summary> Lesion class for Open Virtual Clinical Trials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class Lesion
{
public:

    #pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="phid">The lesion id for this particular phantom. </parm>
    /// <parm name="serial">The abolutely unique lesion id (serial number). </parm>
    /// <parm name="ltyp">The lesion type (see enumeration). </parm>
    /// <parm name="sph">Whether the lesion is a sphere (currently treated as always true). </parm>
    /// <parm name="x">The insert pt (centroid) x displacment from phantom origin (in mm). </parm>
    /// <parm name="y">The insert pt (centroid) y displacment from phantom origin (in mm). </parm>
    /// <parm name="z">The insert pt (centroid) z displacment from phantom origin (in mm). </parm>
    #pragma endregion
    Lesion(int phid, std::string serial, LESION_TYPE ltyp, bool sph, float x, float y, float z)
        : lesion_id(phid),              // unique id within the phantom
          lesion_unique_id(serial),     // overall unique id (serial number)
          voi_unique_id("unknown_uid"), // Corresponding Volume of Interest Unique ID
          type(ltyp),                   // lesion type          is_a_sphere(sph),         // true if the lesion is a sphere
          sph_radius(1.0f),             // inserted sphere's radius (in mm)
          material(5),                  // inserted sphere's material (from phantom's index table)
          insert_x(x),                  // insert pt (centroid) x displacment from phantom origin (in mm)
          insert_y(y),                  // insert pt (centroid) y displacment from phantom origin (in mm)
          insert_z(z)                   // insert pt (centroid) z displacment from phantom origin (in mm)
    {
        ins_date.setNow();
        ins_time.setNow();
    }


    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Lesion()
        : lesion_id(-1),                   // unique id within the phantom
          lesion_unique_id("unknown_id"),  // overall unique id (serial number)
          voi_unique_id("unknown_uid"),    // Corresponding Volume of Interest Unique ID
          type(CALC),                      // lesion type
          is_a_sphere(true),               // true if the lesion is a sphere
          sph_radius(1.0f),                // inserted sphere's radius (in mm)
          material(5),                     // inserted sphere's material (from phantom's index table)
          insert_x(0.0f),                  // insert pt (centroid) x displacment from phantom origin (in mm)
          insert_y(0.0f),                  // insert pt (centroid) y displacment from phantom origin (in mm)
          insert_z(0.0f)                   // insert pt (centroid) z displacment from phantom origin (in mm)
    {
        ins_date.setNow();
        ins_time.setNow();
    }


    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Lesion(const Lesion &rhs)
        : lesion_id(rhs.lesion_id),
          lesion_unique_id(rhs.lesion_unique_id),
          voi_unique_id(rhs.voi_unique_id),
          type(rhs.type),
          is_a_sphere(rhs.is_a_sphere),
          sph_radius(rhs.sph_radius),
          truth(rhs.truth),
          material(rhs.material),
          insert_x(rhs.insert_x),
          insert_y(rhs.insert_y),
          insert_z(rhs.insert_z),
          bb_width(rhs.bb_width),
          bb_height(rhs.bb_height),
          bb_depth(rhs.bb_depth),
          ins_date(rhs.ins_date),
          ins_time(rhs.ins_time),
          ins_org(rhs.ins_org),
          ins_software(rhs.ins_software),
          ins_station(rhs.ins_station)
    {}


    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Lesion()
    {}


    // Manipulators (used to set component values)


    #pragma region Documentation
    /// <summary>Set the insertion's Lesion ID. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">The lesion ID unique to (within) this phantom. </parm>
    #pragma endregion
    void setLesionId(int id)                { lesion_id = id; }


    #pragma region Documentation
    /// <summary>Set the insertion's unique id (serial number). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">The lesion ID serial number (external to this phantom). </parm>
    #pragma endregion
    void setLesionUniqueId(std::string id)  { lesion_unique_id = id; }


    #pragma region Documentation
    /// <summary>Set the insertion's lesion type. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="typ">The lesion type. </parm>
    #pragma endregion
    void setLesionType(LESION_TYPE typ)     { type = typ; }


    #pragma region Documentation
    /// <summary>Set the insertion's affirmation that it is a sphere. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="it_is">The sphere affirmation: true -> it's a sphere (otherwise false). </parm>
    #pragma endregion
    void setIsASphere(bool it_is)           { is_a_sphere = it_is; }


    #pragma region Documentation
    /// <summary>Set the insertion's sphere radius (mm). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="rad">The sphere radius in mm. </parm>
    #pragma endregion
    void setSphereRadius(float rad)         { sph_radius = rad; }


    #pragma region Documentation
    /// <summary>Set the insertion's material index. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="mat">The sphere's material index (must be specified in phantom's index table). </parm>
    #pragma endregion
    void setInsertMaterial(int mat)         { material = mat; }


    #pragma region Documentation
    /// <summary>Set phantom coordinate where the center of the lesion is be. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="x">The phantom's x coordinate (mm). </parm>
    /// <parm name="y">The phantom's y coordinate (mm). </parm>
    /// <parm name="z">The phantom's z coordinate (mm). </parm>
    #pragma endregion
    void setInsertPoint(float x, float y, float z) { insert_x = x; insert_y = y; insert_z = z; }


    #pragma region Documentation
    /// <summary>Set the insertion's date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="date">The date of the insertion. </parm>
    #pragma endregion
    void setDate(Date date)                 { ins_date = date; }


    #pragma region Documentation
    /// <summary>Set the insertion's time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="time">The time of the insertion. </parm>
    #pragma endregion
    void setTime(Time time)                 { ins_time = time; }


    #pragma region Documentation
    /// <summary>Set the insertion's organization. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="org">The organization of this insertion. </parm>
    #pragma endregion
    void setOrganization(Organization org) { ins_org = org; }


    #pragma region Documentation
    /// <summary>Set the insertion's software. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="sw">The insertion software. </parm>
    #pragma endregion
    void setSoftware(Software sw)          { ins_software = sw; }


    #pragma region Documentation
    /// <summary>Set the insertion's station. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="st">The insertion (work)station. </parm>
    #pragma endregion
    void setStation(Station st)            { ins_station = st; }


    // Accessors (used to retrieve component values)


    #pragma region Documentation
    /// <summary>Retrieve the insertion's input phantom filename. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The lesion ID unique to (within) this phantom. </return>
    #pragma endregion
    int              getLesionId()      { return lesion_id; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's unique id (serial number). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The lesion ID serial number (external to this phantom). </return>
    #pragma endregion
    std::string     getLesionUniqueId() { return lesion_unique_id; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's lesion type. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The lesion type. </return>
    #pragma endregion
    LESION_TYPE     getLesionType()     { return type; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's affirmation that it is a sphere. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The sphere affirmation: true -> it's a sphere (otherwise false). </return>
    #pragma endregion
    bool            getIsASphere()      { return is_a_sphere; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's sphere radius (mm). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The sphere radius in mm. </return>
    #pragma endregion
    float           getSphereRadius()   { return sph_radius; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's material index. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The sphere's material index (must be specified in phantom's index table). </return>
    #pragma endregion
    int             getInsertMaterial() { return material; }


    #pragma region Documentation
    /// <summary>Retrieve phantom coordinate where the center of the lesion is be. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="x">The phantom's x coordinate (mm). </parm>
    /// <parm name="y">The phantom's y coordinate (mm). </parm>
    /// <parm name="z">The phantom's z coordinate (mm). </parm>
    #pragma endregion
    void getInsertPoint(float &x, float &y, float &z) { x = insert_x; y = insert_y; z = insert_z; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The date of the insertion. </return>
    #pragma endregion
    Date            getDate()           { return ins_date; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The time of the insertion. </return>
    #pragma endregion
    Time            getTime()           { return ins_time; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's organization. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The organization of this insertion. </return>
    #pragma endregion
    Organization   getOrganization()    { return ins_org; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's software. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The insertion software. </return>
    #pragma endregion
    Software       getSoftware()        { return ins_software; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's station. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The insertion (work)station. </return>
    #pragma endregion
    Station        getStation()         { return ins_station; }


       // XML Interaction routines


    #pragma region Documentation
    /// <summary>Write this insertion's section to the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        clib->writeNodeValueXML("Lesion_ID", lesion_id);
        clib->writeNodeValueXML("Lesion_UID", lesion_unique_id);
        clib->writeNodeValueXML("VOI_UID", voi_unique_id);

        clib->writeNodeValueXML("Type", interpretType(type));
        clib->writeNodeValueXML("Is_A_Sphere", (is_a_sphere ? "true":"false"));
        clib->writeNodeValueXML("Sphere_Radius", sph_radius);
        clib->writeNodeValueXML("Material", material);

        if (clib->addASectionXML("Insert_Point"))
        {
            clib->writeNodeValueXML("X", insert_x);
            clib->writeNodeValueXML("Y", insert_y);
            clib->writeNodeValueXML("Z", insert_z);
            clib->concludeSectionXML();
        }

        if (clib->addASectionXML("Bounding_Box"))
        {
            clib->writeNodeValueXML("Width", bb_width);
            clib->writeNodeValueXML("Height", bb_height);
            clib->writeNodeValueXML("Depth", bb_depth);
            clib->concludeSectionXML();
        }

        if (clib->addASectionXML("Date_Inserted"))
        {
            ins_date.writeXML(clib);
            clib->concludeSectionXML();
        }
        if (clib->addASectionXML("Time_Inserted"))
        {
            ins_time.writeXML(clib);
            clib->concludeSectionXML();
        }

        if (clib->addASectionXML("Organization"))
        {
            ins_org.writeXML(clib);
            clib->concludeSectionXML();
        }

        if (clib->addASectionXML("Software"))
        {
            ins_software.writeXML(clib);
            clib->concludeSectionXML();
        }

        if (clib->addASectionXML("Station"))
        {
            ins_station.writeXML(clib);
            clib->concludeSectionXML();
        }

        clib->concludeSectionXML();
    }


    #pragma region Documentation
    /// <summary>Read the insertion section from the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        clib->getNodeValueXML("Lesion_ID", lesion_id);
        clib->getNodeValueXML("Lesion_UID", lesion_unique_id);
        clib->getNodeValueXML("VOI_UID", voi_unique_id);         // Debate-able legitimacy here

        std::string sphere_string;
        if (clib->getNodeValueXML("Is_A_Sphere", sphere_string))
        {
            std::cout << "Read \"" << sphere_string << "\" for Is_A_Sphere" << std::endl;
            is_a_sphere = false;
            if (sphere_string == "true" || sphere_string == "True")
            {
                is_a_sphere = true;
            }
        } else std::cerr << "Couldn't read \"is_a_sphere\" section" << std::endl;

        std::string radius_string;
        if (clib->getNodeValueXML("Sphere_Radius", radius_string))
        {
            sph_radius = std::stof(radius_string);
            std::cout << "Read radius=" << radius_string << std::endl;
        }
        else std::cerr << "Couldn't read sphere radius" << std::endl;

        std::string type_string;
        if (clib->getNodeValueXML("Type", type_string))
        {
            std::cout << "Read \"" << type_string << "\" for Type" << std::endl;
            type = interpretType(type_string);
        } else std::cerr << "Couldn't read \"lesion Type\"" << std::endl;

        clib->getNodeValueXML("Material", material);
        clib->getNodeValueXML("Truth", truth);

        if (clib->findNodeXML("Insert_Point"))
        {
            if (clib->getNodeValueXML("X", insert_x)) std::cout << "just read X = " << insert_x << std::endl;
            else std::cout << "Couldn't read X" << std::endl;
            clib->getNodeValueXML("Y", insert_y);
            clib->getNodeValueXML("Z", insert_z);
            clib->concludeSectionXML();
        } else std::cerr << "Couldn't find Insert_Point section" << std::endl;

        if (clib->findNodeXML("Bounding_Box"))
        {
            clib->getNodeValueXML("Width",  bb_width);
            clib->getNodeValueXML("Height", bb_height);
            clib->getNodeValueXML("Depth",  bb_depth);
            clib->concludeSectionXML();
        } else std::cerr << "Couldn't find Bounding_Box section" << std::endl;

        if (clib->findNodeXML("Date_Inserted"))
        {
            ins_date.readXML(clib);
            clib->concludeSectionXML();
        }
        if (clib->findNodeXML("Time_Inserted"))
        {
            ins_time.readXML(clib);
            clib->concludeSectionXML();
        }

        std::cout << "\t Lesion_Id " << lesion_id << ":\n"
                  << "\t\t Unique_Id:     " << lesion_unique_id << "\n"
                  << "\t\t VOI Unique_Id: " << voi_unique_id << "\n"
                  << "\t\t Lesion_Type:   " << type_string << "\n"
                  << "\t\t Is_A_Sphere:   " << (is_a_sphere?"true":"false") << "\n"
                  << "\t\t Radius:        " << sph_radius << "\n"
                  << "\t\t Truth:         " << truth << "\n"
                  << "\t\t Material:      " << material << "\n"
                  << "\t\t Insert_X:      " << insert_x << "\n"
                  << "\t\t Insert_Y:      " << insert_y << "\n"
                  << "\t\t Insert_Z:      " << insert_z << "\n"
                  << "\t\t BB width:      " << bb_width << "\n"
                  << "\t\t BB height:     " << bb_height << "\n"
                  << "\t\t BB depth:      " << bb_depth << "\n";
    }


    // String/Enum Translation routines

    #pragma region Documentation
    /// <summary>Convert a string to the corresponding phantom source enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="typ">A string representing a corresponding lesion type enumeration instance. </parm>
    /// <return>The corresponding lesion type enumeration instance (or unknown type if not found). </return>
    #pragma endregion
    LESION_TYPE interpretType(std::string typ)
    {
        std::transform(typ.begin(), typ.end(), typ.begin(), toupper); // convert to upper case

        LESION_TYPE retval(UNK_LESIONTYPE);
        if      (typ.find("BENIGN") != std::string::npos)    retval = BENIGN;
        else if (typ.find("MALIGNANT") != std::string::npos) retval = MALIGNANT;
        else if (typ.find("DCIS") != std::string::npos)      retval = DCIS;
        else if (typ == "INVASIVE_CANCER")                   retval = INVASIVE_CANCER;
        else if (typ == "MASS")                              retval = MASS;
        else if (typ == "CALC")                              retval = CALC;
        return retval;
    }


    #pragma region Documentation
    /// <summary>Convert a lesion type enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="lt">Lesion type enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion
    std::string interpretType(LESION_TYPE lt)
    {
        std::string retval("UNK_LESIONTYPE");

        if      (lt == BENIGN)          retval = "BENIGN";
        else if (lt == MALIGNANT)       retval = "MALIGNANT";
        else if (lt == DCIS)            retval = "DCIS";
        else if (lt == INVASIVE_CANCER) retval = "INVASIVE_CANCER";
        else if (lt == MASS)            retval = "MASS";
        else if (lt == CALC)            retval = "CALC";
        return retval;
    }

private:

    int lesion_id;                 // unique id within the phantom
    std::string lesion_unique_id;  // overall unique id (serial number)
    std::string voi_unique_id;     // corresponding Volume Of Interest unique ID
    LESION_TYPE type;              // lesion type
    bool  is_a_sphere;             // true if the lesion is a sphere
    float sph_radius;              // inserted sphere's radius (in mm)
    std::string truth;             // BIRADS designation
    int   material;                // inserted sphere's material (from phantom's index table)

    float insert_x;                // insert pt (centroid) x displacment from phantom origin (in mm)
    float insert_y;                // insert pt (centroid) y displacment from phantom origin (in mm)
    float insert_z;                // insert pt (centroid) z displacment from phantom origin (in mm)

    float bb_width;                // bounding box width (x) in mm
    float bb_height;               // bounding box height (y) in mm
    float bb_depth;                // bounding box depth (z) in mm

    Date         ins_date;         // date insertion took place
    Time         ins_time;         // time insertion took place
    Organization ins_org;          // organization performing the insertion
    Software     ins_software;     // insertion software used
    Station      ins_station;      // insertions (work)station
};

}
