// SavePhantom.cpp

#include "SavePhantom.h"

#include "XPL_BreastPhantomGenerator.h"
#include "XPL_AttenuationTable.h"
#include "VCT_PrivateData.h"
#include "VCT_CommonLibrary.h"
#include "VCT_Time.h"
#include "VCT_Date.h"
#include "VCT_Organization.h"
#include "VCT_VoxelArray.h"
#include "VCT_IndexTable.h"
#include "VCT_Subject.h"
#include "VCT_Trial.h"
#include "VCT_Phantom.h"
#include "VCT_Vctx.h"

#include "pugixml.hpp"

#include <string>


// Define constant strings
#if defined(_MSC_VER)
std::string SavePhantom::temp_dir("./");
#elif defined(__linux__)
std::string SavePhantom::temp_dir("/tmp/");
#elif defined(__APPLE__)
std::string SavePhantom::temp_dir("/tmp/");
#else
std::string SavePhantom::temp_dir("./");
#endif
std::string SavePhantom::extension(".vctx");


// Constructor
SavePhantom::SavePhantom()
    : verbose(false),
      input_from_xml(false),
      leave_in_temp_folder(false),
      subj_uid("unknown_uid"),
      subj_type("Anthropomorphic"),  // Default type for this program is ANTHROPOMORPHIC
      subj_gen("Female"),            // Default gender for this program is FEMALE
      subj_ped("false"),             // Default pediatric status for this program is FALSE
      subj_age(47),                  // Default age for this program is 47
      subj_dob("1970-12-31"),        // Default Date of Birth for this program is 1970-01-02
      subj_race("unknown_race"),     // No default Race for this program
      subj_eth("unknown_ethnicity"), // No default Ethnicity for this program
      subj_wt(0.0f),
      subj_ht(0.0f),
      subj_bmi(0.0f),
      ph_uid("unknown_uid"),
      ph_src("unknown_src"),
      ph_shape("Breast"),
      ph_type("VOXEL_ARRAY"),
      ph_dm("Indexed"),
      ph_bp("Breast"),
      ph_lat("unknown_laterality"),
      ph_date(""),
      ph_time(""),
      org_name("unknown_name"),
      org_div("unknown_division"),
      org_dept("unknown_dept"),
      org_grp("unknown_group"),
      org_code("unknown_code"),
      org_cntry("unknown_country"),
      org_sta("unknown_state"),
      org_city("unknown_city"),
      org_zip("unknown_zip"),
      utc_offset(0),

      voxel_array_uid("unknown_uid"),
      index_table_uid("unknown_uid"),
      xpl_phantom_UID("unknown_uid"),
      program_name("BreastPhantomGenerator"),
      phantom_name("unknown_name"),
      phantom_shape("Breast"),

      phantom_output_path(""),
      phantom_filename("unknown_filename"),
	  max_ligament_thickness(1.0f), // New field as of 2021-01-13
	  min_ligament_thickness(0.2f), // New field as of 2021-01-13
      voxel_size(0.02f),
      a(5.0f),
      b(5.0f),
      c1(12.0f),
      c2(5.0f),
      seed(24),
      perc_dense(5.0f),
      skin_thickness(0.15f),
      ki(0),
      kp(0),
      ks(0),
      num_compartments(333),
      comp_thickness(0.06f),
      min_seed_dist(0.5f),
      min_speed(0.5f),
      max_speed(2.0f),
      min_ratio(0.5f),
      max_ratio(2.0f),
      beta_p1(2.0f),
      beta_q1(0.5f),
      beta_p2(2.0f),
      beta_q2(3.5f),
      seed_dist_file_in(""),
      seed_dist_file_out("")
{
    // Debug
    if (verbose) dumpParameters("Hard-Coded Default Phantom parameters");
    clib.start_timer();
}

SavePhantom::~SavePhantom()
{}


