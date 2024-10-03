// recursive_data_split.cpp

#include "BreastPhantomGenerator.h"
#include "VCT_CommonLibrary.h"

#include "vector3.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>


void BreastPhantomGenerator::recursive_data_split(float a, float b, float cprim, float csec,
                                                  int max_depth, float Dmin, // Dmin: half Cooper's ligament thickness             
                                                  cl::Buffer &voxelbuffer,
                                                  std::vector<int> origin) // maximum number of levels
{
    int cl_result = 0; // result from OpenCL commands
    
    try 
    {
        int toSplitCount = 1;
        cl_float2 xRoot, yRoot, zRoot; // place holders for voxel size at level 0 
        
        cl::Buffer  oldXMemoryObject, oldYMemoryObject, oldZMemoryObject, 
                    oldLabelMemoryObject, oldToSplitMemoryObject, 
                    oldDistributionCountMemoryObject, oldDistributionMemoryObject;      

        cl_int4 o; // for storing init'l recusion data

        o.s[0] = origin[0];
        o.s[1] = origin[1];
        o.s[2] = origin[2];
        o.s[3] = 0;

        xRoot.s[0] = xLows[0];
        xRoot.s[1] = xHighs[0];
        yRoot.s[0] = yLows[0];
        yRoot.s[1] = yHighs[0];
        zRoot.s[0] = zLows[0];
        zRoot.s[1] = zHighs[0];
    
        cl_float2 xExtent, yExtent, zExtent; // phantom dimensions
        xExtent.s[0] = 0.0f;
        xExtent.s[1] = a;
        yExtent.s[0] = -b;
        yExtent.s[1] = b;
        zExtent.s[0] = -csec;
        zExtent.s[1] = cprim;

        // Memory objects used at level 0
        
        short oldDistributionCount = short(distributionPools[0].size());
        cl_int oldDistributionHeads = 0;

        if (1 > xmemorySize[0])
        {
            xMemoryObject[0] = cl::Buffer(context,
                                          CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                                          sizeof(cl_float2),
                                          &xRoot, NULL);
            yMemoryObject[0] = cl::Buffer(context,
                                          CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                                          sizeof(cl_float2),
                                          &yRoot, NULL);
            zMemoryObject[0] = cl::Buffer(context,
                                          CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                                          sizeof(cl_float2),
                                          &zRoot, NULL);
            LabelMemoryObject[0] = cl::Buffer(context,
                                              CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
                                              sizeof(cl_uchar),
                                              &labels[0], NULL);
            toSplitMemoryObject[0] = cl::Buffer(context,
                                                CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
                                                sizeof(cl_uchar),
                                                &toSplits[0], NULL);

            this->compressedBuffer = new cl_float2[1];

            Xextents  = cl::Buffer(context, // DH 2019-07-02
                                   CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                                   sizeof(cl_float2),
                                   &xExtent, NULL);

            Yextents  = cl::Buffer(context, // DH 2019-07-02
                                   CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                                   sizeof(cl_float2),
                                   &yExtent, NULL);

            Zextents  = cl::Buffer(context, // DH 2019-07-02
                                   CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                                   sizeof(cl_float2),
                                   &zExtent, NULL);
        }
        else
        {
            clCommandQueue.enqueueWriteBuffer(xMemoryObject[0], CL_FALSE, 0, sizeof(cl_float2), &xRoot);
            clCommandQueue.enqueueWriteBuffer(yMemoryObject[0], CL_FALSE, 0, sizeof(cl_float2), &yRoot);
            clCommandQueue.enqueueWriteBuffer(zMemoryObject[0], CL_FALSE, 0, sizeof(cl_float2), &zRoot);
            clCommandQueue.enqueueWriteBuffer(LabelMemoryObject[0], CL_FALSE, 0, sizeof(cl_uchar), &labels[0]);
            clCommandQueue.enqueueWriteBuffer(toSplitMemoryObject[0], CL_FALSE, 0, sizeof(cl_uchar), &toSplits[0]);

            clCommandQueue.enqueueWriteBuffer(Xextents, CL_FALSE, 0, sizeof(cl_float2), &xExtent); // DH 2019-07-02
            clCommandQueue.enqueueWriteBuffer(Yextents, CL_FALSE, 0, sizeof(cl_float2), &yExtent); // DH 2019-07-02
            clCommandQueue.enqueueWriteBuffer(Zextents, CL_FALSE, 0, sizeof(cl_float2), &zExtent); // DH 2019-07-02
        }

        oldXMemoryObject = xMemoryObject[0];
        oldYMemoryObject = yMemoryObject[0];
        oldZMemoryObject = zMemoryObject[0];
        oldLabelMemoryObject = LabelMemoryObject[0];
        oldToSplitMemoryObject = toSplitMemoryObject[0];
    
        cl::Buffer oldDistributionHeadsMemoryObject = headsMemoryObject[0];     
        
        if (1 > distributionSize[0])
        {
            headsMemoryObject[0]= cl::Buffer(context,
                                        CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,
                                        sizeof(cl_int),
                                        &oldDistributionHeads, NULL);
            distributionMemoryObject[0] = cl::Buffer(context,
                                        CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
                                        mDistributionCount*sizeof(cl_short),
                                        &distributionPools[0][0], NULL);
            distributionCountMemoryObject[0] = cl::Buffer(context,
                                        CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
                                        sizeof(cl_short),
                                        &oldDistributionCount, NULL);
             distributionSize[0] = oldDistributionCount;
        }
        else
        {
            clCommandQueue.enqueueWriteBuffer(headsMemoryObject[0], CL_FALSE, 0, sizeof(cl_int), &oldDistributionHeads);
            clCommandQueue.enqueueWriteBuffer(distributionMemoryObject[0], CL_FALSE, 0,mDistributionCount* sizeof(cl_short), &distributionPools[0][0]);
            clCommandQueue.enqueueWriteBuffer(distributionCountMemoryObject[0], CL_FALSE, 0, sizeof(cl_short), &oldDistributionCount);
        }

        oldDistributionHeadsMemoryObject=headsMemoryObject[0];
        oldDistributionMemoryObject = distributionMemoryObject[0];
        oldDistributionCountMemoryObject =distributionCountMemoryObject[0];

        currentNodeCount = 1;
        size_t totalDistributionLength = mDistributionCount;
        size_t maxDistributionLength = mDistributionCount* 64;
    
        int oldNodeCount = 1;

        // Memory objects created for each level (declared this way so they can be cleaned up after recursion)
        
        cl_int splitCount = 0, clDistributionCount = 0;
    
        if (bookKeepingBufferSize < 1)
        {
                bookKeepingBufferSize =  1;
                totaldistributionCountmem  =  cl::Buffer(context,
                                        CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
                                        sizeof(cl_int),
                                        &clDistributionCount, NULL);
                toSplitCountMemoryObject = cl::Buffer(context,
                                        CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
                                        sizeof(cl_int),
                                        &splitCount, NULL);
        }
        else
        {
            clCommandQueue.enqueueWriteBuffer(totaldistributionCountmem, CL_FALSE, 0, sizeof(cl_int),
                                              &clDistributionCount);
            clCommandQueue.enqueueWriteBuffer(toSplitCountMemoryObject, CL_FALSE, 0, sizeof(cl_int),
                                              &splitCount);
        }

        int e1 = addupKernel.setArg(1, toSplitCountMemoryObject);
        int e2 = wsAddupKernel.setArg(1, toSplitCountMemoryObject);

        // Allocate deflate state

        int bufferside = 0;

        newVoxelizeCount = 0; // don't voxelize first. 

        int r1, r2;
        cl_float fmin = Dmin; // Cooper's ligament thickness


        for (int depth = 0; depth < max_depth; depth++) // always start from CURRENT depth in the tree
        {
            if (currentNodeCount == 0) break;

            bufferside = 1 - depth % 2;
            currentNodeCount = currentNodeCount * 8;               // split each node into 8 children 
            totalDistributionLength = totalDistributionLength * 8; // each child has its owned distribution
            maxDistributionLength = maxDistributionLength * 8; 
            newVoxelHalfSize = voxelHalfSize / 2;                  // node size is cut by half
            newDeltaX = deltaX / 2;                                // new delta size but by half 
            newDeltaXSquared = newDeltaX * newDeltaX;              // for fast function computation
    
            voxelHalfSize = newVoxelHalfSize; 
            deltaX = newDeltaX;
            squaredDeltaX = newDeltaXSquared;
    
            totalNodeCount = totalNodeCount + currentNodeCount;
    
            float VoxelSize=voxelHalfSize*2;

            cl_float4 low; 
            low.s[0] = xLow; // corner of the sub volume
            low.s[1] = yLow;
            low.s[2] = zLow;
            low.s[3] = 0.0f;

            // Perturb ligament thickness
            if (Dmin != 0.0) // DH 2019-03: Don't add randomness if ligament thickness is zero
            {      
                r1 = rand() % 2;
                r2 = rand() % 9 + 1;          
                #if defined(STORE_RANDOM)
                vct_rand.addRandom(r1);
                vct_rand.addRandom(r2);
                #elif defined(READ_RANDOM)
                r1 = (int)vct_rand.nextRandom();
                r2 = (int)vct_rand.nextRandom();
                #endif
    
                if (r1 == 0)
                {
                    fmin = static_cast<float>(fmin - r2 * 0.01);
                }
                else
                {
                    fmin = static_cast<float>(fmin + r2 * 0.01);
                }
    
                if (fmin <= Dmin)
                {
                    fmin = Dmin;
                }
    
                if (fmin > 1.5f * Dmin)
                {
                    fmin = 1.5f * Dmin;
                }
            }

            cl_float4 resolution;
            resolution.s[0] = voxelResolution[0];
            resolution.s[1] = voxelResolution[1];
            resolution.s[2] = voxelResolution[2];
            resolution.s[3] = 0.0f;

            // x min max of each child
            if (currentNodeCount > xmemorySize[bufferside])
            {
                // x min max of each child
                xMemoryObject[bufferside] = cl::Buffer(context,
                                                       CL_MEM_READ_WRITE,
                                                       currentNodeCount * sizeof(cl_float2),
                                                       NULL, NULL);
                xmemorySize[bufferside] = currentNodeCount;

                // y min max of each child
                yMemoryObject[bufferside] = cl::Buffer(context,
                                                       CL_MEM_READ_WRITE,
                                                       currentNodeCount * sizeof(cl_float2),
                                                       NULL, NULL);
                // z min max of each child
                zMemoryObject[bufferside] = cl::Buffer(context,
                                                       CL_MEM_READ_WRITE,
                                                       currentNodeCount * sizeof(cl_float2),
                                                       NULL, NULL);
            
                // label of each child
                LabelMemoryObject[bufferside] = cl::Buffer(context,
                                                           CL_MEM_READ_WRITE,
                                                           currentNodeCount * sizeof(cl_uchar),
                                                           NULL, NULL);

                // to split field of each child
                toSplitMemoryObject[bufferside] = cl::Buffer(context,
                                                             CL_MEM_READ_WRITE,
                                                             currentNodeCount * sizeof(cl_uchar),
                                                             NULL, NULL);

                // distribution count for each child
                distributionCountMemoryObject[bufferside] = cl::Buffer(context,
                                                                       CL_MEM_READ_WRITE,
                                                                       currentNodeCount * sizeof(cl_short),
                                                                       NULL, NULL);

                 headsMemoryObject[bufferside] = cl::Buffer(context,
                                                            CL_MEM_READ_WRITE,
                                                            currentNodeCount * sizeof(cl_int),
                                                            NULL, NULL);

                 delete [] compressedBuffer;
                 compressedBuffer = new cl_float2[currentNodeCount];
            }

            if (totalDistributionLength > distributionSize[bufferside])
            {
                // global memory for distributions
                distributionMemoryObject[bufferside] = cl::Buffer(context,
                                            CL_MEM_READ_WRITE,
                                            totalDistributionLength*sizeof(cl_short),
                                            NULL, NULL);
                
                if (totalDistributionLength > distributionSize[1-bufferside])
                {
                    nodeBuffer = cl::Buffer(context,
                                            CL_MEM_READ_WRITE,
                                            totalDistributionLength*sizeof(cl_int),
                                            NULL, NULL);

                    // scratch space for each child
                    distanceBuffer = cl::Buffer(context,
                                                CL_MEM_READ_WRITE,
                                                totalDistributionLength*sizeof(cl_float2),
                                                NULL, NULL);
                }

                distributionSize[bufferside] = totalDistributionLength;
            }

    
            if (newVoxelizeCount > voxelizeBufferSize)
            {
                voxelizeBuffer = cl::Buffer(context,
                                            CL_MEM_READ_WRITE,
                                            newVoxelizeCount*sizeof(cl_long),
                                            NULL, NULL);
                voxelizeBufferSize = newVoxelizeCount;
            }
            else if ((newVoxelizeCount == 0) && (voxelizeBufferSize == 0))
            {
                voxelizeBuffer = cl::Buffer(context,
                                            CL_MEM_READ_WRITE,
                                            sizeof(cl_long),
                                            NULL, NULL);

                voxelizeCountMemoryObject = cl::Buffer(context,
                                                       CL_MEM_READ_WRITE,
                                                       sizeof(cl_int),
                                                       NULL, NULL);
            }
        
			// DEBUG 2020-11-24
            #if defined(DEBUG_SPLITKERNEL)
			static int count = 0;
			std::ofstream ofs("splitKernelArgs.dat", std::ios::app);
			ofs << "\n\n" << count++ << ": " << __TIMESTAMP__ << "\n\n";
			try
			{
				cl_float2 xmo, ymo, zmo;
				clCommandQueue.enqueueReadBuffer(xMemoryObject[bufferside], true, 0, sizeof(cl_float2), &xmo);
				clCommandQueue.enqueueReadBuffer(yMemoryObject[bufferside], true, 0, sizeof(cl_float2), &ymo);
				clCommandQueue.enqueueReadBuffer(zMemoryObject[bufferside], true, 0, sizeof(cl_float2), &zmo);
				ofs << "xMemoryObject[" << bufferside << "]: \t" << xmo.s[0] << ", \t" << xmo.s[1] << "\n";
				ofs << "yMemoryObject[" << bufferside << "]: \t" << ymo.s[0] << ", \t" << ymo.s[1] << "\n";
				ofs << "zMemoryObject[" << bufferside << "]: \t" << zmo.s[0] << ", \t" << zmo.s[1] << "\n\n";
			}
			catch (cl::Error err)
			{
				std::cerr << __FUNCTION__ << ": OpenCL function " << err.what() << ", error code "
					<< err.err() << std::endl;
			}
			ofs << "maskAInverseSquared:     " << maskAInverseSquared << "\n";
			ofs << "maskBInverseSquared:     " << maskBInverseSquared << "\n";
			ofs << "maskCPrimInverseSquared: " << maskCPrimInverseSquared << "\n";
			ofs << "maskCSecInverseSquared:  " << maskCSecInverseSquared << "\n\n";
			ofs << "skinAInverseSquared:     " << maskAInverseSquared << "\n";
			ofs << "skinBInverseSquared:     " << maskBInverseSquared << "\n";
			ofs << "skinCPrimInverseSquared: " << maskCPrimInverseSquared << "\n";
			ofs << "skinCSecInverseSquared:  " << maskCSecInverseSquared << "\n\n";
			ofs.close();
            #endif
			// DEBUG 2020-11-24

            splitKernel.setArg(0, low);
            splitKernel.setArg( 1, fmin);
            splitKernel.setArg( 2, deltaX);
            splitKernel.setArg( 3, squaredDeltaX);
            splitKernel.setArg( 4, voxelHalfSize);
            splitKernel.setArg( 5, voxelbuffer);
            splitKernel.setArg( 6, xMemoryObject[bufferside]); 
            splitKernel.setArg( 7, yMemoryObject[bufferside]);
            splitKernel.setArg( 8, zMemoryObject[bufferside]);
            splitKernel.setArg( 9, LabelMemoryObject[bufferside]);
            splitKernel.setArg( 10, toSplitMemoryObject[bufferside]);
            splitKernel.setArg( 11, resolution);
            splitKernel.setArg( 12, VoxelDimension);
            splitKernel.setArg( 13, maskAInverseSquared);
            splitKernel.setArg( 14, maskBInverseSquared);
            splitKernel.setArg( 15, maskCPrimInverseSquared);
            splitKernel.setArg( 16, maskCSecInverseSquared);
            splitKernel.setArg( 17, skinAInverseSquared);
            splitKernel.setArg( 18, SkinBInverseSquared);
            splitKernel.setArg( 19, skinCPrimInverseSquared);
            splitKernel.setArg( 20, skinCSecInverseSquared);
            splitKernel.setArg( 21, distributionMemoryObject[bufferside]);
            splitKernel.setArg( 22, distributionCountMemoryObject[bufferside]);

            // Shape data set at the initialization once.
            splitKernel.setArg( 29, currentNodeCount);
            splitKernel.setArg( 30, headsMemoryObject[bufferside]);
            splitKernel.setArg( 31, distanceBuffer);    

            splitKernel.setArg( 32, Xextents); // DH 2019-07-02
            splitKernel.setArg( 33, Yextents); // DH 2019-07-02
            splitKernel.setArg( 34, Zextents); // DH 2019-07-02
            splitKernel.setArg( 35, phantom_shape); // DH 2020-11-30           
            
            evalKernel.setArg( 0, xMemoryObject[bufferside]); 
            evalKernel.setArg( 1, yMemoryObject[bufferside]);
            evalKernel.setArg( 2, zMemoryObject[bufferside]);
            evalKernel.setArg( 3, nodeBuffer);
            evalKernel.setArg( 4, distributionMemoryObject[bufferside]);

            evalKernel.setArg( 5,  this->mubuffer);
            evalKernel.setArg( 6,  this->rbuffer);
            evalKernel.setArg( 7,  this->logbuffer);
            evalKernel.setArg( 8,  this->bbbuffer);
            evalKernel.setArg( 9,  this->a1buffer);
            evalKernel.setArg( 10, this->a2buffer);
            evalKernel.setArg( 11, this->a3buffer);
            evalKernel.setArg( 12, this->hbuffer);
            evalKernel.setArg( 13, this->u1buffer);
            evalKernel.setArg( 14, this->u2buffer);
            evalKernel.setArg( 15, this->u3buffer);
            evalKernel.setArg( 16, deltaX);
            evalKernel.setArg( 17, this->distanceBuffer);
            
            cl_int splitCount = 1, clDistributionCount = mDistributionCount;

            // Arguments for the kernel to create child nodes 

            createNodesKernel.setArg( 0, oldToSplitMemoryObject);
            
            createNodesKernel.setArg( 1, oldXMemoryObject);
            createNodesKernel.setArg( 2, oldYMemoryObject);
            createNodesKernel.setArg( 3, oldZMemoryObject);
            createNodesKernel.setArg( 4, oldLabelMemoryObject);
            createNodesKernel.setArg( 5, oldDistributionMemoryObject);
            createNodesKernel.setArg( 6, oldDistributionCountMemoryObject);
            createNodesKernel.setArg( 7, oldDistributionHeadsMemoryObject);
            createNodesKernel.setArg( 8, xMemoryObject[bufferside]);
            createNodesKernel.setArg( 9, yMemoryObject[bufferside]);
            createNodesKernel.setArg( 10, zMemoryObject[bufferside]);
            createNodesKernel.setArg( 11, LabelMemoryObject[bufferside]);
            createNodesKernel.setArg( 12, distributionMemoryObject[bufferside]);
            createNodesKernel.setArg( 13, distributionCountMemoryObject[bufferside]);
            createNodesKernel.setArg( 14, headsMemoryObject[bufferside]);       
            createNodesKernel.setArg( 15, cl::Local(sizeof(cl_int)));            
            createNodesKernel.setArg( 16, cl::Local(sizeof(cl_int)));   
            createNodesKernel.setArg( 17, oldNodeCount); 
            createNodesKernel.setArg( 18, totaldistributionCountmem);           
            createNodesKernel.setArg( 19, toSplitCountMemoryObject);    
            createNodesKernel.setArg( 20, nodeBuffer);
            createNodesKernel.setArg( 21, totalDistributionLength/8);
            createNodesKernel.setArg( 22, (int) currentNodeCount/8);
            createNodesKernel.setArg( 23, voxelizeBuffer);  
            createNodesKernel.setArg( 24, voxelizeCountMemoryObject);
            createNodesKernel.setArg( 25, cl::Local(sizeof(cl_int)));

            createNodesKernel2.setArg( 0, oldToSplitMemoryObject);   // NOT USED
            createNodesKernel2.setArg( 1, xMemoryObject[bufferside]);
            createNodesKernel2.setArg( 2, yMemoryObject[bufferside]);
            createNodesKernel2.setArg( 3, zMemoryObject[bufferside]);
            createNodesKernel2.setArg( 4, LabelMemoryObject[bufferside]);
            createNodesKernel2.setArg( 5, distributionMemoryObject[bufferside]);
            createNodesKernel2.setArg( 6, distributionCountMemoryObject[bufferside]);
            createNodesKernel2.setArg( 7, headsMemoryObject[bufferside]);

            createNodesKernel2.setArg( 8, currentNodeCount/8);  
            createNodesKernel2.setArg( 9, (int)(totalDistributionLength / 8));

            
            // Invoke kernel to create children
            cl::NDRange global_work_size(size_t(64.0 * (ceil(oldNodeCount/64.0)))), local_work_size(64);
            cl::NDRange global_work_sizeb(size_t(64.0 * (ceil(currentNodeCount/8.0/64.0))));    

            
            clCommandQueue.enqueueNDRangeKernel(createNodesKernel, cl::NullRange, global_work_size, local_work_size);
            clCommandQueue.finish();
          
            clCommandQueue.enqueueNDRangeKernel(createNodesKernel2, cl::NullRange, global_work_sizeb, local_work_size);
            clCommandQueue.finish(); // (perform) & sync
            
            if (voxelizeBufferSize)
            {
                cl_float4 clLow; 

                clLow.s[0] = xLow;
                clLow.s[1] = yLow;
                clLow.s[2] = zLow;
                clLow.s[3] = 0.0f;

                cl_float4 resolution4;

                resolution4.s[0] = voxelResolution[0];
                resolution4.s[1] = voxelResolution[1];
                resolution4.s[2] = voxelResolution[2];
                resolution4.s[3] = 0.0f;    // convert recursion result into voxels
            
                voxelizeKernel.setArg( 0, oldXMemoryObject);
                voxelizeKernel.setArg( 1, oldYMemoryObject);
                voxelizeKernel.setArg( 2, oldZMemoryObject);
                voxelizeKernel.setArg( 3, voxelbuffer);
                voxelizeKernel.setArg( 4, oldLabelMemoryObject);
                voxelizeKernel.setArg( 5, clLow);

                int  multiple = 1;
                int nonZeroSize;

                if (newVoxelizeCount == 0) 
                {
                    nonZeroSize = 1;
                }
                else
                {
                    nonZeroSize =  newVoxelizeCount;
                }

                voxelizeKernel.setArg( 6, newVoxelizeCount*multiple);
                voxelizeKernel.setArg( 7, resolution4);
                voxelizeKernel.setArg( 8, VoxelDimension);
                voxelizeKernel.setArg( 9, voxelizeBuffer);
                voxelizeKernel.setArg( 10, (short) multiple );
                voxelizeKernel.setArg( 11, o );

                cl::NDRange global_work_sizevoxel(size_t(256.0 * (ceil((nonZeroSize)/256.0)))), local_work_size_voxel(256);
                
                clCommandQueue.enqueueNDRangeKernel(voxelizeKernel, cl::NullRange, global_work_sizevoxel,  local_work_size_voxel);
                clCommandQueue.finish(); // (perform) & sync

            } // if voxelizeBufferSize
            
            clCommandQueue.enqueueCopyBuffer(LabelMemoryObject[bufferside], LabelMemoryObject[bufferside], 0, currentNodeCount/8, sizeof(cl_uchar)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(LabelMemoryObject[bufferside], LabelMemoryObject[bufferside], 0, 2*currentNodeCount/8, sizeof(cl_uchar)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(LabelMemoryObject[bufferside], LabelMemoryObject[bufferside], 0, 3*currentNodeCount/8, sizeof(cl_uchar)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(LabelMemoryObject[bufferside], LabelMemoryObject[bufferside], 0, 4*currentNodeCount/8, sizeof(cl_uchar)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(LabelMemoryObject[bufferside], LabelMemoryObject[bufferside], 0, 5*currentNodeCount/8, sizeof(cl_uchar)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(LabelMemoryObject[bufferside], LabelMemoryObject[bufferside], 0, 6*currentNodeCount/8, sizeof(cl_uchar)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(LabelMemoryObject[bufferside], LabelMemoryObject[bufferside], 0, 7*currentNodeCount/8, sizeof(cl_uchar)*currentNodeCount/8);            

            clCommandQueue.enqueueCopyBuffer(distributionCountMemoryObject[bufferside], distributionCountMemoryObject[bufferside], 0, sizeof(cl_short)*  currentNodeCount/8, sizeof(cl_short)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(distributionCountMemoryObject[bufferside], distributionCountMemoryObject[bufferside], 0, sizeof(cl_short)*2*currentNodeCount/8, sizeof(cl_short)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(distributionCountMemoryObject[bufferside], distributionCountMemoryObject[bufferside], 0, sizeof(cl_short)*3*currentNodeCount/8, sizeof(cl_short)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(distributionCountMemoryObject[bufferside], distributionCountMemoryObject[bufferside], 0, sizeof(cl_short)*4*currentNodeCount/8, sizeof(cl_short)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(distributionCountMemoryObject[bufferside], distributionCountMemoryObject[bufferside], 0, sizeof(cl_short)*5*currentNodeCount/8, sizeof(cl_short)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(distributionCountMemoryObject[bufferside], distributionCountMemoryObject[bufferside], 0, sizeof(cl_short)*6*currentNodeCount/8, sizeof(cl_short)*currentNodeCount/8);
            clCommandQueue.enqueueCopyBuffer(distributionCountMemoryObject[bufferside], distributionCountMemoryObject[bufferside], 0, sizeof(cl_short)*7*currentNodeCount/8, sizeof(cl_short)*currentNodeCount/8);

            clCommandQueue.enqueueCopyBuffer(distributionMemoryObject[bufferside], distributionMemoryObject[bufferside], 0, sizeof(cl_short)* totalDistributionLength/8, sizeof(cl_short)*totalDistributionLength/8);
            clCommandQueue.enqueueCopyBuffer(distributionMemoryObject[bufferside], distributionMemoryObject[bufferside], 0, sizeof(cl_short)*2*totalDistributionLength/8, sizeof(cl_short)*totalDistributionLength/8);
            clCommandQueue.enqueueCopyBuffer(distributionMemoryObject[bufferside], distributionMemoryObject[bufferside], 0, sizeof(cl_short)*3*totalDistributionLength/8, sizeof(cl_short)*totalDistributionLength/8);
            clCommandQueue.enqueueCopyBuffer(distributionMemoryObject[bufferside], distributionMemoryObject[bufferside], 0, sizeof(cl_short)*4*totalDistributionLength/8, sizeof(cl_short)*totalDistributionLength/8);
            clCommandQueue.enqueueCopyBuffer(distributionMemoryObject[bufferside], distributionMemoryObject[bufferside], 0, sizeof(cl_short)*5*totalDistributionLength/8, sizeof(cl_short)*totalDistributionLength/8);
            clCommandQueue.enqueueCopyBuffer(distributionMemoryObject[bufferside], distributionMemoryObject[bufferside], 0, sizeof(cl_short)*6*totalDistributionLength/8, sizeof(cl_short)*totalDistributionLength/8);
            clCommandQueue.enqueueCopyBuffer(distributionMemoryObject[bufferside], distributionMemoryObject[bufferside], 0, sizeof(cl_short)*7*totalDistributionLength/8, sizeof(cl_short)*totalDistributionLength/8);
            
            cl::NDRange  local_work_size2(64), totalDistributionSize(64*(int)ceil(totalDistributionLength/64.0));
            global_work_size =cl::NDRange(64*(int)ceil(currentNodeCount/64.0));
            evalKernel.setArg( 18, totalDistributionLength);

            // Invoke kernel to decide to split and labeling
            clCommandQueue.enqueueNDRangeKernel(evalKernel, cl::NullRange, totalDistributionSize, local_work_size2);            
            clCommandQueue.finish();
            clCommandQueue.enqueueNDRangeKernel(splitKernel, cl::NullRange, global_work_size, local_work_size2);
            clCommandQueue.finish();             
            
            if (bookKeepingBufferSize < global_work_size[0])
            {
                bookKeepingBufferSize = int(global_work_size[0]);    // ADM   check if round/ceil ??
                totaldistributionCountmem = cl::Buffer(context,
                                                CL_MEM_READ_WRITE,
                                                sizeof(cl_int)*global_work_size[0],
                                                NULL, NULL);

                toSplitCountMemoryObject = cl::Buffer(context,
                                                CL_MEM_READ_WRITE,
                                                sizeof(cl_int)*global_work_size[0],
                                                NULL, NULL);

                voxelizeCountMemoryObject = cl::Buffer(context,
                                                CL_MEM_READ_WRITE,
                                                sizeof(cl_int)*global_work_size[0],
                                                NULL, NULL);
            }
            
            addupKernel.setArg(0, totaldistributionCountmem);   
            addupKernel.setArg(1, toSplitCountMemoryObject);
            addupKernel.setArg(2, distributionCountMemoryObject[bufferside]);
            addupKernel.setArg(3, toSplitMemoryObject[bufferside]); 
            addupKernel.setArg(4, currentNodeCount);
            addupKernel.setArg(5, cl::Local(sizeof(cl_int)));
            addupKernel.setArg(6, cl::Local(sizeof(cl_int)));
            addupKernel.setArg(7, voxelizeCountMemoryObject);
            addupKernel.setArg(8, cl::Local(sizeof(cl_int)));
            
            clCommandQueue.finish();

            // To add the total number of distribution counts and nodes to be split per workgroup
            clCommandQueue.enqueueNDRangeKernel(addupKernel, cl::NullRange, global_work_size, local_work_size2);
            clCommandQueue.finish();
            
            
            cl::NDRange global_group_work_size(size_t(64.0 * ceil( ceil( global_work_size[0] / 64.0 ) / 64.0 )));       
            int groupcount = static_cast<int>(ceil(currentNodeCount/64.0));
            
            int *buf = (int *)clCommandQueue.enqueueMapBuffer(totalDistributionCountMemoryObject, CL_TRUE, CL_MAP_WRITE|CL_MAP_READ,  0,  sizeof(cl_int)*3);    
    
            buf[0] = 0;
            buf[1] = 0;
            buf[2] = 0;
            
            clCommandQueue.enqueueUnmapMemObject(totalDistributionCountMemoryObject, buf);
            clCommandQueue.finish();

            cl_int totalDistributionCount=0, totalSplitCount=0;
        
            wsAddupKernel.setArg(0, totaldistributionCountmem);
            wsAddupKernel.setArg(1, toSplitCountMemoryObject);  
            wsAddupKernel.setArg(2, sizeof(cl_int), &groupcount);   
            wsAddupKernel.setArg(3, totalDistributionCountMemoryObject);
            wsAddupKernel.setArg(4, voxelizeCountMemoryObject);
            
            if (currentNodeCount >labels.size())
            {
                //labels.resize(currentNodeCount);
                labels.resize(currentNodeCount, 5); // DH 2019-08-23
            }
                        
            // To add the total number of distribution counts and nodes to be split.
            clCommandQueue.enqueueNDRangeKernel(wsAddupKernel, cl::NullRange, global_group_work_size, local_work_size2);            
            clCommandQueue.finish();
            

            // New objects become old object at next level               
            oldXMemoryObject = xMemoryObject[bufferside];        
            oldYMemoryObject = yMemoryObject[bufferside];        
            oldZMemoryObject = zMemoryObject[bufferside];   

            oldLabelMemoryObject   = LabelMemoryObject[bufferside];
            oldToSplitMemoryObject = toSplitMemoryObject[bufferside];

            oldDistributionCountMemoryObject = distributionCountMemoryObject[bufferside];
            oldDistributionMemoryObject      = distributionMemoryObject[bufferside];
            oldDistributionHeadsMemoryObject = headsMemoryObject[bufferside];
            
            // Read the total distrbution count and split count from OpenCL for the host to allocate global memory at next level
            buf = (int *)  clCommandQueue.enqueueMapBuffer(totalDistributionCountMemoryObject, CL_TRUE, CL_MAP_WRITE|CL_MAP_READ,  0,  sizeof(cl_int)*3);
            
            oldNodeCount = currentNodeCount; // cache it as old before update
            totalDistributionLength = buf[0];
            currentNodeCount = buf[1];
            newVoxelizeCount =  buf[2];
            clCommandQueue.enqueueUnmapMemObject(totalDistributionCountMemoryObject, buf);
            totalDistributionLength = max(totalDistributionLength, size_t(1)); 
      
        } // next depth
        
        cl_float4 clLow; 
        clLow.s[0] = xLow;
        clLow.s[1] = yLow;
        clLow.s[2] = zLow;
        clLow.s[3] = 0.0f;

        cl_float4 resolution4;
        resolution4.s[0] = voxelResolution[0];
        resolution4.s[1] = voxelResolution[1];
        resolution4.s[2] = voxelResolution[2];
        resolution4.s[3] = 0.0f; // to convert recursion result into voxels

        createLastLevelVoxelsKernel.setArg( 0, xMemoryObject[bufferside]);  //
        createLastLevelVoxelsKernel.setArg( 1, yMemoryObject[bufferside]);  //
        createLastLevelVoxelsKernel.setArg( 2, zMemoryObject[bufferside]);  //
        createLastLevelVoxelsKernel.setArg( 3, oldToSplitMemoryObject);     //
        createLastLevelVoxelsKernel.setArg( 4, voxelbuffer);                //
        createLastLevelVoxelsKernel.setArg( 5, oldLabelMemoryObject);       //
        createLastLevelVoxelsKernel.setArg( 6, clLow);                      //
        createLastLevelVoxelsKernel.setArg( 7, oldNodeCount);               //
        createLastLevelVoxelsKernel.setArg( 8, resolution4);                //
        createLastLevelVoxelsKernel.setArg( 9, VoxelDimension);             //
        createLastLevelVoxelsKernel.setArg( 10, o);                         // Origin
        
        cl::NDRange global_work_sizevoxel(size_t(64*(std::ceil(oldNodeCount/64.0)))), local_work_size_voxel(64);
        clCommandQueue.enqueueNDRangeKernel(createLastLevelVoxelsKernel, cl::NullRange, global_work_sizevoxel,  local_work_size_voxel);
    }
    catch (cl::Error exception)
    {
        vct::CommonLibrary clib;
        std::cerr << " Failure in call to: "<< exception.what() << ", error: " 
                  << clib.getErrorString(exception.err()) << " (" << exception.err()
                  << ")" << std::endl;
    }
}
