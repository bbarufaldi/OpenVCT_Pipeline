// XPL_BreastPhantomGenerator.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_PrivateData.h"
#include <string>
#include <vector>

namespace vct
{

class XPL_BreastPhantomGenerator : public PrivateData
{
public:

    // Manipulators (used to set component values)
    
    void setProgramName(std::string name)   { sw_name = name; }
    void setSWVersion(std::string &vers)    { sw_version = vers; }
    void setUniqueId(std::string &id)       { unique_id = id; }
    void setPhantomName(std::string &name)  { phantom_name = name; }
    void setPhantomType(std::string &shape) { phantom_shape = shape; }
    void setFilename(std::string &filename) { phantom_filename = filename; }
    void setVoxelSize(float vsize)          { voxel_size = vsize; }
    void setXSize(float x)                  { size_x_cm = x; }
    void setYSize(float y)                  { size_y_cm = y; }
    void setCPrimeSize(float cprim)         { size_cprim_cm = cprim; }
    void setCSecSize(float csec)            { size_csec_cm = csec; }

	void setRandomSeed(int seed)            { random_seed = seed; }

    void setDSigma(float dsig)              { dsigma = dsig; }
    void setDensePercent(float dperc)       { dpercent = dperc; }
    void setSkinThickness(float skin)       { skin_cm = skin; }
    void setNumCompartments(int num)        { num_comparts = num; }
    void setCompartThickness(float ct)      { comp_thick_cm = ct; }
    void setMinDistBetweenSeeds(float md)   { min_dist_cm = md; }
    void setMinSpeedOfGrowth(float msg)     { min_speed = msg; }
    void setMaxSpeedOfGrowth(float msg)     { max_speed = msg; }
    void setMinRatio(float mr)              { min_ratio = mr; }
    void setMaxRatio(float mr)              { max_ratio = mr; }
    void setP1(float p1)                    { beta_p1 = p1; }
    void setQ1(float q1)                    { beta_q1 = q1; }
    void setP2(float p2)                    { beta_p2 = p2; }
    void setQ2(float q2)                    { beta_q2 = q2; }
    void setDistrFileOut(std::string out)   { dist_fileOut = out; }
    void setDistrFileIn(std::string in)     { dist_fileIn = in; }

    // Accessors (retrieve data)

    std::string getProgramName()   { return sw_name; }
    std::string getSWVersion()     { return sw_version; }
    std::string getUniqueId()      { return unique_id; }
    std::string getPhantomName()   { return phantom_name; }
    std::string getFilename()      { return phantom_filename; }

    float getVoxelSize()           { return voxel_size; }
    float getXSize()               { return size_x_cm; }
    float getYSize()               { return size_y_cm; }
    float getCPrimeSize()          { return size_cprim_cm; }
    float getCSecSize()            { return size_csec_cm; }

    int   getRandomSeed()          { return random_seed; }

    float getDSigma()              { return dsigma; }
    float getDensePercent()        { return dpercent; }
    float getSkinThickness()       { return skin_cm; }
    int   getNumCompartments()     { return num_comparts; }
    float getCompartThickness()    { return comp_thick_cm; }
    float getMinDistBetweenSeeds() { return min_dist_cm; }
    float getMinSpeedOfGrowth()    { return min_speed; }
    float getMaxSpeedOfGrowth()    { return max_speed; }
    float getMinRatio()            { return min_ratio; }
    float getMaxRatio()            { return max_ratio; }
    float getP1()                  { return beta_p1; }
    float getQ1()                  { return beta_q1; }
    float getP2()                  { return beta_p2; }
    float getq2()                  { return beta_q2; }

    std::string getDistrFileOut()  { return dist_fileOut; }
    std::string getDistrFileIn()   { return dist_fileIn; }

    
    // XML file Interaction 


	void writeXML()
	{
        clib->writeNodeValueXML("Software_Name", sw_name);
        clib->writeNodeValueXML("Software_Version", sw_version);
        clib->writeNodeValueXML("Software_ID", unique_id);
        
        //std::cout << "XPL_BreastPhantomGenerator: " << __FUNCTION__ << ", line " << __LINE__  << ": PHANTOM NAME IS \"" << phantom_name << "\"" << std::endl;

        clib->writeNodeValueXML("Phantom_Name", phantom_name);
        clib->writeNodeValueXML("Phantom_Filename", phantom_filename);

        clib->writeNodeValueXML("Voxel_Size_CM", voxel_size);

        clib->writeNodeValueXML("Size_X", size_x_cm);
        clib->writeNodeValueXML("Size_Y", size_y_cm);
        clib->writeNodeValueXML("Size_CPrime", size_cprim_cm);
        clib->writeNodeValueXML("Size_CSecondary", size_csec_cm);
        
        clib->writeNodeValueXML("Random_Seed", random_seed);

        ///clib->writeNodeValueXML("Dense_Probability", dsigma); // no longer supported

        clib->writeNodeValueXML("Percent_Dense", dpercent);
        clib->writeNodeValueXML("Skin_Thickness", skin_cm);
        clib->writeNodeValueXML("Num_Compartments", num_comparts);
        clib->writeNodeValueXML("Compartment_Thickness", comp_thick_cm);
        clib->writeNodeValueXML("Min_Seed_Distance", min_dist_cm);

        clib->writeNodeValueXML("Minimum_Speed", min_speed);
        clib->writeNodeValueXML("Maximum_Speed", max_speed);
        clib->writeNodeValueXML("Minimum_Ratio", min_ratio);
        clib->writeNodeValueXML("Maximum_Ratio", max_ratio);

        clib->writeNodeValueXML("Beta_p1", beta_p1);
        clib->writeNodeValueXML("Beta_q1", beta_q1);
        clib->writeNodeValueXML("Beta_p2", beta_p2);
        clib->writeNodeValueXML("Beta_q2", beta_q2);

        if (dist_fileOut.size() > 0) clib->writeNodeValueXML("Distribution_File_Out", dist_fileOut);
        if (dist_fileIn.size() > 0)  clib->writeNodeValueXML("Distribution_File_In", dist_fileIn);
    }
    