bool SavePhantom::readConfig()
{
    // Read configuration file(s)
    bool status = clib.loadXML("vct_config.xml");
    if (status)
    {
        status = clib.findNodeXML("vct_config");
        if (status)
        {
            status = clib.findNodeXML("Phantom");
            if (status)
            {
                readPhantom();
            }

            status = clib.findNodeXML("Subject");
            if (status)
            {
                readSubject();
            }

            status = clib.findNodeXML("Organization");
            if (status)
            {
                readOrganization();
            }

            // Program inputs
            status = clib.findNodeXML("Generator_Config");
            if (status)
            {
                readProgramParameters();
            }

            // Read the Index Table from the config file
            if (clib.findNodeXML("Index_Table"))
            {
                i_table.readXML(&clib);
                if (verbose) std::cout << "Index Table Data just read:\n";
                for(int i=0; i<=i_table.getLargestIndex(); i++)
                {
                    vct::Label lab = i_table.getLabel(i);
                    int nummats = lab.getNumMaterials();
                    for(int j=0; j<nummats; ++j)
                    {
                        vct::Material mat = lab.getMaterial(j);
                        if (verbose)
                        {
                            std::cout << "\tLabel " << i << " Material " << j+1 << " name: " << mat.getName()
                                      << "Z is " << mat.getMaterialZ() << std::endl;
                        }
                    }
                }
                if (verbose) std::cout << "\nDone Reading Index Table Data from vct_config.xml\n" << std::endl;

            }
            else
            {
                std::cerr << "\nUh-Oh, couldn't find Index_Table!\n" << std::endl;
            }
        }
    }

    // Set default for phantom date and Times.
    vct::Date date;
    vct::Time time;
    time.setUtcOffset(utc_offset);
    date.setNow();
    time.setNow();
    ph_date = date.toString();
    ph_time = time.toString();

    if (verbose) std::cout << "\nDate and time created: " << ph_date << ", " << ph_time << "\n" << std::endl;

    if (verbose) dumpParameters("After Config File Read Default Phantom parameters");

    return status;
}


bool SavePhantom::readInputXML(std::string xmlfile)
{
    bool retval = false;

    // Attempt to open the file with pugi
    retval = clib.loadXML(xmlfile);
    if (retval)
    {
        input_from_xml = true;
        retval = clib.findNodeXML("Breast_Phantom_Designer");
        if (retval)
        {
            retval = clib.findNodeXML("Subject");
            if (retval)
            {
                readSubject();
            }

            retval = clib.findNodeXML("Phantom");
            if (retval)
            {
                readPhantom();
            }

            /* The organization field has been moved to vct_config.xml
            retval = clib.findNodeXML("Organization");
            if (retval)
            {
                readOrganization();
            }
            else std::cout << __FILE__ << ", " << __FUNCTION__ << ", " << __LINE__ << ", Organization field NOT FOUND!" << std::endl;
            */

            retval = clib.getNodeValueXML("Index_Table_UID", index_table_uid);
            if (retval && verbose)
            {
                std::cout << "\tIndex_Table_UID:\t" << index_table_uid << "\n\n";
            }

            retval = clib.getNodeValueXML("Voxel_Array_UID", voxel_array_uid);
            if (retval && verbose)
            {
                std::cout << "\nVoxel Array UID:\t" << voxel_array_uid << "\n\n";
            }

            // Generator and Voxel Array information will be from the generator program itself

            // Program inputs
            retval = clib.findNodeXML("Generator_Config");
            if (retval)
            {
                readProgramParameters();
                retval = true;
            }

            // Check whether to leave the temporary phantom files in memory when done
            // ..this saves time and effort when the next step is going to be to combine phantoms
            std::string leave_it("false");
            if (!clib.getNodeValueXML("Leave_In_Temp_Folder", leave_it))
            {
                std::cerr << "Couldn't find \"Leave_In_Temp_Folder\" field" << std::endl;
            }
            leave_in_temp_folder = (leave_it == "true" || leave_it == "TRUE");
        }
    }
    else std::cerr << "\n* ERROR: Could not read xml input file \"" << xmlfile << "\" *\n" << std::endl;

    return retval;
}


