// init_oct_tree_empty.cpp

#include "BreastPhantomGenerator.h"

// This function initializes 3D trees
void BreastPhantomGenerator::init_oct_tree_empty(
                            float xlow, float xhigh,
                            float ylow, float yhigh,
                            float zlow, float zhigh,
                            float xminsize, float yminsize, float zminsize,
                            float a, float b, float cprim, float csec, float d, 
                            unsigned char max_depth)
{

    maskAInverseSquared     = 1 / (a * a);
    maskBInverseSquared     = 1 / (b * b);
    maskCPrimInverseSquared = 1 / (cprim *  cprim);
    maskCSecInverseSquared  = 1 / (csec * csec);

    skinAInverseSquared     = 1 / ((a - d) * (a - d));
    SkinBInverseSquared     = 1 / ((b - d) * (b - d));
    skinCPrimInverseSquared = 1 / ((cprim - d)  * (cprim - d));
    skinCSecInverseSquared  = 1 / ((csec - d) * (csec - d));

    PROGRESSINCREMENT = 5;  // auxillary, show progress each 5 percent


    //  Dimensionality=3; // works only for 3D
    xMinSize =  xminsize;
    yMinSize =  yminsize;
    zMinSize =  zminsize; // should be set to 0 if dimensionality is 2
    dimensionality  = 3;

    //  Array is structure of arrays, not vice versa

    currentNodeCount=1; //  new element, number of nodes in the level!
    xLows.clear();
    xHighs.clear();
    yLows.clear();
    yHighs.clear();
    zLows.clear();
    zHighs.clear();

    xLows.push_back(xlow);
    xHighs.push_back(xhigh);
    yLows.push_back(ylow);
    yHighs.push_back(yhigh);
    zLows.push_back(zlow);
    zHighs.push_back(zhigh);

    toSplits.clear();
    toSplits.push_back(1);  // keep in array, since, when saving the last level, there're no more checks to split

    //  Assume that initial (root) node is always split
    if  (labels.empty())
    {
        labels.push_back(4);
    }
    else
    {
        labels[0]   = 4;
    }

    //  Min and max global box dimensions
    xLow  = xlow;
    xHigh = xhigh;
    yLow  = ylow;
    yHigh = yhigh;
    zLow  = zlow;
    zHigh = zhigh;

    voxelHalfSize = (xhigh  - xlow) / 2; // assume voxel size is same in all 3 dimensions (original box is quadratic)
    deltaX         = xhigh  - xlow;
    squaredDeltaX = (xhigh  - xlow) * (xhigh - xlow); // for speed (init'd in recursive_data_split)

    //  Initialize voxels here
    voxelResolution = columnVector((xHigh - xLow) / VoxelDimension, 
                                   (yHigh - yLow) / VoxelDimension, 
                                   (zHigh - zLow) / VoxelDimension);

    distributionPools.clear();  // clean previous content
    std::vector<short>  aa;

    for (int i=0; i<mDistributionCount; i++)
    {
        aa.push_back(i);
    }

    distributionPools.push_back(aa);
}
