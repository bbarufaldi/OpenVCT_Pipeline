// BreastPhantomGenerator.h

#pragma once

#include <vector>
#include <algorithm>
#include "vector3.h"
#include "VCT_Random.h"

#define CL_TARGET_OPENCL_VERSION 120
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_SIZE_T_COMPATIBILITY
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#if defined(__APPLE__)
#include <OpenCL/cl2.hpp>
#else
#if defined __has_include
   #if __has_include (<CL/cl2.hpp>)
      #include <CL/cl2.hpp>
   #else
      #define USING_CL_HPP
      #define __CL_ENABLE_EXCEPTIONS
      #include<vector>
      #include<CL/cl.hpp>
   #endif
#else
   #define USING_CL_HPP
   #define __CL_ENABLE_EXCEPTIONS
   #include<vector>
   #include<CL/cl.hpp>
#endif
#endif


///<summary>Breast phantom simulator.</summary>
///<remarks>Joe Chui, 10/10/2011.</remarks>
class BreastPhantomGenerator
{
public:

   ///<summary>Initializes states(default constructor).</summary>
   ///<remarks>D. Higginbotham, 2016-03-01.</remarks>
   BreastPhantomGenerator();  


   ///<summary>Frees allocated storage upon exit (destructor).</summary>
   ///<remarks>D. Higginbotham, 2016-03-01.</remarks>
   ~BreastPhantomGenerator();
   
   
   ///<summary>Retrieve the phantom voxel dimensions.</summary>
   ///<remarks>D. Higginbotham, 2017-05-01.</remarks>
   ///<param name="xdim">Number of voxels in x direction.</param>
   ///<param name="ydim">Number of voxels in y direction.</param>
   ///<param name="zdim">Number of voxels in z direction.</param>
   void getVoxelDimensions(short &xdim, short &ydim, short &zdim)
   {
        xdim = xsize;
        ydim = ysize;
        zdim = zsize;
   }
   
   
   ///<summary>Access the phantom voxels (retrieve pointer).</summary>
   ///<remarks>D. Higginbotham, 2017-05-01.</remarks>
   ///<return>Pointer to voxels.</return>
   unsigned char *getVoxelPointer()
   {
        return phantom;
   }
   
   ///<summary>Enable storage of Random Numbers this program generates.</summary>
   ///<remarks>D. Higginbotham, 2020-03-01.</remarks>
   void setRandomStore()
   {
        (void) vct_rand.setRandStoreFile("Generator_random_numbers.dat");
   }
   
   
   ///<summary>Enable retrieval of Random Numbers previously generated.</summary>
   ///<remarks>D. Higginbotham, 2020-03-01.</remarks>
   void setRandomRetrieval()
   {
       (void) vct_rand.setRandRetrieveFile("Generator_random_numbers.dat"); 
   }
   
   
   ///<summary>Set verbose mode.</summary>
   ///<remarks>D. Higginbotham, 2020-08-20.</remarks>
   ///<param name="state">The boolean state to set.</param>
   void setVerbose(bool state)
   {
       verbose = state;
   }
   
   
   ///<summary>Retrieve current verbose mode state.</summary>
   ///<remarks>D. Higginbotham, 2020-03-01.</remarks>
   ///<return>The boolean state to the verbose flag.</return>
   bool getVerbose()
   {
       return verbose;
   }
   
   
   ///<summary>Creates the (uncompressed) phantom.</summary>
   ///<remarks>D. Higginbotham, 2016-03-01.</remarks>
   ///<param name="a">Phantom ellipse x size parameter (cm).</param>
   ///<param name="b">Phantom ellipse y size parameter (cm).</param>
   ///<param name="cprim">Phantom larger ellipse z size parameter (cm).</param>
   ///<param name="csec">Phantom smaller ellipse z size parameter (cm).</param>
   ///<param name="deltax">Voxel size (cm).</param>
   ///<param name="number_distributions">Essentially the number of compartments.</param>
   ///<param name="mindistseeds">Minimum distance between seeds (cm).</param>
   ///<param name="minspeed">Minimum speed of (seed) growth.</param>
   ///<param name="maxspeed">Maximum speed of (seed) growth.</param>
   ///<param name="minratio">Minimal ratio of distribution ellipse halfaxis.</param>
   ///<param name="maxratio">Maximal ratio of distribution ellipse halfaxis.</param>
   ///<param name="Lp">Max level of the tree that can fit in memory, rule of thumb: 1GB--Lp=8, 4GB--Lp=9; 16GB--Lp=10; 64GB--Lp=11.</param>
   ///<param name="Thickness">Skin thickness (cm).</param>
   ///<param name="sigma">Gaussian coefficient for probability distribution (of dense compartments).</param>
   ///<param name="Percentage">Percentage of compartments that are dense.</param>
   ///<param name="textureid">OpenGL identifier of 3D Texture reserved for this phantom.</param>
   ///<param name="alpha1">p1 of beta distribution.</param>
   ///<param name="beta1">q1 of beta distribution.</param>
   ///<param name="alpha2">p2 of beta distribution.</param>
   ///<param name="beta2">q2 of beta distribution.</param>
   ///<param name="outputFile">Name of distribution file to load (or null).</param>
   ///<param name="distributionFileIn">Name of distribution file to load (or null).</param>
   ///<param name="distributionFileOut">Name of distribution file to save (or null).</param>
   ///<param name="isGaussian">Whether distribution algorithm is gaussian or beta.</param>
   ///<return>boolean indication of success (true) or failure (false) </return>
   virtual bool recursive_partition_simulation(
					   float a, float b, float d, float cprim, float csec,
					   float deltax, int number_distributions, float mindistseeds,
					   float minspeed, float maxspeed, float minratio, float maxratio,
					   int Lp, float Thickness, float sigma, float Percentage, unsigned int textureid, // textureid is passed in as visualizer->textureID
					   float alpha1, float beta1, float alpha2, float beta2,
					   std::string outputFile, std::string distributionFileIn, std::string distributionFileOut,
					   bool isGaussian);
                       

