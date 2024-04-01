// SimulationOCL.cpp

#if !defined(NO_OPENCL)

// NOTE! kernel program file, tracing_*.cl must reside in the directory where you are running

//#define USE_CPU_PHYSICS
#if defined(USE_CPU_PHYSICS)
#include "physics_cpu.h"
#endif

#include "SimulationOCL.h"
#include "Image.h"
#include "Ray.h"
#include "RayTracer.h"
#include "BooneMaterialTable.h"
#include "LabelMap.h" // for Material
#include "XRayTube.h" // for XRayTube
#include <random>
#include <cfloat>
#include "VCT_CommonLibrary.h"

#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

// Use deprecated cl.hpp if cl2.hpp cannot be found
#if defined __has_include
   #if __has_include (<CL/cl2.hpp>)
      #include <CL/cl2.hpp>
   #elif __has_include (<CL/cl.hpp>)
      #include <vector>
      #define USING_CL_HPP
      #define __CL_ENABLE_EXCEPTIONS
      #include <CL/cl.hpp>
   #endif
#else
   #include <vector>
   #define USING_CL_HPP
   #define __CL_ENABLE_EXCEPTIONS
   #include <CL/cl.hpp>
#endif


#include <fstream> // for DEBUG
#include <sstream> // for DEBUG

#include "dump.h" // 2018-11-16 DH *

#include "ProgressBar.h"
extern ProgressBar progressBar;



// It is critically important that these properly indicate their paths relative to the CWD.
// ..for example, these settings presume you are running from the XPLProjectionSimulation folder:

const char kernel_source_file[] = {"tracing_float.cl"};
const char kernel_include_dir[] = {"-I ."};

/// <summary>
/// Initializes a new instance of the <see cref="Simulation"/> class.
/// </summary>
/// <param name="seed">Random Number seed.</param>
SimulationOCL::SimulationOCL(std::string seed)
    : Simulation(), mSeed(seed), openCLProgramSource(nullptr)
{
    //Note: This constructor is the one being invoked 2018-12-01 DH *
    //std::cout << __FUNCTION__ << ", line " << __LINE__ << ": Prop_Const is " << propConst << std::endl;
}

/// <summary>
/// Initializes a new instance of the <see cref="Simulation"/> class.
/// </summary>
/// <param name="xmlPathName">Name of the configuration XML path.</param>
/// <param name="attenuationPathName">Name of the attenuation path.</param>
/// <param name="filePath">The file path.</param>
/// <param name="seed">Random Number seed.</param>
SimulationOCL::SimulationOCL(std::string xmlPathName, std::string attenuationPathName, std::string filePath, std::string seed) :
   Simulation(xmlPathName, attenuationPathName, filePath), mSeed(seed), openCLProgramSource(nullptr)
{}


/// <summary>
/// Finalizes an instance of the <see cref="Simulation"/> class.
/// </summary>
SimulationOCL::~SimulationOCL()
{
    //std::cout << "IN SimulationOCL DESTRUCTOR" << std::endl;
    if (openCLProgramSource != nullptr)
    {
        delete [] openCLProgramSource;
        openCLProgramSource = nullptr;
    }
}


// Note: This reads a whole binary file into a NULL-terminated character array.
// ----  It is used to read in the OpenCL program file (not to read a phantom).
//       There is nothing magic about this routine; it doesn't belong to a SDK.
//       It appears that Joe was building an SDK and this was going to be in it.
const char *SimulationOCL::Fileopen(const char* fileName)
{
    size_t  size;

    // Open file stream
    std::fstream ifs(fileName);

    // Check if we have opened file stream
    if (ifs.is_open())
    {
        size_t  sizeFile;

        // Find the stream size
        ifs.seekg(0, std::fstream::end);
        size = sizeFile = (size_t)ifs.tellg();
        ifs.seekg(0, std::fstream::beg);

        if (openCLProgramSource != nullptr) delete [] openCLProgramSource;
        openCLProgramSource = new char[size + 1];
        if (!openCLProgramSource)
        {
            ifs.close();
            return nullptr;
        }

        // Read file
        ifs.read(openCLProgramSource, sizeFile);
        ifs.close();
        openCLProgramSource[size] = '\0';

        return openCLProgramSource;
    }

    return nullptr;
}


// Combine the application of noise and generation of the image, both outside of
// ..OpenCL (C++ std poisson distribution not available to OpenCL).
void buildImage(unsigned int j, unsigned int elCntX,
                std::vector<cl_float> &signal, std::vector<cl_float> &invSq,
                std::mt19937 &gen, std::poisson_distribution<> &d2,
                bool useNoise, bool useScaling, bool useInverseSq,
                double gain, double normAirKerma,
                double mAs, double propConst,
                Image<double> *outImage)
{
   double a1, a2;
   double d1_init;
   double invSqAttn = 1.0;

   for (unsigned int i = 0; i<elCntX; i++)
   {
      if (useInverseSq)
      {
          invSqAttn = invSq[i];
      }

      if (useNoise)
      {
          if (useScaling)
          {
              d1_init = signal[i] * normAirKerma * invSqAttn;
          }
          else
          {
              d1_init = signal[i] * mAs * propConst * invSqAttn;
          }

          // Note: poisson_distribution is not available in OpenCL so noise is generated here
          std::poisson_distribution<> d1(d1_init);       // mean is averageValue*normAirKerma
          a1 = d1(gen) * gain;                           // gain noise (poisson) applied to normAirKerma and averageValue
          a2 = d2(gen);                                  // electric noise (poisson)
          outImage->setPixel(i, j, (a1+a2)); // add noise to the image (base electric noise + k*gain)
      }
      else
      {
         if (useScaling)
         {
             outImage->setPixel(i, j, signal[i] * normAirKerma * gain * invSqAttn);
         }
         else
         {
             outImage->setPixel(i, j, signal[i] * mAs * gain * propConst * invSqAttn);
         }
      }
   }
}


