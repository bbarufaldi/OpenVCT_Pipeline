// SavePhantom.h

#pragma once

#include "SavePhantom.h"
#include "VCT_Vctx.h"
#include "VCT_CommonLibrary.h"
#include "VCT_IndexTable.h"
#include "XPL_BreastPhantomGenerator.h"
#include "XPL_AttenuationTable.h"
#include <string>
#include <vector>
#include <ctime>

class SavePhantom
{
public:

    SavePhantom();
    virtual ~SavePhantom();

	void setVerbose(bool state) { verbose = state; }

    bool readConfig();
    bool readInputXML(std::string xmlfile);

    static std::string temp_dir;
    static std::string extension;

    void suggestPhantomName(std::string name);

	void setGenParms(std::string version, std::string id, std::string filename, 
                     float vxlSize, float sizeX, float sizeY, float cprim, float csec, 
		             int seed, float dsig, float dperc, float skin, int compartments, float compthick,
                     float mindist, float minspd, float mxspd, float minrat, float maxrat, 
                     float p1, float q1, float p2, float q2, 
                     std::string distFileOut, std::string distFileIn);
    void save(long long xsize, long long ysize, long long zsize, unsigned char *phantom);

	void dumpParameters(std::string description);

protected:
	
	void   readPhantom();
	void   readSubject();
	void   readOrganization();
	void   readProgramParameters();
    bool   createSmallAttenFile(vct::Vctx &vctx);

private:

	bool verbose;
    bool input_from_xml;
    bool leave_in_temp_folder;
    vct::CommonLibrary clib;
    std::string temp_directory;
    std::string filename;
    std::string private_name;
    vct::IndexTable i_table;
    std::string data_source;
    vct::XPL_BreastPhantomGenerator generator;
    vct::XPL_AttenuationTable atten_table;

public: // for now, until accessors can be created

    // XML Pass-thru data

    std::string subj_uid;
    std::string subj_type;
    std::string subj_gen;
    std::string subj_ped;
    int         subj_age;
    std::string subj_dob;
    std::string subj_race;
    std::string subj_eth;
    float       subj_wt;
    float       subj_ht;
    float       subj_bmi;

    std::string ph_uid;
    std::string ph_src;
    std::string ph_shape;
    std::string ph_type;
    std::string ph_dm;
    std::string ph_bp;
    std::string ph_lat;
    std::string ph_date;
    std::string ph_time;

    std::string org_name;
    std::string org_div;
    std::string org_dept;
    std::string org_grp;
    std::string org_code;
    std::string org_cntry;
    std::string org_sta;
    std::string org_city;
    std::string org_zip;

	int utc_offset;

    // Generator and Voxel Array information will be from the generator program itself
    std::string voxel_array_uid;
	std::string index_table_uid;
    std::string xpl_phantom_UID;
    std::string program_name;
    std::string phantom_name;

    // Program inputs
    std::string phantom_output_path;
    std::string phantom_filename;
    std::string phantom_shape;
    float       voxel_size;
    float       a;  // x
    float       b;  // y
    float       c1; // z1
    float       c2; // z2
    unsigned int seed;
    float       perc_dense;
    float       skin_thickness;
    float       ki;
    float       kp;
    float       ks;
    int         num_compartments;
    float       comp_thickness;
    float       min_seed_dist;
    float       min_speed;
    float       max_speed;
    float       min_ratio;
    float       max_ratio;
    float       beta_p1;
    float       beta_q1;
    float       beta_p2;
    float       beta_q2;
    std::string seed_dist_file_in;
    std::string seed_dist_file_out;
};