void SavePhantom::suggestPhantomName(std::string name)
{
    // Accept this name only if it hasn't already been set
    if (phantom_name.size() < 1 || phantom_name == "unknown_name")
    {
        phantom_name = name;
    }

    if (verbose)
    {
        std::cout << "SavePhantom.cpp: " << __FUNCTION__ << ", line " << __LINE__
                  << ": PHANTOM NAME IS \"" << phantom_name << "\"" << std::endl;
    }
}


void SavePhantom::setGenParms(std::string version, std::string id, std::string filename,
                              float vxlSize, float sizeX, float sizeY, float cprim, float csec,
                              int seed, float dsig, float dperc, float skin, int compartments, float compthick,
                              float mindist, float minspd, float mxspd, float minrat, float maxrat,
                              float p1, float q1, float p2, float q2,
                              std::string distFileOut, std::string distFileIn)
{
    // note:

    this->filename = SavePhantom::temp_dir + filename;
    if (this->filename.find(SavePhantom::extension) == std::string::npos)
    {
        this->filename += SavePhantom::extension;
    }

    if (verbose)
    {
        std::cout << "SavePhantom.cpp: " << __FUNCTION__ << ", line " << __LINE__
                  << ": PHANTOM NAME IS \"" << phantom_name << "\"" << std::endl;
    }

    generator.setProgramName(program_name);
    generator.setSWVersion(version);
    generator.setUniqueId(id);
    generator.setPhantomName(phantom_name);
    generator.setPhantomType(phantom_shape);
    generator.setFilename(filename);
    generator.setVoxelSize(vxlSize);
    generator.setXSize(sizeX);
    generator.setYSize(sizeY);
    generator.setCPrimeSize(cprim);
    generator.setCSecSize(csec);

    generator.setRandomSeed(seed);

    generator.setDSigma(dsig);
    generator.setDensePercent(dperc);
    generator.setSkinThickness(skin);
    generator.setNumCompartments(compartments);
    generator.setCompartThickness(compthick);
    generator.setMinDistBetweenSeeds(mindist);
    generator.setMinSpeedOfGrowth(minspd);
    generator.setMaxSpeedOfGrowth(mxspd);
    generator.setMinRatio(minrat);
    generator.setMaxRatio(maxrat);
    generator.setP1(p1);
    generator.setQ1(q1);
    generator.setP2(p2);
    generator.setQ2(q2);
    generator.setDistrFileOut(distFileOut);
    generator.setDistrFileIn(distFileIn);
}


