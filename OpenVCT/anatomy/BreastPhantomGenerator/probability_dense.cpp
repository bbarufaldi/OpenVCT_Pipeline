// probability_dense.cpp

#include "BreastPhantomGenerator.h"
#include <cmath>

// Function to be written: returns probabilty density that a point belongs to
// a dense region
// 1/13/2011
// Just for test
// I created "distribution" UGLY HARD-CODED
// 1/17/2011 Modified a bit: First: We want to measure distance from [a,0,0]
// not from [0,0,0]. Second, we want to have a constant 
// 
// a,b,cprim, csec are phantom dimensions
// sigma is a proportionality coefficients to compute density


float BreastPhantomGenerator::probability_dense(rowVector point, float a, float b, float cprim, float csec, float sigma)
{
    // global skinAInverseSquared SkinBInverseSquared skinCPrimInverseSquared
    // skinCSecInverseSquared CANNOT BE HERE, NOT DEFINED
    float x = point[0];
    float y = point[1];
    float z = point[2];
    float s = 0;

    if (z >= 0)
    {
        s = s + z * z / (cprim * cprim);
    }
    else
    {
        s = s + z * z / (csec * csec);
    }
    s = s + (x - a) * (x - a) / (a * a) + (y * y) / (b * b); //% 1/17/2011 a subtracted, we measure from a nipple

    return std::exp(-s*sigma); 
}


float BreastPhantomGenerator::probability_denseBeta(rowVector point, float a, float b, float cprim, float csec, 
                                                    float sigma, float alpha1, float beta1, float alpha2, float beta2)
{
    // global skinAInverseSquared SkinBInverseSquared skinCPrimInverseSquared
    // skinCSecInverseSquared CANNOT BE HERE, NOT DEFINED
    float x = point[0];
    float y = point[1];
    float z = point[2];

    float c = (cprim - csec) * 0.5f * (a - x) / a;

    float r;

    float T = float(std::sqrt(1.0 - x * x / (a * a))) * cprim;
    float B = float(std::sqrt(1.0 - x * x / (a * a))) * csec;

    r = max(T-c, B+c);

    float x1 = x/a;
    float x2 = std::sqrt(y*y+(z-c)*(z-c))/r; 

    float output1 = std::pow(x1, alpha1-1.0f);
    float output2 = std::pow(1.0f - x1, beta1-1.0f);
    float output3 = std::pow(x2, alpha2-1.0f);
    float output4 = std::pow(1.0f - x2, beta2-1.0f);

    return output1*output2*output3*output4; 
}
