// split_criterion_distr_thickness_float.cl

// Functions contained in this file:

// - mask_check                                 find max & min dist from voxel to mask and skin
// - PostTransform
// - PreTransform
// - distr_max_box7_imp                         compute the distance of voxel to a shape
// - distr_max_box7                             __kernel routine
// - voxel_contains_boundary_thickness_nostr1   compute the closest shapes to a voxel
// - MyRound(float r)
// - split_criterion_distr_thickness_nostr1     __kernel routine
// - GetMemoryCountForNextLevel                 __kernel routine: sum distr cnt, # of nodes to be split per workgroup
// - WGGetMemoryCountForNextLevel               __kernel routine: sum distr cnt, split count of all workgroup
// - CompactNodes                               __kernel routine: split each node into 8 children
// - GetColor
// - create_voxels_last_level_nostrKernel       __kernel routine: convert the tree nodes into voxels
// - voxelize                                   __kernel routine: convert the tree nodes into voxels
// - SubdivideNodes                             __kernel routine: split each node into 8 children



// ( Local ) Find the max and min distance of the voxel from mask and skin
void mask_check(float xLow, float xhigh,
                float ylow, float yhigh,
                float zlow, float zhigh,
                float2 invasquared, float2 invbsquared,
                float2 invcprimsquared, float2 invcsecsquared,
                float2  *minimal_value, float2 *maximal_value) // We may not need xhigh. CHECK!
{
    // Global new_tree
    float yabsLess = min(fabs(ylow),fabs(yhigh));
    float v = 0.0f;
    float2 tempmin = MAXFLOAT;

    short optim=0, above, tangent;

    if (zhigh == 0) // 11/23/2010 now we have 2 loops
    {
        // Two dimensional optimization
        optim = 2;
        v = max(fabs(ylow), fabs(yhigh));
        *maximal_value = xhigh*xhigh*invasquared + v*v*invbsquared + zlow*zlow*invcsecsquared; // 11/23/2010

    }
    if (zlow == 0)  // zhigh and zlow cannot be 0 simultaneously
    {
        // Two dimensional optimization
        optim = 2;
        v = max(fabs(ylow), fabs(yhigh));
        *maximal_value = xhigh*xhigh*invasquared + v*v*invbsquared + zhigh*zhigh*invcprimsquared; // 11/23/2010
    }

    v = max(fabs(ylow), fabs(yhigh));

    if (zhigh > 0)
    {
        if (zlow < 0) // two dimensional optimization in z=0 plane
        {
            optim = 2;
            *maximal_value = xhigh*xhigh*invasquared + v*v*invbsquared + max(zlow*zhigh*invcsecsquared, zhigh*zhigh*invcprimsquared); // 11/23/2010
        }
        else // zlow > 0  three dimensional optimization, use c'
        {
            optim = 3;
            above = 1;
            *maximal_value = xhigh*xhigh*invasquared + v*v*invbsquared + zhigh*zhigh*invcprimsquared; // 11/23/2010
        }
    }
    else // zhigh < 0, three dimensional optimization use c''
    {
        optim = 3;
        above = 0;
        *maximal_value = xhigh*xhigh*invasquared + v*v*invbsquared + zlow*zlow*invcsecsquared; // 11/23/2010
    }

    if (optim == 2)
    {
        tangent = 0;
        if (0 >= ylow  && 0 <= yhigh)
        {
            tempmin = min(tempmin, xLow*xLow*invasquared);
            tangent = 1;
        }
        if (0 == xLow)
        {
            tempmin = min(tempmin, yabsLess*yabsLess*invbsquared);
            tangent = 1;
        }
        if (tangent == 0)
        {
            tempmin = min(tempmin, xLow*xLow*invasquared + yabsLess*yabsLess*invbsquared);
        }
        *minimal_value = tempmin;
    }
    else  // 3D ellipsoids, plane x3=0 does not intersect voxel or its boundary
    {
        if (above == 1)
        {
            if (0 >= ylow  && 0 <= yhigh) //&& 0<=xhigh // since in our application usually xmin=0, probably 0<=xhigh will never be true! ASSUMPTION: xLow>=0!!!! Hence xhigh cannot be=0
            {
                // 11/20/2010 condition modified
                // first condition most selective!!!
                *minimal_value = xLow*xLow*invasquared + zlow*zlow*invcprimsquared; // since we are above plane x3=0, 0<zmin<zmax, so it is sufficient to calculate only at zmin (we calculate gradient at x=0,y=0)
            }
            else
            {
                *minimal_value = xLow*xLow*invasquared + yabsLess*yabsLess*invbsquared + zlow*zlow*invcprimsquared;
            }
        }
        else // below.
        {
            if  (0 >= ylow  && 0 <= yhigh)
            {
                // 11/20/2010 condition modified
                *minimal_value = xLow*xLow*invasquared + zhigh*zhigh*invcsecsquared; // since we are below plane x3=0, |zlow|>|zhigh|, so it is sufficient to calculate only at zmax (we calculate gradient at x=0,y=0)
            }
            else
            {
                *minimal_value = xLow*xLow*invasquared + yabsLess*yabsLess*invbsquared + zhigh*zhigh*invcsecsquared;
            }
        }
    }
    return;
}

/* For Reference Only (do not uncomment) : found in OpenCLKernel.hpp:
class float16
{
public:
    float16(const float a);
    float x, y, z, w;
    float s0, s1, s2, s3, s4, s5, s6, s7;
    float s8, s9, sA, sB, sC, sD, sE, sF;
    float2 xy, zw, s01, s23, s45, s67, s89, sAB, sCD, sEF;
    float3 xyz;
    float4 xyzw, s0123, s4567, s89AB, sCDEF, s048C, s159D, s26AE, s37BF;
    float8 lo, hi, even, odd;
};
*/


void PostTransform(float4 v, float16 T,float4 *r)
{
    (*r) = (float4) (dot(T.s0123, v), dot(T.s4567, v), dot(T.s89ab, v), 0.0f);
}

void PreTransform(float16 T, float4 v, float4 *r)
{
    (*r) = (float4) (dot(T.s048c, v),  dot(T.s159d, v), dot(T.s26ae, v), 0.0f);
}


