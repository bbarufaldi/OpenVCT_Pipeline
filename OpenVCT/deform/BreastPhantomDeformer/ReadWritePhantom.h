// ReadWritePhantom.cpp

#pragma once

#include "VCT_PrivateData.h"
#include "VCT_Logger.h"
#include "VCT_CommonLibrary.h"
#include "VCT_Vctx.h"
#include <string>
#include <fstream>
#include <sys/timeb.h>


class ReadWritePhantom
{
public:

    ReadWritePhantom();
    virtual ~ReadWritePhantom();

    void setVerbose(bool state) { verbose = state; }
    std::string getPhantomNameIn() { return phantom_name_in; }

    void readConfigFile();

    void setCmdLineArgs(std::string inphantom, std::string inmesh,
                        std::string outmesh, std::string outphantom);

    bool readInputXML(vct::Logger &logger, std::string xmlfile);

    unsigned char *getPhantom(vct::Logger &logger, size_t &x_dim, size_t &y_dim, size_t &z_dim,
                              float &vsizex_mm, float &vsizey_mm, float &vsizez_mm);

    void initVoxelArray();

    std::ofstream &prepareOutputPhantom(int xdim, int ydim, int zdim);
    void finishOutputPhantom();

    std::string           getInputMesh()   { return mesh_in; }
    std::string           getOutputMesh()  { return mesh_out; }
    vct::DEFORMATION_MODE getView()        { return view; }

protected:

    void conditionPhantomFilenames();
    size_t countGlandular(size_t total, unsigned char *voxels, size_t &non_air);

private:

    // Private data
    bool verbose;
    bool input_from_xml;
    struct timeb _starttime, _endtime;
    vct::Vctx vctx;
    vct::CommonLibrary clib;
    vct::IndexTable *it;  
    vct::Deformation def;
    vct::DEFORMATION_MODE view;

    std::string phantom_name_in;    
	std::string input_phantom_filename;
    std::string phantom_name_out;
	std::string output_phantom_filename;
    std::string mesh_in;
    std::string mesh_out;

    std::string  full_def_phantom_path;
    std::ofstream deformed_stream;

    size_t dim_x, dim_y, dim_z;
    float vsiz_x, vsiz_y, vsiz_z; //(mm)
    size_t def_x, def_y, def_z;

    unsigned int seqnum;
    float x_thickness_mm;
    float y_thickness_mm;
    float z_thickness_mm;
    unsigned char *deformed_buf;

    std::string program_name;
	std::string sw_version;
    std::string index_table_uid;
    std::string voxel_array_uid;

    std::string org_name;
    std::string org_div;
    std::string org_dept;
    std::string org_grp;
    std::string org_code;
    std::string org_cntry;
    std::string org_sta;
    std::string org_city;
    std::string org_zip;
    int         utc_offset;

};