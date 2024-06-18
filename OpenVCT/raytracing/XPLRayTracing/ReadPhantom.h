// ReadPhantom.h

#pragma once

#include "VCT_PrivateData.h"
#include "VCT_Logger.h"
#include "VCT_Vctx.h"
#include <string>
#include <fstream>
#include <map>


class Machine;  // req'd forward reference
class LabelMap; // req'd forward reference


class AcqGeom
{
public:

    bool readInputXML(vct::CommonLibrary &clib, vct::Logger &logger);

    int         id;
    std::string uid;
    double  focal_spot_x_mm;
    double  focal_spot_y_mm;
    double  focal_spot_z_mm;
    double  det_pos_x_mm;
    double  det_pos_y_mm;
    double  det_pos_z_mm;
    double  ph_offset_x_mm;     // BB and TV 04/10/2018
    double  ph_offset_y_mm;     // BB and TV 04/10/2018
    double  ph_offset_z_mm;     // BB and TV 04/10/2018
    double  Svec_ori_x;
    double  Svec_ori_y;
    double  Svec_ori_z;
    double  Tvec_ori_x;
    double  Tvec_ori_y;
    double  Tvec_ori_z;
};


// This is a Singleton class, meaning one one instance can be created
class ReadPhantom
{
public:

    // Note using this class as a singleton simplifies things a lot; otherwise:
    // 1. Since data must persist, the phantom would have to be dynamically allocated (and destroyed) to survice between classes
    // 2. The pointer would have to be passed
    //    a. XPLProjectionSimulation to XPLRayTracer
    //    b. XPLRayTracer to Simulation_CPU or Simulation_GPU
    //    c. Simulation_CPU or Simulation_GPU to Simulation
    //    d. Simulation to Volume
    // 3. These constructors would have to have been altered:
    //    a. Simulation_CPU
    //    b. Simulation_GPU
    //    c. Simulation
    //    d. Volume
    // 4. A significant (header file and implementation) dependency would have been imposed on these classes
    //    a. XPLProjectionSimulation
    //    b. XPLRayTracing
    //    c. Simulation_CPU
    //    d. Simulation_GPU
    //    e. Simulation
    //    f. Volume
    //    ...instead, a lightweight dependency has been imposed on these classes
    //    a. XPLProjectionSimulation
    //    b. XPLRayTracing
    //    c. Volume
    // By using a singleton, no header files and no constructors were harmed in the making of this interface
    
    static ReadPhantom &getInstance();
    
    virtual ~ReadPhantom();

    void readConfigFile();


    // Set (manipulator) functions
    void setCmdLineArgs(std::string atten, std::string config, std::string ph_in,
                        std::string comment, std::string seed);

    void setNoise(std::string noise, bool st)               { opt_noise = noise; use_noise = st; }    
    void setElec(std::string elec, double value)            { opt_elec = elec; elec_noise_value = value; elec_set = true; }
    void setGain(std::string gain, double value)            { opt_gain = gain; gain_value = value; gain_set = true; }
    void setMas(std::string mAs, double value)              { opt_mAs = mAs;   mas_value = value; mas_set = true; }
    void setPropConst(std::string pc, double value)         { opt_propconst = pc; prop_const = value; prop_set = true; }
    void setMinGray(std::string mg, unsigned short value)   { opt_mingray = mg; mingray_value = value; mingray_set = true; }
    void setScaling(std::string sc, bool st)                { opt_scaling = sc; use_scaling = st; }
    void setRandQuad(std::string rq, bool st)               { opt_randquad = rq; use_rand_quad = st; }
    void setInvSq(std::string is, bool st)                  { opt_invsq = is; use_inv_sq = st; }

    // Parse DICOM strings and store date & time for a sequence of acquisitions
    void addDicomDate(std::string date_string);
    void addDicomTime(std::string time_string);


    // Accessor (getter) functions
    bool getXmlInputUsed() { return xml_input_used; }
    bool readInputXML(vct::Logger &logger, std::string xmlfile);

    unsigned char *getPhantom(size_t &x_dim, size_t &y_dim, size_t &z_dim,
                              float &vsizex_m, float &vsizey_m, float &vsizez_m);

    void setAcquisitionGeometry(Machine *machine);
    bool buildLabelMap(std::map<unsigned short, LabelMap> *lmap);

    std::string getAttenFile()           { return attenuationFile; }
    std::string getConfigFile()          { return configGeomFile; }
    std::string getPhantomFilename()     { return input_phantom_filename; }
    std::string getOutputFolder()        { return output_folder; }
    std::string getComment()             { return commentString; }
    std::string getSeed()                { return rand_seed; }
    std::string getNoiseOption()         { return opt_noise; }
    std::string getElecNoiseOption()     { return opt_elec; }
    std::string getGainOption()          { return opt_gain; }
    std::string getMAsOption()           { return opt_mAs; }
    std::string getPropConstOption()     { return opt_propconst; }
    std::string getMinGrayOption()       { return opt_mingray; }
    std::string getScalingOption()       { return opt_scaling; }
    std::string getRandomQuadOption()    { return opt_randquad; }
    std::string getInverseSquareOption() { return opt_invsq; }