// ( Local ) Routine to compute the distance of voxel to a shape
void distr_max_box7_imp(const float4 localmu,
                        float16 localR,
                        const float logSqrtDetSigmalogprior,
                        const float4 bb,
                        const float16 A1inv,
                        const float16 A2inv,
                        const float16 A3inv,
                        const float2 x,
                        const float2 y,
                        const float2 z,
                        const float8 H,
                        const float8 localU1,
                        const float8 localU2,
                        const float8 localU3,
                        float deltaX,
                        const float4 xc,
                        float *int_min,
                        float *int_max)
{
    float4 xcmu = xc - localmu;
    float4 xR;

    PostTransform(xcmu, localR, &xR);

    float fc = -0.5f * dot(xR, xR) + logSqrtDetSigmalogprior;
    float8 boundary_values;
    float bmin, bmax;

    boundary_values.s0 = localU1.s0 * xcmu.s0 + localU2.s0 * xcmu.s1 + localU3.s0 * xcmu.s2;
    boundary_values.s1 = localU1.s1 * xcmu.s0 + localU2.s1 * xcmu.s1 + localU3.s1 * xcmu.s2;
    boundary_values.s2 = localU1.s2 * xcmu.s0 + localU2.s2 * xcmu.s1 + localU3.s2 * xcmu.s2;
    boundary_values.s3 = localU1.s3 * xcmu.s0 + localU2.s3 * xcmu.s1 + localU3.s3 * xcmu.s2;
    boundary_values.s4 = localU1.s4 * xcmu.s0 + localU2.s4 * xcmu.s1 + localU3.s4 * xcmu.s2;
    boundary_values.s5 = localU1.s5 * xcmu.s0 + localU2.s5 * xcmu.s1 + localU3.s5 * xcmu.s2;
    boundary_values.s6 = localU1.s6 * xcmu.s0 + localU2.s6 * xcmu.s1 + localU3.s6 * xcmu.s2;
    boundary_values.s7 = localU1.s7 * xcmu.s0 + localU2.s7 * xcmu.s1 + localU3.s7 * xcmu.s2;

    boundary_values = deltaX * (boundary_values + deltaX * H)+ fc;

    bmin = boundary_values.s0;
    bmin = min(bmin, boundary_values.s1);
    bmin = min(bmin, boundary_values.s2);
    bmin = min(bmin, boundary_values.s3);
    bmin = min(bmin, boundary_values.s4);
    bmin = min(bmin, boundary_values.s5);
    bmin = min(bmin, boundary_values.s6);

    *int_min = min(bmin, boundary_values.s7);
    bmax = boundary_values.s0;

    bmax = max(bmax, boundary_values.s1);
    bmax = max(bmax, boundary_values.s2);
    bmax = max(bmax, boundary_values.s3);
    bmax = max(bmax, boundary_values.s4);
    bmax = max(bmax, boundary_values.s5);
    bmax = max(bmax, boundary_values.s6);
    bmax = max(bmax, boundary_values.s7);

    if ((x.x <= localmu.x) && (x.y >= localmu.x) &&
        (y.x <= localmu.y) && (y.y >= localmu.y) &&
        (z.x <= localmu.z) && (z.y >= localmu.z))
    {
        *int_max = logSqrtDetSigmalogprior;
    }
    else
    {
        float localmax = -MAXFLOAT;
        float4 localBB = bb;
        float4 B = (float4)(x.x, localBB.y, localBB.z, 0.0f);
        float4 xx;
        float16 InvA = A1inv;

        PreTransform(InvA, B, &xx);

        if ((xx.y >= y.x) && (xx.y <= y.y) &&
            (xx.z >= z.x) && (xx.z <= z.y))
        {
            xx = xx - localmu;
            PostTransform(xx, localR,&xR);
            fc = -0.5f * dot(xR, xR)+ logSqrtDetSigmalogprior;
            localmax = max(localmax, fc);
        }

        B = (float4)(x.y, localBB.y,  localBB.z, 0.0f);

        PreTransform(InvA, B, &xx);

        if ((xx.y >= y.x) && (xx.y <= y.y) &&
            (xx.z >= z.x) && (xx.z <= z.y))
        {
            xx = xx - localmu;
            PostTransform( xx, localR,&xR);
            fc = -0.5f * dot(xR, xR) + logSqrtDetSigmalogprior;
            localmax = max(localmax, fc);
        }

        B = (float4)(localBB.x, y.x, localBB.z, 0.0f);

        InvA = A2inv;
        PreTransform(InvA, B, &xx);

        if ((xx.x >= x.x) && (xx.x <= x.y) &&
            (xx.z >= z.x) && (xx.z <= z.y))
        {
            xx = xx - localmu;
            PostTransform( xx, localR,&xR);
            fc = -0.5f * dot(xR, xR)+ logSqrtDetSigmalogprior;
            localmax = max(localmax, fc);
        }

        B = (float4)(localBB.x, y.y, localBB.z, 0.0f);;
        PreTransform(InvA, B, &xx);

        if ((xx.x >= x.x) && (xx.x <= x.y) &&
            (xx.z >= z.x) && (xx.z <= z.y))
        {
            xx = xx - localmu;
            PostTransform(xx, localR, &xR);
            fc = -0.5f * dot(xR, xR)+ logSqrtDetSigmalogprior;
            localmax = max(localmax, fc);
        }

        B = (float4)(localBB.x, localBB.y, z.x, 0.0f);

        InvA = A3inv;
        PreTransform(InvA, B, &xx);

        if ((xx.x >= x.x) && (xx.x <= x.y) &&
            (xx.y >= y.x) && (xx.y <= y.y))
        {
            xx = xx - localmu;
            PostTransform( xx, localR,&xR);
            fc = -0.5f * dot(xR, xR) + logSqrtDetSigmalogprior;
            localmax = max(localmax, fc);
        }

        B = (float4)(localBB.x, localBB.y, z.y, 0.0f);

        PreTransform(InvA, B, &xx);

        if ((xx.x >= x.x) && (xx.x <= x.y) &&
            (xx.y >= y.x) && (xx.y <= y.y))
        {
            xx = xx - localmu;
            PostTransform(xx, localR, &xR);
            fc = -0.5f * dot(xR, xR)+ logSqrtDetSigmalogprior;
            localmax = max(localmax, fc);
        }

        *int_max = max(localmax,bmax);
    }
    //printf("dist  %f \n", *int_min);
}