void SavePhantom::save(long long xsize, long long ysize, long long zsize, unsigned char *phantom)
{
    vct::Vctx *vctx = new vct::Vctx;

    // Set whether to leave the phantom in its temporary folder after creating its .vctx file
    vctx->setLeaveTemp(leave_in_temp_folder);
    if (verbose) std::cout << __FUNCTION__ << ": Setting leave_temp flag to " << (leave_in_temp_folder?"True":"False") << std::endl;

    vct::Date date;
    date.setNow();

    vct::Time time;
    time.setUtcOffset(-4);
    time.setNow();

    vct::Trial trial;

    trial.setVctVersion(std::to_string(vct::CURRENT_VCT_VERSION));

    vct::Subject subject;

    subject.setUid(subj_uid);
    subject.setType(subject.interpretType(subj_type));
    subject.setPediatric((subj_ped == "true" ? true : false));
    subject.setGender(subject.interpretGender(subj_gen));
    subject.setAge(subj_age);
    vct::Date subject_date(subj_dob);
    subject.setDateOfBirth(subject_date);
    subject.setRace(subject.interpretRace(subj_race));
    subject.setEthnicity(subject.interpretEthnicity(subj_eth));
    subject.setHeight(subj_ht);
    subject.setWeight(subj_wt);
    subject.setBmi(subj_bmi);

    vct::Software software;

    software.setName("BreastPhantomGenerator");
    software.setVersion("1.0");
    software.setRepository("https://xraylabsvr1.intranet.imagephysics.com/svn/VCTPipelineCode");
    software.setDate(date);
    software.setTime(time);

    // Station
    vct::Station station;
    station.querySystem();

    vct::Organization org;

    org.setName(org_name);            // "University of Pennsylvania");
    org.setDivision(org_div);         // "Perman School of Medicine");
    org.setDepartment(org_dept);      // "Radiology");
    org.setGroup(org_grp);            // "X-ray Physics Lab");
    org.setInstituionCode(org_code);  // "1.2.826.0.1.3680043.2.936");
    org.setCountry(org_cntry);        // "United States of America");
    org.setState(org_sta);            // "Pennsylvania");
    org.setCity(org_city);            // "Philadelphia");
    org.setZip(org_zip);              // "19104");

    vct::Phantom ph;

    if (phantom_name.size() < 1) phantom_name="DEFAULT_NAME";
    if (ph_type.size() < 1) ph_type="VOXEL_ARRAY";

    if (verbose)
    {
        std::cout << "SavePhantom.cpp: " << __FUNCTION__ << ", line " << __LINE__
                  << ": PHANTOM NAME IS \"" << phantom_name << "\"" << std::endl;
    }

    ph.setPhantomName(phantom_name);
    ph.setPhantomShape(phantom_shape);
	ph.setLigamentThicknesses(max_ligament_thickness, min_ligament_thickness);
    ph.setType(ph.interpretType(ph_type));
    ph.setUid(ph_uid);
    ph.setSource(ph.interpretSource(ph_src));        // vct::PENN);       // potential issue: PENN enum vs "University of Pennsylvania"
    ph.setType(ph.interpretType(ph_type));           // vct::VOXEL_ARRAY);// potential issue: VOXEL_ARRAY enum vs "Voxel Array"
    ph.setDataModel(ph.interpretDataModel(ph_dm));   // vct::INDEXED);    // potential issue: INDEXED enum vs "Indexed"
    ph.setBodyPart(ph.interpretBodyPart(ph_bp));     // vct::BREAST);     // potential issue: BREAST enum vs "Breast"
    ph.setLaterality(ph.interpretLaterality(ph_lat));// vct::LEFT);       // potential issue: LEFT enum vs "Left"

    vct::Date phantom_date;
    ph.setDateCreated(phantom_date);
    vct::Time phantom_time;
    phantom_time.setUtcOffset(utc_offset);
    ph.setTimeCreated(phantom_time);

    ph.setFormatVersion(1.0);

    // Add the other objects to the phantom
    ph.setTrial(&trial);
    ph.setSubject(&subject);
    ph.setGenStation(&station);
    ph.setGenOrganization(&org);
    ph.setGenSoftware(&software);

    vct::VoxelArray voxel_array;

    voxel_array.setVoxelArrayUID(voxel_array_uid);
    voxel_array.setVoxelNum(xsize, ysize, zsize);
    voxel_array.setVoxelSize_mm(voxel_size, voxel_size, voxel_size);
    voxel_array.setVoxelOrder("X | Y | Z");
    voxel_array.setVoxelType(vct::V_INT8);
    voxel_array.setBitsPerVoxel(8);
    float dircos[9] = { 1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f }; // no rotation
    voxel_array.setDirectionCosines(dircos);
    voxel_array.setEndian("LITTLE_ENDIAN");

    voxel_array.setVoxels(phantom);
    voxel_array.setTotalSize(static_cast<long long>(xsize * ysize * zsize));

    size_t total_num_voxels = size_t(xsize * ysize * zsize);
    size_t non_air = total_num_voxels;

    // This takes a while, understandably
    size_t glandular_voxels = static_cast<size_t>(voxel_array.countGlandular(i_table, non_air));

    if (verbose)
    {
        std::cout << "There are " << total_num_voxels << " voxels total" << std::endl;
        std::cout << "There are " << non_air << " voxels which are not air, " << std::endl;
        std::cout << "There are " << glandular_voxels << " glandular voxels, " << std::endl;
    }

    ph.setTotalNonAirVoxels(non_air);
    float x_thickness_mm = xsize *  voxel_size;
    float y_thickness_mm = ysize *  voxel_size;
    float z_thickness_mm = zsize *  voxel_size;
    ph.setXThickness_mm(x_thickness_mm);
    ph.setYThickness_mm(y_thickness_mm);
    ph.setZThickness_mm(z_thickness_mm);
    ph.setGlandularCount(glandular_voxels);

    ph.setVoxelArray(&voxel_array);


    // Index Table Stuff
    //                    Name         Weight Density  MaterialZ
    vct::Material air(    "Air",       1.0f,  0.0012f, 201);
    vct::Material glnd1(  "Glandular", 1.0f,  1.04f,   205);
    vct::Material skin(   "Skin",      1.0f,  0.93f,   203);
    vct::Material adipose("Adipose",   1.0f,  0.93f,   204);
    vct::Material glnd2a( "Glandular", 0.25f, 1.04f,   205);
    vct::Material glnd2b( "Glandular", 0.75f, 1.04f,   206);


    vct::Label lab1(0, air);
    vct::Label lab2(1, glnd1);
    vct::Label lab3(2, skin);
    vct::Label lab4(3, adipose);
    vct::Label lab5(4, glnd2a);
    lab5.addMaterial(glnd2b);


    // Write the Index Table
    i_table.setUID(index_table_uid);

    i_table.writeXML(&clib);

    ph.setIndexTable(&i_table);

    // Private Data: Attenuation File
    createSmallAttenFile(*vctx);

    vctx->setPhantom(&ph);

    // Private Data: BreastPhantomGenerator command line arguments
    vctx->addPrivateData(&generator);
    std::string path(phantom_filename);
    std::string::size_type pos = path.find(SavePhantom::extension);
    if (pos != std::string::npos) path = path.substr(0, pos);

    // Strip path preceding path
    pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        path = path.substr(pos+1);
    }

    if (verbose)
    {
        std::cout << __FUNCTION__ << ", line " << __LINE__  << ": path is \"" << path
                  << " AND PHANTOM NAME IS \"" << phantom_filename << "\"" << std::endl;
    }

    vctx->writeVctx(path, phantom_filename);

    // Write out a generation report if input was from an XML file (as opposed to pure command line)
    if (input_from_xml)
    {
        // Now write out a simple, non-compressed xml file for the database to read
        clib.initializeXML("Phantom");

        // Write number of non-air voxels
        clib.writeNodeValueXML("Total_Non_Air_Voxels", non_air);

        // Write thickness section
        clib.addASectionXML("Thickness_mm");
        clib.writeNodeValueXML("X", x_thickness_mm);
        clib.writeNodeValueXML("Y", y_thickness_mm);
        clib.writeNodeValueXML("Z", z_thickness_mm);
        clib.concludeSectionXML();

        // Write glandular count
        clib.writeNodeValueXML("Glandular_Count", glandular_voxels);

        clib.addASectionXML("Date_Created");
        phantom_date.writeXML(&clib);
        clib.concludeSectionXML();

        clib.addASectionXML("Time_Created");
        phantom_time.writeXML(&clib);
        clib.concludeSectionXML();

        clib.addASectionXML("Station");
        station.writeXML(&clib);
        clib.concludeSectionXML();

        clib.writeNodeValueXML("Institution_Code_Sequence", org_code);

        unsigned int diff_ms = clib.get_elapsed_time();
        clib.writeNodeValueXML("Elapsed_Time_msec", diff_ms);


        // Build filename and save file
        std::string filename(phantom_filename);
        std::string::size_type pos = filename.find_last_of("/\\");
        if (pos != std::string::npos) filename = filename.substr(pos+1);
        pos = filename.find(".vctx");
        if (pos != std::string::npos) filename = filename.substr(0, pos);
        filename += "_report.xml";
        clib.saveXML(phantom_output_path + filename);

        if (verbose) std::cout << "deleting Vctx" << std::endl;

        delete vctx;
        vctx = nullptr;
    }
}


