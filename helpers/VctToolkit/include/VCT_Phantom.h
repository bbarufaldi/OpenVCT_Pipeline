// VCT_Phantom.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_Subject.h"
#include "VCT_VoxelArray.h"
#include "VCT_IndexTable.h"
#include "VCT_Organization.h"
#include "VCT_Software.h"
#include "VCT_Station.h"
#include "VCT_Deformation.h"
#include "VCT_Insertion.h"
#include "VCT_Date.h"
#include "VCT_Time.h"

#include <string>
#include <algorithm>

#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

#pragma region Documentation
/// <summary>Who generated the phantom. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum PHANTOM_SOURCE
{
    UNK_SOURCE   = 0,
    PENN         = 1,
    DUKE         = 2,
    FDA          = 3,
    OTHER_SOURCE = 4
};

#pragma region Documentation
/// <summary>The shape of the phantom. </summary>
/// <remarks>D. Higginbotham, 2020-11-25. </remarks>
#pragma endregion
enum PHANTOM_SHAPE
{
    UNK_SHAPE    = 0,
    BREAST_SHAPE = 1,
    CUBE_SHAPE   = 2
};

#pragma region Documentation
/// <summary>The type of the phantom data. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum PHANTOM_TYPE
{
    UNK_TYPE    = 0,
    VOXEL_ARRAY = 1,
    OCTREE      = 2,
    NURBS       = 3
};

#pragma region Documentation
/// <summary>The data model used for the phantom. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum PHANTOM_DATA_MODEL
{
    UNK_DATAMODEL = 0,
    INDEXED       = 1,
    PHYSICAL      = 2
};

#pragma region Documentation
/// <summary>The Body part represented by the phantom. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum PHANTOM_BODY_PART
{
    UNK_BODYPART     = 0,
    ABDOMEN          = 10,
    ANKLE            = 20,
    ARMPIT           = 30,
    BACK             = 40,
    BACK_OF_ELBOW    = 50,
    BACK_OF_KNEE     = 60,
    BREAST           = 70,
    BUTTOCK          = 80,
    CALF             = 90,
    CHEEK            = 100,
    CHEST            = 110,
    CHIN             = 120,
    EAR              = 130,
    EYE              = 140,
    FACE             = 150,
    FINGER           = 160,
    FOOT             = 170,
    FOREARM          = 180,
    FOREHEAD         = 190,
    FRONT_OF_ELBOW   = 200,
    GROIN            = 210,
    HAND             = 220,
    HEAD             = 230,
    HEAL_OF_FOOT     = 240,
    KNEE             = 250,
    KNEECAP          = 260,

    LEG              = 270,
    LOIN             = 280,
    MOUTH            = 290,
    NAVAL            = 300,
    NECK             = 310,
    NOSE             = 320,
    PALM             = 330,
    PELVIS           = 340,
    RIBS             = 350,

    SCAPULA          = 360,
    STERNUM          = 370,
    SHOULDER         = 380,
    SKULL            = 390,
    SOLE_OF_FOOT     = 400,
    THIGH            = 410,
    THUMB            = 420,
    TOE              = 430,
    VERTEBRAE        = 440,
    WRIST            = 450,
    PHYSICAL_PHANTOM = 500
};

#pragma region Documentation
/// <summary>The laterality represented by the phantom. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum PHANTOM_LATERALITY
{
    UNK_LATERALITY = 0,
    LEFT           = 1,
    RIGHT          = 2,
    LEFT_AND_RIGHT = 3,
    WHOLE_BODY     = 4
};