// ( "evalKernel", 4th kernel function invoked) This is the evalKernel function
__kernel  void distr_max_box7(__global const float2 *x,                      //  0
                              __global const float2 *y,                      //  1
                              __global const float2 *z,                      //  2
                              __global const int* nodeMap,                   //  3
                              __global const short* shape,                   //  4
                              __global const float4 *mu,                     //  5
                              __global const float16 *R,                     //  6
                              __global const float *logSqrtDetSigmalogprior, //  7
                              __global const float4 *bb,                     //  8
                              __global const float16 *A1inv,                 //  9
                              __global const float16 *A2inv,                 // 10
                              __global const float16 *A3inv,                 // 11
                              __global const float8 *H,                      // 12
                              __global const float8 *U1,                     // 13
                              __global const float8 *U2,                     // 14
                              __global const float8 *U3,                     // 15
                              float deltaX,                                  // 16
                              __global float2 *answer,                       // 17
                              const long totalDistCount)                     // 18
{
    size_t index = get_global_id(0);
    if (index >= totalDistCount)
    {
        return;
    }

    short i = shape[index];

    int node = nodeMap[index];

    float4 xc = (float4)(0.5f*(x[node].s0+x[node].s1), 0.5f*(y[node].s0+y[node].s1), 0.5f*(z[node].s0+z[node].s1), 0.0f);
    float max, min;
    distr_max_box7_imp(mu[i],R[i],
                       logSqrtDetSigmalogprior[i],bb[i],
                       A1inv[i],A2inv[i],A3inv[i],
                       x[node], y[node], z[node], H[i],U1[i], U2[i], U3[i], deltaX, xc,
                       &min, &max);

    answer[index].s0 = min;
    answer[index].s1 = max;
}

/*
New Ligament Types Idea (December 2019)

REQUIREMENTS:
    When Ligament between Compartments        Then Make Ligament This Material
    --------------- and ---------------       ---------------------------------
    FAT (204)   [3]     FAT (204)   [3]       admixture FAT & LIGAMENT (50/50)
    FAT (204)   [3]     GLAND (205) [4]       admixture FAT & LIGAMENT (20/80)
    GLAND (205) [4]     GLAND (205) [4]       admixture FAT & LIGAMENT (10/90)
                                       ..or.. admixture FAT & LIGAMENT (90/10)
NOTES:
    1. Chose the first ligament definition when between 2 glands (10/90 mix)
    2. Added 4 new ligament indicies [5] 50/50, [6] 20/80, [7] 10/90, [8] 90/10
    3. Updated GetColor() kernel function to include new ligament indicies.
    3. Updated resident vct_config.xml to reflect new ligament indicies.
    4. Phantom's Phantom.xml file will automatically include new indicies.
    5. Phantom's Private\XPL_AttenuationTable.xml will also include them.
    6. The internal label values 6, 7, 8 & 9 end up becoming 5, 6, 7 & 8 after
       invoking the GetColor function (~line 1140 this file). GetColor() is
       invoked when finally writing the 3D texture (volumetic image).
*/

// Determine Ligament Type (see GetColor() for internal indicies)
// param distLabel_1 - internal index for adjacent compartment
// param distLabel_2 - internal index for compartment on opposite site
// returns internal index corresponding to ligament type
int FindLigamentType(int distLabel_1, int distLabel_2)
{
    int label = 1;

    if (distLabel_1 == 3) // fat index (see GetColor)
    {
        if (distLabel_2 == 3) // fat index (see GetColor)
        {
            label = 6; // type 1 ligament between fat & fat (50/50 mix of fat and ligament)
        }
        else if (distLabel_2 == 4 || distLabel_2 == 5) // gland indicies (see GetColor)
        {
            label = 7; // type 2 ligament between fat & gland (20/80 mix of fat and ligament)
        }
    }
    else if (distLabel_1 == 4 || distLabel_1 == 5) // gland indicies (see GetColor)
    {
        if (distLabel_2 == 3) // fat index (see GetColor)
        {
            label = 7; // type 2 ligament between fat & gland (20/80 mix of fat and ligament)
        }
        else if (distLabel_2 == 4) // gland index (see GetColor)
        {
            label = 8; // type 3 ligament between gland & gland (90/10 mix of gland and ligament)
        }
        else if (distLabel_2 == 5) // gland index (see GetColor)
        {
            label = 9; // type 4 ligament between gland & gland (10/90 mix of gland and ligament)
        }
    }

    return label;
}