/// <summary>
/// Processes the specified acq ID.
/// </summary>
/// <param name="acqID">The acq ID.</param>
/// <param name="id">The id.</param>
/// <param name="outImage">The out image.</param>
bool SimulationOCL::process(signed short acqID, int id, Image<double> *outImage)
{
    bool status = true; // return value

   // Setup global CL stuff
   vct::CommonLibrary clib;

   /*std::vector<cl::Platform> platform;
   std::vector<cl::Device> devices;

   cl::Context      context;
   cl::CommandQueue clCommandQueue;
   cl::Kernel       traceKernel, physicsKernel;
   cl::Device       dev;

   try
   {
       cl::Platform::get(&platform); // <-- Dies here on Intel 55000
       std::string cardVendor;
       
       
       // Prefer NVIDIA for processing unless unavailable
       bool nvidiaFound = false;
       std::cout << "platform size" << platform.size() << std::endl;
       for(int i=0; i<platform.size(); i++)
       {
           cardVendor = platform[i].getInfo<CL_PLATFORM_VENDOR>();
           if (cardVendor.find("NVIDIA") != std::string::npos)
           {
               platform[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
               for(int d=0; d<devices.size(); d++)
               {
                  if (devices[d].getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU)
                  {
                     nvidiaFound = true;
                     dev = devices[d];
                     context = cl::Context(devices[d]);
                     break;
                  }
               }
           }
       }

       if (!nvidiaFound)
       {
           // Try again and accept any Vendor with a GPU
           for(int i=0; i<platform.size(); i++)
           {
               cardVendor = platform[i].getInfo<CL_PLATFORM_VENDOR>();
               platform[i].getDevices(CL_DEVICE_TYPE_ALL, &devices);
               for(int d=0; d<devices.size(); d++)
               {
                   if (devices[d].getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_GPU)
                   {
                       dev = devices[d];
                       context = cl::Context(devices[d]);
                       break;
                   }
               }
           }
       }

       if (context() == NULL)
       {
          std::cout << "A valid GPU device is not found, failing back to CPU." << std::endl;
          for(int i=0; i<platform.size(); i++)
          {
             platform[i].getDevices(CL_DEVICE_TYPE_ALL, &devices); // .getInfo<CL_CONTEXT_DEVICES>();
             for(int d=0; d<devices.size(); d++)
             {
                if (devices[d].getInfo<CL_DEVICE_TYPE>() == CL_DEVICE_TYPE_CPU)
                {
                   dev = devices[d];
                   context = cl::Context(devices[d]);
                   break;
                }
             }
          }
       }
   }
   catch(cl::Error const err)
   {
       std::cerr << "\n\nError in " << __FUNCTION__ << "(): OpenCL function "
                 << err.what() << ", error code "
                 << err.err() << ", \"" << clib.getErrorString(err.err()) << "\""
                 << std::endl;
       status = false;
       return status;
   }*/

   cl::Context      context;
   cl::CommandQueue clCommandQueue;
   cl::Kernel       traceKernel, physicsKernel;
   cl::Device       dev;

   try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms); //error here! it is not getting platform
        if (platforms.empty()) {
            std::cout << "Error getting platforms." << std::endl;
            return -1;
        }
        cl::Platform platform = platforms.front();

        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_DEFAULT, &devices);
        if (devices.empty()) {
            std::cout << "Error getting devices." << std::endl;
            return -1;
        }
        dev = devices.front();
        context = cl::Context(dev);
       
    } catch (cl::Error const err) {
        std::cout << "OpenCL error: " << err.what() << ", " << err.err() << ", "<< clib.getErrorString(err.err()) << std::endl;
        return -1;
    }


   // Initialize Poisson Distribution
   std::seed_seq seed(mSeed.begin(), mSeed.end());
   std::mt19937  gen(seed);                          // mersenne_twister
   std::poisson_distribution<> d2(elecNoise);

   // Set up the CommandQueue and kernel program arguments

   // For flexible exception handling messages
   #define build_string(expr) (static_cast<std::ostringstream*>(&(std::ostringstream().flush() << expr))->str())
   
   cl_int err = 0;

   try
   {
      // Create the command queue
      clCommandQueue = cl::CommandQueue(context, dev, 0, &err);
      if (err != CL_SUCCESS) throw build_string("failed creating a command queue: " << err);

      // Open and read the file containing kernel programs' (noise, trace & physics) source code
      if (openCLProgramSource == nullptr)
      {
          const char *code = Fileopen(kernel_source_file);
          if (code == nullptr) throw build_string(std::string("failed opening ") + kernel_source_file);
      }

      // Place the body of source code into a vector
      #if defined(USING_CL_HPP)
      cl::Program::Sources sources(1, std::make_pair(openCLProgramSource, 0));
      #else
      cl::Program::Sources sources(1, std::string(openCLProgramSource));
      #endif

      // Perform runtime source compilation, and obtain splitKernel entry point.

      // Instantiate the cl program, with context source code (and error flag reference)
      cl::Program program(context, sources, &err);
      if (err != CL_SUCCESS) throw build_string("line " << __LINE__ << ": instantiating cl program: " << err);

      //std::cout << __LINE__ << ": About to build program" << std::endl;
      clCommandQueue.finish();

      // Build (compile and link) the cl program, specifying where it can find its include files
      try
      {
          program.build(dev);
      }
      catch (cl::Error& e)
      {
          if (e.err() == CL_BUILD_PROGRAM_FAILURE)
          {
              //for (cl::Device dev : devices)
              //{
                  // Check the build status
                  cl_build_status status = program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
                  if (status != CL_BUILD_ERROR)
                      std::cout << "Continue" << std::endl;

                  // Get the build log
                  std::string name = dev.getInfo<CL_DEVICE_NAME>();
                  std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
                  std::cout << "Build log for " << name << ":" << std::endl
                      << buildlog << std::endl;
              //}
          }
      }
     // exit(-1);

      // Identify the kernel programs contained in the kernel source code file
      traceKernel   = cl::Kernel(program, "trace");
      physicsKernel = cl::Kernel(program, "physics");

      //
      // Setup raytrace
      //

      size_t total = size_t(volume->voxelCount[0]) * size_t(volume->voxelCount[1]) * size_t(volume->voxelCount[2]) * sizeof(cl_uchar);
      cl::Buffer volumeBuffer = cl::Buffer (context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,//|CL_MEM_COPY_HOST_PTR,
                                            total,
                                            volume->voxels,
                                            &err);

      if (err != CL_SUCCESS) throw build_string("Creating volumeBuffer, line " << __LINE__ << ": volumeBuffer err value is: " << err);

      int numMatLbls = static_cast<int>(volume->labelMap.size());    // number of material labels
      int numMaterials = volume->GetNumMaterials();                  // number of materials specified (not necessarily unique)


      cl::Buffer lengthBuffer = cl::Buffer(context, CL_MEM_READ_WRITE,
                                           machine->getDetector()->elementCountX * numMatLbls * sizeof(cl_float) * 4, //{6} // *4 : 4 sides of element
                                           NULL,
                                           &err);

      if (err != CL_SUCCESS) throw build_string("Creating lengthBuffer, line " << __LINE__ << ": lengthBuffer err value is: " << err);


      std::vector<cl_float> invSq(machine->getDetector()->elementCountX);
      cl::Buffer invSquaredBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  invSq.size()*sizeof(cl_float), &invSq.at(0), &err);

      if (err != CL_SUCCESS) throw build_string("Creating invSquaredBuffer, line " << __LINE__ << ": invSquaredBuffer err value is: " << err);


      //BB and TV 05/01/2018
      double detectorPositionX = machine->acquisitionGeometry.at(acqID).detectorPosition[0] + ((machine->getDetector()->elementCountX * machine->getDetector()->elementsizeX)/2);   //BB 05/22/2018: <-- BB half of the detector in cm
      double detectorPositionY = machine->acquisitionGeometry.at(acqID).detectorPosition[1];
      double detectorPositionZ = machine->acquisitionGeometry.at(acqID).detectorPosition[2];

      cl_float3 source, voxelSize;
      cl_float3 detectorPosition; // DH: Important note: Do not confuse this with detectorPositions"      //BB and TV 04/10/2018
      cl_uint4 voxelCount;

      //Fix this! <-- shift to center the object + volumeOffset in X and Y (z detector motion) // it was plus volume offset in Z before

      source.s[0] = static_cast<float>(machine->acquisitionGeometry.at(acqID).focalPosition.v[0] + volume_offsetX);
      source.s[1] = static_cast<float>(machine->acquisitionGeometry.at(acqID).focalPosition.v[1] + volume_offsetY);
      source.s[2] = static_cast<float>(machine->acquisitionGeometry.at(acqID).focalPosition.v[2] - volume_offsetZ);

      detectorPosition.s[0] = static_cast<float>(detectorPositionX);
      detectorPosition.s[1] = static_cast<float>(detectorPositionY);
      detectorPosition.s[2] = static_cast<float>(detectorPositionZ);

      voxelCount.s[0] = static_cast<int>(volume->voxelCount[0]);
      voxelCount.s[1] = static_cast<int>(volume->voxelCount[1]);
      voxelCount.s[2] = static_cast<int>(volume->voxelCount[2]);

      voxelSize.s[0] = static_cast<float>(volume->voxelSize.v[0]);
      voxelSize.s[1] = static_cast<float>(volume->voxelSize.v[1]);
      voxelSize.s[2] = static_cast<float>(volume->voxelSize.v[2]);

      std::vector<cl_float3> detectorPositions(machine->getDetector()->elementCountX); // # detector elements in a row
      cl::Buffer detectorBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                detectorPositions.size() * sizeof(cl_float3),
                                &detectorPositions[0],
                                &err);

      if (err != CL_SUCCESS) throw build_string("Creating detectorBuffer, line " << __LINE__ << ": detectorBuffer err value is: " << err);

      std::vector<cl_float> signal(machine->getDetector()->elementCountX, 0.0f);
      cl::Buffer signalBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                              signal.size()*sizeof(cl_float), &signal[0], &err);

      if (err != CL_SUCCESS) throw build_string("Creating signalBuffer, line " << __LINE__ << ": signalBuffer err value is: " << err);

      //double FSD = machine->acquisitionGeometry[acqID].focalPosition[2]; //Original

      double FSDx = machine->acquisitionGeometry.at(acqID).focalPosition[0] * machine->acquisitionGeometry.at(acqID).focalPosition[0];
      double FSDy = machine->acquisitionGeometry.at(acqID).focalPosition[1] * machine->acquisitionGeometry.at(acqID).focalPosition[1];
      double FSDz = machine->acquisitionGeometry.at(acqID).focalPosition[2] * machine->acquisitionGeometry.at(acqID).focalPosition[2];

      //double FSDsquared = (FSD * FSD); //Original
      double FSDsquared = FSDx + FSDy + FSDz;

      //TODO: instead of passing the vectors - consisting of 9 values - just pass in the three cosine values: cosX, cosY, and cosZ
      // where cosz = dot product of

      // Set Up Direction Cosines from Detector Orientation
      Acquisition *ap = &machine->acquisitionGeometry.at(acqID);

      std::vector<cl_float3> detCosines(2); // [0] cos, [1] sin

      // Cosines
      detCosines.at(0).s[0] = static_cast<float>(ap->detectorDirectionS[0]);   // S dotted with 1,0,0
      detCosines.at(0).s[1] = static_cast<float>(ap->detectorDirectionT[1]);   // T dotted with 0,1,0
      detCosines.at(0).s[2] = static_cast<float>(ap->detectorDirectionR[2]);   // R dotted with 0,0,1

      // Sines
      detCosines.at(1).s[0] = static_cast<float>(sqrt(1.0 - detCosines.at(0).s[0])); // sqrt(1-cos^2)
      detCosines.at(1).s[1] = static_cast<float>(sqrt(1.0 - detCosines.at(0).s[1])); // sqrt(1-cos^2)
      detCosines.at(1).s[2] = static_cast<float>(sqrt(1.0 - detCosines.at(0).s[2])); // sqrt(1-cos^2)

      #if 0
      static bool first_time = false;
      if (first_time)
      {
          first_time = false;
          std::cout << "\tDetector vectors"
                    << ": R: " << ap->detectorDirectionR[0] << "/" << ap->detectorDirectionR[1] << "/" << ap->detectorDirectionR[2]
                    << ", S: " << ap->detectorDirectionS[0] << "/" << ap->detectorDirectionS[1] << "/" << ap->detectorDirectionS[2]
                    << ", T: " << ap->detectorDirectionT[0] << "/" << ap->detectorDirectionT[1] << "/" << ap->detectorDirectionT[2]
                    << ", Detector Cosines: " << detCosines[0].s[0] << ", " << detCosines[0].s[1] << ", " << detCosines[0].s[2] << std::endl;
      }
      #endif

      cl::Buffer cosineBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                              detCosines.size() * sizeof(cl_float3),
                              &detCosines[0],
                              &err);

      if (err != CL_SUCCESS) throw build_string("Creating cosineBuffer, line " << __LINE__ << ": cosineBuffer err value is: " << err);


      double qElSizeX = 0.5 *machine->getDetector()->elementsizeX;
      double qElSizeY = 0.5 *machine->getDetector()->elementsizeY;
      cl_int randomize = (cl_int)(Acquisition::randomize_element_quad == true);

      /* //DEBUG
      std::cout << "Size Arguments"<< std::endl;
      std::cout << "detectorBuffer: " << detectorPositions.size() << std::endl;
      std::cout << "source[0]: " << source.s[0] << std::endl;
      std::cout << "source[1]: " << source.s[1] << std::endl;
      std::cout << "source[2]: " << source.s[2] << std::endl;
      std::cout << "voxelCount[0]: " << voxelCount.s[0] << std::endl;
      std::cout << "voxelCount[1]: " << voxelCount.s[1] << std::endl;
      std::cout << "voxelCount[2]: " << voxelCount.s[2] << std::endl;
      std::cout << "voxelSize[0]: " << voxelSize.s[0] << std::endl;
      std::cout << "voxelSize[1]: " << voxelSize.s[1] << std::endl;
      std::cout << "voxelSize[2]: " << voxelSize.s[2] << std::endl;
      std::cout << "volumeBuffer: " << volume->voxelCount[0]* volume->voxelCount[1] * volume->voxelCount[2] << std::endl;
      std::cout << "lengthBuffer: " << machine->getDetector()->elementCountX * numMatLbls << std::endl;
      std::cout << "detectorPositions: " << detectorPositions.size() << std::endl;
      std::cout << "qElSizeX: " << qElSizeX << std::endl;
      std::cout << "qElSizeY: " << qElSizeY << std::endl;
      std::cout << "detectorPosition[0]: " << detectorPosition.s[0] << std::endl;
      std::cout << "detectorPosition[1]: " << detectorPosition.s[1] << std::endl;
      std::cout << "detectorPosition[2]: " << detectorPosition.s[2] << std::endl;
      */

      err = traceKernel.setArg(0, detectorBuffer);                    if (err != CL_SUCCESS) throw build_string("trace arg 0, detectorBuffer: " << err);
      err = traceKernel.setArg(1, source);                            if (err != CL_SUCCESS) throw build_string("trace arg 1, source: " << err);
      err = traceKernel.setArg(2, voxelCount);                        if (err != CL_SUCCESS) throw build_string("trace arg 2, voxelCount: " << err);
      err = traceKernel.setArg(3, voxelSize);                         if (err != CL_SUCCESS) throw build_string("trace arg 3, voxelSize: " << err);
      err = traceKernel.setArg(4, volumeBuffer);                      if (err != CL_SUCCESS) throw build_string("trace arg 4, volumeBuffer: " << err);
      err = traceKernel.setArg(5, lengthBuffer);                      if (err != CL_SUCCESS) throw build_string("trace arg 5, lengthBuffer: " << err);
      err = traceKernel.setArg(6, invSquaredBuffer);                  if (err != CL_SUCCESS) throw build_string("trace arg 6, invSquaredBuffer: " << err);
      err = traceKernel.setArg(7, (cl_int)detectorPositions.size());  if (err != CL_SUCCESS) throw build_string("trace arg 7, detectorPositions.size(): " << err);
    //err = traceKernel.setArg(8, (cl_float)FSDsquared);              if (err != CL_SUCCESS) throw build_string("trace arg 8, FSDsquared: " << err); NO LONGER USED DH 2020-01-07
    //err = traceKernel.setArg(8, cosineBuffer);                      if (err != CL_SUCCESS) throw build_string("trace arg 8, detCosines: " << err);
      err = traceKernel.setArg(8, (cl_float)qElSizeX);                if (err != CL_SUCCESS) throw build_string("trace arg 9, qElSizeX: " << err);
      err = traceKernel.setArg(9, (cl_float)qElSizeY);               if (err != CL_SUCCESS) throw build_string("trace arg 10, qElSizeY: " << err);
      err = traceKernel.setArg(10, (cl_int)numMatLbls);               if (err != CL_SUCCESS) throw build_string("trace arg 11, numMatLbls: " << err);
      err = traceKernel.setArg(11, (cl_int)randomize);                if (err != CL_SUCCESS) throw build_string("trace arg 12, randomize: " << err);
      err = traceKernel.setArg(12, detectorPosition);                 if (err != CL_SUCCESS) throw build_string("trace arg 13, detector positions: " << err);


      ////////////////////
      // Set up physics //
      ////////////////////

      XRayTube *xraytube = machine->getXRayTube();
      unsigned short kVp = xraytube->getkVP();
      unsigned short kVp_x2_m1 = kVp * 2 - 1;
      unsigned short spectrumID = machine->getXRayTube()->getSpectrumID();

      BooneMaterialTable matTable;
      BooneCoefficientTable coeffTable = matTable.getMaterialTable((BooneMaterialTable::Material) spectrumID);//DH//, kVp);

      std::vector<float> filter(kVp_x2_m1);

      std::vector<rowVector> SpectrumVector;
      coeffTable.getSpectrum(kVp, SpectrumVector); // resizes vector to (kVp*2-1) and populates

      unsigned short filterMaterialZ = xraytube->getFilterMaterialZ();
      double filterDensity = attenuationTable.getDensity(filterMaterialZ);
      double filterThickness = xraytube->getFilterThickness();


      for(unsigned short spec=0; spec<kVp_x2_m1; spec++) // set up the attenuation for the filter.
      {
          filter.at(spec) = static_cast<float>(exp(-1.0*attenuationTable.getAttenuation(filterMaterialZ, spec)*filterThickness*filterDensity));
          if (filter.at(spec) == 0.0f) filter.at(spec) = FLT_MIN;
      }


      Detector* detector = machine->getDetector();
      float multiplier = static_cast<float>(0.5 * detector->elementsizeX * 0.5 * detector->elementsizeY);  // some thing needs to be multipled for each energy

      std::vector<float> spectrum(SpectrumVector.size() * 2);
      // There are no 2D arrays in OpenCL so this kind of indexing is necessary
      for (int s = 0; s <SpectrumVector.size(); ++s)
      {
          spectrum.at(s * 2) = static_cast<float>(SpectrumVector.at(s).v[0]);
          spectrum.at(s * 2 + 1) = static_cast<float>(SpectrumVector.at(s).v[1]);
      }


      // Prepare compatible OpenCL buffers to hold attenuation and density values
      int num_counts = numMaterials * kVp_x2_m1;


      // Interim tables NOT passed on to the OpenCL Physics kernel program
      int numUniqueMats = 0;       // 2019-11-22 Initially a counter, then as the final count
      std::vector<int>matZ;        // ID of materialZ, table not passed to kernel pgm
      std::vector<int>uniqueMats;  // unique integer Material IDs, not passed to kernel pgm

      // Tables passed on to the OpenCL Physics kernel program
      std::vector<float> densities(numMaterials, 0.0f);
      std::vector<float> weights(numMaterials, 0.0f);
      std::vector<int>   labMatOffs(numMaterials, 0);
      std::vector<int>   numLabMats(numMatLbls, 0);
      std::vector<float> attenuations(num_counts, 0.0f);

      // Note: ALL non-opencl storage allocations must be made before ANY opencl buffers are created
      cl::Buffer spectrumBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             SpectrumVector.size()* 2 * sizeof(cl_float),
                                             &spectrum.at(0), &err);

      if (err != CL_SUCCESS) throw build_string("Creating spectrumBuffer, line " << __LINE__ << ": spectrumBuffer err value is: " << err);

      cl::Buffer filterBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                           filter.size() * sizeof(cl_float), &filter[0], &err);

      if (err != CL_SUCCESS) throw build_string("Creating filterBuffer, line " << __LINE__ << ": filterBuffer err value is: " << err);

      int realZ;
      double density;
      double weight;

      int offset = 0;
      int nmat = 0;


      // 2020-01-20 DEBUG
      std::ofstream dbg_out("DEBUG_FOR_PHYSICS_DATA.txt");
      // 2020-01-20 DEBUG

      for(int k=0; k<numMatLbls; k++) // {6} for each label specified in the config XML file (up to numMatLbls max)
      {
          std::map<unsigned short, LabelMap>::iterator key = volume->labelMap.find(k);
          if (key != volume->labelMap.end())
          {
              auto &map = key->second.getMaterialMap();

              density = 0.0;
              weight  = 0.0;
              numLabMats.at(k) = static_cast<int>(map.size());
              offset += static_cast<int>(map.size());

              for (size_t i=0; i<map.size(); ++i)
              {
                  if (i >= map.size()) break;

                  Material mat = map.at(i); // only treat the first material with this label
                  realZ = mat.materialZ; // this is how SimulationCPU gets realZ

                  if (mat.density >= 0.0) // mat.density initialized to -1. configLabel.xml overrides the default
                  {
                      density = mat.density;
                  }
                  else
                  {
                      // This line should never be reached since density must always be supplied in GPU XML file
                      density = attenuationTable.getDensity(realZ);
                  }

                  densities.at(nmat) = static_cast<float>(density);
                  weights.at(nmat)   = static_cast<float>(mat.weight);

                  matZ.push_back(realZ); // Store ID of this material

                  // 2020-01-20 DEBUG
                  if (dbg_out)
                  {
                      dbg_out << "mat " << realZ << ", density: " << density << ", weight: " << mat.weight << "\nAttenuations:\n";
                  }
                  // 2020-01-20 DEBUG

                  // Add attentuationTable data only if this is a unique material (not already encountered)
                  if (std::find(uniqueMats.begin(), uniqueMats.end(), realZ) == uniqueMats.end())
                  {
                      uniqueMats.push_back(realZ);
                      std::vector<double> *table = attenuationTable.getAttenuationTable(realZ);
                      if (table)
                      {
                          for (int ndx = 0; ndx < kVp_x2_m1; ++ndx)
                          {
                              attenuations.at((numUniqueMats * kVp_x2_m1) + ndx) = static_cast<float>(table->at(ndx));
                              // 2020-01-20 DEBUG
                              if (dbg_out)
                              {
                                  dbg_out << "\t" << static_cast<float>(table->at(ndx)) << "\n";
                              }
                              // 2020-01-20 DEBUG
                          }
                      }// 2020-01-20 DEBUG
                      else
                      {
                          if (dbg_out)
                          {
                              dbg_out << "\tTABLE FOR MATERIAL " << realZ << " NOT FOUND!!\n";
                          }
                      }
                      // 2020-01-20 DEBUG

                      numUniqueMats = static_cast<int>(uniqueMats.size());
                  }

                  nmat += 1;
              }
          }
      } // next numMatLbls

      // 2020-01-20 DEBUG
      if (dbg_out)
      {
          dbg_out.close();
      }
      // 2020-01-20 DEBUG


      // Ensure that labMatOffs vector contains the same number of elements as matZ
      labMatOffs.resize(matZ.size()); // added 2020-01-29 DH

      attenuations.resize(numUniqueMats * kVp_x2_m1, 0.0f); // 2019-11-22 numUniqueMats is at final count at this point

      // Set labMatOffs vector to point into uniqueMatZs vector
      for (int mZndx=0; mZndx < matZ.size(); ++mZndx) // matZ contains the overall number of materials listed, including dumplicates
      {
          int mZ = matZ.at(mZndx);
          for (int indx = 0; indx < uniqueMats.size(); ++indx) // uniqueMats contains the number of unique materials (non-repeating) uniqueMats is a subset of matZ
          {
              if (uniqueMats.at(indx) == mZ)
              {
                  labMatOffs.at(mZndx) = indx; // labMatOffs should contain the same number of elements as matZ
              }
          }
      }


      #define DEBUG_DUMP_DATA_STRUCTURES
      #if defined(DEBUG_DUMP_DATA_STRUCTURES)
      dumpAttenuations(attenuations, kVp_x2_m1, uniqueMats);
      dumpWeights(weights);
      dumpDensities(densities);
      dumplabMatOffs(labMatOffs);
      dumpnumLabMats(numLabMats);
      dumpmatZValue(matZ);
      dumpuniqueMatZs(uniqueMats);
      dumpSpectrum(kVp, spectrumID, spectrum);
      dumpFilter(filter);
      int row = 100;
      dumpPhysics_double(int(kVp_x2_m1), nullptr, densities, weights, attenuations, numMatLbls, spectrum, int(spectrum.size()),
                  filter, multiplier, signal, int(detectorPositions.size()), labMatOffs, numLabMats, row, matZ);
      #endif


      cl::Buffer attenuationTableBuffer = cl::Buffer(context,  CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                     /////numMaterials * kVp_x2_m1 * sizeof(cl_float),
                                                     numUniqueMats * kVp_x2_m1 * sizeof(cl_float),
                                                      &attenuations[0], &err);

      if (err != CL_SUCCESS) throw build_string("Creating attenuationTableBuffer, line " << __LINE__ << ": attenuationTableBuffer err value is: " << err);

      cl::Buffer densityBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                            numMaterials * sizeof(cl_float),
                                            &densities[0], &err);

      if (err != CL_SUCCESS) throw build_string("Creating densityBuffer, line " << __LINE__ << ": densityBuffer err value is: " << err);

      cl::Buffer weightsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                            numMaterials * sizeof(cl_float),
                                            &weights[0], &err);

      if (err != CL_SUCCESS) throw build_string("Creating weightsBuffer, line " << __LINE__ << ": weightsBuffer err value is: " << err);

      cl::Buffer labMatOffsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                               numMatLbls * sizeof(cl_int),
                                               &labMatOffs[0], &err);

      if (err != CL_SUCCESS) throw build_string("Creating labMatOffsBuffer, line " << __LINE__ << ": labMatOffsBuffer err value is: " << err);

      cl::Buffer numLabMatsBuffer = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                               numMatLbls * sizeof(cl_int),
                                               &numLabMats[0], &err);

      if (err != CL_SUCCESS) throw build_string("Creating numLabMatsBuffer, line " << __LINE__ << ": numLabMatsBuffer err value is: " << err);


      err = physicsKernel.setArg(0, (cl_int)kVp_x2_m1);                  if (err != CL_SUCCESS) throw build_string("physics arg 0: " << err);
      err = physicsKernel.setArg(1, lengthBuffer);                       if (err != CL_SUCCESS) throw build_string("physics arg 1: " << err);
      err = physicsKernel.setArg(2, densityBuffer);                      if (err != CL_SUCCESS) throw build_string("physics arg 2: " << err);
      err = physicsKernel.setArg(3, weightsBuffer);                      if (err != CL_SUCCESS) throw build_string("physics arg 3: " << err);
      err = physicsKernel.setArg(4, attenuationTableBuffer);             if (err != CL_SUCCESS) throw build_string("physics arg 4: " << err);
      err = physicsKernel.setArg(5, (cl_int)numMatLbls);                 if (err != CL_SUCCESS) throw build_string("physics arg 5: " << err);
      err = physicsKernel.setArg(6, spectrumBuffer);                     if (err != CL_SUCCESS) throw build_string("physics arg 6: " << err);
      err = physicsKernel.setArg(7, (cl_int)SpectrumVector.size());      if (err != CL_SUCCESS) throw build_string("physics arg 7: " << err);
      err = physicsKernel.setArg(8, filterBuffer);                       if (err != CL_SUCCESS) throw build_string("physics arg 8: " << err);
      err = physicsKernel.setArg(9, (cl_float)multiplier);               if (err != CL_SUCCESS) throw build_string("physics arg 9: " << err);
      err = physicsKernel.setArg(10, signalBuffer);                      if (err != CL_SUCCESS) throw build_string("physics arg 10: " << err);
      err = physicsKernel.setArg(11, (cl_int)detectorPositions.size());  if (err != CL_SUCCESS) throw build_string("physics arg 11: " << err);
      err = physicsKernel.setArg(12, labMatOffsBuffer);                  if (err != CL_SUCCESS) throw build_string("physics arg 12 labMatOffs: " << err);
      err = physicsKernel.setArg(13, numLabMatsBuffer);                  if (err != CL_SUCCESS) throw build_string("physics arg 13 numLabMats: " << err);


      // Noise is no longer handled in OpenCL
      // Set up noise kernel arguments
      /*
      err = noiseKernel.setArg(0, signalBuffer);                         if (err != CL_SUCCESS) throw build_string("noise arg 0: " << err);
      err = noiseKernel.setArg(2, (cl_long)detectorPositions.size());    if (err != CL_SUCCESS) throw build_string("noise arg 2: " << err);
      err = noiseKernel.setArg(3, (cl_int)useNoise);                     if (err != CL_SUCCESS) throw build_string("noise arg 3: " << err);
      err = noiseKernel.setArg(4, (cl_int)useScaling);                   if (err != CL_SUCCESS) throw build_string("noise arg 4: " << err);
      err = noiseKernel.setArg(5, (cl_float)normAirKerma);               if (err != CL_SUCCESS) throw build_string("noise arg 5: " << err);
      err = noiseKernel.setArg(6, (cl_float)gain);                       if (err != CL_SUCCESS) throw build_string("noise arg 6: " << err);
      err = noiseKernel.setArg(7, (cl_float)mAs);                        if (err != CL_SUCCESS) throw build_string("noise arg 7: " << err);
      err = noiseKernel.setArg(8, (cl_float)propConst);                  if (err != CL_SUCCESS) throw build_string("noise arg 8: " << err);
      */



      // Begin Simulation


      // Iterate over each detector element, processing rows in parallels
      rowVector ElementLocation;
      unsigned int elementCountX = machine->getDetector()->elementCountX;
      unsigned int elementCountY = machine->getDetector()->elementCountY;
      unsigned int half_element_count_y = elementCountY / 2;

      for (unsigned int i = id; i<elementCountY;i++)
      {
         // Noise is no longer handled in OpenCL
         // Set seed to the noise
         //err = noiseKernel.setArg(1, (cl_int) i);    if (err != CL_SUCCESS) throw build_string("noise arg 1: " << err);
         //std::cout << "On element row " << i << " of " << machine->getDetector()->elementCountY << std::endl;

         for (unsigned int j = 0; j<elementCountX; j++) //set up a row of detector positions
         {
            ElementLocation = machine->acquisitionGeometry[acqID].getDetectorElementLocation(*(machine->getDetector()), double(j), double(i));

            // Note: these positions do not yet account for the four sides of the detector element
            detectorPositions.at(j).s[0] = static_cast<float>(ElementLocation.v[0] + volume_offsetX);
            detectorPositions.at(j).s[1] = static_cast<float>(ElementLocation.v[1] + volume_offsetY);
            detectorPositions.at(j).s[2] = static_cast<float>(ElementLocation.v[2] + volume_offsetZ);
         }  // next j


         err = clCommandQueue.enqueueWriteBuffer(detectorBuffer, false, 0, detectorPositions.size() * sizeof(cl_float3), &detectorPositions.at(0));

         if (err != CL_SUCCESS) throw build_string("enqueuing WriteBuffer with detectorBuffer, line " << __LINE__ << ":  err: " << err);

         size_t s = static_cast<size_t>(64 * (ceil(detectorPositions.size() / 64.0)));        // make this a variable to support optimization

         // * RAY TRACE * (ACTUALLY EXECUTE the traceKernel program) *
         // We invoke this command once for each row of the detector...
         // HOWEVER, the kernel routine itself gets invoked once for each detector in that row!!
         // FURTHERMORE, up to 64 simultaneous invocations can be made

         err = clCommandQueue.enqueueNDRangeKernel(traceKernel, cl::NullRange, cl::NDRange(s), cl::NDRange(64));

         //std::cout << __LINE__ << ": Just past enqueueNDRangeKernel " << std::endl;

         if (err != CL_SUCCESS) throw build_string("enqueuing NDRangeKernel with traceKernel (running trace kernel program), line " << __LINE__ << ":  err: " << err);

          #if defined(DEBUG_DUMP_DATA_STRUCTURES)
              if (i == 832)
              {
                    // DEBUG ONLY - Read the length buffer back
                    std::ofstream lb_ofs("DEBUG_OCL_LengthBuffer.txt");
                    lb_ofs << "Elem countX: " << elementCountX << ", numMatLbls: " << numMatLbls << "\n\n";


                    size_t num_floats = elementCountX * numMatLbls * 4;
                    size_t buffer_size = num_floats * sizeof(cl_float);
                    std::vector<cl_float> tmplenbuf(num_floats);
                    err = clCommandQueue.enqueueReadBuffer(lengthBuffer, true, 0, buffer_size, &tmplenbuf[0]);
                    if (err != CL_SUCCESS) throw build_string("reading length buffer" << err);
                    for(int lbi=0; lbi<num_floats; lbi=lbi+4)
                    {
                        float len = tmplenbuf[lbi];
                        if (len > 0)  lb_ofs << "elem col " << lbi/4 << ": " << tmplenbuf[lbi] << '\n';
                    }


                    lb_ofs << "\n---\n"; // DEBUG_OCL_LengthBuffer.txt
                    lb_ofs.close();
              }
          #endif

         // Read the inversed square values back from the trace kernel (averaging around 0.96)
         if (useInverseSq)
         {
             err = clCommandQueue.enqueueReadBuffer(invSquaredBuffer, true, 0, detectorPositions.size()*sizeof(cl_float), &invSq[0]);
             if (err != CL_SUCCESS) throw build_string("enqueueReadBuffer with invSquaredBuffer (reading inverse square buffer), line " << __LINE__ << ":  err: " << err);
         }

        // std::cout << __LINE__ << ": Just past enqueueReadBuffer " << std::endl;

         #if defined(USE_CPU_PHYSICS)

             // Read the length buffer (which usually stays in the GPU environment between tracer and physics kernel execution)
             size_t num_doubles = elementCountX * numMatLbls * 4;
             size_t buffer_size = num_doubles * sizeof(cl_double);
             std::vector<cl_double> tmplenbuf(num_doubles);
             err = clCommandQueue.enqueueReadBuffer(lengthBuffer, true, 0, buffer_size, &tmplenbuf[0]);

             for (unsigned int j = 0; j<elementCountX; j++) //set up a row of detector positions
             {
                 set_global_id(j);
                 physics(kVp_x2_m1, &tmplenbuf[0],
                         &densities[0], &weights[0],
                         &attenuations[0], numMatLbls,
                         &spectrum[0], spectrum.size(),
                         &filter[0], multiplier,
                         &signal[0],  detectorPositions.size(),
                         &labMatOffs[0], &numLabMats[0]);
             }
         #else
         
         //std::cout << "Line " << __LINE__ << " About to enqueueNDRangeKernel" << std::endl;

             // * PHYSICS * Crunch attenuation (ACTUALLY EXECUTE the physicsKernel) *// We invoke this command once for each row of the detector...
             // HOWEVER, the kernel routine itself gets invoked once for each detector in that row!!
             // FURTHERMORE, up to 64 simultaneous invocations can be made
             err = clCommandQueue.enqueueNDRangeKernel(physicsKernel, cl::NullRange, cl::NDRange(s), cl::NDRange(64));
             if (err != CL_SUCCESS) throw build_string("enqueuing NDRangeKernel with physicsKernel (running physics kernel program), line " << __LINE__ << ":  err: " << err);
                         
             /*std::cout << "Line " << __LINE__
                       << ": about to read buffer before physics Kernel Args"
                       << ", detectorPositions.size()*sizeof(cl_float): " << detectorPositions.size()*sizeof(cl_float)
                       << ", signalBuffer size: " << signal.size()*sizeof(cl_float)
                       << std::endl;*/
             

             // Read the signal back
             err = clCommandQueue.enqueueReadBuffer(signalBuffer, true, 0, detectorPositions.size()*sizeof(cl_float), &signal[0]);
             if (err != CL_SUCCESS) throw build_string("enqueueReadBuffer with signalBuffer (reading signal buffer), line " << __LINE__ << ":  err: " << err);

         #endif

         #if defined(DEBUG_DUMP_DATA_STRUCTURES)
         if (i == half_element_count_y)
         {
             std::ofstream sig_ofs("DEBUG_OCL_SIGNAL.txt");
             sig_ofs << "signal at elementX " << i << ":\n\n";
             for (unsigned int j = 0; j<elementCountX; j++)
             {
                 sig_ofs << signal[j] << '\n';
             }
             sig_ofs.close();
         }
         #endif

         // Noise is no longer handled in OpenCL, it is handled in regular C++ to use std::poisson_distribution
         // Add noise
         //err = clCommandQueue.enqueueNDRangeKernel(noiseKernel, cl::NullRange, cl::NDRange(s), cl::NDRange(64));
         //if (err != CL_SUCCESS) throw build_string("invoking noiseKernel" << err);


         //std::cout << "Line " << __LINE__ << ": ...just before buildImage call" << std::endl;

         // Done with OpenCL for now, use the signal buffer to apply noise and generate the image
         buildImage(i, elementCountX, signal, invSq,
                    gen, d2, useNoise, useScaling, useInverseSq, gain, normAirKerma, mAs, propConst,
                    outImage);

         //std::cout << "Line " << __LINE__ << ": ...just after buildImage call" << std::endl;


         if (i % 100 == 0) progressBar.print(i*100/ machine->getDetector()->elementCountY);

      } // next element Y (i)

   }
   catch(cl::Error const err)
   {
       std::cerr << "\n\nError in " << __FUNCTION__ << "(): OpenCL function "
                 << err.what() << ", error code "
                 << err.err() << ", \"" << clib.getErrorString(err.err()) << "\""
                 << std::endl;
       status = false;
   }
   catch(std::out_of_range &err)
   {
       std::cerr << "Out of Range error: " << __FUNCTION__ << err.what() << std::endl;
       status = false;
   }
   catch(const std::string &msg)
   {
       std::cerr << "\nFailure: " << __FUNCTION__ << ": " << msg << std::endl;
       status = false;
   }

   return status;
}

#endif  //if !defined(NO_OPENCL)