   ///<summary>Set the output filename.</summary>
   ///<remarks>D. Higginbotham, 2017-02-21.</remarks>
   ///<param name="name">Phantom outputfilename.</param>
   void setOutputFileName(std::string name) { outputFileName = name; }
                       

   ///<summary>Set the phantom shape.</summary>
   ///<remarks>D. Higginbotham, 2020-11-25.</remarks>
   ///<param name="shape">Phantom shape.</param>
   void setPhantomShape(std::string shape)
   {
      std::transform(shape.begin(), shape.end(), shape.begin(), toupper); // convert to upper case
      if      (shape == "BREAST") phantom_shape = 0; // Breast
      else if (shape == "CUBE")   phantom_shape = 1; // Cube;
   }


protected:

    float dblRand();

	///<summary>Create a matrix representing ellipsoid orientation, given parameters.</summary>
	///<remarks>D. Higginbotham, 2016-03-01.</remarks>
	///<param name="Origin">Vector of origin coordinates (cm).</param>
	///<param name="a">Upper phantom ellipsoid dimension (cm).</param>
	///<param name="bu">Upper phantom ellipsoid dimension (cm).</param>
	///<param name="bd">Lower phantom ellipsoid dimension (cm).</param>
	///<param name="c">Lower phantom ellipsoid dimension (cm).</param>
	///<param name="pts">Vector of ellipsoid position (from distribution param file.</param>
	///<returns>A matrix with the ellipsoid's orientation.</returns>
	matrix EllipsoidOrientation(rowVector Origin, float a, float bu, float bd, float c, rowVector pts);