// Inlined routine to compute the closest shapes to a voxel
void voxel_contains_boundary_thickness_nostr1(float2 x,
                                              float2 y,
                                              float2 z,
                                              float Dmin,
                                              __global short* distribution,
                                              __global short* clDistributionCount,
                                              __global const float4 *mu,
                                              __global const float16 *R,
                                              __global const float *logSqrtDetSigmalogprior,
                                              __global const float4 *bb,
                                              __global const float16 *distributionInvSigma,
                                              __global const char *distributionLabel,
                                              float deltaX,
                                              float voxelHalfSize, // arg #14
                                              char* yes_no,
                                              char* label,
                                              __global const float2 *distance)
{
    float4 xc = (float4)(0.5f*(x.s0+x.s1), 0.5f*(y.s0+y.s1), 0.5f*(z.s0+z.s1), 0.0f); // Center of compartment ?

    float2 fiminmax = (float2)(MAXFLOAT,-MAXFLOAT);
    short i_max1 = 0;
    short s = *clDistributionCount;

    if (s < 1) return;
    if (s == 1) // Initial condition; phantom is one, big compartment
    {
        *yes_no=0;
        *clDistributionCount = 1;
        *label=distributionLabel[distribution[0]]; // 1/13/2011, instead of assigning the label to be
        return;
    }

    float voxelsizeToCompare = max(voxelHalfSize*2.0f, Dmin);

    for (short iii=0; iii<s; iii++)
    {
        float2 d = distance[iii];//max, min;

        if (d.s1 > fiminmax.s1)
        {
            fiminmax = d;
            i_max1 = iii;
        }
        // WHERE TO PUT deltaX, deltaXSquared???
    }

    short nextIndex = 0;

    for(short i=0; i<s; i++)
    {
        if (distance[i].s1 >= fiminmax.s0)
        {
            nextIndex++;
        }
    }

    if (nextIndex<=2)
    {
        float fjmax = -MAXFLOAT;
        short i_max2temp=-1;

        for (short i = 0; i < s; i++)
        {
            if (i != i_max1)
            {
                float value = distance[i].s1;
                if (value > fjmax)
                {
                    i_max2temp = i;
                    fjmax = value;
                }
            }
        }

        short i_max2=i_max2temp;

        // MODIFIED 12/3/2010
        short f1_distribution=distribution[i_max1];
        short f2_distribution=distribution[i_max2];
        float16 sub =  distributionInvSigma[f2_distribution] - distributionInvSigma[f1_distribution];
        float4 transformedResult;
        PreTransform(sub, xc, &transformedResult);

        float4 GradXc= transformedResult+bb[f1_distribution]-bb[f2_distribution];
        float normGradXc2 = length(GradXc); // Euclid norm of the gradient // for some reason, norm did not work!!! (because its OpenCL?)

        if (f2_distribution == f1_distribution) // It will happen if the caller did not use the yes_no returned to its voxel/node splitting parent.
        {
            *yes_no = 0;
            *clDistributionCount = 1;
            distribution[0] = f1_distribution;
            *label=distributionLabel[f1_distribution]; // assigning
        }
        else
        {
            float4 xcmu1 = xc - mu[f1_distribution];
            float4 xcmu2 = xc - mu[f2_distribution];
            float4 transformed_xcmu1, transformed_xcmu2;
            float16 Rf1 = R[f1_distribution], Rf2 = R[f2_distribution];

            PostTransform(xcmu1, Rf1, &(transformed_xcmu1));
            PostTransform(xcmu2, Rf2, &(transformed_xcmu2));

            float Fc=fabs(( -0.5f*dot(transformed_xcmu1, transformed_xcmu1) +logSqrtDetSigmalogprior[f1_distribution]) +
                   ( +0.5f*dot(transformed_xcmu2, transformed_xcmu2)-logSqrtDetSigmalogprior[f2_distribution]));

            float normGradXc1 = fabs(GradXc.s0)+fabs(GradXc.s1)+fabs(GradXc.s2);
            float center_dist = Fc/normGradXc2;
            float correction = voxelHalfSize*(normGradXc1/normGradXc2);

            float corr_center_dist = center_dist + correction;
            if ( (corr_center_dist < voxelsizeToCompare) && (corr_center_dist >= 0.0f) )
            {
                *label = 1; // cooper's ligament

                // Determine Ligament Type
                *label = FindLigamentType(distributionLabel[f1_distribution],
                                          distributionLabel[f2_distribution]);

                if (Dmin == 0.0f)
                {
                    *label = distributionLabel[f1_distribution]; // 2019-05-02 ESSENTIAL! KEEP!!
                    if (distributionLabel[f1_distribution] == 1)
                    {
                        if (distributionLabel[f2_distribution] > 2) *label = distributionLabel[f2_distribution];
                    }
                }

                if (Dmin > voxelHalfSize * 2.0f) // never true when Dmin is zero; therefore, split will occur
                {
                    *yes_no=0;
                }
                else
                {
                    *yes_no=1;
                }
                distribution[0] = f1_distribution;
                distribution[1] = f2_distribution;
                *clDistributionCount = 2;
            }
            else
            {
                if (center_dist-correction>=voxelsizeToCompare)
                {
                    *yes_no=0;

                    // Corresponding distribution should be the one determined by i_max1
                    distribution[0] = distribution[i_max1];
                    *clDistributionCount = 1;
                    *label=distributionLabel[distribution[0]]; // 1/13/2011, instead of assigning the label to be 3
                }
                else
                {
                    // However, the corresponding distributions should be those
                    // corresponding to i_max1 and correctly computed i_max2
                    *label = 1; // cooper's ligament index
                    if (true || Dmin == 0.0f) // This is essential for variable-thickness coopers ligaments
                    {
                        *label = distributionLabel[f1_distribution]; // 2019-05-02 ESSENTIAL! KEEP!! //??*$*$*
                        if (distributionLabel[f1_distribution] == 1)
                        {
                            if (distributionLabel[f2_distribution] > 2) *label = distributionLabel[f2_distribution];
                        }
                    }

                    *yes_no = 1; // further split will occur (if not too deep)

                    distribution[0] = f1_distribution;
                    distribution[1] = f2_distribution;
                    *clDistributionCount = 2;
                }
            }
        }
    }
    else //if (nextIndex > 2) // three distributions ...or more? ..that are farther apart than the fiminmax
    {
        *yes_no = 1;
        *label = 1;  // this is cooper's ligament label as skin always

        // Determine Ligament Type
        *label = FindLigamentType(distributionLabel[distribution[i_max1]],
                                  distributionLabel[distribution[0]]);

        if (Dmin == 0.0f)
        {
            *label = distributionLabel[distribution[i_max1]]; // 2019-05-02
            if (distributionLabel[distribution[i_max1]] == 1)
            {
                if (distributionLabel[distribution[0]] > 2) *label = distributionLabel[distribution[0]];
            }
        }

         if (nextIndex != s)
         {
            short offset=0;
            for(short i=0; i<s; i++)
            {
                if (distance[i].s1 >= fiminmax.s0)
                {
                    distribution[offset] = distribution[i];
                    offset++;
                }
            }

            *clDistributionCount = nextIndex;
        }
    }
}


short MyRound(float r)
{
    return (short) (r+0.5f);
}

float FindRadius(const float a, // minor radius of prolate spheroid (horizontal)
                 const float c, // major radius of prolate spheroid (vertical)
                 float z)       // z coordinate of point (location of plane)
{
    return a * sqrt( 1.0f - ((z * z) / (c * c))  ); // parens around z and c absolutely crucial!!!
}

float FindDThick(const float maxthick,
                 const float minthick,
                 const float radius,
                 float x, float y, float z)
{
    // Note max thick is at x=0, y=0, z=0
    float thickness = minthick;
    float delta = sqrt((x * x) + (y * y) + (z * z));
    if (delta < radius)
    {
        float d_thick = maxthick - minthick;
        float d_dist  = (delta / radius);
        thickness = maxthick - (d_dist * d_thick);
    }
    return thickness;
}

