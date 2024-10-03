// BreastPhantomGenerator.cpp

#include "BreastPhantomGenerator.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>


BreastPhantomGenerator::BreastPhantomGenerator() 
    : phantom_shape(0), // Breast
      xsize(0), ysize(0), zsize(0),
      phantom(nullptr),
      voxelValues(nullptr),
      totalNodeCount(0),
      mDistributionCount(0),
      bookKeepingBufferSize(0),
      voxelizeBufferSize(0),
      verbose(false)
{
     xmemorySize[0]      = xmemorySize[1]      = 0;
     distributionSize[0] = distributionSize[1] = 0;
}



BreastPhantomGenerator::~BreastPhantomGenerator()
{    
    if (voxelValues != nullptr)
    {
        delete [] voxelValues;
        voxelValues = nullptr;
    }
    
    if (phantom != nullptr)
    {
        delete phantom;
        phantom = nullptr;
    }
    
   distributionPools.clear();
   labels.clear(); 
   NodeX.clear();
   NodeY.clear();
   NodeZ.clear(); 

   toSplits.clear(); 

   xLows.clear(); 
   xHighs.clear(); 
   yLows.clear(); 

   yHighs.clear(); 
   zLows.clear(); 
   zHighs.clear(); 

   AVERAGEDISTR.clear();
   AVERAGEDISTRALL.clear();
  
   // Distribution vectors of matrices
   distributionA1InverseMatrices.clear(); 
   distributionA2InverseMatrices.clear();
   distributionA3InverseMatrices.clear();
   distributionHMatrices.clear();
   distributionSigmaInverseMatrices.clear();
   distributionSigmaInverseMUVectors.clear();
   distributionLabels.clear();
   distributionLogSqrtSigmaLogPriors.clear();
   distributionMUVectors.clear();
   distributionPriors.clear();
   distributionRMatrices.clear();
   distributionUMatrices.clear();  

   NUMBER_AIRLEAVES.clear();
   NUMBER_LEAVES.clear();
   NUMBER_NONLEAVES.clear();

   //DH 2019-03: SUSPECT_NOT_USED!  TOTALCOOPER.clear();
   //DH 2019-03: SUSPECT_NOT_USED!  TOTALCOOPERALL.clear();

   // TOTAL_DEPTH
   TOTALDISTR.clear();
   TOTALDISTRALL.clear();

   if (compressedBuffer != nullptr)
   {
       delete [] compressedBuffer;
       compressedBuffer = nullptr;
   }
  
   devices.clear();   
}
