// VCT_Subject.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_Date.h"

#include <string>
#include <algorithm>

#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

#pragma region Documentation
/// <summary>The subject type (currently two choices, anthropomorphic or physics; e.g., step-wedge). </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum SUBJECT_TYPE
{
    UNK_SUBJ_TYPE   = 0,
    ANTHROPOMORPHIC = 1,
    PHYSICS         = 2
};


#pragma region Documentation
/// <summary>The gender of the subject. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum GENDER
{
    UNK_GENDER    = 0,
    FEMALE        = 1,
    MALE          = 2,
    HERMAPHRODITE = 3,
    NONE          = 4
};


#pragma region Documentation
/// <summary>The race (major group) of the subject. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum RACE
{
    UNK_RACE  = 0,
    CAUCASIAN = 1,
    MONGOLOID = 2,
    NEGROID   = 3
};


#pragma region Documentation
/// <summary>The ethnicity of the subject. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum ETHNICITY  // Source: List of contemporary ethnic groups From Wikipedia, the free encyclopedia
{
    UNK_ETHNICITY = 0,
    NIL           = 1, // NULL is a reserved word in C++
    ACHOLI        = 2,
    AKAN          = 3,
    ALBANIANS     = 4,
    AFAR          = 5,
    AFRIKANERS    = 6,
    AMHARA        = 7,
    ARABS         = 8,
    ARMENIANS     = 9,
    ASSAMESE      = 10,
    ASSYRIANS     = 11,
    AZERBAIJANIS  = 12,
            
    BALOCHIS    =   13,
    BAMARS      =   14,
    BAMBARA     =   15,
    BASHKIRS    =   16,
    BASQUES     =   17,
    BELARUSIANS =   18,
    BEMBA       =   19,
    BENGALI     =   20,
    BERBERS     =   21,
    BETI_PAHUIN =   22,
    BOSNIAKS    =   23,
    BRAHUI      =   24,
    BRITISH     =   25,
    BULGARIANS  =   26,
            
    CATALANS    =   27,
    CHUVASH     =   28,
    CIRCASSIANS =   29,
    CHEWA       =   30,
    CORNISH     =   31,
    CORNWALL    =   32,
    CROATS      =   33,
    CZECHS      =   34,
            
    DANES       =   35,
    DINKA       =   36,
    DUTCH       =   37,
            
    ENGLISH     =   38,
    ESTONIANS   =   39,
    FAROESE     =   40,
    FINNS       =   41,
    FRENCH      =   42,
    FRISIANS    =   43,
    FULA        =   44,
            
    GA_ADANGBE  =   45,
    GAGAUZ      =   46,
    GALICIAN    =   47,
    GANDA       =   48,
    GERMANS     =   49,
    GREEKS      =   50,
    GEORGIANS   =   51,
    GUJARATI    =   52,
            
    HAN_CHINESE =   53,
    HAUSA       =   54,
    HINDUSTANI  =   55,
    HUI         =   56,
    HUNGARIANS  =   57,
            
    IBIBIO      =   58,
    ICELANDERS  =   59,
    IGBO        =   60,
    IJAW        =   61,
    IRISH       =   62,
    ITALIANS    =   63,
            
    JAPANESE    =   64,
    JAVANESE    =   65,
    JEWS        =   66,
            
    KANNADA     =   67,
    KAZAKHS     =   68,
    KIKUYU      =   69,
    KONGO       =   70,
    KONKANI     =   71,
    KOREANS     =   72,
    KURDS       =   73,
    KYRGYZ      =   74,
            
    LANGO       =   75,
    LATVIANS    =   76,
    LITHUANIANS =   77,
    LAZ         =   78,
    LUBA        =   79,
    LUO         =   80,
            
    MACEDONIANS =   81,
    MALAYS      =   82,
    MALAYALI    =   83,
    MALTESE     =   84,
    MANCHU      =   85,
    MANDINKA    =   86,
    MARATHI     =   87,
    MOLDOVANS   =   88,
    MONGO       =   89,
    MONGOLS     =   90,
            
    NEPALI      =   91,
    NORWEGIANS  =   92,
    NUER        =   93,
            
    ODIA        =   94,
    OROMO       =   95,
            
    PASHTUNS    =   96,
    PERSIANS    =   97,
    POLES       =   98,
    PORTUGUESE  =   99,
    PUNJABI     =   100,
    PEDI        =   101,
            
    ROMANIANS   =   102,
    ROMANI      =   103,
    RUSSIANS    =   104,
            
    SARA        =   105,
    SARDINIANS  =   106,
    SCOTTISH    =   107,
    SERBS       =   108,
    SHONA       =   109,
    SICILIANS   =   110,
    SINDHIS     =   111,
    SINHALESE   =   112,
    SLOVAKS     =   113,
    SLOVENES    =   114,
    SOGA        =   115,
    SOMALIS     =   116,
    SONGHAI     =   117,
    SONINKE     =   118,
    SOTHO       =   119,
    SPANIARDS   =   120,
    SUNDANESE   =   121,
    SUKUMA      =   122,
    SWAZI       =   123,
    SWEDES      =   124,
            
    TAJIKS      =   125,
    TAMILS      =   126,
    TELUGU      =   127,
    TAIS        =   128,
    TIBETANS    =   129,
    TSWANA      =   130,
    TUAREGS     =   131,
    TURKS       =   132,
    TURKMENS    =   133,
            
    UKRAINIANS  =   134,
    UYGHUR      =   135,
    UZBEK       =   136,
            
    VIETNAMESE  =   137,
    VOLGA_TATARS=   138,
            
    WELSH       =   139,
            
    XHOSA       =   140,
            
    YAKUTS      =   141,
    YORUBA      =   142,
            
    ZHUANG      =   143,
    ZULU        =   144 
};