float FindLigThickness(const float max_thickness,
                       const float min_thickness,
                       const float2 x_bound,            // ~x coord of vertex
                       const float2 y_bound,            // ~y coord of vertex
                       const float2 z_bound,            // ~z coord of vertex
                       const __global float2 *Xextents, // X phantom min & max
                       const __global float2 *Yextents, // Y phantom min & max
                       const __global float2 *Zextents, // Z phantom min & max
                       const uint phantom_shape)        // 0=Breast, 1=Cube
{
    float thickness = min_thickness;

    float xpos = (x_bound.s0 + x_bound.s1) * 0.5f;
    float xmin = Xextents[0].s0;
    float xmax = Xextents[0].s1;
    float xsiz = xmax - xmin;

    float ypos = (y_bound.s0 + y_bound.s1) * 0.5f;
    float ymin = Yextents[0].s0;
    float ymax = Yextents[0].s1;

    float zpos = (z_bound.s0 + z_bound.s1) * 0.5f;
    float zmin = Zextents[0].s0;
    float zmax = Zextents[0].s1;


    if (true || phantom_shape == 0) // Breast
    {
        // Ellipse Cross-section Based on Radius -----------------------------------

        if (xpos > 0.0f) // test proximity to chest wall along X (empirical threshold)
        {
            if (fabs(ypos) < ymax) // test lateral proximity (empirical threshold)
            {
                if (zpos >= 0.0f) // is vertex in upper ellipsoid
                {
                    if (true || ((zmax - zpos) > (zmax * 0.5f ))) // define top threshold (empirical threshold)
                    {
                        // Algorithms #3 & #4 ("Sweet Spot" center of crosssection)
                        float xy_distance = sqrt(xpos*xpos + ypos*ypos);
                        float radius = FindRadius(xmax, zmax, zpos);
                        if (radius != 0.0f)
                        {
                            // Algorithm #4 sweet spot at chest wall
                            thickness = FindDThick(max_thickness, min_thickness,
                                                   radius, xpos, ypos, zpos);
                        }
                    }
                }
                else // vertex occurs in lower ellipsoid
                {
                    if (true || (zpos > (zmin * 0.5f))) // define bottom threshold
                    {
                        // Algorithms #3 & #4 ("Sweet Spot" center of crosssection)
                        float xy_distance = sqrt(xpos*xpos + ypos*ypos);
                        float radius = FindRadius(xmax, fabs(zmin), fabs(zpos));

                        if (radius != 0.0f)
                        {
                            // Algorithm #4 sweet spot at chest wall
                            thickness = FindDThick(max_thickness, min_thickness,
                                                   radius, xpos, ypos, zpos);

                        }
                    }
                }

                if (thickness < min_thickness) thickness = min_thickness;
            }
        }
    }
    else if (phantom_shape == 1) // Cube
    {
        float dx = fabs(xmax/xpos);
        float dy = fabs(ymax/ypos);
        float dz = 0.0f;
        if (zpos != 0.0f) dz = fabs((zpos<0.0f)?(zmin/zpos):(zmax/zpos));
        float dthick = max_thickness - min_thickness;
        thickness = min_thickness + (dthick * ((dx + dy + dz) / 3.0f));
    }


    return thickness;
}


// ( "splitKernel", 5th kernel function invoked )