#pragma region Documentation
/// <summary>This class is responsible for retaining phantom-related states and functions. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
class Phantom
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Phantom()
        : phantom_name("unnamed"),
          phantom_uid("unknown uid"),
          phantom_source(vct::UNK_SOURCE),
          phantom_type(vct::UNK_TYPE),
          data_model(vct::UNK_DATAMODEL),
          body_part(vct::UNK_BODYPART),
          laterality(vct::UNK_LATERALITY),
          glandular_count(0),
          x_thickness_mm(0.0),
          y_thickness_mm(0.0),
          z_thickness_mm(0.0),
          total_nair_vxls(0),
          phFormatVersion(CURRENT_VCT_VERSION),
          clib(nullptr),
          trial(nullptr),
          subject(nullptr),
          ph_org(nullptr),
          gen_software(nullptr),
          gen_station(nullptr),
          deformation(nullptr),
          insertion(nullptr),
          voxel_array(nullptr),
          index_table(nullptr)
    {}
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Phantom(const Phantom &rhs)
        : phantom_name(rhs.phantom_name),
          phantom_uid(rhs.phantom_uid),
          phantom_source(rhs.phantom_source),
          phantom_type(rhs.phantom_type),
          data_model(rhs.data_model),
          body_part(rhs.body_part),
          laterality(rhs.laterality),
          glandular_count(rhs.glandular_count),
          x_thickness_mm(rhs.x_thickness_mm),
          y_thickness_mm(rhs.y_thickness_mm),
          z_thickness_mm(rhs.z_thickness_mm),
          total_nair_vxls(rhs.total_nair_vxls),
          date_created(rhs.date_created),
          time_created(rhs.time_created),
          phFormatVersion(rhs.phFormatVersion),
          clib(nullptr),
          trial(nullptr),
          subject(nullptr),
          ph_org(nullptr),
          gen_software(nullptr),
          gen_station(nullptr),
          deformation(nullptr),
          insertion(nullptr),
          voxel_array(nullptr),
          index_table(nullptr)
    {
        if (rhs.clib != nullptr)         clib = new vct::CommonLibrary(*rhs.clib);
        if (rhs.trial != nullptr)        trial = new vct::Trial(*rhs.trial);
        if (rhs.subject != nullptr)      subject = new vct::Subject(*rhs.subject);
        if (rhs.ph_org != nullptr)       ph_org = new vct::Organization(*rhs.ph_org);
        if (rhs.gen_software != nullptr) gen_software = new vct::Software(*rhs.gen_software);
        if (rhs.gen_station != nullptr)  gen_station = new vct::Station(*rhs.gen_station);
        if (rhs.deformation != nullptr)  deformation = new vct::Deformation(*rhs.deformation);
        if (rhs.insertion != nullptr)    insertion = new vct::Insertion(*rhs.insertion);
        if (rhs.voxel_array != nullptr)  voxel_array = new vct::VoxelArray(*rhs.voxel_array);
        if (rhs.index_table != nullptr)  index_table = new vct::IndexTable(*rhs.index_table);
    }
    

    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Phantom() 
	{}


    // Manipulators (used to set component values)

    #pragma region Documentation
    /// <summary>Set phantom name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="name">The name of the phantom. </parm>
    #pragma endregion
    void setPhantomName(std::string name)          { phantom_name = name; }

    #pragma region Documentation
    /// <summary>Set phantom shape. </summary>
    /// <remarks>D. Higginbotham, 2020-11-25. </remarks>
    /// <parm name="name">The shape of the phantom. </parm>
    #pragma endregion
    void setPhantomShape(std::string shape)        { phantom_shape = interpretShape(shape); }


    #pragma region Documentation
    /// <summary>Set phantom shape. </summary>
    /// <remarks>D. Higginbotham, 2020-11-25. </remarks>
    /// <parm name="name">The shape of the phantom. </parm>
    #pragma endregion
    void setPhantomShape(vct::PHANTOM_SHAPE shape) { phantom_shape = shape; }


    #pragma region Documentation
    /// <summary>Set phantom ligament thicknesses. </summary>
    /// <remarks>D. Higginbotham, 2021-01-13. </remarks>
    /// <parm name="max_lthick">The maximum ligament thickness of the phantom. </parm>
    /// <parm name="min_lthick">The minimum ligament thickness of the phantom. </parm>
    #pragma endregion
    void setLigamentThicknesses(const float max_lthick, const float min_lthick )
    {
        max_lig_thick = max_lthick; 
        min_lig_thick = min_lthick; 
    }
    
    
    #pragma region Documentation
    /// <summary>Set the UID of the phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="uid">The phantom UID. </parm>
    #pragma endregion
    void setUid(std::string uid)                   { phantom_uid = uid; }
    
    
    #pragma region Documentation
    /// <summary>Set the phantom source. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="src">The phantom source. </parm>
    #pragma endregion
    void setSource(vct::PHANTOM_SOURCE src)        { phantom_source = src; }
    
    
    #pragma region Documentation
    /// <summary>Set the phantom type. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="typ">The phantom type. </parm>
    #pragma endregion
    void setType(vct::PHANTOM_TYPE typ)            { phantom_type = typ; }
    
    
    #pragma region Documentation
    /// <summary>Set the phantom data model. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="dm">The phantom data model. </parm>
    #pragma endregion
    void setDataModel(vct::PHANTOM_DATA_MODEL dm)  { data_model = dm; }
    
    
    #pragma region Documentation
    /// <summary>Set the phantom body part. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="bp">The phantom body part. </parm>
    #pragma endregion
    void setBodyPart(vct::PHANTOM_BODY_PART bp)    { body_part = bp; }
    
    
    #pragma region Documentation
    /// <summary>Set the phantom laterality. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="la">The phantom laterality. </parm>
    #pragma endregion
    void setLaterality(vct::PHANTOM_LATERALITY la) { laterality = la; }


    #pragma region Documentation
    /// <summary>Set the glandular count. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <parm name="cnt">The glandular count. </parm>
    #pragma endregion
    void setGlandularCount(size_t cnt)       { glandular_count = cnt; }
    
    
    #pragma region Documentation
    /// <summary>Set the phantom thickness in X. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <parm name="thk">The phantom thickness - extent X axis (mm). </parm>
    #pragma endregion
    void setXThickness_mm(double thk)        { x_thickness_mm = thk; }
    
    
    #pragma region Documentation
    /// <summary>Set the phantom thickness in Y. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <parm name="thk">The phantom thickness - extent Y axis (mm). </parm>
    #pragma endregion
    void setYThickness_mm(double thk)        { y_thickness_mm = thk; }
    
    
    #pragma region Documentation
    /// <summary>Set the phantom thickness in Z. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <parm name="thk">The phantom thickness - extent Z axis (mm). </parm>
    #pragma endregion
    void setZThickness_mm(double thk)        { z_thickness_mm = thk; }


    #pragma region Documentation
    /// <summary>Set the number of non-air voxels. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <parm name="vxls">The total number of non-air voxels. </parm>
    #pragma endregion
    void setTotalNonAirVoxels(size_t vxls)   { total_nair_vxls = vxls; }

    
    #pragma region Documentation
    /// <summary>Set the phantom creation date. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="date">The phantom creation date. </parm>
    #pragma endregion
    void setDateCreated(vct::Date date)      { date_created = date; }
    
    
    #pragma region Documentation
    /// <summary>Set the phantom creation time. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="time">Phantom creation time. </parm>
    #pragma endregion
    void setTimeCreated(vct::Time time)      { time_created = time; }

    
    #pragma region Documentation
    /// <summary>Set the phantom format version. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vers">The phantom format version. </parm>
    #pragma endregion
    void setFormatVersion(float vers)        { phFormatVersion = vers; }
    
    
    #pragma region Documentation
    /// <summary>Set the corresponding Trial instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="tr">The corresponding Trial instance. </parm>
    #pragma endregion
    void setTrial(Trial *tr)                 { trial = tr; }
    
    
    #pragma region Documentation
    /// <summary>Set the corresponding Subject instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="sub">The corresponding Subject instance. </parm>
    #pragma endregion
    void setSubject(Subject *sub)            { subject = sub; }
    
    
    #pragma region Documentation
    /// <summary>Set the Generating Organization. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="org">The generating organization. </parm>
    #pragma endregion
    void setGenOrganization(Organization *org) { ph_org = org; }
    
    
    #pragma region Documentation
    /// <summary>Set the Generating Software instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="sw">The Generating Software instance. </parm>
    #pragma endregion
    void setGenSoftware(Software  *sw)       { gen_software = sw; }
    
    
    #pragma region Documentation
    /// <summary>Set the Generating Station instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="st">The Generating Station instance. </parm>
    #pragma endregion
    void setGenStation(Station *st)          { gen_station = st; }
    
    
    #pragma region Documentation
    /// <summary>Set the VoxelArray instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="va">The VoxelArray instance. </parm>
    #pragma endregion
    void setVoxelArray(VoxelArray *va)       { voxel_array = va; }
    
    
    #pragma region Documentation
    /// <summary>Set the IndexTable instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="it">The IndexTable instance. </parm>
    #pragma endregion
    void setIndexTable(IndexTable *it)       { index_table = it; }

    
    #pragma region Documentation
    /// <summary>Set the Deformation instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="def">The Deformation instance. </parm>
    #pragma endregion
    #pragma endregion
    void setDeformation(Deformation *def)    { deformation = def; }
    
    
    #pragma region Documentation
    /// <summary>Set the Insertion instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ins">The Insertion instance. </parm>
    #pragma endregion
    void setInsertion(Insertion *ins)        { insertion = ins; }

    
    #pragma region Documentation
    /// <summary>Set the Common Library Instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib_ptr">The Common Library Instance. </parm>
    #pragma endregion
    void setCommonLib(CommonLibrary *clib_ptr) { clib = clib_ptr; }


    // Accessors (used to retrieve component values)

    
    #pragma region Documentation
    /// <summary>Retrieve the phantom name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom name. </return>
    #pragma endregion
    std::string getPhantomName()             { return phantom_name; }

    
    #pragma region Documentation
    /// <summary>Retrieve the phantom shape. </summary>
    /// <remarks>D. Higginbotham, 2020-11-25. </remarks>
    /// <return>The Phantom shape (enum). </return>
    #pragma endregion
    vct::PHANTOM_SHAPE getPhantomShape()     { return phantom_shape; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom UID. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom UID. </return>
    #pragma endregion
    std::string getUid()                     { return phantom_uid; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom source. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom source. </return>
    #pragma endregion
    vct::PHANTOM_SOURCE getSource()          { return phantom_source; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom type. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom type. </return>
    #pragma endregion
    vct::PHANTOM_TYPE getType()              { return phantom_type; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom data model. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom data model. </return>
    #pragma endregion
    vct::PHANTOM_DATA_MODEL getDataModel()   { return data_model; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom body part. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom body part. </return>
    #pragma endregion
    vct::PHANTOM_BODY_PART getBodyPart()     { return body_part; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom laterality. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom laterality. </return>
    #pragma endregion
    vct::PHANTOM_LATERALITY getLaterality()  { return laterality; }


    #pragma region Documentation
    /// <summary>Retrieve the glandular count. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <return>The glandular count. </return>
    #pragma endregion
    size_t getGlandularCount()              { return glandular_count; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom thickness in X. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <return>The phantom thickness - extent X axis (mm). </return>
    #pragma endregion
    double getXThickness_mm()               { return x_thickness_mm; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom thickness in Y. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <return>The phantom thickness - extent Y axis (mm). </return>
    #pragma endregion
    double getYThickness_mm()              { return y_thickness_mm; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom thickness in Z. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <return>The phantom thickness - extent Z axis (mm). </return>
    #pragma endregion
    double getZThickness_mm()              { return z_thickness_mm; }


    #pragma region Documentation
    /// <summary>Retrieve the number of non-air voxels. </summary>
    /// <remarks>D. Higginbotham, 2017-06-06. </remarks>
    /// <return>The total number of non-air voxels. </return>
    #pragma endregion
    size_t getTotalNonAirVoxels()           { return total_nair_vxls; }

    
    #pragma region Documentation
    /// <summary>Retrieve the phantom date created. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom date created. </return>
    #pragma endregion
    vct::Date getDateCreated()              { return date_created; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom time created. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom time created. </return>
    #pragma endregion
    vct::Time getTimeCreated()              { return time_created; }

    
    #pragma region Documentation
    /// <summary>Retrieve the phantom format version. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom format version. </return>
    #pragma endregion
    float getFormatVersion()                { return phFormatVersion; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom Trial instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom Trial instance. </return>
    #pragma endregion
    Trial *getTrial()                       { return trial; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom Subject instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom Subject instance. </return>
    #pragma endregion
    Subject *getSubject()                   { return subject; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom Organization instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom Organization instance. </return>
    #pragma endregion
    Organization *getGenOrganization()      { return ph_org; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom Software instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom Software instance. </return>
    #pragma endregion
    Software *getGenSoftware()              { return gen_software; }    
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom Station instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom Station instance. </return>
    #pragma endregion
    Station *getGenStation()                { return gen_station; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom VoxelArray instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom VoxelArray instance. </return>
    #pragma endregion
    VoxelArray *getVoxelArray()             { return voxel_array; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the IndexTable instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="va">The Phantom IndexTable instance. </parm>
    #pragma endregion
    IndexTable *getIndexTable()             { return index_table; }

    
    #pragma region Documentation
    /// <summary>Retrieve the phantom Deformation instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom Deformation instance. </return>
    #pragma endregion
    Deformation *getDeformation()           { return deformation; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the phantom Insertion instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom Insertion instance. </return>
    #pragma endregion
    Insertion *getInsertion()               { return insertion; }

    
    #pragma region Documentation
    /// <summary>Retrieve the phantom CommonLibrary instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The Phantom CommonLibrary instance. </return>
    #pragma endregion
    CommonLibrary *getCommonLib()           { return clib; }


    // XML file Interaction 


    #pragma region Documentation
    /// <summary>Hi-level entry point for reading XML files from the VCTX phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void readXML()
    {
        // Using current node, read phantom
        if (clib)
        {
            if (clib->findNodeXML("VCT_Phantom"))
            {
                readPhantomXML();
            }
        }            
    }


    #pragma region Documentation
    /// <summary>Read Subject from (current) XML file in the VCTX phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void readSubjectXML()
    {
        if (clib->findNodeXML("Subject"))
        {
            if (subject == nullptr) subject = new Subject;
            if (subject != nullptr)
            {
                subject->readXML(clib);                
            }
            clib->concludeSectionXML(); // return above VCT_Subject
        } else std::cerr << "Phantom::readSubjectXML: \"Subject\" field not found" << std::endl;
    }


    #pragma region Documentation
    /// <summary>Read Trial from (current) XML file in the VCTX phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void readTrialXML()
    {
        if (clib->findNodeXML("Trial"))
        {
            if (trial == nullptr) trial = new Trial;
            if (trial != nullptr)
            {
                trial->readXML(clib);
            }
            clib->concludeSectionXML(); // return above VCT_Trial node
        } else std::cerr << "Phantom::readTrialXML: \"Trial\" field not found" << std::endl;
    }

 
    #pragma region Documentation
    /// <summary>Lower-level read Phantom from (current) XML file in the VCTX phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void readPhantomXML()
    {
        clib->getNodeValueXML("Phantom_Name", phantom_name);
        std::string shape;
        clib->getNodeValueXML("Phantom_Shape", shape);
        phantom_shape = interpretShape(shape);
        clib->getNodeValueXML("Max_Ligament_Thickness", max_lig_thick);
        clib->getNodeValueXML("Min_Ligament_Thickness", min_lig_thick);
        clib->getNodeValueXML("Phantom_UID", phantom_uid);

        // Read Phantom Source
        std::string source;
        if (clib->getNodeValueXML("Phantom_Source", source))
        {
            phantom_source = interpretSource(source);
        } else std::cerr << "Phantom::readXML: \"Phantom_Source\" field not found" << std::endl;

        // Read Phantom Type
        std::string ph_type;
        if (clib->getNodeValueXML("Phantom_Type", ph_type))
        {
            phantom_type = interpretType(ph_type);
        } else std::cerr << "Phantom::readXML: \"Phantom_Type\" field not found" << std::endl;

        // Read Data Model
        std::string dmodel;
        if (clib->getNodeValueXML("Data_Model", dmodel))
        {
            data_model = interpretDataModel(dmodel);
        } else std::cerr << "Phantom::readXML: \"Data_Model\" field not found" << std::endl;

        // Read Body Part
        std::string part;
        if (clib->getNodeValueXML("Body_Part", part))
        {
            body_part = interpretBodyPart(part);
        } else std::cerr << "Phantom::readXML: \"Body_Part\" field not found" << std::endl;

        // Read laterality
        std::string lat;
        if (clib->getNodeValueXML("Laterality", lat))
        {
            laterality = interpretLaterality(lat);
        } else std::cerr << "Phantom::readXML: \"Laterality\" field not found" << std::endl;

        // Read Non-Air voxels
        clib->getNodeValueXML("Total_Non_Air_Voxels", total_nair_vxls);

        // Read  thickness section
        if (clib->findNodeXML("Thickness_mm"))
        {
            clib->getNodeValueXML("X", x_thickness_mm);
            clib->getNodeValueXML("Y", y_thickness_mm);
            clib->getNodeValueXML("Z", z_thickness_mm);
            clib->concludeSectionXML();
        } else std::cerr << "Phantom::readXML: \"Thickness_mm\" field not found" << std::endl;

        // Read glandular count
        clib->getNodeValueXML("Glandular_Count", glandular_count);
        
        // Read Date Created 
        if (clib->findNodeXML("Date_Created"))
        {
            date_created.readXML(clib);
            clib->concludeSectionXML();
        } else std::cerr << "Phantom::readXML: \"Date_Created\" field not found" << std::endl; 
        
        // Read Time Created
        if (clib->findNodeXML("Time_Created"))
        {
            time_created.readXML(clib);
            clib->concludeSectionXML();
        } else std::cerr << "Phantom::readXML: \"Time_Created\" field not found" << std::endl;

        readSubjectXML();
        readTrialXML();                

        // Read organization if present
        if (clib->findNodeXML("Organization"))
        {
            if (ph_org == nullptr) ph_org = new Organization;
            if (ph_org != nullptr)
            {
                ph_org->readXML(clib);                
            }
            clib->concludeSectionXML(); // return above Originating Organization
        } else std::cerr << "Phantom::readXML: \"Organization\" field not found" << std::endl;
        
        // Read Generation Software if present
        if (clib->findNodeXML("Generation_Software"))
        {
            if (gen_software == nullptr) gen_software = new Software;
            if (gen_software != nullptr)
            {
                gen_software->readXML(clib);                
            }
            clib->concludeSectionXML(); // return above Generation_Software
        } else std::cerr << "Phantom::readXML: \"Generation_Software\" field not found" << std::endl;
        
        // Read Generation Station if present
        if (clib->findNodeXML("Generation_Station"))
        {
            if (gen_station == nullptr) gen_station = new Station;
            if (gen_station != nullptr)
            {
                gen_station->readXML(clib);                
            }
            clib->concludeSectionXML(); // return above Generation_Station
        } else std::cerr << "Phantom::readXML: \"Generation_Station\" field not found" << std::endl;

        // Read the Voxel Array appropriately
        if (phantom_type == vct::VOXEL_ARRAY && clib->findNodeXML("Voxel_Array"))
        {
            if (voxel_array == nullptr) voxel_array = new VoxelArray;
            if (voxel_array != nullptr)
            {
                voxel_array->readXML(clib);                
            }
            clib->concludeSectionXML(); // return above voxel_array
        } else std::cerr << "Phantom::readXML: \"Voxel_Array\" field not found" << std::endl;
        

        // Read the Index Table appropriately
        if (phantom_type == vct::VOXEL_ARRAY && clib->findNodeXML("Index_Table"))
        {
            if (index_table == nullptr) index_table = new IndexTable;
            if (index_table != nullptr)
            {
                index_table->readXML(clib);                
            }
            clib->concludeSectionXML(); // return above index_table
        } else std::cerr << "Phantom::readXML: \"Index_Table\" field not found" << std::endl;

        // Read Deformation if present
        if (clib->findNodeXML("Deformation"))
        {
            if (deformation == nullptr) deformation = new Deformation;
            if (deformation != nullptr)
            {
                deformation->readXML(clib);
            }
            clib->concludeSectionXML(); // return above Deformation
        } else std::cout << "Phantom::readXML: FYI: \"Deformation\" field not found; phantom has not been deformed." << std::endl;

        // Read Insertion if present (it's perfectly okay for them NOT to be present)
        if (clib->findNodeXML("Insertions"))
        {
            if (insertion == nullptr) insertion = new Insertion;
            if (insertion != nullptr)
            {
                insertion->readXML(clib);
            }
            clib->concludeSectionXML(); // return above Insertion
        }

        clib->concludeSectionXML(); // return above VCT_Subject
    }



    #pragma region Documentation
    /// <summary>Write the VCTX phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void writePhantomXML()
    {
        clib->writeNodeValueXML("Phantom_Name", phantom_name);
        clib->writeNodeValueXML("Phantom_Shape", interpretShape(phantom_shape));
        clib->writeNodeValueXML("Max_Ligament_Thickness", max_lig_thick);
        clib->writeNodeValueXML("Min_Ligament_Thickness", min_lig_thick);
        if (phantom_uid.size() > 0)
        {
            clib->writeNodeValueXML("Phantom_UID", phantom_uid);
        }

        // Write the Phantom Source
        std::string source = interpretSource(phantom_source);
        clib->writeNodeValueXML("Phantom_Source", source);

        // Write the Phantom Type
        std::string ph_type = interpretType(phantom_type);
        clib->writeNodeValueXML("Phantom_Type", ph_type);

        // Write the Data Model
        std::string dmodel = interpretDataModel(data_model);
        clib->writeNodeValueXML("Data_Model", dmodel);

        // Write the Body Part
        std::string part = interpretBodyPart(body_part);
        clib->writeNodeValueXML("Body_Part", part);

        // Write the laterality
        std::string lat = interpretLaterality(laterality);
        clib->writeNodeValueXML("Laterality", lat);

        // Write Non-Air voxels
        clib->writeNodeValueXML("Total_Non_Air_Voxels", total_nair_vxls);
        
        // Write thickness section
        clib->addASectionXML("Thickness_mm");
        clib->writeNodeValueXML("X", x_thickness_mm);
        clib->writeNodeValueXML("Y", y_thickness_mm);
        clib->writeNodeValueXML("Z", z_thickness_mm);
        clib->concludeSectionXML();

        // Write glandular count
        clib->writeNodeValueXML("Glandular_Count", glandular_count);
        
        clib->addASectionXML("Date_Created");
        date_created.writeXML(clib);
        clib->concludeSectionXML();
        
        clib->addASectionXML("Time_Created");        
        time_created.writeXML(clib);
        clib->concludeSectionXML();

        // Write the subject if subject object is present
        if (subject != nullptr)
        {
            clib->addASectionXML("Subject");
            subject->writeXML(clib);
            clib->concludeSectionXML();
        }

        // Write the trial if trial object is present
        if (trial != nullptr)
        {
            if (clib->addASectionXML("Trial"))
            {
                if (trial != nullptr)
                {
                    trial->writeXML(clib);
                }
            }
            clib->concludeSectionXML(); // return above Originating Organization
        }

        // Write the organization if organization object is present
        if (ph_org != nullptr)
        {
            if (clib->addASectionXML("Organization"))
            {
                if (ph_org != nullptr)
                {
                    ph_org->writeXML(clib);
                }
            }
            clib->concludeSectionXML(); // return above Originating Organization
        }
        
        // Write the Generation Software information if software object is present
        if (gen_software != nullptr)
        {
            if (clib->addASectionXML("Generation_Software"))
            {
                if (gen_software != nullptr)
                {
                    gen_software->writeXML(clib);
                }
            }
            clib->concludeSectionXML(); // return above Originating Organization
        }

        // Write the Generation Software information if software object is present
        if (gen_station != nullptr)
        {
            if (clib->addASectionXML("Generation_Station"))
            {
                if (gen_station != nullptr)
                {
                    gen_station->writeXML(clib);
                }
            }
            clib->concludeSectionXML(); // return above Originating Organization
        }

        // Write the Voxel Array section appropriately
        if (phantom_type == vct::VOXEL_ARRAY && voxel_array != nullptr)
        {
            if (clib->addASectionXML("Voxel_Array"))
            {
                 voxel_array->writeXML(clib);
                 clib->concludeSectionXML(); // return above voxel_array
            }
        }

        // Write the Index Table section appropriately
        if (phantom_type == vct::VOXEL_ARRAY && index_table != nullptr)
        {
            if (clib->addASectionXML("Index_Table"))
            {
                index_table->writeXML(clib);
                clib->concludeSectionXML(); // return above index_table
            }
        }

        // Write the Deformation information if present
        if (deformation != nullptr)
        {
            if (clib->addASectionXML("Deformation"))
            {
                if (deformation != nullptr)
                {
                    deformation->writeXML(clib); 
                }
                clib->concludeSectionXML(); // return above Originating Organization
            }
        }

        // Write the Insertion information if present
        if (insertion != nullptr)
        {
            if (clib->addASectionXML("Insertions"))
            {
                insertion->writeXML(clib); 
                clib->concludeSectionXML(); // return above 
            }
        }

        clib->concludeSectionXML(); // return above VCT_Subject
    }


    #pragma region Documentation
    /// <summary>Read the phantom data from the VCTX phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="phantomDat">Name of phantom data file. </parm>
    #pragma endregion
    void readPhantomData(std::string phantomDat)  
    {
        if (voxel_array != nullptr)
        {
            voxel_array->readPhantomData(clib, phantomDat);
        }
    }

    
    #pragma region Documentation
    /// <summary>Write the Phantom's data into the VCTX phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="phantomDat">Name of phantom data file. </parm>
    #pragma endregion
    void writePhantomData(std::string phantomDat)  
    {
        if (voxel_array != nullptr)
        {
            voxel_array->writePhantomData(clib, phantomDat);
        }
    }


    #pragma region Documentation
    /// <summary>Convert a string to the corresponding phantom source enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="src">A string representing a corresponding phantom source enumeration instance. </parm>
    /// <return>The corresponding Phantom source enumeration instance (or unknown source if not found). </return>
    #pragma endregion  
    PHANTOM_SOURCE interpretSource(std::string src)
    {
        std::transform(src.begin(), src.end(), src.begin(), toupper); // convert to upper case

        PHANTOM_SOURCE retval(UNK_SOURCE);
        if      (src.find("PENN") != std::string::npos) retval = PENN;
        else if (src.find("DUKE") != std::string::npos) retval = DUKE;
        else if (src.find("FDA") != std::string::npos)  retval = FDA;
        else if (src == "OTHER_SOURCE")                 retval = OTHER_SOURCE;
        return retval;
    }
    
    
    #pragma region Documentation
    /// <summary>Convert a phantom source enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ps">Phantom source enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretSource(PHANTOM_SOURCE ps)
    {
        std::string retval("UNK_SOURCE");

        if      (ps == PENN)         retval = "PENN";
        else if (ps == DUKE)         retval = "DUKE";
        else if (ps == FDA)          retval = "FDA";
        else if (ps == OTHER_SOURCE) retval = "OTHER_SOURCE";
        return retval;
    }
    
    
    #pragma region Documentation
    /// <summary>Convert a string to the corresponding phantom shape enumeration. </summary>
    /// <remarks>D. Higginbotham, 2020-11-25. </remarks>
    /// <parm name="type">A string representing a corresponding phantom shape enumeration instance. </parm>
    /// <return>The corresponding Phantom shape enumeration instance (or unknown shape if not found). </return>
    #pragma endregion  
    PHANTOM_SHAPE interpretShape(std::string shape)
    {
        std::transform(shape.begin(), shape.end(), shape.begin(), toupper); // convert to upper case

        PHANTOM_SHAPE retval(UNK_SHAPE);
        if      (shape == "BREAST") retval = BREAST_SHAPE;
        else if (shape == "CUBE")   retval = CUBE_SHAPE;
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a phantom type enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2020-11-25. </remarks>
    /// <parm name="type">Phantom shape enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretShape(PHANTOM_SHAPE shape)
    {
        std::string retval("UNK_SHAPE");
        if      (shape == vct::BREAST_SHAPE)  retval = "BREAST";
        else if (shape == vct::CUBE_SHAPE)    retval = "CUBE";
        return retval;
    }


#pragma region Documentation
    /// <summary>Convert a string to the corresponding phantom type enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="type">A string representing a corresponding phantom type enumeration instance. </parm>
    /// <return>The corresponding Phantom type enumeration instance (or unknown type if not found). </return>
    #pragma endregion  
    PHANTOM_TYPE interpretType(std::string type)
    {
        std::transform(type.begin(), type.end(), type.begin(), toupper); // convert to upper case

        PHANTOM_TYPE retval(UNK_TYPE);
        if      (type == "VOXEL_ARRAY") retval = VOXEL_ARRAY;
        else if (type == "VOXEL ARRAY") retval = VOXEL_ARRAY;
        else if (type == "OCTREE")      retval = OCTREE;
        else if (type == "NURBS")       retval = NURBS;
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a phantom type enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="type">Phantom type enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretType(PHANTOM_TYPE type)
    {
        std::string retval("UNK_TYPE");
        if      (type == vct::VOXEL_ARRAY)  retval = "VOXEL_ARRAY";
        else if (type == vct::OCTREE)       retval = "OCTREE";
        else if (type == vct::NURBS)        retval = "NURBS";
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a string to the corresponding phantom data model enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="model">A string representing a corresponding phantom data model enumeration instance. </parm>
    /// <return>The corresponding Phantom data model enumeration instance (or unknown data model if not found). </return>
    #pragma endregion  
    PHANTOM_DATA_MODEL interpretDataModel(std::string model)
    {
        std::transform(model.begin(), model.end(), model.begin(), toupper); // convert to upper case

        PHANTOM_DATA_MODEL retval(UNK_DATAMODEL);
        if      (model.find("INDEXED") != std::string::npos)   retval = vct::INDEXED;
        else if (model.find("PHYSICAL") != std::string::npos)  retval = vct::PHYSICAL;
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a phantom data model enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="model">Phantom data model enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretDataModel(PHANTOM_DATA_MODEL model)
    {
        std::string retval("UNK_DATAMODEL");
        if      (model == vct::INDEXED)   retval = "INDEXED";
        else if (model == vct::PHYSICAL)  retval = "PHYSICAL";
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a string to the corresponding phantom body part enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="part">A string representing a corresponding phantom body part enumeration instance. </parm>
    /// <return>The corresponding Phantom body part enumeration instance (or unknown body part if not found). </return>
    #pragma endregion  
    PHANTOM_BODY_PART interpretBodyPart(std::string part)
    {
        std::transform(part.begin(), part.end(), part.begin(), toupper); // convert to upper case

        PHANTOM_BODY_PART retval(UNK_BODYPART);
            if      (part == "ABDOMEN")       retval = vct::ABDOMEN;
            else if (part == "ANKLE")         retval = vct::ANKLE;
            else if (part == "ARMPIT")        retval = vct::ARMPIT;
            else if (part == "BACK")          retval = vct::BACK;
            else if (part == "BACK_OF_ELBOW") retval = vct::BACK_OF_ELBOW;
            else if (part == "BACK_OF_KNEE")  retval = vct::BACK_OF_KNEE;
            else if (part == "BREAST")        retval = vct::BREAST;
            else if (part == "BUTTOCK")       retval = vct::BUTTOCK;
            else if (part == "CALF")          retval = vct::CALF;
            else if (part == "CHEEK")         retval = vct::CHEEK;
            else if (part == "CHEST")         retval = vct::CHEST;
            else if (part == "CHIN")          retval = vct::CHIN;
            else if (part == "EAR")           retval = vct::EAR;
            else if (part == "EYE")           retval = vct::EYE;
            else if (part == "FACE")          retval = vct::FACE;
            else if (part == "FINGER")        retval = vct::FINGER;
            else if (part == "FOOT")          retval = vct::FOOT;
            else if (part == "FOREARM")       retval = vct::FOREARM;
            else if (part == "FOREHEAD")      retval = vct::FOREHEAD;
            else if (part == "FRONT_OF_ELBOW")  retval = vct::FRONT_OF_ELBOW;
            else if (part == "GROIN")         retval = vct::GROIN;
            else if (part == "HAND")          retval = vct::HAND;
            else if (part == "HEAD")          retval = vct::HEAD;
            else if (part == "HEAL_OF_FOOT")  retval = vct::HEAL_OF_FOOT;
            else if (part == "KNEE")          retval = vct::KNEE;
            else if (part == "KNEECAP")       retval = vct::KNEECAP;
            else if (part == "LEG")           retval = vct::LEG;
            else if (part == "LOIN")          retval = vct::LOIN;
            else if (part == "MOUTH")         retval = vct::MOUTH;
            else if (part == "NAVAL")         retval = vct::NAVAL;
            else if (part == "NECK")          retval = vct::NECK;
            else if (part == "NOSE")          retval = vct::NOSE;
            else if (part == "PALM")          retval = vct::PALM;
            else if (part == "PELVIS")        retval = vct::PELVIS;
            else if (part == "RIBS")          retval = vct::RIBS;
            else if (part == "SCAPULA")       retval = vct::SCAPULA;
            else if (part == "STERNUM")       retval = vct::STERNUM;
            else if (part == "SHOULDER")      retval = vct::SHOULDER;
            else if (part == "SKULL")         retval = vct::SKULL;
            else if (part == "SOLE_OF_FOOT")  retval = vct::SOLE_OF_FOOT;
            else if (part == "THIGH")         retval = vct::THIGH;
            else if (part == "THUMB")         retval = vct::THUMB;
            else if (part == "TOE")           retval = vct::TOE;
            else if (part == "VERTEBRAE")     retval = vct::VERTEBRAE;
            else if (part == "WRIST")         retval = vct::WRIST;
            else if (part == "PHYSICAL_PHANTOM") retval = vct::PHYSICAL_PHANTOM;
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a phantom body part enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="part">Phantom body part enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretBodyPart(PHANTOM_BODY_PART part)
    {
        std::string retval("UNK_BODYPART");
        if      (part == vct::ABDOMEN)         retval = "ABDOMEN";
        else if (part == vct::ANKLE)           retval = "ANKLE";
        else if (part == vct::ARMPIT)          retval = "ARMPIT";
        else if (part == vct::BACK)            retval = "BACK";
        else if (part == vct::BACK_OF_ELBOW)   retval = "BACK_OF_ELBOW";
        else if (part == vct::BACK_OF_KNEE)    retval = "BACK_OF_KNEE";
        else if (part == vct::BREAST)          retval = "BREAST";
        else if (part == vct::BUTTOCK)         retval = "BUTTOCK";
        else if (part == vct::CALF)            retval = "CALF";
        else if (part == vct::CHEEK)           retval = "CHEEK";
        else if (part == vct::CHEST)           retval = "CHEST";
        else if (part == vct::CHIN)            retval = "CHIN";
        else if (part == vct::EAR)             retval = "EAR";
        else if (part == vct::EYE)             retval = "EYE";
        else if (part == vct::FACE)            retval = "FACE";
        else if (part == vct::FINGER)          retval = "FINGER";
        else if (part == vct::FOOT)            retval = "FOOT";
        else if (part == vct::FOREARM)         retval = "FOREARM";
        else if (part == vct::FOREHEAD)        retval = "FOREHEAD";
        else if (part == vct::FRONT_OF_ELBOW)  retval = "FRONT_OF_ELBOW";
        else if (part == vct::GROIN)           retval = "GROIN";
        else if (part == vct::HAND)            retval = "HAND";
        else if (part == vct::HEAD)            retval = "HEAD";
        else if (part == vct::HEAL_OF_FOOT)    retval = "HEAL_OF_FOOT";
        else if (part == vct::KNEE)            retval = "KNEE";
        else if (part == vct::KNEECAP)         retval = "KNEECAP";
        else if (part == vct::LEG)             retval = "LEG";
        else if (part == vct::LOIN)            retval = "LOIN";
        else if (part == vct::MOUTH)           retval = "MOUTH";
        else if (part == vct::NAVAL)           retval = "NAVAL";
        else if (part == vct::NECK)            retval = "NECK";
        else if (part == vct::NOSE)            retval = "NOSE";
        else if (part == vct::PALM)            retval = "PALM";
        else if (part == vct::PELVIS)          retval = "PELVIS";
        else if (part == vct::RIBS)            retval = "RIBS";
        else if (part == vct::SCAPULA)         retval = "SCAPULA";
        else if (part == vct::STERNUM)         retval = "STERNUM";
        else if (part == vct::SHOULDER)        retval = "SHOULDER";
        else if (part == vct::SKULL)           retval = "SKULL";
        else if (part == vct::SOLE_OF_FOOT)    retval = "SOLE_OF_FOOT";
        else if (part == vct::THIGH)           retval = "THIGH";
        else if (part == vct::THUMB)           retval = "THUMB";
        else if (part == vct::TOE)             retval = "TOE";
        else if (part == vct::VERTEBRAE)       retval = "VERTEBRAE";
        else if (part == vct::WRIST)           retval = "WRIST";
        else if (part == vct::PHYSICAL_PHANTOM)retval = "PHYSICAL_PHANTOM";
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a string to the corresponding phantom laterality enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="lat">A string representing a corresponding phantom laterality enumeration instance. </parm>
    /// <return>The corresponding Phantom laterality enumeration instance (or unknown laterality if not found). </return>
    #pragma endregion  
    PHANTOM_LATERALITY interpretLaterality(std::string lat)
    {
        std::transform(lat.begin(), lat.end(), lat.begin(), toupper); // convert to upper case

        PHANTOM_LATERALITY retval(UNK_LATERALITY);
        if      (lat == "LEFT")            retval = vct::LEFT;
        else if (lat == "RIGHT")           retval = vct::RIGHT;
        else if (lat == "LEFT")            retval = vct::LEFT;
        else if (lat == "LEFT_AND_RIGHT")  retval = vct::LEFT_AND_RIGHT;
        else if (lat == "LEFT AND RIGHT")  retval = vct::LEFT_AND_RIGHT;
        else if (lat == "WHOLE_BODY")      retval = vct::WHOLE_BODY;
        else if (lat == "WHOLE BODY")      retval = vct::WHOLE_BODY;
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a phantom laterality enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="lat">Phantom laterality enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretLaterality(PHANTOM_LATERALITY lat)
    {
        std::string retval("UNK_LATERALITY");
        if      (lat == vct::LEFT)            retval = "LEFT";
        else if (lat == vct::RIGHT)           retval = "RIGHT";
        else if (lat == vct::LEFT)            retval = "LEFT";
        else if (lat == vct::LEFT_AND_RIGHT)  retval = "LEFT_AND_RIGHT";
        else if (lat == vct::WHOLE_BODY)      retval = "WHOLE_BODY";
        return retval;
    }


private:

    std::string         phantom_name;   /// phantom name
    PHANTOM_SHAPE       phantom_shape;  /// phantom shape
    float               max_lig_thick;  /// maximum ligament thickness for Cube shape
    float               min_lig_thick;  /// minimum ligament thickness for Cube shape
    std::string         phantom_uid;    /// phantom unique identifier
    PHANTOM_SOURCE      phantom_source; /// where the phantom came from (enumeration)
    PHANTOM_TYPE        phantom_type;   /// phantom data type (enumeration)
    PHANTOM_DATA_MODEL  data_model;     /// phantom data model (enumeration)
    PHANTOM_BODY_PART   body_part;      /// body part the phantom represents (enumeration)
    PHANTOM_LATERALITY  laterality;     /// laterality of the body part the phantom represents (enumeration)

    size_t       glandular_count;  /// the number of glandular voxels
    double       x_thickness_mm;   /// the phantom extent along the x axis
    double       y_thickness_mm;   /// the phantom extent along the y axis
    double       z_thickness_mm;   /// the phantom extent along the z axis
    size_t       total_nair_vxls;  /// the total of non-air voxels

    Date         date_created;     /// the date the phantom was created
    Time         time_created;     /// the time the phanotm was created

    float        phFormatVersion;  /// version of the phantom format
    
    CommonLibrary *clib;           /// pointer to the current common library instance
    Trial         *trial;          /// pointer to the corresponding phantom trial instance
    Subject       *subject;        /// pointer to the corresponding phantom subject instance
    Organization  *ph_org;         /// pointer to the corresponding phantom generation organization instance
    Software      *gen_software;   /// pointer to the corresponding phantom generation software instance
    Station       *gen_station;    /// pointer to the corresponding phantom generation workstation instance
    Deformation   *deformation;    /// pointer to the corresponding phantom deformation instance
    Insertion     *insertion;      /// pointer to the corresponding phantom insertion instance
    VoxelArray    *voxel_array;    /// pointer to the corresponding phantom voxel array instance
    IndexTable    *index_table;    /// pointer to the corresponding phantom index table instance
};

} // namespace vct
