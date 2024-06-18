// physics_cpu.h

#pragma once

// This source file, in C++, is a CPU-based alternative to the physics kernel program (OpenCL)
// It exists to help debug the kernel program, since OpenCL is almost impossible to debug

#include <cmath>
#include <iostream>
#include <fstream>


size_t global_id = 0;
void   set_global_id(size_t id)
{
    global_id = id;
}
size_t get_global_id(int i)
{
    return global_id;
}


/*
Compute the total signal(noiseless) at the detector
*/


#ifdef USE_FLOAT

void physics(const int    kVp_x2_m1,         // 0 kilovolts, peak times 2 minus 1
                   float  *tbl,              // 1 length table (lengthBuffer)
             const float *densities,         // 2 densities of each material type (densityBuffer)
             const float *weights,           // 3 weights of each material type (weightsBuffer)
             const float *attenuationTable,  // 4 attenuation table energies
             const int    numMatLbls,        // 5 number of materials represented in the attenuationTableBuffer
             const float *spectrum,          // 6 spectrum buffer
             const int    spRowCount,        // 7 number of rows in the above array
             const float *filter,            // 8 filter attenuation (filterBuffer)
             const float  multiplier,        // 9 a multipler precalculated on the host
                   float *signalBuffer,      // 10 output signal (a.k.a. signal buffer)
             const int    count,             // 11 number of detector elements (detectorPositions.size)
             const int   *labMatOffs,        // 12 offset to first of materials represented by this material label
             const int   *numLabMats)        // 13 number of materials represented by this material label
#else
void physics(const int            kVp_x2_m1,        // 0 kilovolts, peak times 2 minus 1
                   double * const tbl,              // 1 length table (lengthBuffer)
             const double * const densities,        // 2 densities of each material type (densityBuffer)
             const double * const weights,          // 3 weights of each material type (weightsBuffer)
             const double * const attenuationTable, // 4 attenuation table energies
             const int            numMatLbls,       // 5 number of materials represented in the attenuationTableBuffer
             const double * const spectrum,         // 6 spectrum buffer
             const int            spRowCount,       // 7 number of rows in the above array
             const double * const filter,           // 8 filter attenuation (filterBuffer)
             const double         multiplier,       // 9 a multipler precalculated on the host
                   double * const signalBuffer,     // 10 output signal (a.k.a. signal buffer)
             const int            count,            // 11 number of detector elements (detectorPositions.size)
             const int    * const labMatOffs,       // 12 offset to first of materials represented by this material label
             const int    * const numLabMats)       // 13 number of materials represented by this material label
#endif
{

    size_t row = get_global_id(0);
    if (row > count)
    {
        return; // typically occurs (not an error)
    }

    #ifdef USE_FLOAT
    float averageValue = 0.0f;
    float length = 0.0f;
    float density = 0.0f;
    float weight = 0.0f;
    float ldw = 0.0f;
    #else
    double averageValue = 0.0;
    double length = 0.0;
    double density = 0.0;
    double weight = 0.0;
    double ldw = 0.0;
    #endif

    for(int m = 0; m < 4; ++m)
    {
        // Declare and Initialize SumUT
        #ifdef USE_FLOAT
        float sumUT[128];  // 100 is kVp*2 for mammography spectra w/0.5 kV steps
        for(int ndx=0; ndx<kVp_x2_m1; ndx++)  sumUT[ndx] = 0.0f;  // initialize the array
        #else
        double sumUT[128];  // 100 is kVp*2 for mammography spectra w/0.5 kV steps
        for(int ndx=0; ndx<kVp_x2_m1; ndx++)  sumUT[ndx] = 0.0;  // initialize the array
        #endif

        // Calculate the sum over the materials in the x-ray path of
        // mass_attn * density * thickness for each energy bin
        // FIX - should also loop over the set of materials for composite materials
        
        int smi = 0; // sequential material index
        
        for(int k=0; k<numMatLbls; k++) // Cycle thru all labels
        {
            length = tbl[ (((row * numMatLbls) + k) * 4) + m ]; // length table corresponds to labels, not materials

            for(int j=0; j<numLabMats[k]; ++j)  // Cycle thru all materials represented by this label
            {
                //if (length > 0.0)  // Don't process if the current label does not occur in this ray
                if (length > 0.5)  // Don't process if the current label does not occur in this ray
                {
                    density = densities[smi]; // use the sequential material index not the offset
                    weight = weights[smi];    // use the sequential material index not the offset

                    //  Loop over each energy bin
                    int offset = labMatOffs[smi];   // Find index into attenuation table
                    ldw = length * density * weight;

                    for(int ndx=0; ndx<kVp_x2_m1; ndx++)
                    {
                        //  MISTAKE ---  FIX THIS
                        //  Spectra in half kV steps
                        //  Attn coefs in kV steps

                        sumUT[ndx] = sumUT[ndx] + attenuationTable[(offset * kVp_x2_m1) + ndx] * ldw;

                    } // next energy bin

                } // if ray is intersecting a voxel with the current label value

                smi = smi + 1; // increment sequential material index

            }  // next material represented by this label

        } // next k (labels found in the length table)

        // Loop over energy bins calculating
        // Io * area^2 * attn_of_filter * E * exp(-mu t)

        #ifdef USE_FLOAT
            float sumIUT=0.0f;
            for (int spec=0; spec<kVp_x2_m1; spec++)
            {
                float temp1 = spectrum[spec*2+1];
                float temp2 = exp(-1.0f * sumUT[spec]);
                sumIUT = sumIUT + (temp1 * filter[spec] * temp2 * spectrum[spec*2] * multiplier);
            }

            // Average over the 4 rays per pixel used in the Siddon method
            averageValue = averageValue + 0.25f * sumIUT;

            // Clear out this element from the lengthTable
            for (int kk=0; kk<numMatLbls; kk++)
            {
                tbl[ (((row * numMatLbls) + kk) * 4) + m ] = 0.0f;
            }
        #else
            double sumIUT=0.0;
            for (int spec=0; spec<kVp_x2_m1; spec++)
            {
                double temp1 = spectrum[spec*2+1];
                double temp2 = exp(-1.0f * sumUT[spec]);
                sumIUT = sumIUT + (temp1 * filter[spec] * temp2 * spectrum[spec*2] * multiplier);
            }

            // Average over the 4 rays per pixel used in the Siddon method
            averageValue = averageValue + 0.25 * sumIUT;

            /////// Clear out this element from the lengthTable <-- THIS IS COMPLETELY UNNECESSARY
            /////for (int kk=0; kk<numMatLbls; kk++)
            /////{
            /////    tbl[ (((row * numMatLbls) + kk) * 4) + m ] = 0.0;
            /////}
        #endif
    } // next m

    signalBuffer[row] = averageValue;//sumIUT; // critical
}
