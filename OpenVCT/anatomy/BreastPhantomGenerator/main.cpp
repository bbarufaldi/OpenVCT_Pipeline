// BreastPhantomGenerator.cpp : Defines the entry point for the console application.


#include "BreastPhantomGenerator.h"
#include "SavePhantom.h"
#include "VCT_Random.h"
#include "VCT_CommonLibrary.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <ctime>


int main(int argc, char* argv[])
{
    const static std::string filename("$Id$ ");
    const static std::string file_revision("$Rev$");
    const static std::string build_date(", Build Date: ");
    const static std::string time_stamp(__TIMESTAMP__);
    const static std::string VERSION(filename + file_revision + build_date + time_stamp);

    if (argc == 2)
    {
		std::string prog_name(argv[0]);
		auto pos = prog_name.find_last_of("\\/");
		if (pos != std::string::npos) prog_name = prog_name.substr(pos+1);
        std::string s_arg(argv[1]);
        std::transform(s_arg.begin(), s_arg.end(), s_arg.begin(), tolower); // convert to lower case
        
        if (s_arg.find("version") != std::string::npos) // -version or --version or /version
        {
            std::cout << prog_name << ": " << VERSION << std::endl;
            return 0;  // exit program with success
        }
        else if (s_arg.find("help")  != std::string::npos
              || s_arg.find("usage") != std::string::npos
              || s_arg.find("?")     != std::string::npos)
        {
            std::cout << "\nUsage:   " << prog_name << " -xml_input <xml_input_file>.xml\n\n";
            std::cout << "   or:   "   << prog_name << " -version   ..to check program version info. \n\n";
            std::cout << "   or:   "   << prog_name << " -example   ..to see an example xml_input file.\n\n";
            return 0;  // exit program with success
        }
        else if (s_arg.find("example") != std::string::npos)
        {                
            std::cout << "\nThe xml file might look like this:\n\n";
            std::cout << "    <?xml version=\"1.0\" ?>\n";
            std::cout << "    <Breast_Phantom_Designer>\n";
            std::cout << "        <Subject> <!-- see VCT_Subject.h for valid enumerations -->\n";
            std::cout << "            <Subject_UID>000.123456789012.20200101000000000</Subject_UID>\n";
            std::cout << "            <Subject_Type>Anthropomorphic</Subject_Type>\n";
            std::cout << "            <Gender>Female</Gender>\n";
            std::cout << "            <Pediatric>false</Pediatric>\n";
            std::cout << "            <Age>120</Age>\n";
            std::cout << "            <Date_Of_Birth>1900-01-01</Date_Of_Birth>\n";
            std::cout << "            <Race>UNK_RACE</Race>\n";
            std::cout << "            <Ethnicity>UNK_ETHNICITY</Ethnicity>\n";
            std::cout << "            <Weight>60.0</Weight>\n";
            std::cout << "            <Height>1.65</Height>\n";
            std::cout << "            <BMI>22.0</BMI>\n";
            std::cout << "        </Subject>\n";
            std::cout << "        <Phantom> <!-- see VCT_Phantom.h for valid enumerations -->\n";
            std::cout << "            <Phantom_UID>000.123456789012.20200101000000000</Phantom_UID>\n";
            std::cout << "            <Phantom_Source>University of Pennsylvania</Phantom_Source>\n";
            std::cout << "            <Phantom_Shape>Breast</Phantom_Shape>\n";
            std::cout << "            <Phantom_Type>Voxel Array</Phantom_Type>\n";
            std::cout << "            <Data_Model>Indexed</Data_Model>\n";
            std::cout << "            <Body_Part>Breast</Body_Part>\n";
            std::cout << "            <Laterality>Left</Laterality>\n";
            std::cout << "            <Volume>699</Volume>\n";
            std::cout << "        </Phantom>\n";
            std::cout << "        <Generator_Config>\n";
            std::cout << "            <ProgramName>BreastPhantomGenerator</ProgramName>\n";
            std::cout << "            <XPLPhantom_UID>999999.0.123456789012.20200101000000000</XPLPhantom_UID>\n";
            std::cout << "            <Phantom_Name>Example</Phantom_Name>\n";
            std::cout << "            <Phantom_Filename>Example_999999.0.123456789012.20200101000000000.vctx</Phantom_Filename>\n";
            std::cout << "            <Phantom_Shape>Breast</Phantom_Shape>\n";
			std::cout << "            <Max_Ligament_Thickness>2.0</Max_Ligament_Thickness>\n";  // New field as of 2021-01-13
			std::cout << "            <Min_Ligament_Thickness>0.2</Min_Ligament_Thickness>\n";  // New field as of 2021-01-13
            std::cout << "            <Voxel_Size>0.25</Voxel_Size>\n";
            std::cout << "            <Size_X>6.27</Size_X>\n";
            std::cout << "            <Size_Y>6.27</Size_Y>\n";
            std::cout << "            <Size_CPrime>10.73</Size_CPrime>\n";
            std::cout << "            <Size_CSecondary>6.27</Size_CSecondary>\n";
            std::cout << "            <Random_Seed>19</Random_Seed>\n";
            std::cout << "            <Percent_Dense>35.0</Percent_Dense>\n";
            std::cout << "            <Skin_Thickness>0.15</Skin_Thickness>\n";
            std::cout << "            <Num_Compartments>699</Num_Compartments>\n";
            std::cout << "            <Ki>469</Ki>\n";
            std::cout << "            <Kp>79</Kp>\n";
            std::cout << "            <Ks>151</Ks>\n";
            std::cout << "            <Num_Compartments>699</Num_Compartments>\n";
            std::cout << "            <Compartment_Thickness>0.02</Compartment_Thickness>\n";
            std::cout << "            <Min_Seed_Distance>0.1</Min_Seed_Distance>\n";
            std::cout << "            <Minimum_Speed>0.5</Minimum_Speed>\n";
            std::cout << "            <Maximum_Speed>2.0</Maximum_Speed>\n";
            std::cout << "            <Minimum_Ratio>0.5</Minimum_Ratio>\n";
            std::cout << "            <Maximum_Ratio>2.0</Maximum_Ratio>\n";
            std::cout << "            <Beta_p1>0.5</Beta_p1>\n";
            std::cout << "            <Beta_p2>0.5</Beta_p2>\n";
            std::cout << "            <Beta_q1>0.5</Beta_q1>\n";
            std::cout << "            <Beta_q2>3.5</Beta_q2>\n";
            std::cout << "            <Dist_File_In></Dist_File_In>\n";
            std::cout << "            <Dist_File_out></Dist_File_out>\n";
            std::cout << "        </Generator_Config>\n";
            std::cout << "        <Leave_In_Temp_Folder>false</Leave_In_Temp_Folder>\n";
            std::cout << "    </Breast_Phantom_Designer>\n";
            return 0;  // exit program with success
        }
        else
        {
            std::cerr << "Incorrect set of arguments. Type \"" << argv[0] << " -help\" to see more.\n";
            exit(0);
        }
    }

    SavePhantom *sp = new SavePhantom;
    if (sp == nullptr)
    {
        std::cout << "Couldn't allocate memory for SavePhantom instantiation. Fatal error." << std::endl;
        return -1;
    }
        
    unsigned int rand_seed = 0; // this will be accessed externally by recursive_partition_simulator

    // Dimensions of phantom: these dimensions are for Bcup breast
    float a     = 5;    // phantom dimension (cm)
    float b     = 5;    // phantom dimension (cm)
    float cprim = 12;   // phantom dimension (cm)
    float csec  = 5;    // phantom dimension (cm)

    sp->a = a;
    sp->b = b;
    sp->c1 = cprim;
    sp->c2 = csec;

    // Skin thickness
    float d = 0.15f;    // skin depth        (cm)
    sp->skin_thickness = d;

    // Parameters equivalent to the parameters from region growing:

    float mindistseeds=0.5; // minimal distance between seeds (cm)
    sp->min_seed_dist = mindistseeds;

    std::vector<float> minspeed_archive, maxspeed_archive;
    std::vector<float> minratio_archive, maxratio_archive;
    std::vector<float> ThicknessArchive;
    std::vector<float> deltaxArchive;

    std::vector<int> number_distributionsArchive; // NOT IN ORIGINAL DH 2016-06-24

    // Start portable timer
    vct::CommonLibrary clib;
    clib.start_timer();

    minspeed_archive.push_back(0.5); // minimal speed of growth (corresponding to similar parameter of Cuiping)
    maxspeed_archive.push_back(2);   // maximal speed of growth (corresponding to similar parameter of Cuiping)
    minratio_archive.push_back(0.5); // minimal ratio of distribution ellipse halfaxis
    maxratio_archive.push_back(2);   // maximal ratio of distribution ellipse halfaxis

    int Lp = 9;            // maximal level of the tree that can fit in memory, rule of thumb: 1GB--Lp=8 (who has only 1GB?); 4GB--Lp=9; 16GB--Lp=10; 64GB--Lp=11
              
    float sigma = 5;       // coefficient for probability distribution of dense compartments
    float Percentage = 5;  // percentage of compartments that are dense %I took small percentage!

    sp->perc_dense = sigma;             //  ADM  - there is a mistake here
    sp->perc_dense = Percentage;

    float p1 = 2.0;
    float q1 = 0.5;
    float p2 = 2.0;
    float q2 = 3.5;

    sp->beta_p1 = p1;
    sp->beta_q1 = q1;
    sp->beta_p2 = p2;
    sp->beta_q2 = q2;

    ThicknessArchive.push_back(0.06f);          // Minimal thickness of Cooper ligaments in cm
    number_distributionsArchive.push_back(333); // [75 100 150 200 250 333 500]; Number of compartments in the phantom
    deltaxArchive.push_back(0.02f);             // voxel size in cm
     
    std::string distributionFileOut, distributionFileIn;

    int counter=0; // internal variable, counts the number of generated phantoms in the file

    float deltax             = deltaxArchive[0];
    float Thickness          = ThicknessArchive[0];
    int number_distributions = number_distributionsArchive[0];

    sp->comp_thickness = Thickness;
    sp->num_compartments = number_distributions;

    float minspeed = minspeed_archive[0];
    float maxspeed = maxspeed_archive[0];
    float minratio = minratio_archive[0];
    float maxratio = maxratio_archive[0];

    sp->min_speed = minspeed;
    sp->max_speed = maxspeed;
    sp->min_ratio = minratio;
    sp->max_ratio = maxratio;

    std::string outputFileName("Phantom.vctx"); 
    std::string phantom_shape("Breast");
	float max_ligament_thickness(1.0f);   // New field as of 2021-01-13
	float min_ligament_thickness(0.5f);   // New field as of 2021-01-13

    sp->a = a;
    bool config_file_read = false;
    for (int n = 1; n < argc; n=n+1)
    {  
        std::string arg(argv[n]);

        if (n+1 >= argc)
        {
            std::cerr << "Incorrect set of arguments: " << arg.c_str() << ". Exiting without generating phantom.\n";
        }
        else
        {
            // Randomly (use system clock) an initial random seed which may be overridden by cmd line arg
            rand_seed = (unsigned int) time(NULL);

            // Convert argument to lower case to negate case-sensitivity for command line arguments
            std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
            if (arg == "-xml_input")
            {
                std::string xml_file(argv[n+1]);
                if (xml_file.find(".xml") == std::string::npos && xml_file.find(".XML") == std::string::npos)
                {
                    std::cerr << "xml_input argument, \"" << xml_file << "\" doesn't appear to be a XML file." 
                              << std::endl;
                    return -1; // bail
                }
                
                (void) sp->readConfig();
                config_file_read = true;
                bool status = sp->readInputXML(xml_file);
                if (status)
                {
                    deltax = sp->voxel_size * 0.1f; // deltax is in CM not MM
                    a = sp->a;
                    b = sp->b;
                    cprim = sp->c1;
                    csec = sp->c2;
                    rand_seed = sp->seed;
                    srand(rand_seed);
                    outputFileName = sp->phantom_filename;
                    phantom_shape = sp->phantom_shape;
					max_ligament_thickness = sp->max_ligament_thickness;  // New field as of 2021-01-13
					min_ligament_thickness = sp->min_ligament_thickness;  // New field as of 2021-01-13
                    Percentage = sp->perc_dense;
                    d = sp->skin_thickness;
                    number_distributions = sp->num_compartments;
                    Thickness = sp->comp_thickness;
                    mindistseeds = sp->min_seed_dist;
                    minspeed = sp->min_speed;
                    maxspeed = sp->max_speed;
                    minratio = sp->min_ratio;
                    maxratio = sp->max_ratio;
                    p1 = sp->beta_p1;
                    q1 = sp->beta_q1;
                    p2 = sp->beta_p2;
                    q2 = sp->beta_q2;
                    distributionFileIn = sp->seed_dist_file_in;
                    distributionFileOut = sp->seed_dist_file_out;

                    break;
                }
                else
                {
                    std::cerr << "couldn't read XML file " << xml_file << std::endl;
                }
            }
            else
            {
                std::cerr << "Invalid argument: " << arg.c_str() << ". Exit without generating phantom.\n";
                exit(0);
            }
            n++;
        }
    }

    std::cout << "\nPhantom Parameters:\n\n";
    std::cout << "\tOutput file (phantom): " << outputFileName.c_str() << "\n";
    std::cout << "\tPhantom Shape:         " << phantom_shape << "\n";
	std::cout << "\tMax ligament thicness: " << max_ligament_thickness << "\n";  // New field as of 2021-01-13
	std::cout << "\tMin ligament thicness: " << min_ligament_thickness << "\n";  // New field as of 2021-01-13
    std::cout << "\tVoxel size(mm):        " << deltax * 10.0f << " \n"; // deltax is in cm at this point
    std::cout << "\tDimension(mm) in X:    " << a << "\n";
    std::cout << "\tDimension(mm) in Y:    " << b << "\n";
    std::cout << "\tDimension(mm) in Z (primary):   "  << cprim << "\n";
    std::cout << "\tDimension(mm) in Z (secondary): "  << csec  << "\n";

    std::cout << "\tRandom number seed:  " << rand_seed << "\n";

    std::cout << "\tCoefficient for probability distribution of dense compartments: "  << sigma << " \n";
    std::cout << "\tPercentage of compartments that are dense: " << Percentage << "\n";
    std::cout << "\tSkin thickness (mm): " << d << "\n";

    std::cout << "\tNumber of compartments:     "                     << number_distributions << "\n";
    std::cout << "\tCompartment thickness (mm): "                     << Thickness << "\n";
    std::cout << "\tMinimal distance between seeds (mm): "            << mindistseeds << "\n";
    std::cout << "\tMinimal speed of growth:    "                     << minspeed << "\n";
    std::cout << "\tMaximal speed of growth :   "                     << maxspeed << "\n";
    std::cout << "\tMinimal ratio of distribution ellipse halfaxis: " << minratio << "\n";
    std::cout << "\tMaximal ratio of distribution ellipse halfaxis: " << maxratio << "\n";

    std::cout << "\tp1 of beta distribution: " << p1 << "\n";
    std::cout << "\tq1 of beta distribution: " << q1 << "\n";
    std::cout << "\tp2 of beta distribution: " << p2 << "\n";
    std::cout << "\tq2 of beta distribution: " << q2 << "\n";
    std::cout << "\tDist File In:            " << (distributionFileIn.size()  < 1? "(not specified)" : distributionFileIn)  << "\n";
    std::cout << "\tDist File Out:           " << (distributionFileOut.size() < 1? "(not specified)" : distributionFileOut) << "\n";
    std::cout << std::endl; 

    if (!config_file_read) sp->readConfig();

    BreastPhantomGenerator *sim = new BreastPhantomGenerator;

    sim->setOutputFileName(outputFileName);
    sim->setPhantomShape(phantom_shape);
	sim->setLigamentThicknesses(max_ligament_thickness, min_ligament_thickness);  // New fields as of 2021-01-13
    
    
    #if defined(STORE_RANDOM)
        std::cout << "* Storing Random Numbers * " << std::endl;
        sim->setRandomStore();
    #elif defined(READ_RANDOM)
        sim->setRandomRetrieval();              
    #endif
    
    bool status =
    sim->recursive_partition_simulation(a, b, d, cprim, csec,
                                        deltax, number_distributions,
                                        mindistseeds, minspeed, maxspeed,
                                        minratio, maxratio, Lp, 
                                        Thickness, sigma,
                                        Percentage, 
                                        0,
                                        p1, q1, p2, q2,
                                        outputFileName, 
                                        distributionFileIn, 
                                        distributionFileOut, false);
    
    if (status == false)
    {
        if (sim) delete sim;
        if (sp)  delete sp;
        exit(0);
    }

    // Private Data: BreastPhantomGenerator command line arguments
    std::string version("1.0");
    std::string id("BreastPhantomGenerator");
    
    // Strip off .dat, .vctx, or .txt if they are the current file extension
    std::string phantom_name(outputFileName);
    std::string::size_type pos = phantom_name.find_last_of(".dat");
    if (pos != std::string::npos)
    {
        phantom_name = phantom_name.substr(0, pos);    
    }
    pos = phantom_name.find_last_of(".vctx");
    if (pos != std::string::npos)
    {
        phantom_name = phantom_name.substr(0, pos);    
    }
    pos = phantom_name.find_last_of(".txt");
    if (pos != std::string::npos)
    {
        phantom_name = phantom_name.substr(0, pos);    
    }

    sp->suggestPhantomName(phantom_name);

    short xsize, ysize, zsize;
    sim->getVoxelDimensions(xsize, ysize, zsize);
    sp->setGenParms(version, id, outputFileName, deltax, a, b, cprim, csec, 
                   rand_seed, sigma, Percentage, d, number_distributions, Thickness,
                   mindistseeds, minspeed, maxspeed, minratio, maxratio,
                   p1, q1, p2, q2,
                   distributionFileOut, distributionFileIn);

    sp->save((long long)xsize, (long long) ysize, (long long) zsize, sim->getVoxelPointer());

                                             
    // Report Elapsed Time
    double elapsed_time = double(clib.get_elapsed_time()) * 0.001;
    std::cout << "Elapsed time is " << std::fixed << std::setprecision(2) << elapsed_time << " sec." << std::endl;  
    std::cout << "Output Filename is \"" << outputFileName << "\"\n" << std::endl;

    minspeed_archive.clear();
    maxspeed_archive.clear();
    minratio_archive.clear();
    maxratio_archive.clear();
    ThicknessArchive.clear();
    deltaxArchive.clear();
    number_distributionsArchive.clear();

    #if defined(LOG_ACTIVITY)
    std::ofstream dbg;
    dbg.open("../../../Debug.log", std::ofstream::app);
    dbg << __FUNCTION__ << ", line " << __LINE__ << ": ...back from sim->recursive_data_split" << std::endl;
    dbg.close();
    #endif

    // The sleep added here is to allow any semi-autonomous operating system interactions to complete
    clib.sleep(100);
    
    if (sim != nullptr)
    {
        delete sim;
        sim = nullptr;
    }

    return 0;
}