__kernel  void split_criterion_distr_thickness(const float4 LOW,                             //  0
                                               const float Dmin,                             //  1
                                               const float deltaX,                           //  2
                                               const float deltaXSquared,                    //  3
                                               const float voxel_halfsize,                   //  4
                                               /////__write_only image3d_t voxelValues,           //  5
                                               __global uchar *voxelValues,                  //  5
                                               const __global float2 *x,                     //  6
                                               const __global float2 *y,                     //  7
                                               const __global float2 *z,                     //  8
                                               __global char *labels,                        //  9
                                               __global char *toSplits,                      // 10
                                               const float4 voxelResolution,                 // 11
                                               const int VoxelDimension,                     // 12
                                               const float maskAInverseSquared,              // 13
                                               const float maskBInverseSquared,              // 14
                                               const float maskCPrimInverseSquared,          // 15
                                               const float maskCSecInverseSquared,           // 16
                                               const float skinAInverseSquared,              // 17
                                               const float SkinBInverseSquared,              // 18
                                               const float skinCPrimInverseSquared,          // 19
                                               const float skinCSecInverseSquared,           // 20
                                               __global short *distribution,                 // 21
                                               __global short *clDistributionCount,          // 22
                                               __global const float4 *mu,                    // 23
                                               __global const float16 *R,                    // 24
                                               __global const float *logSqrtDetSigmalogprior,// 25
                                               __global const float4 *bb,                    // 26
                                               __global const float16 *distributionInvSigma, // 27
                                               __global const char *distributionLabel,       // 28 labelBuffer (CLSetup.cpp)
                                               int currentNodeCount,                         // 29
                                               __global const int *distributionHead,         // 30
                                               const __global float2 *distance,              // 31
                                               const __global float2 *Xextents,              // 32
                                               const __global float2 *Yextents,              // 33
                                               const __global float2 *Zextents,              // 34
                                               const uint phantom_shape,                     // 35
                                               const float max_ligament_thickness,           // 36  New field as of 2021-01-13
                                               const float min_ligament_thickness)           // 37  New field as of 2021-01-13
{
    size_t index = get_global_id(0);

    if (index >= currentNodeCount)
    {
        return;
    }


    float2 x_bound=x[index];
    float2 y_bound=y[index];
    float2 z_bound=z[index];

    float2 minimal_value; //0 air. 1 skin
    float2 maximal_value;

    float2 INVASQUARED = (float2) (maskAInverseSquared, skinAInverseSquared);
    float2 INVBSQUARED = (float2) (maskBInverseSquared, SkinBInverseSquared);

    float2 INVCPRIMSQUARED = (float2) (maskCPrimInverseSquared, skinCPrimInverseSquared);
    float2 INVCSECSQUARED = (float2) (maskCSecInverseSquared, skinCSecInverseSquared);

    if (phantom_shape == 1)
    {
        minimal_value = min_ligament_thickness; // DH 2021-01-13      0.2f;//
        maximal_value = max_ligament_thickness; // DH 2021-01-13      1.0f;//
    }
    else
    {
        mask_check(x_bound.s0, x_bound.s1,
                   y_bound.s0, y_bound.s1,
                   z_bound.s0, z_bound.s1,
                   INVASQUARED, INVBSQUARED,
                   INVCPRIMSQUARED, INVCSECSQUARED,
                   &minimal_value, &maximal_value);
    }

    //printf("%f %f %f %f %f %f %f %f\n", maskAInverseSquared, skinAInverseSquared, maskBInverseSquared, SkinBInverseSquared, maskCPrimInverseSquared, skinCPrimInverseSquared, maskCSecInverseSquared, skinCSecInverseSquared);
    //printf("maxair %f  \n",  maximal_value.s0);
    //printf("maxskin %f  \n",  maximal_value.s1);
    //printf("minair %f  \n",  minimal_value.s0);
    //printf("minskin %d  \n",  distributionHead[index]);
    //printf("max %d \n", distributionHead[index]);
    //bool fill = false;

    char yes_no = 0;
    char label =  labels[index];
    char oldLabel = label;

    // Find Dmin as a function of proximity to skin or chest wall
    float max_Dmin = Dmin;// * 4.0;//1.25f;    // Arbitrary and empirical (by eye)
    float min_Dmin = voxel_halfsize * 0.2f; // Size of a Voxel

    //if (Xextents[0].s0 < 0.0) max_Dmin = 0.0f;  //DEBUG: OK
    //if (Xextents[0].s1 < 0.0) max_Dmin = 0.0f;  //DEBUG: OK
    //if (Yextents[0].s1 < 0.0) max_Dmin = 0.0f;  //DEBUG: OK
    //if (Zextents[0].s1 < 0.0) max_Dmin = 0.0f;  //DEBUG: OK
    //if (Yextents[0].s0 < 0.0) max_Dmin = 0.0f;  //DEBUG: DEFINITELY ACTIVATED.
    //if (Zextents[0].s0 < 0.0) max_Dmin = 0.0f;  //DEBUG:  DEFINITELY ACTIVATED.
    //if (fabs(x_bound.s1 - x_bound.s0) > 0.02f) max_Dmin = 0.0f;  //DEBUG: Between 0.0125 and 0.025 (.02 causes striping!)
    //if (fabs(y_bound.s1 - y_bound.s0) > 0.02f) max_Dmin = 0.0f;  //DEBUG: Between 0.0125 and 0.025 (.02 causes striping!)
    //if (fabs(x_bound.s1 - x_bound.s0) > 0.02f) max_Dmin = 0.0f;  //DEBUG: Between 0.0125 and 0.025 (.02 causes striping!)

    // Findings:
    // 1. X ranges from 0.0 (chest wall) to max X (nipple) - no negative values
    // 2. Y ranges from -dimension to +dimension (side to side)
    // 3. Z ranges from -csec to +cprim, where 0 is where bottom and top ellipsoids meet
    // 4. On the final iteration, the X, Y, and Z bounds differ by roughly 0.02 mms

    float ligament_thickness =  FindLigThickness(max_Dmin, min_Dmin,
                                                 x_bound, y_bound, z_bound,
                                                 Xextents, Yextents, Zextents, phantom_shape);

    voxel_contains_boundary_thickness_nostr1(x_bound, y_bound, z_bound, ligament_thickness,
                                             &distribution[distributionHead[index]], &clDistributionCount[index], mu, R,
                                             logSqrtDetSigmalogprior, bb, distributionInvSigma, distributionLabel,
                                             deltaX, voxel_halfsize*2, &yes_no, &label,
                                             distance+distributionHead[index]);  // 12/3/2010

    char tosplit;


    if (oldLabel==1) // inherited label from the previous level, 11/25/2010 should save a number of mask evaluations
    {
        tosplit = yes_no; // Can have multiple values {0,1,2}
        labels[index]=label;
        if (yes_no==0)
        {
            tosplit = 2;
        }
    }
    else if (oldLabel==2)
    {
        // The idea: if it is marked as skin and should be split, then it is
        // either skin or air, but cannot be fat/dense tissue or Coopers ligaments
        if ((maximal_value.s0>1.0f) && (minimal_value.s0<=1.0f))
        {
            tosplit=1;
            clDistributionCount[index] = 0;
        }
        else
        {
            tosplit = (maximal_value.s0 <= 1.0f) ? 2 : 0; // value will be 2 or 0
        }
    }
    else if (minimal_value.s1<1.0f)
    {
        if (maximal_value.s1<=1.0f)
        {
            tosplit = yes_no ? 1 : 2;
            //fill = 1 - yes_no;
            labels[index]=label;
        }
        else // maximal_value_skin > 1
        {
            tosplit=1;
            labels[index]=4; // label of skin 11/25/2010 INTRODUCED to distinguish other cases when skin is assumed!

        }
    }
    else // minimal_value_skin >= 1
    {
        tosplit=0;
        if  ((maximal_value.s0>1.0f) &&  (minimal_value.s0<=1.0f))
        {
            tosplit=1;
            labels[index]=2; // label of skin
            clDistributionCount[index] = 0;
        }
        else if (maximal_value.s0<1.0)
        {
            tosplit = 2;
        }
    }
    toSplits[index]=tosplit;

    return;
}


// ( "addupKernel", 6th kernel function invoked ) This is the sum up the distribution count and number of nodes to be split per workgroup
__kernel void GetMemoryCountForNextLevel(__global int *totaldistributionCount,      // 0
                                         __global int *splitCount,                  // 1
                                         __global short *clDistributionCount,       // 2
                                         __global char *toSplits,                   // 3
                                         const int nodeCount,                       // 4
                                         __local int *localTotaldistributionCount,  // 5
                                         __local int *localSplitCount,              // 6
                                         __global int *voxelizeCount,               // 7
                                         __local int *localVoxelizeCount)           // 8

{
    int index = get_global_id(0);
    int group = get_group_id(0);
    __local int ss;

    if (get_local_id(0) == 0)
    {
        localTotaldistributionCount[0] = 0;
        localSplitCount[0] = 0;
        localVoxelizeCount[0] = 0;
        ss = 0;
    }
    barrier(CLK_LOCAL_MEM_FENCE|CLK_GLOBAL_MEM_FENCE );
    if (nodeCount > index)
    {
        if ((toSplits[index] == 1) ||(toSplits[index] == 3))
        {
            atomic_add(&ss, 1);
            atomic_add(localTotaldistributionCount, clDistributionCount[index] );
        }
        if (toSplits[index] & 2)
        {
            atomic_inc(localVoxelizeCount);
        }
    }
    barrier(CLK_LOCAL_MEM_FENCE|CLK_GLOBAL_MEM_FENCE );
    if (get_local_id(0) == 0)
    {
        totaldistributionCount[group] = localTotaldistributionCount[0];
        splitCount[group] = ss;
        voxelizeCount[group] = localVoxelizeCount[0];
    }
}


// ( "wsAddupKernel", 7th kernel function invoked ) This is to sum up the distribution count and split count of all workgroup.
__kernel void WGGetMemoryCountForNextLevel(__global int *totaldistributionCount,    // 0
                                           __global int *splitCount,                // 1
                                           const int nodeCount,                     // 2
                                           __global int *accTotaldistributionCount, // 3
                                           __global int *voxelizeCount)             // 4
{
    int index = get_global_id(0);
    int s;

    if (index>= nodeCount) return;

    s = totaldistributionCount[index];
    totaldistributionCount[index] = atomic_add(accTotaldistributionCount, s);
    s =  splitCount[index];
    splitCount[index] = atomic_add(accTotaldistributionCount+1, s);
    voxelizeCount[index] = atomic_add(accTotaldistributionCount+2, voxelizeCount[index]);
}


