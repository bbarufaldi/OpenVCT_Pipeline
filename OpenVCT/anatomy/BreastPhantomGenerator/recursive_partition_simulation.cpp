// recursive_partition_simulation.cpp

#include "BreastPhantomGenerator.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "VCT_CommonLibrary.h"

#if defined(_MSC_VER)
#include <Windows.h>
#endif

const double PI = 4.0 * std::atan(1.0);

/*
//// 11/27/2010>script11_26_2010_4 automatic calculation of number of levels and the
//// bounding boxes B0, Bi.
////
//// Parameters (and suggested values)

// a=5;          //phantom dimension (cm)
// b=5;          //phantom dimension (cm)
// cprim=12;     //phantom dimension (cm)
// csec=5;       //phantom dimension (cm)
// d=0.1;        //skin depth        (cm)
// deltax=0.01;  //voxel resolution  (cm)
// number_distributions=333 //number of seeds for fat/dense tissue
// mindistseeds=0.5; //minimal distance between seeds (cm)
// minspeed=1    //minimal speed of growth (corresponding to similar  parameter of Cuiping)
// maxspeed=1.5  //maximal speed of growth (corresponding to similar// parameter of Cuiping)
// minratio=1.5  //minimal ratio of distribution ellipse halfaxis
// maxratio=2    //maximal ratio of distribution ellipse halfaxis
// Lp=9          //maximal level of the tree that can fit in memory, rule of thumb: 1GB--Lp=8 (who has only 1GB?); 4GB--Lp=9; 16GB--Lp=10; 64GB--Lp=11;
// outputFileName //Filename, the output stored in <outputFileName>.mat
// Thickness     //Minimal thickness of the Cooper ligaments
// sigma         //Parameter to determine distribution of dense
// Percentage    //Percentage of dense compartments
// DistributionLoadFile  //Optional! If specified, the distribution parameters are read
// from this file

// Note: due to hard coding below, deltax/2 must be larger than 0.00001
// Note: the code may not work properly (due to calculation of indices, lines 136-152) when
// a/deltax, b/deltay, (cprim+csec)/2deltaz are not integers (e.w.
// deltax=0.3)
// >recursive_partition_simulation, just testing voxel assignment
// BECAUSE OF FINITE PRECISION, do not use floor, ceil!
// 11/29/2010 bookkeeping of processed volume updated BOOKKEEPING need be
// improved!!!

// 11/30/2010 >recursive_partition_simulation2  calls
// recursive_data_split_BFS_15, keeps thickness of Cooper ligaments of at
// least Thickness DOES NOT WORK do not use!!!
// 12/3/2010 >recursive_partition_simulation 2a. uses
// init_oct_tree_empty_2f.

// 12/3/2010 >recursive_partition_simulation2b. Two major changes. First, we
// would use the classic algorithm is the voxel size is larger or equal to
// the half of the thickness. Second, we would further improve the minimal
// and maximal bound 

// 12/4/2010> recursive_partition_simulation3, commented computation of
// percentages TO BE USED WITH 50um PHANTOM!!!
// 12/11/2010 >recursive_partition_simulation3a no structures
// 12/16/2010 bookeeping of non-air nonleaf nodes and leaf nodes at each
// level

// 12/22/2010 deltaX squaredDeltaX...globals12/22/2010
// 12/22/2010 unnecessary globals removed
// 1/8/2011 also counts the numbers of cooper ligament nodes that need be
// split in the next level, the total number of the associated
// distributions, and the average number of the associated distributions.
// Used to estimate the storage needed
// 1/18/2011 ,sigma,Percentage added (sigma is normalization coefficient for
// distribution of dense compartments, Percentage is the percentage of dense
// compartments

// 2/2/2011 distributionMUVectors distributionSigmaInverseMatrices distributionPriors
// added to the list of saved variables (ground truth for seeds of
// compartments and their shape)

// 2/3/2011 Always saves in 7.3 format to prevent problems with >2GB phantoms
// 2/5/2011 Modification. The input file for distribution parameters is also
// entered. If we say we want to load distribution parameters, we DO NOT
// execute initialize_distribution_parameters,  but load the file. In
// contrary, if we execute initialize_distribution_parameters, we save
// all distribution parameters. 

// 2/6/2011 tried to modify for a general size phantom!
// 2/26/2011 ALWAYS runs with thickness control, but the minimal thickness
// will be max(Thickness,2*deltax). This way, hopefully, we will avoid
// effects of rough cooper ligament boundaries (Pedja's idea from 2/25/2011 meeting)

// 3/26/2011 Accounting of the percentage of processed phantom is corrected.
//            NOTE: We normalize to find percent with THEORETICAL volume
//            (and not with the ACTUAL volume of voxels that will belong to
//            the phantom). Hence, the percentage is approximative.
//            Also, we display the percent by comparing the absolute, not
//            relative volumes, which lead to slight (1//) improvement of
//            performance.

// examined_volume_percent removed as global
*/
bool BreastPhantomGenerator::recursive_partition_simulation(
	                        float a, float b, float d, float cprim, float csec,
	                        float deltax, int number_distributions, float mindistseeds, 
	                        float minspeed, float maxspeed, float minratio, float maxratio,	
	                        int Lp, float Thickness, float sigma, float Percentage, unsigned int textureid, // textureid is passed in as visualizer->textureID
	                        float alpha1, float beta1, float alpha2, float beta2, 
	                        std::string outputFile, std::string distributionFileIn, std::string distributionFileOut, 
	                        bool isGaussian)
{ 
    int err = 0;
    vct::CommonLibrary clib;
                
	// The voxel is assumed to have the same size in all 3 dimensions
	float deltay = deltax;
	float deltaz = deltax;

	// Minimal bounding rectangle B0'
	float x0primmin = 0;
	float x0primmax = a;
	float y0primmin = -b;
	float y0primmax = b;
	float z0primmin = -csec;
	float z0primmax = cprim;

	// 2/6/2011 I make it integers in terms of deltax, deltay, deltaz
	x0primmin = std::floor(x0primmin / deltax) * deltax;
	x0primmax = std::ceil(x0primmax / deltax)  * deltax;

	y0primmin = std::floor(y0primmin / deltay) * deltay;
	y0primmax = std::ceil(y0primmax / deltay)  * deltay;

	z0primmin = std::floor(z0primmin / deltaz) * deltaz;
	z0primmax = std::ceil(z0primmax / deltaz)  * deltaz;    

	// Determination of splitting
	int Lx = int(std::ceil(std::log((x0primmax - x0primmin) / deltax) / std::log(2.0)));
	int Ly = int(std::ceil(std::log((x0primmax - y0primmin) / deltay) / std::log(2.0)));
	int Lz = int(std::ceil(std::log((z0primmax - z0primmin) / deltaz) / std::log(2.0)));

	std::vector<int> l;
	l.push_back(Lx);
	l.push_back(Ly);
	l.push_back(Lz);
	l.push_back(Lp);

	int L= *std::min_element(l.begin(), l.end());

	for (int i=0; i<L; i++)
	{
		NUMBER_NONLEAVES.push_back(0);  // non-air non-leafs
		NUMBER_LEAVES.push_back(0);     // leaf nodes in each level (except in the last level, where this is trickier)
		NUMBER_AIRLEAVES.push_back(0);  // leaf nodes in each level corresponding to air (those are NOT saved)
	}

	float Nx = float(std::pow(2.0, (Lx-L)));
	float Ny = float(std::pow(2.0, (Ly-L)));
	float Nz = float(std::pow(2.0, (Lz-L)));

	float N = Nx * Ny * Nz;

	// Boundaries of B0
    //ADM -- yhigh/low  and zihgh/low seem to be wrong -- CHECK
	float xlow0  = 0;
	float xhigh0 = Nx * float(pow(2, (double)L)) * deltax;
	float yhigh0 = float(std::ceil((y0primmax + y0primmin) / 2.0f / deltay) + Ny * std::pow(2.0, (L-1))) * deltay;
	float ylow0  = float(std::ceil((y0primmax + y0primmin) / 2.0f / deltay) - Ny * std::pow(2.0, (L-1))) * deltay;
	float zhigh0 = float(std::ceil((z0primmax + z0primmin) / 2.0f / deltaz) + Nz * std::pow(2.0, (L-1))) * deltaz;
	float zlow0  = float(std::ceil((z0primmax + z0primmin) / 2.0f / deltaz) - Nz * std::pow(2.0, (L-1))) * deltaz;

	// Here is little correction with ceil to make case of deltax=0.04 working
	// (when (c'+c'')/2deltax is not integer but (c'+c'')/deltax is)

	// For displaying progress
	//
	// volume=(xhigh0-xlow0)*(yhigh0-ylow0)*(zhigh0-zlow0); // whole volume at once!
	// take ONLY the part inside.

	volume = float(PI) / 3.0f * a * b * (cprim + csec);

	CURRENTPROGRESS   = -1 * volume / 100;  // Convert into absolute value of volume
	PROGRESSINCREMENT =  5 * volume / 100;  // Convert into absolute value of volume

	// Initialize phantom (this isn't really necessary, but it won't hurt anything, either)
	if (phantom != nullptr)
	{
		delete phantom;
		phantom = nullptr;
	}

    //  ADM  - DELETE THIS OR FIX

	size_t sizeX, sizeY, sizeZ;
	sizeX = static_cast<size_t>(round((x0primmax - x0primmin) / deltax));
	sizeY = static_cast<size_t>(round((y0primmax - y0primmin) / deltay));
	sizeZ = static_cast<size_t>(round((z0primmax - z0primmin) / deltaz));

	size_t total = sizeX * sizeY * sizeZ + 1;


	// This is now used by both the standalone program and the demo program
	phantom = new unsigned char[ total ]; // TODO: WHY ARE WE ALLOCATING MEMORY FOR TWO COPIES OF THE PHANTOM?!
	if (phantom == nullptr)
    { 
        std::cerr << __FILE__ << ", " << __LINE__ << "Could not allocate memory (" << total 
                  << " bytes) for phantom." << std::endl; 
        return false; 
    }
	else 
	{
		std::cout << "* Allocated a phantom of size " << sizeX << " x " << sizeY << " x " << sizeZ 
                  << " = " << total << " *\n" << std::endl;
        #if defined(LOG_ACTIVITY)
		std::ofstream ofs("../../../PHANTOM_CREATION.log");
		ofs << "* * * ALLOCATED A PHANTOM! * * *\n" << __FUNCTION__ << ", line " << __LINE__ << '\n';
		ofs.close();
        #endif
	}
	
    // ADM  End of delete

	xsize = static_cast<short>(std::round((x0primmax - x0primmin) / deltax));
	ysize = static_cast<short>(std::round((y0primmax - y0primmin) / deltay));
	zsize = static_cast<short>(std::round((z0primmax - z0primmin) / deltaz));

    #if defined(LOG_ACTIVITY)
	std::ofstream dbg("../../../Debug.log");
	dbg << __FUNCTION__ << ", line " << __LINE__ << ": About to initialize seed and distributional ellipsoids" << std::endl;
	dbg.close();
    #endif

	// Initialize seed and distributional ellipsoids
	
	initialize_distribution_parameters(number_distributions,a,b,
			                           cprim, csec, minspeed, maxspeed, minratio, maxratio,
			                           xlow0, xhigh0, ylow0, yhigh0, zhigh0, zlow0,
			                           mindistseeds, sigma, Percentage, distributionFileIn, distributionFileOut,  
			                           alpha1, beta1, alpha2, beta2, isGaussian);

    #if defined(LOG_ACTIVITY)
	dbg.open("../../../Debug.log", std::ofstream::app);
	dbg << __FUNCTION__ << ", line " << __LINE__ << ": About to SetupCLForVoxelParallelization" << std::endl;
	dbg.close();
    #endif

	bool success = SetupCLForVoxelParallelization(number_distributions);

	if (!success)
	{
		std::cerr << __FUNCTION__ << ": Attempt to set up CL for Voxel Parallelization failed.\n"
                  << "Terminating Program." << std::endl;
		std::time_t time_result = std::time(nullptr); // for logging (see below)
					
		#if defined(LOG_ACTIVITY)
		std::ofstream log("ERROR_LOG.log", std::ofstream::app);
		if (log)
		{
			log /*<< std::asctime(std::localtime(&time_result)) << ": " << __FUNCTION__ */
				<< __FUNCTION__ << ": Attempt to set up CL for Voxel Parallelization failed.\n"
                << "Terminating Program.\n";
			log.close();
		}
        #endif

		return false; // bail out, it's not going to success (hopefully prevent a crash) - DH 2016-02-16
	}

    #if defined(_MSC_VER)
	::OutputDebugStringA("start");
    #endif

	// Small box boundaries
	std::vector<float> xiarray;
	std::vector<float> yiarray;
	std::vector<float> ziarray;
    
    if (verbose)
    {
        std::cout << "xhigh0: " << xhigh0 << "\n";
        std::cout << "xlow0:  " << xlow0  << "\n";
        std::cout << "yhigh0: " << yhigh0 << "\n";
        std::cout << "ylow0:  " << ylow0  << "\n";
        std::cout << "zhigh0: " << zhigh0 << "\n";
        std::cout << "zlow0:  " << zlow0  << "\n";

        std::cout << "Nx: " << Nx << "\n";
        std::cout << "Ny: " << Ny << "\n";
        std::cout << "Nz: " << Nz << "\n";

        std::cout << std::endl;    
    }
    
    
    int   ndx   = 0;

    float val   = xlow0;
    float incr  = (xhigh0 - xlow0) / Nx;

    for (ndx = 0; ndx <= int(Nx); ++ndx)
    {
        xiarray.push_back(val);
        val += incr;
    }

    incr = (yhigh0 - ylow0) / Ny;
    val = ylow0;

    for (ndx = 0; ndx <= int(Ny); ++ndx)
    {
        yiarray.push_back(val);
        val += incr;
    }
    
    incr = (zhigh0 - zlow0) / Nz;
    val = zlow0;

    for (ndx = 0; ndx <= int(Nz); ++ndx)
    {
        ziarray.push_back(val);
        val += incr;
    }
	
    #if defined(LOG_ACTIVITY)
	dbg.open("../../../Debug.log", std::ofstream::app);
	dbg << __FUNCTION__ << ", line " << __LINE__ << ": About to begin nested loops" << std::endl;
	dbg.close();
    #endif

	size_t size = xiarray.size() *yiarray.size() *ziarray.size();
	std::vector<float> xhigh(size), xlow(size), ylow(size), yhigh(size), zhigh(size), zlow(size);
 
	int counterR = 0;
	for (int i1=0; i1<xiarray.size()-1; ++i1)
	{
		for (int i2=0; i2<yiarray.size()-1; ++i2)
		{
			for (int i3=0; i3<ziarray.size()-1; ++i3)
			{
				xlow[counterR]  = xiarray[i1];
				xhigh[counterR] = xiarray[i1+1];

				ylow[counterR]  = yiarray[i2];
				yhigh[counterR] = yiarray[i2+1];

				zlow[counterR]  = ziarray[i3];
				zhigh[counterR] = ziarray[i3+1];
				counterR = counterR + 1;
			}
		}
	}
    
    if (verbose)
    {
        std::cout << "countR: " << counterR << std::endl;
        std::cout << "xiarray.size(): " << xiarray.size() << std::endl;
        std::cout << "yiarray.size(): " << yiarray.size() << std::endl;
        std::cout << "ziarray.size(): " << ziarray.size() << std::endl;
    }

	std::vector<int> nodes_at_level;
	
    #if defined(LOG_ACTIVITY)
	dbg.open("../../../Debug.log", std::ofstream::app);
	dbg << __FUNCTION__ << ", line " << __LINE__ << ": About to declare OpenCL VECTOR_CLASS<cl::Memory> buffers" << std::endl;
	dbg.close();
    #endif
    
	VoxelDimension = 1<<L; // Choose arbitrary initial phantom dimensions (x/y/z)

	if (!voxelValues)
    {
        // One byte per voxel now when not using OpenGL Image type
        voxelValues = new unsigned char [VoxelDimension*VoxelDimension*VoxelDimension]; // DH 2020-08-18
        
        #if defined(LOG_ACTIVITY)
	    dbg.open("../../../Debug.log", std::ofstream::app);
	    dbg << __FUNCTION__ << ", line " << __LINE__ << ": L is " << L 
            << ", VoxelDimension is " << VoxelDimension 
            << "voxelValues is 0x" << std::hex << (long long) voxelValues << std::dec 
            << std::endl;
	    dbg.close();
        #endif       
    }    
    
    try
    {
        clCommandQueue.finish();
        texture = cl::Buffer(context, CL_MEM_READ_WRITE,
                             (size_t)(VoxelDimension * VoxelDimension * VoxelDimension * sizeof(cl_uchar)), // 1 byte per voxel
                             NULL, &err); // Added use of error flag to check for success
        
        // Check error flag in case exception-handling is turned off
        if (err != CL_SUCCESS)
        {
            std::cerr << __FUNCTION__ << ": Error creating cl::Buffer texture, error code: " 
                      << err << ", \"" << clib.getErrorString(err) << "\"" << std::endl;
        }
    }
    catch(cl::Error err) 
    {
        std::cerr << __FUNCTION__ << ": OpenCL function " << err.what() << ", error code " 
                  << err.err() << ", \"" << clib.getErrorString(err.err()) << "\"" 
                  << std::endl;      
        return false;
    }   
        
	for (int i=0; i<N; i++)
	{
		TOTALDISTR.clear();
		AVERAGEDISTR.clear();

		for (int j=0; j<L+1; j++)
		{
			TOTALDISTR.push_back(0); 
			AVERAGEDISTR.push_back(0);
		}

		if (!(xhigh[i]<=x0primmin || x0primmax<=xlow[i] || yhigh[i]<=y0primmin || y0primmax<=ylow[i] || zhigh[i]<=z0primmin || z0primmax<=zlow[i])) 	// ADM Check		
		{
			// The intersection of the region of interest and i-th box is not empty
			// Compute intersection of a ith small box,i.e. B'i.    
			// This is region of interest, minimum bounding rectangle of the phantom       
			float xiprimmin = std::max(xlow[i],  x0primmin);
			float xiprimmax = std::min(xhigh[i], x0primmax);
			float yiprimmin = std::max(ylow[i],  y0primmin);
			float yiprimmax = std::min(yhigh[i], y0primmax);
			float ziprimmin = std::max(zlow[i],  z0primmin);
			float ziprimmax = std::min(zhigh[i], z0primmax);

			// Init skin and voxels 3D array as uint8
			init_oct_tree_empty(xlow[i], xhigh[i], 
				                ylow[i], yhigh[i], 
								zlow[i], zhigh[i],
				                deltax/2-0.00001f,     // ADM  fix local vs global variable
								deltay/2-0.00001f, 
								deltaz/2-0.00001f,
				                a, b, cprim, csec, d, L); // go 1 level beyond, w/some slack(-0.00001)
            
			nodes_at_level.push_back(currentNodeCount);

			clCommandQueue.finish(); // sync point
			
			// Works with mask and skin, replaces label "4"  -skin that might be split (into Cooper ligaments & fat/dense tissue) into 2 (regular skin)

			// Store voxels from the region in the matrix of voxels for phantom


			// Indices of voxels in box Bi corresponding to the region of interest
			int Jixmin = static_cast<int>(round((xiprimmin - xlow[i]) / deltax));
			int Jixmax = static_cast<int>(round((xiprimmax - xlow[i]) / deltax)) - 1;

			int Jiymin = static_cast<int>(round((yiprimmin - ylow[i]) / deltay));
			int Jiymax = static_cast<int>(round((yiprimmax - ylow[i]) / deltay)) - 1;

			int Jizmin = static_cast<int>(round((ziprimmin - zlow[i]) / deltaz));
			int Jizmax = static_cast<int>(round((ziprimmax - zlow[i]) / deltaz)) - 1;


			// Indices of voxels corresponding to the phantom
			int J0xmin = static_cast<int>(std::round(Jixmin + (xlow[i] - x0primmin) / deltax));
			int J0xmax = static_cast<int>(std::round(Jixmax + (xlow[i] - x0primmin) / deltax));
			int J0ymin = static_cast<int>(std::round(Jiymin + (ylow[i] - y0primmin) / deltay));
			int J0ymax = static_cast<int>(std::round(Jiymax + (ylow[i] - y0primmin) / deltay));
			int J0zmin = static_cast<int>(std::round(Jizmin + (zlow[i] - z0primmin) / deltaz));
			int J0zmax = static_cast<int>(std::round(Jizmax + (zlow[i] - z0primmin) / deltaz));

			std::vector<int> origin;
            origin.push_back(0);
			origin.push_back(0);
			origin.push_back(0);            
            
			cl::size_t<3> region;
			region[0] = (J0xmax - J0xmin + 1);
			region[1] = (J0ymax - J0ymin + 1);
			region[2] = (J0zmax - J0zmin + 1);

			cl::size_t<3> origin2;
			origin2[0] = (0);
			origin2[1] = (0);
			origin2[2] = (0);

			cl::size_t<3> region2;

            region2[0] = (VoxelDimension);
			region2[1] = (VoxelDimension);
			region2[2] = (VoxelDimension);

            #if defined(LOG_ACTIVITY)
			dbg.open("../../../Debug.log", std::ofstream::app);
			dbg /*<< std::asctime(std::localtime(&time_result)) << ": " */<< __FUNCTION__ << ", line " << __LINE__ << ": About to enqueueWriteImage" << std::endl;
			dbg.close();
            #endif
            
            // Initialize - zero-out - the texture buffer
            cl_uchar ai = 0;
            clCommandQueue.enqueueFillBuffer(texture, ai, 0, VoxelDimension*VoxelDimension*VoxelDimension*sizeof(cl_uchar), nullptr, nullptr);
            
            #if defined(LOG_ACTIVITY)
			dbg.open("../../../Debug.log", std::ofstream::app);
			dbg /*<< std::asctime(std::localtime(&time_result)) << ": " */<< __FUNCTION__ << ", line " << __LINE__ << ": About to call recursive_data_split" << std::endl;
			dbg.close();
            #endif
            
			//recursive_data_split(L, Thickness/2, texture, origin);  // Keeps the Cooper ligaments with thickness of max(Thickness, 2*deltax).
            recursive_data_split(a, b, cprim, csec, L, Thickness/2, texture, origin); // DH 2019-07-02 
            
            try
            {
                // Read the phantom voxels back out from GPU texture memory (function has no error flag)
                size_t num_voxels = VoxelDimension * VoxelDimension * VoxelDimension;
                size_t buffer_size = num_voxels * sizeof(cl_uchar);
                err = clCommandQueue.enqueueReadBuffer(texture, true, 0, buffer_size, &voxelValues[0]);
            }
	        catch (cl::Error err)
            {
                std::cerr << __FUNCTION__ << ": OpenCL function " << err.what() << ", error code " 
                          << err.err() << ", \"" << clib.getErrorString(err.err()) << "\"" 
                          << ", origin is " << origin2[0] << ", " << origin2[1] << ", " << origin[2] 
                          << " and region is " << region2[0] << ", " << region2[1] << ", " << region2[2]
                          << ", voxelValues is 0x" << std::hex << (unsigned long long) voxelValues << std::dec << std::endl;
                
                return false;
            }
            
            
            #if defined(LOG_ACTIVITY)
			dbg.open("../../../Debug.log", std::ofstream::app);
			dbg /*<< std::asctime(std::localtime(&time_result)) << ": " */<< __FUNCTION__ << ", line " << __LINE__ << ": About to call clCommandQueue.flush();" << std::endl;
			dbg.close();
            #endif

			clCommandQueue.flush();
                
			unsigned long long index1 = 0L;
            unsigned long long index2 = 0L;
			for(long long z = 0L; z<=J0zmax-J0zmin; z++)    
			{
                for(long long y = 0L; y<=J0ymax-J0ymin; y++)
                {
                    for (long long x = 0L; x<=J0xmax-J0xmin; x++)
                    {
                        index1 = VoxelDimension * (VoxelDimension * (z+Jizmin) + y+Jiymin) + Jixmin+x;
                        index2 = xsize * (ysize * (z+J0zmin) + y+J0ymin) + J0xmin+x;
                        //std::cout << __FUNCTION__ << ", line " << __LINE__ << ": Index 1 is " << index1 << ", Index2 is " << index2 << std::endl;
                        unsigned char s = voxelValues[index1];
                        phantom[index2] = s;
                    }			
                }
			}	
		} 
        else
        {
            std::cerr << __FUNCTION__ << ": OUTSIDE OF VOLUME!!\n" << std::endl;
        }

		
        #if defined(LOG_ACTIVITY)
		dbg.open("../../../Debug.log", std::ofstream::app);
		dbg /*<< std::asctime(std::localtime(&time_result)) << ": " */<< __FUNCTION__ << ", line " << __LINE__ << ": About to push back TOTALDISTR AVERAGEDISTR and TOTALCOOPER" << std::endl;
		dbg.close();
        #endif
            
		TOTALDISTRALL.push_back(TOTALDISTR);
		AVERAGEDISTRALL.push_back(AVERAGEDISTR);
        
	} // for (int i=0; i<N; i++)

	
    #if defined(LOG_ACTIVITY)
	dbg.open("../../../Debug.log", std::ofstream::app);
	dbg /*<< std::asctime(std::localtime(&time_result)) << ": " */ << __FUNCTION__ << ", line " << __LINE__ << ": done with for (int i=0; i<N; i++)" << std::endl;
	dbg.close();
    #endif
    
	return success;
}



typedef CL_API_ENTRY cl_int (CL_API_CALL *P1) (const cl_context_properties *properties,
											   cl_gl_context_info param_name,
											   size_t param_value_size,
											   void *param_value,
											   size_t *param_value_size_ret);

CL_API_ENTRY cl_int (CL_API_CALL *myclGetGLContextInfoKHR)(const cl_context_properties *properties,
														   cl_gl_context_info param_name,
														   size_t param_value_size,
														   void *param_value,
														   size_t *param_value_size_ret) = NULL;