	///<summary>Initialize the phantom generation distribution parameters.</summary>
	///<remarks>D. Higginbotham, 2016-03-01.</remarks>
	///<param name="number_distributions">Essentially the number of compartments.</param>
	///<param name="a">Phantom ellipse x size parameter (cm).</param>
	///<param name="b">Phantom ellipse y size parameter (cm).</param>
	///<param name="cprim">Phantom larger ellipse z size parameter (cm).</param>
	///<param name="csec">Phantom smaller ellipse z size parameter (cm).</param>
	///<param name="minspeed">Minimum speed of (seed) growth.</param>
	///<param name="maxspeed">Maximum speed of (seed) growth.</param>
	///<param name="minratio">Minimal ratio of distribution ellipse halfaxis.</param>
	///<param name="maxratio">Maximal ratio of distribution ellipse halfaxis.</param>
	///<param name="xlow0">Boundary of BO.</param>
	///<param name="xhigh0">Boundary of BO'.</param>
	///<param name="ylow0">Boundary of BO.</param>
	///<param name="yhigh0">Boundary of BO'.</param>
	///<param name="zhigh0">Boundary of BO'.</param>
	///<param name="zlow0">Boundary of BO.</param>
	///<param name="mindistseeds">Minimum distance between seeds (cm).</param>
	///<param name="sigma">Gaussian coefficient for probability distribution (of dense compartments).</param>
	///<param name="Percentage">Percentage of compartments that are dense.</param>
	///<param name="DistributionLoadFile">Name of distribution file to load (or null).</param>
	///<param name="DistributionSaveFile">Name of distribution file to save (or null).</param>
	///<param name="alpha1">p1 of beta distribution.</param>
	///<param name="beta1">q1 of beta distribution.</param>
	///<param name="alpha2">p2 of beta distribution.</param>
	///<param name="beta2">q2 of beta distribution.</param>
	///<param name="isGaussian">Whether distribution algorithm is gaussian or beta.</param>
	void initialize_distribution_parameters(int number_distributions,
													float a, float b,
													float cprim, float csec,
													float minspeed, float maxspeed,
													float minratio, float maxratio,
													float xlow0, float xhigh0,
													float ylow0, float yhigh0,
													float zhigh0, float zlow0,
													float mindistseeds, float sigma,
													float Percentage,
													std::wstring DistributionLoadFile,
													std::wstring DistributionSaveFile,
													float alpha1, float beta1,
													float alpha2, float beta2,
													bool isGaussian);


	///<summary>Determines the probability of a compartment being dense.</summary>
	///<remarks>D. Higginbotham, 2016-03-01.</remarks>
	///<param name="point">Coordinate of a point in the compartment (its center).</param>
	///<param name="a">Phantom ellipse x size parameter (cm).</param>
	///<param name="b">Phantom ellipse y size parameter (cm).</param>
	///<param name="cprim">Phantom larger ellipse z size parameter (cm).</param>
	///<param name="csec">Phantom smaller ellipse z size parameter (cm).</param>
	///<param name="sigma">Gaussian coefficient for probability distribution (of dense compartments).</param>
	///<returns>The probability that the specified compartment is dense tissue.</returns>
	float probability_dense(rowVector point,
							float a, float b,
							float cprim, float csec,
							float sigma);


	///<summary>Determines the probability of a compartment being dense.</summary>
	///<remarks>D. Higginbotham, 2016-03-01.</remarks>
	///<param name="point">Coordinate of a point in the compartment (its center).</param>
	///<param name="a">Phantom ellipse x size parameter (cm).</param>
	///<param name="b">Phantom ellipse y size parameter (cm).</param>
	///<param name="cprim">Phantom larger ellipse z size parameter (cm).</param>
	///<param name="csec">Phantom smaller ellipse z size parameter (cm).</param>
	///<param name="sigma">Gaussian coefficient for probability distribution (of dense compartments).</param>
	///<param name="alpha1">p1 of beta distribution.</param>
	///<param name="beta1">q1 of beta distribution.</param>
	///<param name="alpha2">p2 of beta distribution.</param>
	///<param name="beta2">q2 of beta distribution.</param>
	///<returns>The probability that the specified compartment is dense tissue.</returns>
	float probability_denseBeta(rowVector point,
								float a, float b,
								float cprim, float csec, float sigma,
								float alpha1, float beta1,
								float alpha2, float beta2);


	///<summary>Create voxels at the last level (of recursion).</summary>
	///<remarks>D. Higginbotham, 2016-03-01.</remarks>
	void create_voxels_last_level();