// ( "CreateNodesKernel", 1st kernel function invoked) This is to split each node into 8 children
__kernel void CompactNodes( __global const char *toSplits,              //  0
                            __global const float2 *x,                   //  1
                            __global const float2 *y,                   //  2
                            __global const float2 *z,                   //  3
                            __global const char *labels,                //  4
                            __global const short *distribution,         //  5
                            __global const short *clDistributionCount,  //  6
                            __global const int *distributionHead,       //  7
                            __global float2 *NewX,                      //  8
                            __global float2 *NewY,                      //  9
                            __global float2 *NewZ,                      // 10
                            __global char *NewARRAY_LABEL,              // 11
                            __global short *Newdistribution,            // 12
                            __global short *NewdistributionCount,       // 13
                            __global int *NewdistributionHead,          // 14
                            __local int *localDistributionCount,        // 15
                            __local int *localSplitCount,               // 16
                            const int nodeCount,                        // 17
                            __global const int *distributioGroupCount,  // 18
                            __global const int *splitGroupCount,        // 19
                            __global int *nodeMap,                      // 20
                            const long distCount,                       // 21
                            const int newNodeCount,                     // 22
                            __global long *VoxelIndex,                  // 23
                            __global const int *voxelizeGroupCount,     // 24
                            __local int *localVoxelizeCount)            // 25
{
    int index = get_global_id(0);      // node of octree
    int groupIndex = get_group_id(0);  // workgroup
    if (get_local_id(0) == 0)          // initialized by the first thread
    {
        localDistributionCount[0] = 0;
        localSplitCount[0] = 0;
        localVoxelizeCount[0] = 0;
    }

    barrier(CLK_LOCAL_MEM_FENCE);      //wait for everybody
    if (index < nodeCount)
    {
        if (toSplits[index] & 1) // if value is 1 or 3
        {
            int count = clDistributionCount[index];       //old distribution count
            int head = distributioGroupCount[groupIndex]+ atomic_add(localDistributionCount, count);   //reserve new distribution
            int newIndex = splitGroupCount[groupIndex]+ atomic_inc(localSplitCount);  //

            // Set the coordinates of each child
            NewX[newIndex] = x[index];
            NewY[newIndex] = y[index];
            NewZ[newIndex] = z[index];

            int fhead = distributionHead[index];
            char label = labels[index];

            // Copy each distribution to each child
            for (int j=0; j < count; j++)
            {
                Newdistribution[head + j] = distribution[fhead+j];
                for (int k=0; k<8; k++)
                {
                    nodeMap[k*distCount+head + j] = k*newNodeCount + newIndex;
                }
            }

            // Copy each label to each child
            NewARRAY_LABEL[newIndex] = label;
            NewdistributionCount[newIndex] = count;
            NewdistributionHead[newIndex] = head;
        }
        if (toSplits[index] & 2)
        {
            VoxelIndex[voxelizeGroupCount[groupIndex]+atomic_inc(localVoxelizeCount)] = index;
        }
    }
}


uchar GetColor(uchar index)
{
    // These were the values in Joe Chui's last-checked-in version
    // The repetition of (4, 4, 4, 4) is intentional and needs to be retained.         DH 2019-12-11
    // Added 4 new indicies for new ligament types (see FindLigamentType() this file)  DH 2019-12-11
    const uchar colormap[10] = {(uchar) 0,  // [0] Air
                                (uchar) 1,  // [1] cooper's
                                (uchar) 2,  // [2] skin
                                (uchar) 3,  // [3] adipose
                                (uchar) 4,  // [4] glandular(uint4) Apparently not used
                                (uchar) 4,  // [5] glandular again  This one is used
                                (uchar) 5,  // [6] ligament type 1
                                (uchar) 6,  // [7] ligament type 2
                                (uchar) 7,  // [8] ligament type 3
                                (uchar) 8}; // [9] ligament type 4

    return colormap[index];
}


//  ( "createLastLevelVoxelsKernel", 8th kernel function invoked ) Kernel to convert the tree nodes into voxels
__kernel void create_voxels_last_level(__global const float2 *x,           // 0 coordinates
                                       __global const float2 *y,           // 1
                                       __global const float2 *z,           // 2
                                       __global const char *toSplits,      // 3
                                       /////__write_only image3d_t voxelValues, // 4
                                       __global uchar *voxelValues,        // 4
                                       __global const char *labels,        // 5
                                       const float4 LOW,                   // 6 the corner
                                       const int nodeCount,                // 7
                                       const float4 voxelResolution,       // 8
                                       const int v,                        // 9  voxelDimension (same in x, y, z)
                                       const int4 origin)                  // 10
{
    int index = get_global_id(0);
    if (index >= nodeCount) return;

    if (toSplits[index] ) // check the STORED index whether the node to be split 11/23/2010
    {
        float xLow  = x[index].s0;
        float xhigh = x[index].s1;

        float ylow  = y[index].s0;
        float yhigh = y[index].s1;

        float zlow  = z[index].s0;
        float zhigh = z[index].s1;

        short x_index[2] = {MyRound((xLow-LOW.x + voxelResolution.x) / voxelResolution.x), MyRound((xhigh - LOW.x) / voxelResolution.x)};
        short y_index[2] = {MyRound((ylow-LOW.y + voxelResolution.y) / voxelResolution.y), MyRound((yhigh - LOW.y) / voxelResolution.y)};
        short z_index[2] = {MyRound((zlow-LOW.z + voxelResolution.z) / voxelResolution.z), MyRound((zhigh - LOW.z) / voxelResolution.z)};

        // 11/25/2010. label, if equal to 4 (corresponding to skin that MAY
        // also contain Cooper ligaments or fat/dense tissue if split
        // further, is changed to 2 since we do not in reality distinguish 2
        // kinds of skin
        char label = labels[index];
        if (label == 4) label = 2; // Skin can be either 2 or 4 up until now

        uchar color = 0;
        uint  vndex = 0;
        for(short z = z_index[0]-1; z<=z_index[1]-1; z++)
        {
            for(short y = y_index[0]-1; y<=y_index[1]-1; y++)
            {
                for(short x = x_index[0]-1; x<=x_index[1]-1; x++)
                {
                    // DH 2080-08-18: The color is expressed as 4 separate integers (although we use the same value in all 4).
                    // ...write_imagegui() crams those values into a single, unsigned byte
                    ////write_imageui(voxelValues, (unsigned int4) (x,y,z,0)+origin, GetColor(label));
                    // Since the color values are not unique across the int4, we will plop one instance of the value in the 8-bit volume
                    vndex = (z * v * v) + (y * v) + x;
                    color = GetColor(label);
                    voxelValues[vndex] = color;
                }
            }
        }
    }
}