// Protected Methods


void SavePhantom::readPhantom()
{
    clib.getNodeValueXML("Phantom_UID",    ph_uid);
    clib.getNodeValueXML("Phantom_Source", ph_src);
    clib.getNodeValueXML("Phantom_Shape",  ph_shape);
    clib.getNodeValueXML("Phantom_Type",   ph_type);
    clib.getNodeValueXML("Data_Model",     ph_dm);
    clib.getNodeValueXML("Body_Part",      ph_bp);
    clib.getNodeValueXML("Laterality",     ph_lat);
    clib.getNodeValueXML("Date_Created",   ph_date);
    clib.getNodeValueXML("Time_Created",   ph_time);
    clib.concludeSectionXML();

    std::cout << __FUNCTION__ <<", line " << __LINE__ << ": Phantom Parameters are:\n";
    std::cout << "\tPhantom_UID:\t"    << ph_uid << "\n"
              << "\tPhantom_Source:\t" << ph_src << "\n"
              << "\tPhantom_Shape:\t"  << ph_shape << "\n"
              << "\tPhantom_Type:\t"   << ph_type << "\n"
              << "\tData_Model:\t"     << ph_dm << "\n"
              << "\tBody_Part:\t"      << ph_bp << "\n"
              << "\tLaterality:\t"     << ph_lat << "\n"
              << "\tDate_Created:\t"   << ph_date << "\n"
              << "\tTime_Created:\t"   << ph_time << "\n";

}


