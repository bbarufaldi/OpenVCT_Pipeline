// tracing_II.cl

// PLAN: Make this raytracing program act as SimulationCPU does. - DH 2016-03-09
// Notes:
// 1. Cannot use double or bool as parameter types in kernel programs
// 2. This is an unintentended correlation between number of label materials
//    specififed in the config file and the largest voxel value in the phantom

/*
Efficient implementation of the 3D-DDA ray traversal algorithm on GPU and its application in radiation dose calculation
Source:
Medical physics [0094-2405] Xiao, K yr:2012 vol:39 iss:12 pg:7619
*/

__kernel void trace(__global const float3 *detector,     // 0 location of the detectors (detectorBuffer)
                    const float3 source,                 // 1 location of the source
                    const uint4 voxelCount,               // 2 voxel counts in 3 dimensions
                    const float3 vsize,                  // 3 voxel size in 3 dimensions
                    __global const unsigned char *volume,// 4 voxels, aka volumeBuffer
                    __global float *tbl,                           // 5 length table (lengthBuffer)
                    __global float *invsqb,                        // 6 inverse Square attenuations (invSquaredBuffer)
                    const int count,                     // 7 count of total detector Y positions at a single X position
                    //const float FSDsquared,              // 8 focal pt source distance
                    const float qElSizeX,                // 9 1/4 detector element size in X direction
                    const float qElSizeY,                // 10 1/4 detector element size in Y direction
                    const int   numMatLbls,              // 11 number of material labels
                    const int   randomize,               // 12 switch: randomize element quad positions
                    const float3 detectorPosition)                 // 13 location of the detector