// ( "VoxelizeKernel", 3rd kernel function invoked) Kernel to convert the tree nodes into voxels

__kernel void voxelize(__global const float2 *x,            //  0 coordinates
                       __global const float2 *y,            //  1
                       __global const float2 *z,            //  2
                       /////__write_only image3d_t voxelValues,  //  3
                       __global uchar *voxelValues,         //  3
                       __global const char *labels,         //  4
                       const float4 LOW,                    //  5 the corner
                       const int nodeCount,                 //  6
                       const float4 voxelResolution,        //  7
                       const int v,                         //  8  voxelDimension (same in x, y, z)
                       __global long *voxelIndex,           //  9
                       const short mod,                     // 10
                       const int4 origin)                   // 11
{
    int index = get_global_id(0);

    if (index >= nodeCount)
    {
        return;
    }

    index = voxelIndex[index];
    float xLow  = x[index].s0;
    float xhigh = x[index].s1;
    float ylow  = y[index].s0;
    float yhigh = y[index].s1;
    float zlow  = z[index].s0;
    float zhigh = z[index].s1;

    short x_index[2] = {MyRound((xLow - LOW.x + voxelResolution.x) / voxelResolution.x), MyRound((xhigh - LOW.x) / voxelResolution.x)};
    short y_index[2] = {MyRound((ylow - LOW.y + voxelResolution.y) / voxelResolution.y), MyRound((yhigh - LOW.y) / voxelResolution.y)};
    short z_index[2] = {MyRound((zlow - LOW.z + voxelResolution.z) / voxelResolution.z), MyRound((zhigh - LOW.z) / voxelResolution.z)};

    // 11/25/2010. label, if equal to 4 (corresponding to skin that MAY
    // also contain Cooper ligaments or fat/dense tissue if split
    // further, is changed to 2 since we do not in reality distinguish 2
    // kinds of skin
    char label = labels[index];

    if (label == 4) label=2;

    uchar color = 0;
    uint  vndex = 0;
    for(short z = z_index[0]-1; z<=z_index[1]-1; z++)
    {
        for(short y = y_index[0]-1; y<=y_index[1]-1; y++)
        {
            for(short x = x_index[0]-1; x<=x_index[1]-1; x++)
            {
                // DH 2080-08-18: The color is expressed as 4 separate integers (although we use the same value in all 4).
                // ...write_imagegui() crams those values into a single, unsigned byte
                ////write_imageui(voxelValues, (unsigned int4) (x,y,z,0)+origin, GetColor(label));
                // Since the color values are not unique across the int4, we will plop one instance of the value in the 8-bit volume
                vndex = (z * v * v) + (y * v) + x;
                color = GetColor(label);
                voxelValues[vndex] = color;
            }
        }
    }
}


// ( "CreateNodesKernel2", 2nd kernel function invoked ) This is to split each node into 8 children

__kernel void SubdivideNodes(__global const char *toSplits,         // 0 UNUSED!
                             __global  float2 *x,                   // 1
                             __global  float2 *y,                   // 2
                             __global  float2 *z,                   // 3
                             __global  char *labels,                // 4
                             __global  short *distribution,         // 5
                             __global  short *clDistributionCount,  // 6
                             __global  int *distributionHead,       // 7
                             const int nodeCount,                   // 8
                             const int totalDCount)                 // 9
{
    int index = get_global_id(0);
    if (index < nodeCount)
    {
        int head = distributionHead[index];       //reserve new distribution

        float2 pt = x[index];
        float4 newx = (float4)(pt.s0, 0.5f*(pt.s0 + pt.s1), pt.s1, 0.0f);

        // Set the coordinates of each child
        x[index]   = (float2) (newx.s0, newx.s1);
        x[index+nodeCount] = (float2) (newx.s0, newx.s1);
        x[index+2*nodeCount] = (float2) (newx.s0, newx.s1);
        x[index+3*nodeCount] = (float2) (newx.s0, newx.s1);
        x[index+4*nodeCount] = (float2) (newx.s1, newx.s2);
        x[index+5*nodeCount] = (float2) (newx.s1, newx.s2);
        x[index+6*nodeCount] = (float2) (newx.s1, newx.s2);
        x[index+7*nodeCount] = (float2) (newx.s1, newx.s2);

        pt = y[index];
        newx = (float4)(pt.s0, 0.5f*(pt.s0 + pt.s1), pt.s1, 0.0f);

        y[index]   = (float2) (newx.s0, newx.s1);
        y[index+nodeCount] = (float2) (newx.s0, newx.s1);
        y[index+2*nodeCount] = (float2) (newx.s1, newx.s2);
        y[index+3*nodeCount] = (float2) (newx.s1, newx.s2);
        y[index+4*nodeCount] = (float2) (newx.s0, newx.s1);
        y[index+5*nodeCount] = (float2) (newx.s0, newx.s1);
        y[index+6*nodeCount] = (float2) (newx.s1, newx.s2);
        y[index+7*nodeCount] = (float2) (newx.s1, newx.s2);

        pt = z[index];
        newx = (float4)(pt.s0, 0.5f*(pt.s0 + pt.s1), pt.s1, 0.0f);

        z[index]   = (float2) (newx.s0, newx.s1);
        z[index+nodeCount] = (float2) (newx.s1, newx.s2);
        z[index+2*nodeCount] = (float2) (newx.s0, newx.s1);
        z[index+3*nodeCount] = (float2) (newx.s1, newx.s2);
        z[index+4*nodeCount] = (float2) (newx.s0, newx.s1);
        z[index+5*nodeCount] = (float2) (newx.s1, newx.s2);
        z[index+6*nodeCount] = (float2) (newx.s0, newx.s1);
        z[index+7*nodeCount] = (float2) (newx.s1, newx.s2);


        // Copy each distribution to each child

        // Copy each label to each child
        #pragma unroll
        for (int i=1; i<8; i++)
        {
            distributionHead[index+i*nodeCount] = head+ i*totalDCount;
        }
    }
}