void SavePhantom::readSubject()
{
    clib.getNodeValueXML("Subject_UID",   subj_uid);
    clib.getNodeValueXML("Subject_Type",  subj_type);
    clib.getNodeValueXML("Gender",        subj_gen);
    clib.getNodeValueXML("Pediatric",     subj_ped);
    clib.getNodeValueXML("Age",           subj_age);
    clib.getNodeValueXML("Date_Of_Birth", subj_dob);
    clib.getNodeValueXML("Race",          subj_race);
    clib.getNodeValueXML("Ethnicity",     subj_eth);
    clib.getNodeValueXML("Weight",        subj_wt);
    clib.getNodeValueXML("Height",        subj_ht);
    clib.getNodeValueXML("BMI",           subj_bmi);
    clib.concludeSectionXML();

    /*
    std::cout << __FUNCTION__ << ": Subject Parameters are:\n";
    std::cout << "\tSubject_UID:\t"     << subj_uid << "\n"
                << "\tSubject_Type:\t"  << subj_type << "\n"
                << "\tGender:\t\t"      << subj_gen << "\n"
                << "\tPediatric:\t"     << subj_ped << "\n"
                << "\tAge:\t\t"         << subj_age << "\n"
                << "\tDate_Of_Birth:\t" << subj_dob << "\n"
                << "\tRace:\t\t"        << subj_race << "\n"
                << "\tEthnicity:\t"     << subj_eth << "\n"
                << "\tWeight:\t\t"      << subj_wt << "\n"
                << "\tHeight:\t\t"      << subj_ht << "\n"
                << "\tBMI:\t\t"         << subj_bmi << "\n";
    */
}


void SavePhantom::readOrganization()
{
    clib.getNodeValueXML("Organization_Name", org_name);
    clib.getNodeValueXML("Division",          org_div);
    clib.getNodeValueXML("Department",        org_dept);
    clib.getNodeValueXML("Group",             org_grp);
    clib.getNodeValueXML("Institution_Code_Sequence", org_code);
    clib.getNodeValueXML("Country",           org_cntry);
    clib.getNodeValueXML("State",             org_sta);
    clib.getNodeValueXML("City",              org_city);
    clib.getNodeValueXML("ZipCode",           org_zip);
    clib.getNodeValueXML("UTC_Offset",        utc_offset);
    clib.concludeSectionXML();
    /*
    std::cout << __FUNCTION__ << ": Organization Parameters are:\n";
    std::cout << "\tOrganization_Name:\t" << org_name << "\n"
                << "\tDivision:\t" << org_div << "\n"
                << "\tDepartment:\t" << org_dept << "\n"
                << "\tGroup:\t" << org_grp << "\n"
                << "\tInstitution_Code_Sequence:\t" << org_code << "\n"
                << "\tCountry:\t" << org_cntry << "\n"
                << "\tState:\t" << org_sta << "\n"
                << "\tCity:\t" << org_city << "\n"
                << "\tZipCode:\t" << org_zip << "\n";
    */
}