#pragma region Documentation
/// <summary>This class is responsible for retaining subject-related states and functions. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
class Subject
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Subject()
        : subject_uid("0.0.0.0"),
          subject_type(UNK_SUBJ_TYPE),
          gender(UNK_GENDER), 
          pediatric(false),       
          subject_age(0),
          date_of_birth(),
          ethnicity(NIL),
          race(UNK_RACE),
          height(0.0f),
          weight(0.0f),
          subject_bmi(0.0f)          
    {}
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Subject(const Subject &rhs)
        : subject_uid(rhs.subject_uid),
          subject_type(rhs.subject_type),
          pediatric(rhs.pediatric),
          gender(rhs.gender),
          subject_age(rhs.subject_age),
          date_of_birth(rhs.date_of_birth),
          race(rhs.race),
          ethnicity(rhs.ethnicity),
          height(rhs.height),
          weight(rhs.weight),
          subject_bmi(rhs.subject_bmi)
    {}

    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Subject()
    {}
    
    
    // Manipulators (used to set component values)

    #pragma region Documentation
    /// <summary>Set the subject's unique identifier (UID). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="uid">The unique identifier. </parm>
    #pragma endregion
    void setUid(std::string uid)          { subject_uid = uid; }
    
    
    #pragma region Documentation
    /// <summary>Set the subject's type. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="type">The subject's type. </parm>
    #pragma endregion
    void setType(SUBJECT_TYPE type)       { subject_type = type; }
    
    
    #pragma region Documentation
    /// <summary>Set whether the subject is pediatric. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ped">A boolean indicating whether the subject is pediatric. </parm>
    #pragma endregion
    void setPediatric(bool ped)           { pediatric = ped; }
    
    
    #pragma region Documentation
    /// <summary>Set the subject's gender. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="gen">The subject's gender. </parm>
    #pragma endregion
    void setGender(GENDER gen)            { gender = gen; }
    
    
    #pragma region Documentation
    /// <summary>Set the subject's age. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="age">The subject's age. </parm>
    #pragma endregion
    void setAge(unsigned int age)         { subject_age = age; }
    
    
    #pragma region Documentation
    /// <summary>Set the subject's date of birth. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="dob">The subject's date of birth. </parm>
    #pragma endregion
    void setDateOfBirth(Date dob)         { date_of_birth = dob; }
    
    
    #pragma region Documentation
    /// <summary>Set the subject's race. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="sr">The subject's race. </parm>
    #pragma endregion
    void setRace(RACE sr)                  { race = sr; }
    
    
    #pragma region Documentation
    /// <summary>Set the subject's ethnicity. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="eth">The subject's ethnicity. </parm>
    #pragma endregion
    void setEthnicity(ETHNICITY eth)      { ethnicity = eth; }
    
    
    #pragma region Documentation
    /// <summary>Set the subject's height (in meters). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="hm">The subject's height (in meters). </parm>
    #pragma endregion
    void setHeight(float hm)              { height = hm; }     // meters
    
    
    #pragma region Documentation
    /// <summary>Set the subject's weight (in kilograms). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="wk">The subject's weight (in kilograms). </parm>
    #pragma endregion
    void setWeight(float wk)              { weight = wk; }     // kilograms
    
    
    #pragma region Documentation
    /// <summary>Set the subject's BMI (Body Mass Index). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="bmi">The subject's BMI (Body Mass Index). </parm>
    #pragma endregion
    void setBmi(float bmi)                { subject_bmi = bmi; }

    
    // Accessors (used to retrieve component values)

    
    #pragma region Documentation
    /// <summary>Retrieve the subject's UID (Unique IDentifier). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's UID (Unique IDentifier). </return>
    #pragma endregion
    std::string  getUid()          { return subject_uid; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the subject's type. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's type. </return>
    #pragma endregion
    SUBJECT_TYPE getType()         { return subject_type; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve whether the subject is pediatric. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>True if the subject is pediatric (otherwise false). </return>
    #pragma endregion
    bool getPediatric()            { return pediatric; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the subject'sgender. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's gender. </return>
    #pragma endregion
    GENDER getGender()             { return gender; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the subject's age. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's age. </return>
    #pragma endregion
    unsigned int getAge()          { return subject_age; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the subject's date of birth. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's date of birth. </return>
    #pragma endregion
    Date getDateOfBirth()          { return date_of_birth; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the subject's race. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's race. </return>
    #pragma endregion
    RACE getRace()                 { return race; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the subject's ethnicity. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's ethnicity. </return>
    #pragma endregion
    ETHNICITY getEthnicity()       { return ethnicity; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the subject's height (in meters). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's height (in meters). </return>
    #pragma endregion
    float getHeight()              { return height; }     // meters
    
    
    #pragma region Documentation
    /// <summary>Retrieve the subject's weight (in kilograms). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's weight (in kilograms). </return>
    #pragma endregion
    float getWeight()              { return weight; }     // kilograms
    
    
    #pragma region Documentation
    /// <summary>Retrieve the subject's BMI  (Body Mass Index). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The subject's  BMI  (Body Mass Index). </return>
    #pragma endregion
    float getBmi()                 { return subject_bmi; }


    #pragma region Documentation
    /// <summary>Read the XML section pertaining to subject. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->getNodeValueXML("Subject_UID", subject_uid);

            std::string type_string;
            if (clib->getNodeValueXML("Subject_Type", type_string))
            {    
                subject_type = interpretType(type_string);
            }

            std::string gender_string;
            if (clib->getNodeValueXML("Gender", gender_string))
            {
                gender = interpretGender(gender_string);
            }

            clib->getNodeValueXML("Pediatric", pediatric);
            clib->getNodeValueXML("Subject_Age", subject_age);

            // Attempt to read date
            clib->findNodeXML("Date_Of_Birth");
            date_of_birth.readXML(clib);
            clib->concludeSectionXML();

            std::string race_string;
            if (clib->getNodeValueXML("Race", race_string))
            {
                race = interpretRace(race_string);
            }
            
            std::string ethnicity_string;
            if (clib->getNodeValueXML("Ethnicity", ethnicity_string))
            {
                ethnicity = interpretEthnicity(ethnicity_string);
            } else std::cerr << "\n* COULDN'T READ \"Ethnicity\"" << std::endl;

            clib->getNodeValueXML("Height", height);
            clib->getNodeValueXML("Weight", weight);
            clib->getNodeValueXML("BMI", subject_bmi);
        }
    }


    #pragma region Documentation
    /// <summary>Write this subject's XML section. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->writeNodeValueXML("Subject_UID", subject_uid);

            std::string type_string = interpretType(subject_type);
            clib->writeNodeValueXML("Subject_Type", type_string);

            std::string gender_string = interpretGender(gender);
            clib->writeNodeValueXML("Gender", gender_string);

            clib->writeNodeValueXML("Pediatric", pediatric);
            clib->writeNodeValueXML("Subject_Age", subject_age);

            clib->addASectionXML("Date_Of_Birth");
            date_of_birth.writeXML(clib);
            clib->concludeSectionXML();

            std::string race_string = interpretRace(race);
            clib->writeNodeValueXML("Race", race_string);

            std::string ethnicity_string = interpretEthnicity(ethnicity);
            clib->writeNodeValueXML("Ethnicity", ethnicity_string);                         

            clib->writeNodeValueXML("Height", height);
            clib->writeNodeValueXML("Weight", weight);
            clib->writeNodeValueXML("BMI", subject_bmi);
        }
    }

    
    #pragma region Documentation
    /// <summary>Convert a subject type enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="type_string">Subject type enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    SUBJECT_TYPE interpretType(std::string type_string)
    {
        std::transform(type_string.begin(), type_string.end(), type_string.begin(), toupper); // convert to upper case

        SUBJECT_TYPE retval(UNK_SUBJ_TYPE);
        if      (type_string == "ANTHROPOMORPHIC") retval = ANTHROPOMORPHIC;
        else if (type_string == "PHYSICS")         retval = PHYSICS;
        return retval;
    }


    #pragma region Documentation
    /// <summary>Convert a subject type enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="subj_type">Subject type enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretType(SUBJECT_TYPE subj_type)
    {
        std::string retval("UNK_SUBJ_TYPE");
        if      (subj_type == ANTHROPOMORPHIC) retval = "ANTHROPOMORPHIC";
        else if (subj_type == PHYSICS)         retval = "PHYSICS";
        return retval;
    }


    #pragma region Documentation
    /// <summary>Convert a subject gender string to an gender enumeration instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="gen_string">Subject gender string. </parm>
    /// <return>The corresponding gender enumeration instance. </return>
    #pragma endregion  
    GENDER interpretGender(std::string gen_string)
    {
        std::transform(gen_string.begin(), gen_string.end(), gen_string.begin(), toupper); // convert to upper case

        GENDER retval(UNK_GENDER);
        if      (gen_string == "FEMALE")        retval = FEMALE;
        else if (gen_string == "MALE")          retval = MALE;
        else if (gen_string == "HERMAPHRODITE") retval = HERMAPHRODITE;
        else if (gen_string == "NONE")          retval = NONE;
        return retval;
    }


    #pragma region Documentation
    /// <summary>Convert a subject gender enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="gen">Subject gender enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretGender(GENDER gen)
    {
        std::string retval("UNK_GENDER");
        if      (gen == FEMALE)        retval = "FEMALE";
        else if (gen == MALE)          retval = "MALE";
        else if (gen == HERMAPHRODITE) retval = "HERMAPHRODITE";
        else if (gen == NONE)          retval = "NONE";
        return retval;
    }


    #pragma region Documentation
    /// <summary>Convert a subject race string to an ethnicity enumeration instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="race_string">Subject race string. </parm>
    /// <return>The corresponding race enumeration instance. </return>
    #pragma endregion  
    RACE interpretRace(std::string race_string)
    {
        std::transform(race_string.begin(), race_string.end(), race_string.begin(), toupper); // convert to upper case

        RACE retval(UNK_RACE);
        if      (race_string == "CAUCASIAN") retval = CAUCASIAN;
        else if (race_string == "MONGOLOID") retval = MONGOLOID;
        else if (race_string == "NEGROID")   retval = NEGROID;
        return retval;
    }


    #pragma region Documentation
    /// <summary>Convert a subject type reace instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="race">Subject type race instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretRace(RACE race)
    {
        std::string retval("UNK_RACE");
        if      (race == CAUCASIAN)  retval = "CAUCASIAN";
        else if (race == MONGOLOID)  retval = "MONGOLOID";
        else if (race == NEGROID)    retval = "NEGROID";
        return retval;
    }


    #pragma region Documentation
    /// <summary>Convert a subject ethnicity string to an ethnicity enumeration instance. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ethnicity_string">Subject ethnicity string. </parm>
    /// <return>The corresponding ethnicity enumeration instance. </return>
    #pragma endregion  
    ETHNICITY interpretEthnicity(std::string ethnicity_string)
    {
        std::transform(ethnicity_string.begin(), ethnicity_string.end(), ethnicity_string.begin(), toupper); // convert to upper case

        ETHNICITY retval(UNK_ETHNICITY);

        if      (ethnicity_string == "NULL")           retval = NIL;  // NULL is a reserved word in C++ 
        else if (ethnicity_string == "ACHOLI")         retval = ACHOLI;
        else if (ethnicity_string == "AKAN")           retval = AKAN;
        else if (ethnicity_string == "ALBANIANS")      retval = ALBANIANS;
        else if (ethnicity_string == "AFAR")           retval = AFAR;
        else if (ethnicity_string == "AFRIKANERS")     retval = AFRIKANERS;
        else if (ethnicity_string == "AMHARA")         retval = AMHARA;
        else if (ethnicity_string == "ARABS")          retval = ARABS;
        else if (ethnicity_string == "ARMENIANS")      retval = ARMENIANS;
        else if (ethnicity_string == "ASSAMESE")       retval = ASSAMESE;
        else if (ethnicity_string == "ASSYRIANS")      retval = ASSYRIANS;
        else if (ethnicity_string == "AZERBAIJANIS")   retval = AZERBAIJANIS;
                
        else if (ethnicity_string == "BALOCHIS")       retval = BALOCHIS;
        else if (ethnicity_string == "BAMARS")         retval = BAMARS;
        else if (ethnicity_string == "BAMBARA")        retval = BAMBARA;
        else if (ethnicity_string == "BASHKIRS")       retval = BASHKIRS;
        else if (ethnicity_string == "BASQUES")        retval = BASQUES;
        else if (ethnicity_string == "BELARUSIANS")    retval = BELARUSIANS;
        else if (ethnicity_string == "BEMBA")          retval = BEMBA;
        else if (ethnicity_string == "BENGALI")        retval = BENGALI;
        else if (ethnicity_string == "BERBERS")        retval = BERBERS;
        else if (ethnicity_string == "BETI_PAHUIN")    retval = BETI_PAHUIN;
        else if (ethnicity_string == "BOSNIAKS")       retval = BOSNIAKS;
        else if (ethnicity_string == "BRAHUI")         retval = BRAHUI;
        else if (ethnicity_string == "BRITISH")        retval = BRITISH;
        else if (ethnicity_string == "BULGARIANS")     retval = BULGARIANS;
                
        // Extended if implemented in pieces due to compiler limitation
        if (ethnicity == UNK_ETHNICITY)
        {
            if      (ethnicity_string == "CATALANS")       retval = CATALANS;
            else if (ethnicity_string == "CHUVASH")        retval = CHUVASH;
            else if (ethnicity_string == "CIRCASSIANS")    retval = CIRCASSIANS;
            else if (ethnicity_string == "CHEWA")          retval = CHEWA;
            else if (ethnicity_string == "CORNISH")        retval = CORNISH;
            else if (ethnicity_string == "CORNWALL")       retval = CORNWALL;
            else if (ethnicity_string == "CROATS")         retval = CROATS;
            else if (ethnicity_string == "CZECHS")         retval = CZECHS;

            else if (ethnicity_string == "DANES")          retval = DANES;
            else if (ethnicity_string == "DINKA")          retval = DINKA;
            else if (ethnicity_string == "DUTCH")          retval = DUTCH;

            else if (ethnicity_string == "ENGLISH")        retval = ENGLISH;
            else if (ethnicity_string == "ESTONIANS")      retval = ESTONIANS;

            else if (ethnicity_string == "FAROESE")        retval = FAROESE;
            else if (ethnicity_string == "FINNS")          retval = FINNS;
            else if (ethnicity_string == "FRENCH")         retval = FRENCH;
            else if (ethnicity_string == "FRISIANS")       retval = FRISIANS;
            else if (ethnicity_string == "FULA")           retval = FULA;

            else if (ethnicity_string == "GA_ADANGBE")     retval = GA_ADANGBE;
            else if (ethnicity_string == "GAGAUZ")         retval = GAGAUZ;
            else if (ethnicity_string == "GALICIAN")       retval = GALICIAN;
            else if (ethnicity_string == "GANDA")          retval = GANDA;
            else if (ethnicity_string == "GERMANS")        retval = GERMANS;
            else if (ethnicity_string == "GREEKS")         retval = GREEKS;
            else if (ethnicity_string == "GEORGIANS")      retval = GEORGIANS;
            else if (ethnicity_string == "GUJARATI")       retval = GUJARATI;

            else if (ethnicity_string == "HAN_CHINESE")    retval = HAN_CHINESE;
            else if (ethnicity_string == "HAUSA")          retval = HAUSA;
            else if (ethnicity_string == "HINDUSTANI")     retval = HINDUSTANI;
            else if (ethnicity_string == "HUI")            retval = HUI;
            else if (ethnicity_string == "HUNGARIANS")     retval = HUNGARIANS;

            else if (ethnicity_string == "IBIBIO")         retval = IBIBIO;
            else if (ethnicity_string == "ICELANDERS")     retval = ICELANDERS;
            else if (ethnicity_string == "IGBO")           retval = IGBO;
            else if (ethnicity_string == "IJAW")           retval = IJAW;
            else if (ethnicity_string == "IRISH")          retval = IRISH;
            else if (ethnicity_string == "ITALIANS")       retval = ITALIANS;

            else if (ethnicity_string == "JAPANESE")       retval = JAPANESE;
            else if (ethnicity_string == "JAVANESE")       retval = JAVANESE;
            else if (ethnicity_string == "JEWS")           retval = JEWS;

            else if (ethnicity_string == "KANNADA")        retval = KANNADA;
            else if (ethnicity_string == "KAZAKHS")        retval = KAZAKHS;
            else if (ethnicity_string == "KIKUYU")         retval = KIKUYU;
            else if (ethnicity_string == "KONGO")          retval = KONGO;
            else if (ethnicity_string == "KONKANI")        retval = KONKANI;
            else if (ethnicity_string == "KOREANS")        retval = KOREANS;
            else if (ethnicity_string == "KURDS")          retval = KURDS;
            else if (ethnicity_string == "KYRGYZ")         retval = KYRGYZ;

            else if (ethnicity_string == "LANGO")          retval = LANGO;
            else if (ethnicity_string == "LATVIANS")       retval = LATVIANS;                
            else if (ethnicity_string == "LITHUANIANS")    retval = LITHUANIANS;
            else if (ethnicity_string == "LAZ")            retval = LAZ;
            else if (ethnicity_string == "LUBA")           retval = LUBA;
            else if (ethnicity_string == "LUO")            retval = LUO;

            else if (ethnicity_string == "MACEDONIANS")    retval = MACEDONIANS;
            else if (ethnicity_string == "MALAYS")         retval = MALAYS;
            else if (ethnicity_string == "MALAYALI")       retval = MALAYALI;
            else if (ethnicity_string == "MALTESE")        retval = MALTESE;
            else if (ethnicity_string == "MANCHU")         retval = MANCHU;
            else if (ethnicity_string == "MANDINKA")       retval = MANDINKA;
            else if (ethnicity_string == "MARATHI")        retval = MARATHI;
            else if (ethnicity_string == "MOLDOVANS")      retval = MOLDOVANS;                
            else if (ethnicity_string == "MONGO")          retval = MONGO;
            else if (ethnicity_string == "MONGOLS")        retval = MONGOLS;

            else if (ethnicity_string == "NEPALI")         retval = NEPALI;
            else if (ethnicity_string == "NORWEGIANS")     retval = NORWEGIANS;
            else if (ethnicity_string == "NUER")           retval = NUER;

            else if (ethnicity_string == "ODIA")           retval = ODIA;
            else if (ethnicity_string == "OROMO")          retval = OROMO;

            else if (ethnicity_string == "PASHTUNS")       retval = PASHTUNS;
            else if (ethnicity_string == "PERSIANS")       retval = PERSIANS;
            else if (ethnicity_string == "POLES")          retval = POLES;
            else if (ethnicity_string == "PORTUGUESE")     retval = PORTUGUESE;
            else if (ethnicity_string == "PUNJABI")        retval = PUNJABI;                
            else if (ethnicity_string == "PEDI")           retval = PEDI;

            else if (ethnicity_string == "ROMANIANS")      retval = ROMANIANS;
            else if (ethnicity_string == "ROMANI")         retval = ROMANI;
            else if (ethnicity_string == "RUSSIANS")       retval = RUSSIANS;
        }

        if (ethnicity == UNK_ETHNICITY)
        {
            if      (ethnicity_string == "SARA")           retval = SARA;
            else if (ethnicity_string == "SARDINIANS")     retval = SARDINIANS;
            else if (ethnicity_string == "SCOTTISH")       retval = SCOTTISH;
            else if (ethnicity_string == "SERBS")          retval = SERBS;
            else if (ethnicity_string == "SHONA")          retval = SHONA;
            else if (ethnicity_string == "SICILIANS")      retval = SICILIANS;
            else if (ethnicity_string == "SINDHIS")        retval = SINDHIS;
            else if (ethnicity_string == "SINHALESE")      retval = SINHALESE;                
            else if (ethnicity_string == "SLOVAKS")        retval = SLOVAKS;
            else if (ethnicity_string == "SLOVENES")       retval = SLOVENES;
            else if (ethnicity_string == "SOGA")           retval = SOGA;
            else if (ethnicity_string == "SOMALIS")        retval = SOMALIS;
            else if (ethnicity_string == "SONGHAI")        retval = SONGHAI;
            else if (ethnicity_string == "SONINKE")        retval = SONINKE;
            else if (ethnicity_string == "SOTHO")          retval = SOTHO;
            else if (ethnicity_string == "SPANIARDS")      retval = SPANIARDS;
            else if (ethnicity_string == "SUNDANESE")      retval = SUNDANESE;
            else if (ethnicity_string == "SUKUMA")         retval = SUKUMA;
            else if (ethnicity_string == "SWAZI")          retval = SWAZI;
            else if (ethnicity_string == "SWEDES")         retval = SWEDES;

            else if (ethnicity_string == "TAJIKS")         retval = TAJIKS;
            else if (ethnicity_string == "TAMILS")         retval = TAMILS;
            else if (ethnicity_string == "TELUGU")         retval = TELUGU;
            else if (ethnicity_string == "TAIS")           retval = TAIS;
            else if (ethnicity_string == "TIBETANS")       retval = TIBETANS;
            else if (ethnicity_string == "TSWANA")         retval = TSWANA;
            else if (ethnicity_string == "TUAREGS")        retval = TUAREGS;
            else if (ethnicity_string == "TURKS")          retval = TURKS;
            else if (ethnicity_string == "TURKMENS")       retval = TURKMENS;

            else if (ethnicity_string == "UKRAINIANS")     retval = UKRAINIANS;
            else if (ethnicity_string == "UYGHUR")         retval = UYGHUR;
            else if (ethnicity_string == "UZBEK")          retval = UZBEK; 

            else if (ethnicity_string == "VIETNAMESE")     retval = VIETNAMESE;
            else if (ethnicity_string == "VOLGA_TATARS")   retval = VOLGA_TATARS;

            else if (ethnicity_string == "WELSH")          retval = WELSH;

            else if (ethnicity_string == "XHOSA")          retval = XHOSA;

            else if (ethnicity_string == "YAKUTS")         retval = YAKUTS;
            else if (ethnicity_string == "YORUBA")         retval = YORUBA;

            else if (ethnicity_string == "ZHUANG")         retval = ZHUANG;
            else if (ethnicity_string == "ZULU")           retval = ZULU;
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Convert a subject ethnicity enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="eth">Subject ethnicity enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretEthnicity(ETHNICITY eth)
    {
        std::string retval("UNK_ETHNIC");

        if      (eth == NIL)            retval = "NULL";  // NULL is a reserved word in C++
        else if (eth == ACHOLI)         retval = "ACHOLI";
        else if (eth == AKAN)           retval = "AKAN";
        else if (eth == ALBANIANS)      retval = "ALBANIANS";
        else if (eth == AFAR)           retval = "AFAR";
        else if (eth == AFRIKANERS)     retval = "AFRIKANERS";
        else if (eth == AMHARA)         retval = "AMHARA";
        else if (eth == ARABS)          retval = "ARABS";
        else if (eth == ARMENIANS)      retval = "ARMENIANS";
        else if (eth == ASSAMESE)       retval = "ASSAMESE";
        else if (eth == ASSYRIANS)      retval = "ASSYRIANS";
        else if (eth == AZERBAIJANIS)   retval = "AZERBAIJANIS";

        else if (eth == BALOCHIS)       retval = "BALOCHIS";
        else if (eth == BAMARS)         retval = "BAMARS";
        else if (eth == BAMBARA)        retval = "BAMBARA";
        else if (eth == BASHKIRS)       retval = "BASHKIRS";
        else if (eth == BASQUES)        retval = "BASQUES";
        else if (eth == BELARUSIANS)    retval = "BELARUSIANS";
        else if (eth == BEMBA)          retval = "BEMBA";
        else if (eth == BENGALI)        retval = "BENGALI";
        else if (eth == BERBERS)        retval = "BERBERS";
        else if (eth == BETI_PAHUIN)    retval = "BETI_PAHUIN";
        else if (eth == BOSNIAKS)       retval = "BOSNIAKS";
        else if (eth == BRAHUI)         retval = "BRAHUI";
        else if (eth == BRITISH)        retval = "BRITISH";
        else if (eth == BULGARIANS)     retval = "BULGARIANS";

        // Extended if implemented in pieces due to compiler limitation
        if (ethnicity != UNK_ETHNICITY)
        {
            if      (eth == CATALANS)       retval = "CATALANS";
            else if (eth == CHUVASH)        retval = "CHUVASH";
            else if (eth == CIRCASSIANS)    retval = "CIRCASSIANS";
            else if (eth == CHEWA)          retval = "CHEWA";
            else if (eth == CORNISH)        retval = "CORNISH";
            else if (eth == CORNWALL)       retval = "CORNWALL";
            else if (eth == CROATS)         retval = "CROATS";
            else if (eth == CZECHS)         retval = "CZECHS";

            else if (eth == DANES)          retval = "DANES";
            else if (eth == DINKA)          retval = "DINKA";
            else if (eth == DUTCH)          retval = "DUTCH";

            else if (eth == ENGLISH)        retval = "ENGLISH";
            else if (eth == ESTONIANS)      retval = "ESTONIANS";

            else if (eth == FAROESE)        retval = "FAROESE";
            else if (eth == FINNS)          retval = "FINNS";
            else if (eth == FRENCH)         retval = "FRENCH";
            else if (eth == FRISIANS)       retval = "FRISIANS";
            else if (eth == FULA)           retval = "FULA";

            else if (eth == GA_ADANGBE)     retval = "GA_ADANGBE";
            else if (eth == GAGAUZ)         retval = "GAGAUZ";
            else if (eth == GALICIAN)       retval = "GALICIAN";
            else if (eth == GANDA)          retval = "GANDA";
            else if (eth == GERMANS)        retval = "GERMANS";
            else if (eth == GREEKS)         retval = "GREEKS";
            else if (eth == GEORGIANS)      retval = "GEORGIANS";
            else if (eth == GUJARATI)       retval = "GUJARATI";

            else if (eth == HAN_CHINESE)    retval = "HAN_CHINESE";
            else if (eth == HAUSA)          retval = "HAUSA";
            else if (eth == HINDUSTANI)     retval = "HINDUSTANI";
            else if (eth == HUI)            retval = "HUI";
            else if (eth == HUNGARIANS)     retval = "HUNGARIANS";

            else if (eth == IBIBIO)         retval = "IBIBIO";
            else if (eth == ICELANDERS)     retval = "ICELANDERS";
            else if (eth == IGBO)           retval = "IGBO";
            else if (eth == IJAW)           retval = "IJAW";
            else if (eth == IRISH)          retval = "IRISH";
            else if (eth == ITALIANS)       retval = "ITALIANS";

            else if (eth == JAPANESE)       retval = "JAPANESE";
            else if (eth == JAVANESE)       retval = "JAVANESE";
            else if (eth == JEWS)           retval = "JEWS";

            else if (eth == KANNADA)        retval = "KANNADA";
            else if (eth == KAZAKHS)        retval = "KAZAKHS";
            else if (eth == KIKUYU)         retval = "KIKUYU";
            else if (eth == KONGO)          retval = "KONGO";
            else if (eth == KONKANI)        retval = "KONKANI";
            else if (eth == KOREANS)        retval = "KOREANS";
            else if (eth == KURDS)          retval = "KURDS";
            else if (eth == KYRGYZ)         retval = "KYRGYZ";

            else if (eth == LANGO)          retval = "LANGO";
            else if (eth == LATVIANS)       retval = "LATVIANS";
            else if (eth == LITHUANIANS)    retval = "LITHUANIANS";
            else if (eth == LAZ)            retval = "LAZ";
            else if (eth == LUBA)           retval = "LUBA";
            else if (eth == LUO)            retval = "LUO";

            else if (eth == MACEDONIANS)    retval = "MACEDONIANS";
            else if (eth == MALAYS)         retval = "MALAYS";
            else if (eth == MALAYALI)       retval = "MALAYALI";
            else if (eth == MALTESE)        retval = "MALTESE";
            else if (eth == MANCHU)         retval = "MANCHU";
            else if (eth == MANDINKA)       retval = "MANDINKA";
            else if (eth == MARATHI)        retval = "MARATHI";
            else if (eth == MOLDOVANS)      retval = "MOLDOVANS";
            else if (eth == MONGO)          retval = "MONGO";
            else if (eth == MONGOLS)        retval = "MONGOLS";

            else if (eth == NEPALI)         retval = "NEPALI";
            else if (eth == NORWEGIANS)     retval = "NORWEGIANS";
            else if (eth == NUER)           retval = "NUER";

            else if (eth == ODIA)           retval = "ODIA";
            else if (eth == OROMO)          retval = "OROMO";

            else if (eth == PASHTUNS)       retval = "PASHTUNS";
            else if (eth == PERSIANS)       retval = "PERSIANS";
            else if (eth == POLES)          retval = "POLES";
            else if (eth == PORTUGUESE)     retval = "PORTUGUESE";
            else if (eth == PUNJABI)        retval = "PUNJABI";
            else if (eth == PEDI)           retval = "PEDI";

            else if (eth == ROMANIANS)      retval = "ROMANIANS";
            else if (eth == ROMANI)         retval = "ROMANI";
            else if (eth == RUSSIANS)       retval = "RUSSIANS";
        }

        if (eth != UNK_ETHNICITY)
        {
            if      (eth == SARA)           retval = "SARA";
            else if (eth == SARDINIANS)     retval = "SARDINIANS";
            else if (eth == SCOTTISH)       retval = "SCOTTISH";
            else if (eth == SERBS)          retval = "SERBS";
            else if (eth == SHONA)          retval = "SHONA";
            else if (eth == SICILIANS)      retval = "SICILIANS";
            else if (eth == SINDHIS)        retval = "SINDHIS";
            else if (eth == SINHALESE)      retval = "SINHALESE";
            else if (eth == SLOVAKS)        retval = "SLOVAKS";
            else if (eth == SLOVENES)       retval = "SLOVENES";
            else if (eth == SOGA)           retval = "SOGA";
            else if (eth == SOMALIS)        retval = "SOMALIS";
            else if (eth == SONGHAI)        retval = "SONGHAI";
            else if (eth == SONINKE)        retval = "SONINKE";
            else if (eth == SOTHO)          retval = "SOTHO";
            else if (eth == SPANIARDS)      retval = "SPANIARDS";
            else if (eth == SUNDANESE)      retval = "SUNDANESE";
            else if (eth == SUKUMA)         retval = "SUKUMA";
            else if (eth == SWAZI)          retval = "SWAZI";
            else if (eth == SWEDES)         retval = "SWEDES";

            else if (eth == TAJIKS)         retval = "TAJIKS";
            else if (eth == TAMILS)         retval = "TAMILS";
            else if (eth == TELUGU)         retval = "TELUGU";
            else if (eth == TAIS)           retval = "TAIS";
            else if (eth == TIBETANS)       retval = "TIBETANS";
            else if (eth == TSWANA)         retval = "TSWANA";
            else if (eth == TUAREGS)        retval = "TUAREGS";
            else if (eth == TURKS)          retval = "TURKS";
            else if (eth == TURKMENS)       retval = "TURKMENS";

            else if (eth == UKRAINIANS)     retval = "UKRAINIANS";
            else if (eth == UYGHUR)         retval = "UYGHUR";
            else if (eth == UZBEK)          retval = "UZBEK";

            else if (eth == VIETNAMESE)     retval = "VIETNAMESE";
            else if (eth == VOLGA_TATARS)   retval = "VOLGA_TATARS";

            else if (eth == WELSH)          retval = "WELSH";

            else if (eth == XHOSA)          retval = "XHOSA";

            else if (eth == YAKUTS)         retval = "YAKUTS";
            else if (eth == YORUBA)         retval = "YORUBA";

            else if (eth == ZHUANG)         retval = "ZHUANG";
            else if (eth == ZULU)           retval = "ZULU";
        }
        return retval;
    }

private:

    std::string   subject_uid;   /// subject UID
    SUBJECT_TYPE  subject_type;  /// subject type
    bool          pediatric;     /// whether the subject is pediatric
    GENDER        gender;        /// subject gender
    unsigned int  subject_age;   /// subject's age
    Date          date_of_birth; /// subject's date of birth

    RACE          race;          /// subject's race
    ETHNICITY     ethnicity;     /// subject's ethnicity

    float         height;        /// subject's height in meters
    float         weight;        /// subject's weight in kilograms
    float         subject_bmi;   /// subject's BMI (Body Mass Index)
};

}