    bool    getUseNoise()        { return use_noise; }
    bool    getUseScaling()      { return use_scaling; }
    bool    getUseInvSq()        { return use_inv_sq; }

    bool    getElecNoiseSet()    { return elec_set; }
    bool    getGainSet()         { return gain_set; }
    bool    getMasSet()          { return mas_set; }
    bool    getPropConstSet()    { return prop_set; }
    bool    getMinGraySet()      { return mingray_set; }
    
    double  getElecNoiseValue()  { return elec_noise_value; } 
    double  getGainValue()       { return gain_value; }
    double  getMasValue()        { return mas_value; }
    double  getPropConstValue()  { return prop_const; }

    unsigned short getMinGrayValue() { return mingray_value; }

    // XRayTube Accessors
    unsigned short getSpectrumID()          { return spectrumID; }
    unsigned short getkVp()                 { return kVP; }
    std::string    getAnodeMaterialName()   { return anode_material_name; }
    std::string    getFilterMaterialName()  { return filter_material_name; }
    unsigned short getFilterMaterialZ()     { return filterMaterialZ; }
    double         getFilterThickness_mm()  { return filterThickness_mm; }
    double         getPhantomThickness_mm() { return phantom_thickness_mm; }
    double         getDensityCoefficient()  { return density_coefficient; }

    // Detector Accessors
    unsigned int   getElementCountX()       { return elementCountX; }
    unsigned int   getElementCountY()       { return elementCountY; }
    double         getElementSizeX_mm()     { return elementsizeX_mm; }
    double         getElementSizeY_mm()     { return elementsizeY_mm; }
    double         getDetThickness_mm()     { return det_thickness_mm; }
    unsigned short getDetMaterial()         { return det_material; }
    short          getAllocatedBits()       { return allocatedbits; }
    short          getStoredBits()          { return storedbits; }
    short          getHighBit()             { return highbit; }
    bool           getUseXYVolOffsets()     { return useXYVolumeOffsets; }
    double         getOptVolOffsetX_mm()    { return volumeOffsetX_mm; }
    double         getOptVolOffsetY_mm()    { return volumeOffsetY_mm; }
    double         getVolumeOffsetZ_mm()    { return volumeOffsetZ_mm; }

    // Generate output report
    void generateReport();

protected:

    ReadPhantom();
    void conditionPhantomFilename();
    bool rotate90X();

private:

    // Private data
    std::clock_t _starttime, _endtime;
    vct::Vctx vctx;

    // Command Line arguments
    std::string attenuationFile;
    std::string configGeomFile;
    std::string commentString;
    std::string rand_seed;
    std::string opt_noise;
    std::string opt_elec;
    std::string opt_gain;
    std::string opt_mAs;
    std::string opt_propconst;
    std::string opt_mingray;
    std::string opt_scaling;
    std::string opt_randquad;
    std::string opt_invsq;

    std::string phantom_name_in;    
    std::string input_phantom_filename;
    std::string output_folder;
    std::string full_def_phantom_path;

    unsigned char *voxels;
    size_t dim_x, dim_y, dim_z;
    float vsiz_x, vsiz_y, vsiz_z;

    unsigned int seqnum;
    bool xml_input_used;

    std::string program_name;
    std::string sw_version;
    std::string index_table_uid;
    std::string voxel_array_uid;

    std::string imaging_system_name;

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

    bool use_noise;
    bool use_scaling;
    bool use_rand_quad;
    bool use_inv_sq;

    bool elec_set;
    bool gain_set;
    bool mas_set;
    bool prop_set;
    bool mingray_set;

    double elec_noise_value;
    double gain_value;
    double mas_value;
    double prop_const;
    unsigned short mingray_value;

    // XRayTube 
    unsigned short spectrumID;
    unsigned short kVP;
    std::string    anode_material_name;
    std::string    filter_material_name;
    unsigned short filterMaterialZ;
    double         filterThickness_mm;
    double         phantom_thickness_mm;
    double         density_coefficient;

    // Detector
    unsigned int   elementCountX;
    unsigned int   elementCountY;
    double         elementsizeX_mm; // pixel size in mm
    double         elementsizeY_mm; // pixel size in mm
    double         det_thickness_mm;
    unsigned short det_material;
    short          allocatedbits;
    short          storedbits;
    short          highbit;
    bool           useXYVolumeOffsets;
    double         volumeOffsetX_mm; // optional offset override
    double         volumeOffsetY_mm; // optional offset override
    double         volumeOffsetZ_mm; // absolute offset

    // Acquisitions
    int number_of_acquisitions;
    std::vector<AcqGeom> geometries; // acquisition geometries read from input xml
    std::vector<vct::Date> acq_dates;
    std::vector<vct::Time> acq_times;
};