void SavePhantom::readProgramParameters()
{
    // Before reading the seed, generate one randomly in case it is not specified
    seed = (unsigned int)time(NULL);

    clib.getNodeValueXML("XPLPhantom_UID",    xpl_phantom_UID);
    clib.getNodeValueXML("Phantom_Name",      phantom_name);
    clib.getNodeValueXML("Phantom_Filename",  phantom_filename);
    clib.getNodeValueXML("Phantom_Shape",     phantom_shape);
	clib.getNodeValueXML("Max_Ligament_Thickness", max_ligament_thickness); // New field as of 2021-01-13
	clib.getNodeValueXML("Min_Ligament_Thickness", min_ligament_thickness); // New field as of 2021-01-13
	clib.getNodeValueXML("Phantom_Shape", phantom_shape);
    clib.getNodeValueXML("Voxel_Size",        voxel_size);
    clib.getNodeValueXML("Size_X",            a);
    clib.getNodeValueXML("Size_Y",            b);
    clib.getNodeValueXML("Size_CPrime",       c1);
    clib.getNodeValueXML("Size_CSecondary",   c2);
    clib.getNodeValueXML("Random_Seed",       seed);

    // Notify User that Sigma is no longer used
    float sigma = 0.0f;
    bool found = clib.getNodeValueXML("Sigma", sigma);
    if (found || sigma != 0.0f)
    {
        std::cerr << "Sigma is no longer used. Please remove this field from your XML input or set its value to zero." << std::endl;
    }

    clib.getNodeValueXML("Percent_Dense",     perc_dense);
    clib.getNodeValueXML("Skin_Thickness",    skin_thickness);
    clib.getNodeValueXML("Ki",                ki);
    clib.getNodeValueXML("Kp",                kp);
    clib.getNodeValueXML("Ks",                ks);
    clib.getNodeValueXML("Num_Compartments",  num_compartments);
    clib.getNodeValueXML("Compartment_Thickness", comp_thickness);
    clib.getNodeValueXML("Min_Seed_Distance", min_seed_dist);
    clib.getNodeValueXML("Minimum_Speed",     min_speed);
    clib.getNodeValueXML("Maximum_Speed",     max_speed);
    clib.getNodeValueXML("Minimum_Ratio",     min_ratio);
    clib.getNodeValueXML("Maximum_Ratio",     max_ratio);
    clib.getNodeValueXML("Beta_p1",           beta_p1);
    clib.getNodeValueXML("Beta_q1",           beta_q1);
    clib.getNodeValueXML("Beta_p2",           beta_p2);
    clib.getNodeValueXML("Beta_q2",           beta_q2);
    clib.getNodeValueXML("Dist_File_In",      seed_dist_file_in);
    clib.getNodeValueXML("Dist_File_Out",     seed_dist_file_out);

    clib.concludeSectionXML();

    // Determine output path
    std::string::size_type pos = phantom_filename.find_last_of("/\\");
    if (pos != std::string::npos)
    {
        phantom_output_path = phantom_filename.substr(0, pos+1); // path includes delineator
    }

    std::cout << __FUNCTION__ <<", line " << __LINE__ << ": Phantom Parameters are:\n";
    std::cout << "\tXPLPhantom_UID:\t" << xpl_phantom_UID << "\n"
              << "\tPhantomFilename:\t" << phantom_name << "\n"
              << "\tPhantomShape:\t" << phantom_shape << "\n"
              << "\tVoxel_Size:\t" << voxel_size << "\n"
              << "\tSize_X:\t" << a << "\n"
              << "\tSize_Y:\t" << b << "\n"
              << "\tSize_CPrime:\t" << c1 << "\n"
              << "\tSize_CSecondary:\t" << c2 << "\n"
              << "\tRandom_Seed:\t" << seed << "\n"
              << "\tPercent_Dense:\t" << perc_dense << "\n"
              << "\tSkin_Thickness:\t" << skin_thickness << "\n"
              << "\tNum_Compartments:\t" << num_compartments << "\n"
              << "\tCompartment_Thickness:\t" << comp_thickness << "\n"
              << "\tMin_Seed_Distance:\t" << min_seed_dist << "\n"
              << "\tMinimum_Speed:\t" << min_speed << "\n"
              << "\tMaximum_Speed:\t" << max_speed << "\n"
              << "\tMinimum_Ratio:\t" << min_ratio << "\n"
              << "\tMaximum_Ratio:\t" << max_ratio << "\n"
              << "\tBeta_p1:\t" << beta_p1 << "\n"
              << "\tBeta_q1:\t" << beta_q1 << "\n"
              << "\tBeta_p2:\t" << beta_p2 << "\n"
              << "\tBeta_q2:\t" << beta_q2 << "\n"
              << "\tDist_File_In:\t" << seed_dist_file_in << "\n"
              << "\tDist_File_Out:\t" << seed_dist_file_out << "\n";

}



