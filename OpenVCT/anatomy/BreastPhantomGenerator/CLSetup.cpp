// CLSetup.cpp

#include "BreastPhantomGenerator.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "VCT_CommonLibrary.h"

size_t sizeFile = 0;
const char *source = nullptr;
const char *opencl_file = "split_criterion_distr_thickness.cl";

bool SDKFileopen(std::string fileName)   //!< file name
{
    bool retval = false;
    char*  str;

    // Open file stream
    std::ifstream ifs(fileName);

    if (ifs.is_open())
    {
        // Find the stream size
        ifs.seekg(0, std::fstream::end);
        sizeFile = (size_t)ifs.tellg();
        ifs.seekg(0, std::fstream::beg);

        str = new char[sizeFile + 1];
        if (str != nullptr)
        {

            // Read file
            ifs.read(str, sizeFile);
            ifs.close();
            str[sizeFile] = '\0';
            source = str;
            retval = true;
        }
    }

    return retval;
}


#pragma region Documentation
/// <summary>Sets up the cl contexts for voxel parallelization.</summary>
/// <remarks>Joe Chui, 10/10/2011. </remarks>
/// <param name="clDistributionCount">Number of cl distributions.</param>
#pragma endregion

bool BreastPhantomGenerator::SetupCLForVoxelParallelization(int clDistributionCount)
{
    bool retval = false;
    vct::CommonLibrary clib;

    try
    {

        cl::Context      context;
        cl::CommandQueue clCommandQueue;
        cl::Kernel       traceKernel, physicsKernel;
        cl::Device       dev;
        cl::Platform platform;
        
        // Get all available platforms
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        if (platforms.empty()) {
            std::cout << "Error: No platforms found!" << std::endl;
            return -1;
        }

        // Iterate through platforms and list devices
        for (size_t i = 0; i < platforms.size(); ++i) {
            platform = platforms[i];

            // Get platform info
            std::string platform_name = platform.getInfo<CL_PLATFORM_NAME>();
            std::string platform_version = platform.getInfo<CL_PLATFORM_VERSION>();
            std::string platform_vendor = platform.getInfo<CL_PLATFORM_VENDOR>();
            std::cout << "Platform " << i + 1 << ": " << platform_name << "\n";
            std::cout << "  Vendor: " << platform_vendor << "\n";
            std::cout << "  Version: " << platform_version << "\n";

            // Get all devices on this platform
            std::vector<cl::Device> devices;
            platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

            if (devices.empty()) {
                std::cout << "  Error: No devices found for this platform!" << std::endl;
                continue;
            }

            // Print device details
            for (size_t j = 0; j < devices.size(); ++j) {
                cl::Device device = devices[j];
                std::string device_name = device.getInfo<CL_DEVICE_NAME>();
                cl_device_type device_type = device.getInfo<CL_DEVICE_TYPE>();

                std::cout << "  Device " << j + 1 << ": " << device_name << "\n";
                
                // Identify device type
                if (device_type == CL_DEVICE_TYPE_CPU) {
                    std::cout << "    Type: CPU\n";
                } else if (device_type == CL_DEVICE_TYPE_GPU) {
                    std::cout << "    Type: GPU\n";
                } else if (device_type == CL_DEVICE_TYPE_ACCELERATOR) {
                    std::cout << "    Type: Accelerator\n";
                } else {
                    std::cout << "    Type: Other/Unknown\n";
                }
            }
        }

        if (!this->context())
        {
              
            cl_int err = 0;
            clCommandQueue = cl::CommandQueue(context, dev, 0, &err);           
                
            bool success = SDKFileopen(opencl_file);

            cl::Program::Sources sources;//(1, std::make_pair(source, 0));
            sources.push_back( { source, sizeFile } );

            // 2. Perform runtime source compilation, and obtain splitKernel entry point.
            cl::Program program(context, sources);

            try
            {
                program.build();
            }
            catch (...)
            {
                std::string buildlog;
                program.getBuildInfo(dev, (cl_program_build_info)CL_PROGRAM_BUILD_LOG, &buildlog);
                std::cerr << "Error Building Kernel Program!, OpenCL Build Log:\n" << buildlog 
                          << std::endl;
                std::cerr << "End OpenCL Build Log." << std::endl;
                throw; // jump to outer catch block since we cannot run
            }
            
            evalKernel  = cl::Kernel(program, "distr_max_box7");
            splitKernel = cl::Kernel(program, "split_criterion_distr_thickness");

            createNodesKernel  = cl::Kernel( program, "CompactNodes");
            createNodesKernel2 = cl::Kernel( program, "SubdivideNodes");

            createLastLevelVoxelsKernel = cl::Kernel( program, "create_voxels_last_level");
            voxelizeKernel = cl::Kernel( program, "voxelize");

            addupKernel   = cl::Kernel(program, "GetMemoryCountForNextLevel");
            wsAddupKernel = cl::Kernel(program, "WGGetMemoryCountForNextLevel");
        }


        cl_float4 * mu = new cl_float4[clDistributionCount];
        if (mu == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            mu[i].s[0] = (cl_float) distributionMUVectors[i][0];
            mu[i].s[1] = (cl_float) distributionMUVectors[i][1];
            mu[i].s[2] = (cl_float) distributionMUVectors[i][2];
            mu[i].s[3] = (cl_float) 0.0f;
        }

        cl_int err;
        mubuffer = cl::Buffer (context,
                               CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR ,
                               clDistributionCount * sizeof(cl_float4),
                               mu, 
                               &err);

        splitKernel.setArg( 23, mubuffer);
    
        delete[] mu;
        mu = nullptr;


        cl_float16 * r = new cl_float16[clDistributionCount];
        if (r == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            r[i].s[0]  = (cl_float) distributionRMatrices[i][0][0];
            r[i].s[1]  = (cl_float) distributionRMatrices[i][0][1];
            r[i].s[2]  = (cl_float) distributionRMatrices[i][0][2];
            r[i].s[4]  = (cl_float) distributionRMatrices[i][1][0];
            r[i].s[5]  = (cl_float) distributionRMatrices[i][1][1];
            r[i].s[6]  = (cl_float) distributionRMatrices[i][1][2];
            r[i].s[8]  = (cl_float) distributionRMatrices[i][2][0];
            r[i].s[9]  = (cl_float) distributionRMatrices[i][2][1];
            r[i].s[10] = (cl_float) distributionRMatrices[i][2][2];

            r[i].s[3]  = r[i].s[7]  = r[i].s[11] = r[i].s[12] = 
                         r[i].s[13] = r[i].s[14] = r[i].s[15] = 0.0;
        }

        rbuffer = cl::Buffer(context,
                             CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                             clDistributionCount * sizeof(cl_float16),
                             r, 
                             nullptr);

        splitKernel.setArg(24, rbuffer);

        delete[] r;
        r = nullptr;


        cl_float * log = new cl_float[clDistributionCount];
        if (log == nullptr)
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            log[i] = (cl_float) distributionLogSqrtSigmaLogPriors[i];
        }

        logbuffer = cl::Buffer(context,
                               CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                               clDistributionCount * sizeof(cl_float),
                               log, 
                               nullptr);

        splitKernel.setArg(25, logbuffer);
    
        delete[] log;
        log = nullptr;

        
        cl_float4 * bb = new cl_float4[clDistributionCount];
        if (bb == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ 
                      << "Could not allocate memory for " << clDistributionCount 
                      << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            bb[i].s[0] = (cl_float) distributionSigmaInverseMUVectors[i][0];
            bb[i].s[1] = (cl_float) distributionSigmaInverseMUVectors[i][1];
            bb[i].s[2] = (cl_float) distributionSigmaInverseMUVectors[i][2];
            bb[i].s[3] = (cl_float) 0.0f;
        }

        bbbuffer = cl::Buffer(context,
                              CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                              clDistributionCount * sizeof(cl_float4),
                              bb, 
                              nullptr);

        splitKernel.setArg(26, bbbuffer);

        delete[] bb;
        bb = nullptr;


        cl_float16 * a1 = new cl_float16[clDistributionCount];
        if (a1 == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            a1[i].s[0]  = (cl_float) distributionA1InverseMatrices[i][0][0];
            a1[i].s[1]  = (cl_float) distributionA1InverseMatrices[i][0][1];
            a1[i].s[2]  = (cl_float) distributionA1InverseMatrices[i][0][2];
            a1[i].s[4]  = (cl_float) distributionA1InverseMatrices[i][1][0];
            a1[i].s[5]  = (cl_float) distributionA1InverseMatrices[i][1][1];
            a1[i].s[6]  = (cl_float) distributionA1InverseMatrices[i][1][2];
            a1[i].s[8]  = (cl_float) distributionA1InverseMatrices[i][2][0];
            a1[i].s[9]  = (cl_float) distributionA1InverseMatrices[i][2][1];
            a1[i].s[10] = (cl_float) distributionA1InverseMatrices[i][2][2];

            a1[i].s[3]  = a1[i].s[7]  = a1[i].s[11] = a1[i].s[12] = 
                          a1[i].s[13] = a1[i].s[14] = a1[i].s[15] = 0.0;    
        }

        a1buffer = cl::Buffer(context,
                              CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                              clDistributionCount * sizeof(cl_float16),
                              a1, 
                              nullptr);

        delete[] a1;
        a1 = nullptr;


        cl_float16 * a2 = new cl_float16[clDistributionCount];
        if (a2 == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            a2[i].s[0]  = (cl_float) distributionA2InverseMatrices[i][0][0];
            a2[i].s[1]  = (cl_float) distributionA2InverseMatrices[i][0][1];
            a2[i].s[2]  = (cl_float) distributionA2InverseMatrices[i][0][2];
            a2[i].s[4]  = (cl_float) distributionA2InverseMatrices[i][1][0];
            a2[i].s[5]  = (cl_float) distributionA2InverseMatrices[i][1][1];
            a2[i].s[6]  = (cl_float) distributionA2InverseMatrices[i][1][2];
            a2[i].s[8]  = (cl_float) distributionA2InverseMatrices[i][2][0];
            a2[i].s[9]  = (cl_float) distributionA2InverseMatrices[i][2][1];
            a2[i].s[10] = (cl_float) distributionA2InverseMatrices[i][2][2];

            a2[i].s[3] = a2[i].s[7]  = a2[i].s[11] = a2[i].s[12] = 
                         a2[i].s[13] = a2[i].s[14] = a2[i].s[15] = 0.0; 
        }

        a2buffer = cl::Buffer(context,
                              CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                              clDistributionCount * sizeof(cl_float16),
                              a2, 
                              nullptr);

        //splitKernel.setArg(28, a2buffer);

        delete[] a2;
        a2 = nullptr;


        cl_float16 * a3 = new cl_float16[clDistributionCount];
        if (a3 == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0);
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            a3[i].s[0]  = (cl_float) distributionA3InverseMatrices[i][0][0];
            a3[i].s[1]  = (cl_float) distributionA3InverseMatrices[i][0][1];
            a3[i].s[2]  = (cl_float) distributionA3InverseMatrices[i][0][2];
            a3[i].s[4]  = (cl_float) distributionA3InverseMatrices[i][1][0];
            a3[i].s[5]  = (cl_float) distributionA3InverseMatrices[i][1][1];
            a3[i].s[6]  = (cl_float) distributionA3InverseMatrices[i][1][2];
            a3[i].s[8]  = (cl_float) distributionA3InverseMatrices[i][2][0];
            a3[i].s[9]  = (cl_float) distributionA3InverseMatrices[i][2][1];
            a3[i].s[10] = (cl_float) distributionA3InverseMatrices[i][2][2];

            a3[i].s[3]  = a3[i].s[7]  = a3[i].s[11] = a3[i].s[12] = 
                          a3[i].s[13] = a3[i].s[14] = a3[i].s[15] = 0.0;    
        }

        a3buffer = cl::Buffer(context,
                              CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                              clDistributionCount * sizeof(cl_float16),
                              a3, 
                              nullptr);

        //splitKernel.setArg(29, a3buffer);

        delete[] a3;
        a3 = nullptr;


        cl_float8 * h = new cl_float8[clDistributionCount];
        if (h == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            h[i].s[0] = (cl_float) distributionHMatrices[i][0];
            h[i].s[1] = (cl_float) distributionHMatrices[i][1];
            h[i].s[2] = (cl_float) distributionHMatrices[i][2];
            h[i].s[3] = (cl_float) distributionHMatrices[i][3];
            h[i].s[4] = (cl_float) distributionHMatrices[i][4];
            h[i].s[5] = (cl_float) distributionHMatrices[i][5];
            h[i].s[6] = (cl_float) distributionHMatrices[i][6];
            h[i].s[7] = (cl_float) distributionHMatrices[i][7];
        }

        hbuffer = cl::Buffer(context,
                             CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                             clDistributionCount * sizeof(cl_float8),
                             h, 
                             nullptr);

        //splitKernel.setArg(30, hbuffer);

        delete[] h;
        h = nullptr;


        cl_float8 * u1 = new cl_float8[clDistributionCount];
        if (u1 == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            u1[i].s[0] = (cl_float) distributionUMatrices[i][0][0];
            u1[i].s[1] = (cl_float) distributionUMatrices[i][1][0];
            u1[i].s[2] = (cl_float) distributionUMatrices[i][2][0];
            u1[i].s[3] = (cl_float) distributionUMatrices[i][3][0];
            u1[i].s[4] = (cl_float) distributionUMatrices[i][4][0];
            u1[i].s[5] = (cl_float) distributionUMatrices[i][5][0];
            u1[i].s[6] = (cl_float) distributionUMatrices[i][6][0];
            u1[i].s[7] = (cl_float) distributionUMatrices[i][7][0];
        }

        u1buffer = cl::Buffer(context,
                              CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                              clDistributionCount * sizeof(cl_float8),
                              u1, 
                              nullptr);

        // splitKernel.setArg(31, u1buffer);
    
        delete[] u1;
        u1 = nullptr;


        cl_float8 * u2 = new cl_float8[clDistributionCount];
        if (u2 == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            u2[i].s[0] = (cl_float) distributionUMatrices[i][0][1];
            u2[i].s[1] = (cl_float) distributionUMatrices[i][1][1];
            u2[i].s[2] = (cl_float) distributionUMatrices[i][2][1];
            u2[i].s[3] = (cl_float) distributionUMatrices[i][3][1];
            u2[i].s[4] = (cl_float) distributionUMatrices[i][4][1];
            u2[i].s[5] = (cl_float) distributionUMatrices[i][5][1];
            u2[i].s[6] = (cl_float) distributionUMatrices[i][6][1];
            u2[i].s[7] = (cl_float) distributionUMatrices[i][7][1];
        }

        u2buffer = cl::Buffer(context,
                              CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                              clDistributionCount * sizeof(cl_float8),
                              u2, 
                              nullptr);

        delete[] u2;
        u2 = nullptr;

        cl_float8 * u3 = new cl_float8[clDistributionCount];
        if (u3 == nullptr) 
        { 
            std::cerr << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            u3[i].s[0] = (cl_float) distributionUMatrices[i][0][2];
            u3[i].s[1] = (cl_float) distributionUMatrices[i][1][2];
            u3[i].s[2] = (cl_float) distributionUMatrices[i][2][2];
            u3[i].s[3] = (cl_float) distributionUMatrices[i][3][2];
            u3[i].s[4] = (cl_float) distributionUMatrices[i][4][2];
            u3[i].s[5] = (cl_float) distributionUMatrices[i][5][2];
            u3[i].s[6] = (cl_float) distributionUMatrices[i][6][2];
            u3[i].s[7] = (cl_float) distributionUMatrices[i][7][2];
        }

        u3buffer = cl::Buffer(context,
                              CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                              clDistributionCount * sizeof(cl_float8),
                              u3, 
                              nullptr);

        // splitKernel.setArg(33, u3buffer);

        delete[] u3;
        u3 = nullptr;

        cl_float16 * sigmainv = new cl_float16[clDistributionCount];
        if (sigmainv == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            sigmainv[i].s[0] = (cl_float) distributionSigmaInverseMatrices[i][0][0];
            sigmainv[i].s[1] = (cl_float) distributionSigmaInverseMatrices[i][0][1];
            sigmainv[i].s[2] = (cl_float) distributionSigmaInverseMatrices[i][0][2];
            sigmainv[i].s[4] = (cl_float) distributionSigmaInverseMatrices[i][1][0];
            sigmainv[i].s[5] = (cl_float) distributionSigmaInverseMatrices[i][1][1];
            sigmainv[i].s[6] = (cl_float) distributionSigmaInverseMatrices[i][1][2];
            sigmainv[i].s[8] = (cl_float) distributionSigmaInverseMatrices[i][2][0];
            sigmainv[i].s[9] = (cl_float) distributionSigmaInverseMatrices[i][2][1];
            sigmainv[i].s[10]= (cl_float) distributionSigmaInverseMatrices[i][2][2];

            sigmainv[i].s[3] = sigmainv[i].s[7]  = sigmainv[i].s[11] = sigmainv[i].s[12] = 
                               sigmainv[i].s[13] = sigmainv[i].s[14] = sigmainv[i].s[15] = 0.0; 
        }

        sigmainvbuffer = cl::Buffer(context,
                                    CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                                    clDistributionCount * sizeof(cl_float16),
                                    sigmainv, 
                                    nullptr);

        splitKernel.setArg(27, sigmainvbuffer);

        delete[] sigmainv;
        sigmainv = nullptr;

        cl_uchar *label = new cl_uchar[clDistributionCount];
        if (label == nullptr) 
        { 
            std::cout << __FILE__ << ", " << __LINE__ << "Could not allocate memory for " 
                      << clDistributionCount << std::endl; 
            exit(0); 
        }

        for (int i = 0; i<clDistributionCount; i++)
        {
            label[i] = (cl_uchar) distributionLabels[i];
        }

        labelbuffer = cl::Buffer(context,
                                 CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                                 clDistributionCount * sizeof(cl_uchar),
                                 label, 
                                 nullptr);

        splitKernel.setArg(28, labelbuffer);

        delete[] label;
        label = nullptr;

        countbuffer[0] = 0;
        countbuffer[1] = 0;
        countbuffer[2] = 0; 

        totalDistributionCountMemoryObject =  cl::Buffer(context,
                                                         CL_MEM_READ_WRITE,
                                                         sizeof(cl_int)*30, //30 ???
                                                         nullptr, 
                                                         nullptr);
        retval = true;
    }
    catch (cl::Error err)
    {
        
        std::cerr << __FUNCTION__ << ": OpenCL function " << err.what() << ", error code " 
                  << err.err() << ", \"" << clib.getErrorString(err.err()) << "\"" 
                  << std::endl;
        retval = false;
    }
    catch(cl_int err)
    {
        std::cerr << "\nException caught: " << __FUNCTION__ << ": OpenCL Error = " << err << "\n";
        switch(err)
        {
        case -1000 : 
            std::cerr << "  clGetGLContextInfoKHR, clCreateContext  CL and GL not on the same device (only when using a GPU)." << std::endl;
            break;
        }
        retval = false;
    }

    return retval;
}