{
    // This kernel program function is invoked once for each individual detector element in the current detector Y position
    size_t row = get_global_id(0); // row identifies the which detector element in the X direction is being handled
    if (row > count)
    {
        //printf("OPENCL TRACE ERROR! row > count: row=%u, count=%d\n", row, count);
        return;
    }

    float localtbl[128]; // magic number: maximum value of label material 2018-11-07 DH: Do NOT make this __local :)
    for(int i=0; i<numMatLbls; i++) localtbl[i] = 0.0;

    float3 element;

    // Break the element into four quads, calculate each and average the result
    for(int m = 0; m < 4; ++m)
    {
        if (m == 0)                                // DH 2016-10-?? Changed these to match CPU raytracer
        {
          element.x = detector[row].x;             //element.x = detector[row].x - qElSizeX;
          element.y = detector[row].y;             //element.y = detector[row].y - qElSizeY;
        }
        else if (m == 1)
        {
          element.x = detector[row].x;             //element.x = detector[row].x + qElSizeX;
          element.y = detector[row].y + qElSizeY;  //element.y = detector[row].y - qElSizeY;
        }
        else if (m == 2)
        {
          element.x = detector[row].x + qElSizeX;  //element.x = detector[row].x + qElSizeX;
          element.y = detector[row].y;             //element.x = detector[row].x + qElSizeX;
        }
        else
        {
          element.x = detector[row].x + qElSizeX;  //element.x = detector[row].x - qElSizeX;
          element.y = detector[row].y + qElSizeY;  //element.y = detector[row].y + qElSizeY;
        }
        element.z = detector[row].z;

        // ADM - need to add transform from detector surface to location in space.  This works
        // right now because we do not rotate the detector.... FIX ME

        float3 direction = element - source;

        // Find length
        float dir_x_squared = direction.x * direction.x;
        float dir_y_squared = direction.y * direction.y;
        float dir_z_squared = direction.z * direction.z;
        float length = sqrt(dir_x_squared + dir_y_squared + dir_z_squared);

        //BB
        float fsd_x = source.x * source.x;
        float fsd_y = source.y * source.y;
        float fsd_z = source.z * source.z;

        //invsqb[row] = FSDsquared / ( dir_x_squared + dir_y_squared + dir_z_squared );  // DH 2016-03-17
        invsqb[row] = (fsd_x + fsd_y + fsd_z) / ( dir_x_squared + dir_y_squared + dir_z_squared ); // BB


        float x1 = 0.0, xn = 1.0, y1 = 0.0, yn = 1.0, z1 = 0.0, zn = 1.0;

        //  ADM - it is likely that the if/else block below needs to have the phantom offset added to it
        //  since it looks like it presumes the phantom is sitting on the breast support


        // Siddeon EQ (4) find parametric values corresponding to the sides (of the voxel)
        if (direction.x != 0.0)
        {
           x1 = -source.x / direction.x;
           xn = ((voxelCount.x * vsize.x)- source.x) / direction.x;
        }
        else
        {
           direction.x = 1.0e-10;
        }

        if (direction.y != 0.0)
        {
           y1 = -source.y / direction.y;
           yn = ((voxelCount.y*vsize.y)- source.y) / direction.y;
        }
        else
        {
           direction.y = 1.0e-10;
        }

        if (direction.z != 0.0)
        {
           z1 = -source.z / direction.z;
           zn = ((voxelCount.z*vsize.z)- source.z) / direction.z;
        }
        else
        {
           direction.z = 1.0e-10;
        }

        // Siddon EQ(5) Find whether the ray intersects the CT array

        // TLV add phantom_offset to the alpha min and max (Maybe)
        float alpha_min = max(max(0.0f, min(x1, xn)),
                              max(min(y1, yn), min(z1, zn)));

        float alpha_max = min(min(1.0f, max(x1, xn)),
                              min(max(y1, yn), max(z1, zn)));


        float inDistance = length;
        float outDistance = 0.0;
        int3 step;
        float3 invDirection, deltaT, vT;
        int3 voxellncr = (int3)(0, 0, 0);
        int3 currentV;
        int3 offset;
        float3 tmp2;
        float oldT;
        float t;
        int i = 0, j = 0, k = 0;
        long mat_ndx;
        unsigned char mat;

        if (alpha_max > alpha_min)
        {
            // The ray intersects the CT array
           inDistance  = alpha_min * length;
           outDistance = (1.0 - alpha_max) * length;

            // Siddon EQ(7)

            i = (element.x >= source.x) ? (voxelCount.x - (voxelCount.x * vsize.x + (detectorPosition.x * vsize.x) - alpha_min * (direction.x) - source.x) / vsize.x) : ((source.x + alpha_min * (direction.x)) / vsize.x - 1);
            j = (element.y >= source.y) ? (voxelCount.y - (voxelCount.y * vsize.y + (detectorPosition.y * vsize.y) - alpha_min * (direction.y) - source.y) / vsize.y) : ((source.y + alpha_min * (direction.y)) / vsize.y - 1);
            k = (element.z >= source.z) ? (voxelCount.z - (voxelCount.z * vsize.z + (detectorPosition.z * vsize.z) - alpha_min * (direction.z) - source.z) / vsize.z) : ((source.z + alpha_min * (direction.z)) / vsize.z - 1);

            currentV = (int3)(i,j,k);

            step = (int3)((direction.x<0.0f) ? -1:1, (direction.y<0.0f) ? -1:1, (direction.z<0.0f) ? -1:1);

            offset = (int3)((direction.x<0.0f) ? 0:1, (direction.y<0.0f) ? 0:1, (direction.z<0.0f) ? 0:1);
            invDirection = (float3) (fabs(1.0f / direction.x), fabs(1.0f / direction.y), fabs(1.0f / direction.z));
            deltaT = (float3) (vsize.x * invDirection.x, vsize.y * invDirection.y, vsize.z * invDirection.z);

            oldT = alpha_min;

            tmp2.x = (float)(currentV.x + offset.x) * vsize.x;
            tmp2.y = (float)(currentV.y + offset.y) * vsize.y;
            tmp2.z = (float)(currentV.z + offset.z) * vsize.z;
            vT = (tmp2 - source) / direction;

            //  while inside the phantom
            while((voxelCount.x > currentV.x) && (0 <= currentV.x) &&
                  (voxelCount.y > currentV.y) && (0 <= currentV.y) &&
                  (voxelCount.z > currentV.z) && (0 <= currentV.z) &&
                  (oldT <= alpha_max))  //(oldT < 1.0)) // CurrentV is inside the traversal region
            {

               // corresponds to T mat = vol.getVoxel((unsigned int)CurrentV[0], (unsigned int) CurrentV[1], (unsigned int)CurrentV[2]); (complexity is due to indexing)
               // [(voxelCount[0] - i - 1)*voxelCount[1]*voxelCount[2]  + (voxelCount[2] - k - 1)  *voxelCount[1]+j]
               
			   mat_ndx = (voxelCount.x - currentV.x - 1) * (long)voxelCount.y * (long)voxelCount.z + ((long)voxelCount.z - currentV.z - 1) * (long)voxelCount.y + currentV.y;


               mat = volume[mat_ndx];   //  2018-11-08 DH: This is where the material index value is acquired!! * * * * * *
                                        //  mat_ndx is perhaps a misnomer. it's an index into the volume at which a material index value is stored


               // where next?
               voxellncr = (int3)((vT.x <= vT.y) && (vT.x <= vT.z) ? 1 : 0,
                                  (vT.y < vT.x ) && (vT.y <= vT.z) ? 1 : 0,
                                  (vT.z < vT.x ) && (vT.z < vT.y)  ? 1 : 0);

               t = min(vT.x, min(vT.y, vT.z));

               // Average over the four element quadrant locations (corresponds to lambda function execution in CPU sim)
               //localtbl[mat] = localtbl[mat] + (( min(1.0f, t) - oldT) * length); // Don't always count the whole voxel.
               localtbl[mat] = localtbl[mat] + ((t - oldT) * length); // Don't always count the whole voxel.

               // corresponds to vT.step(VoxelLncr, DeltatT) in CPU sim
               vT = vT + (float3) ( (float)voxellncr.x * deltaT.x,  (float)voxellncr.y * deltaT.y,  (float)voxellncr.z * deltaT.z);

               // corresponds to CurrentV.step(Voxellncr, Step); in CPU sim
               currentV = currentV + (int3)(voxellncr.x * step.x, voxellncr.y * step.y, voxellncr.z * step.z);

               oldT = t;

            } // endwh

        }

        // This line ensures that the phantom is embedded within air
        localtbl[0] = localtbl[0] + inDistance + outDistance; // DH 2016-11-14 start and end ( as in SimulationCPU.h )

        //((z * ydim) + y) * xdim + x;   where (xdim=4, x is m), (ydim=10, y is mat), (zdim->???, z is row)
        for (int mndx = 0; mndx<numMatLbls; mndx++)
        {
            tbl[((row * numMatLbls) + mndx) * 4 + m] = localtbl[mndx];
            localtbl[mndx] = 0.0; // added 2017-01-12 DH
        }

    } // next m (detector element quad)
}