	void readXML()
	{
        clib->getNodeValueXML("Software_Name", sw_name);
        clib->getNodeValueXML("Software_Version", sw_version);
        clib->getNodeValueXML("Software_ID", unique_id);

        clib->getNodeValueXML("PhantomFilename", phantom_filename);
        clib->getNodeValueXML("Phantom_Shape", phantom_shape);

        clib->getNodeValueXML("Voxel_Size", voxel_size);

        clib->getNodeValueXML("Size_X", size_x_cm);
        clib->getNodeValueXML("Size_Y", size_y_cm);
        clib->getNodeValueXML("Size_CPrime", size_cprim_cm);
        clib->getNodeValueXML("Size_CSecondary", size_csec_cm);
        
        clib->getNodeValueXML("Random_Seed", random_seed);

        clib->getNodeValueXML("Dense_Probability", dsigma);
        clib->getNodeValueXML("Percent_Dense", dpercent);
        clib->getNodeValueXML("Skin_Thickness", skin_cm);
        clib->getNodeValueXML("Num_Compartments", num_comparts);
        clib->getNodeValueXML("Compartment_Thickness", comp_thick_cm);
        clib->getNodeValueXML("Min_Seed_Distance", min_dist_cm);

        clib->getNodeValueXML("Minimum_Speed", min_speed);
        clib->getNodeValueXML("Maximum_Speed", max_speed);
        clib->getNodeValueXML("Minimum_Ratio", min_ratio);
        clib->getNodeValueXML("Maximum_Ratio", max_ratio);

        clib->getNodeValueXML("Beta_p1", beta_p1);
        clib->getNodeValueXML("Beta_q1", beta_q1);
        clib->getNodeValueXML("Beta_p2", beta_p2);
        clib->getNodeValueXML("Beta_q2", beta_q2);

        clib->getNodeValueXML("Distribution_File_Out", dist_fileOut);
        clib->getNodeValueXML("Distribution_File_In", dist_fileIn);
    }



    // Constructor(s)
    
	XPL_BreastPhantomGenerator()
        : sw_name("BreastPhantomGenerator"),
          sw_version("0"),
          unique_id("0"),
		  phantom_name(),
		  phantom_filename(),
		  phantom_shape(),
          voxel_size(0.02f),
          size_x_cm(0),
          size_y_cm(0),
          size_cprim_cm(0),
          size_csec_cm(0),
          random_seed(0),
          dsigma(0),
          dpercent(0),
          skin_cm(0),
          num_comparts(0),
          comp_thick_cm(0),
          min_dist_cm(0),
          min_speed(0),
          max_speed(0),
          min_ratio(0),
          max_ratio(0),
          beta_p1(0),
          beta_q1(0),
          beta_p2(0),
          beta_q2(0),
          dist_fileOut(),
          dist_fileIn()
    {
        PrivateData::setProgramName("BreastPhantomGenerator");
    }

private:

    std::string sw_name;
    std::string sw_version;
    std::string unique_id;
    std::string phantom_name;
    std::string phantom_filename; // -f arg
    std::string phantom_shape;    // -shape arg
    float       voxel_size;       // -d arg
    float       size_x_cm;        // -x arg
    float       size_y_cm;        // -y arg
    float       size_cprim_cm;    // -z1 arg
    float       size_csec_cm;     // -z2 arg
    int         random_seed;      // -srand arg
    float       dsigma;           // -DSigma arg (coef for prob distr of dense compartments)
    float       dpercent;         // -DPercent arg (percentage of dense compartments)
    float       skin_cm;          // -skin arg (skin thickness, in cm)
    int         num_comparts;     // -c arg (number of compartments)
    float       comp_thick_cm;    // -t arg (compartment thickness in cm)
    float       min_dist_cm;      // -minDist arg (minimum distance between seeds in cm)
    float       min_speed;        // -minSpeed arg (minimum speed of growth)
    float       max_speed;        // -maxSpeed arg (maximum speed of growth)
    float       min_ratio;        // -minRatio arg (minimum ratio of distr ellipse half-axis)
    float       max_ratio;        // -maxRatio arg (maximum ratio of distr ellipse half-axis)
    float       beta_p1;          // -p1 arg (p1 of beta distribution)
    float       beta_q1;          // -q1 arg (q1 of beta distribution)
    float       beta_p2;          // -p2 arg (p2 of beta distribution)
    float       beta_q2;          // -q2 arg (q2 of beta distribution)
    std::string dist_fileOut;     // -distFileOut arg (save seed distribution file)
    std::string dist_fileIn;      // -distFileIn arg (input seed distribution file)
};

}