void SavePhantom::dumpParameters(std::string description)
{
    std::cout << "\n" << description << "\n";
    std::cout << "output file: "                        << phantom_filename.c_str() << "\n";
    std::cout << "Voxel size(cm): "                     << voxel_size << " \n";
    std::cout << "Dimension(cm) in X: "                 << a << "\n";
    std::cout << "Dimension(cm) in Y: "                 << b << "\n";
    std::cout << "Dimension(cm) in Z (primary): "       << c1 << "\n";
    std::cout << "Dimension(cm) in Z (secondary): "     << c2  << "\n";
    std::cout << "Random number seed: "                 << seed << "\n";

    ///std::cout << "Coefficient for probability distribution of dense compartments: "  << sigma << " \n"; // NOTE: THERE IS NO SIGMA ANYMORE

    std::cout << "Percentage of compartments that are dense:"         << perc_dense     << "\n";
    std::cout << "Skin thickness (cm): "                              << skin_thickness << "\n";

    std::cout << "ki: " << ki << "\n";
    std::cout << "kp: " << kp << "\n";
    std::cout << "ks: " << ks << "\n";

    std::cout << "Number of compartments: "                           << num_compartments << "\n";
    std::cout << "Compartment thickness (cm): "                       << comp_thickness << "\n";
    std::cout << "Minimal distance between seeds (cm): "              << min_seed_dist << "\n";
    std::cout << "Minimal speed of growth: "                          << min_speed << "\n";
    std::cout << "Maximal speed of growth : "                         << max_speed << "\n";
    std::cout << "Minimal ratio of distribution ellipse halfaxis : "  << min_ratio << "\n";
    std::cout << "Maximal ratio of distribution ellipse halfaxis: "   << max_ratio << "\n";

    std::cout << "p1 of beta distribution: "  << beta_p1 << "\n";
    std::cout << "q1 of beta distribution: "  << beta_q1 << "\n";
    std::cout << "p2 of beta distribution: "  << beta_p2 << "\n";
    std::cout << "q2 of beta distribution: "  << beta_q2 << "\n";

    std::cout << "Seed distribution file In: " << seed_dist_file_in << "\n";
    std::cout << "Seed distribution file Out: " << seed_dist_file_out << "\n";
}


bool SavePhantom::createSmallAttenFile(vct::Vctx &vctx)
{
    bool retval = false;

    if (verbose) std::cout << "\nBuilding abbreviated attenuation table...\n";

    int maxIndex = i_table.getLargestIndex();
    if (verbose) std::cout  << "maxIndex is " << maxIndex << std::endl;

    for(int i=0; i<=maxIndex; ++i)
    {
        vct::Label lbl = i_table.getLabel(i);
        int numMats = lbl.getNumMaterials();
        if (verbose) std::cout << "lbl " << lbl.getId() << " has " << numMats << " materials" << std::endl;
        for(int m=0; m < numMats; ++m)
        {
            vct::Material mat = lbl.getMaterial(m);
            atten_table.addMaterial(mat.getMaterialZ());
        }
    }

    atten_table.buildAttenTable();
    vctx.addPrivateData(&atten_table);

    return retval;
}