// IDEA: PERFORM TRACE AND PHYSICS IN ONE, SINGLE ROUTINE (help 'm' problem).
// PLAN: (meh)


/*
Compute the total signal(noiseless) at the detector
*/
__kernel void physics(int kVp_x2_m1,                          // 0 kilovolts, peak times 2 minus 1
                      __global float *tbl,                    // 1 length table (lengthBuffer)
                      __global const float *densities,        // 2 densities of each material type (densityBuffer)
                      __global const float *weights,          // 3 weights of each material type (weightsBuffer)
                      __global const float *attenuationTable, // 4 attenuation table energies
                      const int numMatLbls,                   // 5 number of materials represented in the attenuationTableBuffer
                      __global const float *spectrum,         // 6 spectrum buffer
                      const int spRowCount,                   // 7 number of rows in the above array
                      __global const float *filter,           // 8 filter attenuation (filterBuffer)
                      const float multiplier,                 // 9 a multipler precalculated on the host
                      __global float *signalBuffer,                     // 10 output signal (a.k.a. signal buffer)
                      const int count,                        // 11 number of detector elements (detectorPositions.size)
                      __global const int *labMatOffs,         // 12 offset to first of materials represented by this material label
                      __global const int *numLabMats)         // 13 number of materials represented by this material label
{

    size_t row = get_global_id(0);
    if (row > count)
    {
        return; // typically occurs (not an error)
    }

    float averageValue = 0.0f;
    float length = 0.0f;
    float density = 0.0f;
    float weight = 0.0f;

    for(int m = 0; m < 4; ++m)
    {
        // Declare and Initialize SumUT
        float sumUT[128];  // 100 is kVp*2 for mammography spectra w/0.5 kV steps
        for(int ndx=0; ndx<127; ndx++)  sumUT[ndx] = 0.0f;  // initialize the array

        // Calculate the sum over the materials in the x-ray path of
        // mass_attn * density * thickness for each energy bin
        // FIX - should also loop over the set of materials for composite materials
        
        int smi = 0; // sequential material index
        
        for(int k=0; k<numMatLbls; k++)
        {
            length = tbl[ (((row * numMatLbls) + k) * 4) + m ]; // length table corresponds to labels, not materials

            
                for(int j=0; j<numLabMats[k]; ++j)  // Cycle thru all materials represented by this label
                {
					if (length > 0.0)  // Don't process if the current label does not occur in this ray
					{
						density = densities[smi]; // use the sequential material index not the offset
						weight = weights[smi];    // use the sequential material index not the offset

						//  Loop over each energy bin
						int offset = labMatOffs[smi];   // Find index into attenuation table
						float ldw = length * density * weight;
						for(int ndx=0; ndx<kVp_x2_m1; ndx++)
						{
							//  MISTAKE ---  FIX THIS
							//  Spectra in half kV steps
							//  Attn coefs in kV steps

							sumUT[ndx] = sumUT[ndx] + attenuationTable[(offset * kVp_x2_m1) + ndx] * ldw;

						} // next energy bin
					} // if ray is intersecting a voxel with the current label value
                    smi = smi + 1; // increment sequential material index

                } // next material represented by this label // 2018-11-13 DH *

            

        } // next k (labels found in the length table)

        // loop over energy bins calculating
        // Io * area^2 * attn_of_filter * E * exp(-mu t)
        float sumIUT=0.0f;
        for (int spec=0; spec<kVp_x2_m1; spec++)
        {
            float temp1 = spectrum[spec*2+1];
            float temp2 = exp(-1.0f * sumUT[spec]);
            sumIUT = sumIUT + (temp1 * filter[spec] * temp2 * spectrum[spec*2] * multiplier);
        }

        // average over the 4 rays per pixel used in the Siddon method
        averageValue = averageValue + 0.25 * sumIUT;

        // Clear out this element from the lengthTable
        for (int kk=0; kk<numMatLbls; kk++)
        {
            tbl[ (((row * numMatLbls) + kk) * 4) + m ] = 0.0f;
        }
    } // next m

    signalBuffer[row] = averageValue;//sumIUT; // critical
}