	///<summary>Initialize the phantom generation distribution parameters.</summary>
	///<remarks>D. Higginbotham, 2016-03-01.</remarks>
	///<param name="number_distributions">Essentially the number of compartments.</param>
	///<param name="a">Phantom ellipse x size parameter (cm).</param>
	///<param name="b">Phantom ellipse y size parameter (cm).</param>
	///<param name="cprim">Phantom larger ellipse z size parameter (cm).</param>
	///<param name="csec">Phantom smaller ellipse z size parameter (cm).</param>
	///<param name="minspeed">Minimum speed of (seed) growth.</param>
	///<param name="maxspeed">Maximum speed of (seed) growth.</param>
	///<param name="minratio">Minimal ratio of distribution ellipse halfaxis.</param>
	///<param name="maxratio">Maximal ratio of distribution ellipse halfaxis.</param>
	///<param name="xlow0">Boundary of BO.</param>
	///<param name="xhigh0">Boundary of BO'.</param>
	///<param name="ylow0">Boundary of BO.</param>
	///<param name="yhigh0">Boundary of BO'.</param>
	///<param name="zhigh0">Boundary of BO'.</param>
	///<param name="zlow0">Boundary of BO.</param>
	///<param name="mindistseeds">Minimum distance between seeds (cm).</param>
	///<param name="sigma">Gaussian coefficient for probability distribution (of dense compartments).</param>
	///<param name="Percentage">Percentage of compartments that are dense.</param>
	///<param name="DistributionLoadFile">Name of distribution file to load (or null).</param>
	///<param name="DistributionSaveFile">Name of distribution file to save (or null).</param>
	///<param name="alpha1">p1 of beta distribution.</param>
	///<param name="beta1">q1 of beta distribution.</param>
	///<param name="alpha2">p2 of beta distribution.</param>
	///<param name="beta2">q2 of beta distribution.</param>
	///<param name="isGaussian">Whether using gaussian model or not.</param>
	void initialize_distribution_parameters(int number_distributions,
												  float a, float b,
												  float cprim, float csec,
												  float minspeed, float maxspeed,
												  float minratio, float maxratio,
												  float xlow0, float xhigh0,
												  float ylow0, float yhigh0,
												  float zhigh0, float zlow0,
												  float mindistseeds, float sigma, float Percentage,
												  std::string DistributionLoadFile,
												  std::string DistributionSaveFile,
												  float alpha1, float beta1,
												  float alpha2, float beta2,
												  bool isGaussian);


	///<summary>Initialize the octree structure that will represent the phantom.</summary>
	///<remarks>D. Higginbotham, 2016-03-01.</remarks>
	///<param name="xlow0">Boundary of BO.</param>
	///<param name="xhigh0">Boundary of BO'.</param>
	///<param name="ylow0">Boundary of BO.</param>
	///<param name="yhigh0">Boundary of BO'.</param>
	///<param name="zhigh0">Boundary of BO'.</param>
	///<param name="zlow0">Boundary of BO.</param>
	///<param name="xminsize">Minimum X size this compartment.</param>
	///<param name="yminsize">Minimum Y size this compartment.</param>
	///<param name="zminsize">Minimum Z size this compartment.</param>
	///<param name="a">Phantom ellipse x size parameter (cm).</param>
	///<param name="b">Phantom ellipse y size parameter (cm).</param>
	///<param name="cprim">Phantom larger ellipse z size parameter (cm).</param>
	///<param name="csec">Phantom smaller ellipse z size parameter (cm).</param>
	///<param name="d">Skin depth (cm).</param>
	///<param name="max_depth">Maximum depth of the octree data structure (recusion limit).</param>
	void init_oct_tree_empty( float xlow, float xhigh,
							  float ylow, float yhigh,
						   	  float zlow, float zhigh,
							  float xminsize,
							  float yminsize,
							  float zminsize,
							  float a, float b,
							  float cprim,
							  float csec, float d,
							  unsigned char max_depth);


	///<summary>Divides an Octree Node.</summary>
	///<remarks>D. Higginbotham, 2016-03-01.</remarks>
	///<param name="width">Viewport width (in pixels).</param>
	///<param name="height">Viewport height (in pixels).</param>
	///<param name="height">Viewport height (in pixels).</param>
	void recursive_data_split(float a, float b, float cprim, float csec, // DH 2019-07-02
                              int max_depth, float Dmin,
                              ////cl::Image3D & buffer,
                              cl::Buffer &buffer,
		                      std::vector<int>origin);


	///<summary>Sets up the cl contexts for voxel parallelization.</summary>
	///<remarks>Joe Chui, 10/10/2011.</remarks>
	///<param name="clDistributionCount">Number of cl distributions.</param>
	bool SetupCLForVoxelParallelization(int dCount);


private:

   unsigned int phantom_shape;
  
   int   currentNodeCount;
   float deltaX; 
   float squaredDeltaX;

   size_t xmemorySize[2], distributionSize[2];
   std::vector<std::vector<short>>distributionPools;
   std::vector<cl_uchar>labels; 
   std::vector<cl_float2>NodeX, NodeY, NodeZ; 

   std::vector<cl_uchar>toSplits; 

   cl::Buffer Xextents, Yextents, Zextents; // min/max phantom dimension extents DH 2019-07-02
   cl::Buffer xMemoryObject[2], yMemoryObject[2], zMemoryObject[2];
   cl::Buffer toSplitMemoryObject[2];
   cl::Buffer LabelMemoryObject[2];
   cl::Buffer distributionCountMemoryObject[2];
   cl::Buffer distributionMemoryObject[2];
   cl::Buffer headsMemoryObject[2];
   
   cl::Buffer totaldistributionCountmem;
   cl::Buffer toSplitCountMemoryObject;
   cl::Buffer voxelizeCountMemoryObject;

   float voxelHalfSize;
   cl::Buffer totalDistributionCountMemoryObject;

   std::vector<float>xLows; 
   std::vector<float>xHighs; 
   std::vector<float>yLows; 

   std::vector<float>yHighs; 
   std::vector<float>zLows; 
   std::vector<float>zHighs; 

   cl_int countbuffer[30];

   std::vector<float>AVERAGEDISTR;
   std::vector<std::vector<float>>AVERAGEDISTRALL;

   float CURRENTPROGRESS;
   int DEPTH;
   int dimensionality;

   // Distribution vectors of matrices
   std::vector<matrix>distributionA1InverseMatrices; 
   std::vector<matrix>distributionA2InverseMatrices;
   std::vector<matrix>distributionA3InverseMatrices;
   std::vector<columnVector8>distributionHMatrices;
   std::vector<matrix>distributionSigmaInverseMatrices;
   std::vector<columnVector>distributionSigmaInverseMUVectors;
   std::vector<int>distributionLabels;
   std::vector<float>distributionLogSqrtSigmaLogPriors;
   std::vector<rowVector>distributionMUVectors;
   std::vector<float>distributionPriors;
   std::vector<matrix>distributionRMatrices;
   std::vector<matrix8x3>distributionUMatrices;

   float maskAInverseSquared, maskBInverseSquared;
   float maskCPrimInverseSquared, maskCSecInverseSquared;

   float newDeltaX;         
   float newDeltaXSquared;

   float newVoxelHalfSize;
   
   int mDistributionCount;
   std::vector<int>NUMBER_AIRLEAVES;
   std::vector<int>NUMBER_LEAVES;
   std::vector<int>NUMBER_NONLEAVES;

   float  PROGRESSINCREMENT;

   float  skinAInverseSquared;
   float  SkinBInverseSquared;
   float  skinCPrimInverseSquared;
   float  skinCSecInverseSquared;
   int    bookKeepingBufferSize, voxelizeBufferSize, newVoxelizeCount;


   // TOTAL_DEPTH
   std::vector<int>TOTALDISTR;
   std::vector<std::vector<int>>TOTALDISTRALL;

   float  volume;

   cl_int VoxelDimension;
   columnVector voxelResolution;
   unsigned char  *voxelValues;

   float  xLow; 
   float  xHigh; 
   float  xMinSize;

   float  yLow;
   float  yHigh; 
   float  yMinSize;

   float  zLow;
   float  zHigh;
   float  zMinSize;

   cl::Kernel  splitKernel;
   cl::Kernel  createNodesKernel;
   cl::Kernel  createNodesKernel2;
   cl::Kernel  createLastLevelVoxelsKernel;
   cl::Kernel  addupKernel;
   cl::Kernel  wsAddupKernel;
   cl::Kernel  evalKernel;
   cl::Kernel  voxelizeKernel;

   cl::Context      context;
   cl::CommandQueue clCommandQueue;

   cl::Buffer  mubuffer,  rbuffer;
   cl::Buffer  logbuffer, bbbuffer;
   cl::Buffer  a1buffer,  a2buffer,  a3buffer;
   cl::Buffer  hbuffer,   u1buffer;
   cl::Buffer  u2buffer,  u3buffer; 

   cl::Buffer  sigmainvbuffer;
   cl::Buffer  labelbuffer;
   cl::Buffer  voxelizeBuffer;
   cl::Buffer  distanceBuffer;
   cl::Buffer  nodeBuffer;

   cl_float2 *compressedBuffer;

   cl::Buffer texture;


   size_t totalNodeCount;
   short  xsize, ysize, zsize;
   unsigned char *phantom;

   std::vector<cl::Device>devices;   

   std::string outputFileName;
   
   long long  t1;

   // Random number store and retrieval
   vct::Random vct_rand;
   
   bool  verbose;
};
