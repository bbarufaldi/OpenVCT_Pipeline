// RayTracer.h

#pragma once

#include <functional>
#include "Volume.h"
#include "Ray.h"
#include <iostream> // for debug
#include <cmath>    // for std::fabs

template <class T> class RayTracer
{
public:
	virtual	bool trace( Ray &ray,  Volume<T> &vol, 
                        std::function<void(std::vector<double> &lengthTable, T &voxel, double length)>, 
                        std::vector<double> &lengthTable, double & inDistance, double &outDistance, bool) = 0;
};

template <class T> class ThreeDDDARayTracer : RayTracer<T>
{
public:
	bool trace( Ray &ray,  Volume<T> &vol, std::function<void(std::vector<double> &lengthTable, T &voxel, double length)>,  std::vector<double> &lengthTable, double & inDistance, double &outDistance, bool);
	void get_volume_options(Volume<T> &vol);
	bool view_slice;
	double upper_bound;
	double lower_bound;
};

template <class T> void ThreeDDDARayTracer<T>::get_volume_options(Volume<T> &vol)
{
	// Locate the volume slice on the detector  DH 08/29/2014
	view_slice = vol.GetViewSlice();
	if (view_slice)
	{
		// Use the override offsets specified in the XML detector section
		upper_bound = vol.GetSliceUpperBound();
		lower_bound = vol.GetSliceLowerBound();
	}
}

template <class T> bool ThreeDDDARayTracer<T>::trace(Ray &ray, Volume<T> &vol, 
													 std::function<void(std::vector<double> &lengthTable, T &voxel, double length)> func, 
													 std::vector<double> &lengthTable, double &inDistance, double &outDistance, bool okaytoPrint)
{

	rowVector Dir = ray.endPoint - ray.startPoint;
	double length = Dir.length();

	double x1=0.0, xn=1.0, y1=0.0, yn=1.0, z1=0.0, zn=1.0;
	
	// Siddon EQ (4) find parametric values corresponding to the sides (of the voxel)
	if (Dir[0] != 0.0)
	{	
		x1 = -ray.startPoint[0] / Dir[0];
		xn = ((vol.voxelCount[0]*vol.voxelSize[0]) - ray.startPoint[0]) / Dir[0];
	}
    else
    {
        Dir.v[0]= 1.0e-10;
    }

	if (Dir[1] != 0.0)
	{	
		y1 = -ray.startPoint[1] / Dir[1];
		yn = ((vol.voxelCount[1]*vol.voxelSize[1]) - ray.startPoint[1]) / Dir[1];
	}
    else
    {
		Dir.v[1]= 1.0e-10;
    }

	if (Dir[2] != 0.0)
	{	
		z1 = -ray.startPoint[2] / Dir[2];
		zn = ((vol.voxelCount[2]*vol.voxelSize[2]) - ray.startPoint[2]) / Dir[2];
	}
    else
    {
		Dir.v[2]= 1.0e-10;
    }

    if (okaytoPrint)
    {
        std::cout << "\nVoxelCount = { " << vol.voxelCount[0] << " / " << vol.voxelCount[1] << " / " << vol.voxelCount[2] << "}\n"
                  << "VoxelSize  = { " << vol.voxelSize[0]  << " / " << vol.voxelSize[1]  << " / " << vol.voxelSize[2]  << "}\n"
                  << "StartPoint = { " << ray.startPoint[0] << " / " << ray.startPoint[1] << " / " << ray.startPoint[2] << "}\n"
                  << "EndPoint   = { " << ray.endPoint[0]   << " / " << ray.endPoint[1]   << " / " << ray.endPoint[2]   << "}\n";
        std::cout << "x1=" << x1 << ", xn= " << xn << ", y1=" << y1 << ", yn= " << yn << "z1=" << z1 << ", zn= " << zn << std::endl;
    }

	// Siddon EQ(5) Find whether the ray intersects the CT array - dh-2014-09-30
    #if 0 //def USE_ORIG
	double alpha_min = max<double>(max<double>(0.0, min<double>(x1, xn)), 
						   max<double>(min<double>(y1, yn), min<double>(z1, zn))); 

	double alpha_max = min<double>(min<double>(1.0, max<double>(x1, xn)), 
						   min<double>(max<double>(y1, yn), max<double>(z1, zn))); 
    #else
	
	double alpha_min = std::max<double>(0.0, std::max<double>(std::min<double>(x1, xn), std::max<double>(std::min<double>(y1, yn), std::min<double>(z1, zn))));

	double alpha_max = std::min<double>(1.0, std::min<double>(std::max<double>(x1, xn), std::min<double>(std::max<double>(y1, yn), std::max<double>(z1, zn))));
    #endif

	// The ray intersects the CT array- dh-2014-09-30
	if (alpha_max > alpha_min)
	{
		inDistance = alpha_min * length;
		outDistance = (1.0 - alpha_max) * length;		
	}
	else // The ray does not intersect the CT array - dh-2014-09-30
	{
		inDistance = length;
		outDistance = 0.0;
        return false;
	}

    if (okaytoPrint) std::cout << "\ninDistance=" << inDistance << ", outDistance=" << outDistance << std::endl;

	rowVector Step, invDir, DeltaT, Voxellncr(0.0, 0.0, 0.0), vT, CurrentV;



     // Siddon EQ(7)
	int i,j,k;

    if (ray.endPoint[0] >= ray.startPoint[0])
    {
		i = (int)(vol.voxelCount[0] - ((vol.voxelCount[0]* vol.voxelSize[0]) - alpha_min*(Dir[0]) - ray.startPoint[0])/vol.voxelSize[0]);
        if (okaytoPrint) 
        {
            std::cout << "\nray endpoint >= startpoint: i = " << i << ", ray.startPoint[0]=" << ray.startPoint[0] << ", ray.endPoint[0]=" << ray.endPoint[0] 
                      << ", alpha_max=" << alpha_max <<  ", alpha_min=" << alpha_min << ", Dir[0]=" << Dir[0]
                      << ", vol.voxelSize[0]=" << vol.voxelSize[0] << std::endl;
        }
    }
	else
    {
		i = (int)((ray.startPoint[0] + alpha_min*(Dir[0]))/vol.voxelSize[0]-1);
        if (okaytoPrint) 
        {
            std::cout << "\nray endpoint < startpoint: i = " << i << ", ray.startPoint[0]=" << ray.startPoint[0] << ", ray.endPoint[0]=" << ray.endPoint[0]
                      << ", alpha_max=" << alpha_max << ", alpha_min=" << alpha_min << ", Dir[0]=" << Dir[0]
                      << ", vol.voxelSize[0]=" << vol.voxelSize[0] << std::endl;
        }

    }
	
	if (ray.endPoint[1] >= ray.startPoint[1])
		j = (int)(vol.voxelCount[1] - ((vol.voxelCount[1]* vol.voxelSize[1]) - alpha_min*(Dir[1]) - ray.startPoint[1])/vol.voxelSize[1]);
	else
		j = (int)((ray.startPoint[1] + alpha_min*(Dir[1]))/vol.voxelSize[1]-1);

	if (ray.endPoint[2] >= ray.startPoint[2])
		k = (int)(vol.voxelCount[2] - ((vol.voxelCount[2]* vol.voxelSize[2]) - alpha_min*(Dir[2]) - ray.startPoint[2])/vol.voxelSize[2]);
	else
		k = (int)((ray.startPoint[2] + alpha_min*(Dir[2]))/vol.voxelSize[2]-1);

	CurrentV.set(i,j,k);

	// DH: changed constants from floats to doubles
	Step = rowVector((Dir[0]<0.0) ? -1.0:1.0, (Dir[1]<0.0) ? -1.0:1.0, (Dir[2]<0.0) ? -1.0:1.0); // contributes to causing of CurrentV[0] to go negative DH 2015-06-11
	rowVector offset((Dir[0]<0.0) ?  0.0:1.0, (Dir[1]<0.0) ?  0.0:1.0, (Dir[2]<0.0) ?  0.0:1.0);

	invDir = rowVector( std::fabs(1.0 / Dir[0]), std::fabs(1.0 / Dir[1]), std::fabs(1.0 / Dir[2]));
	DeltaT = vol.voxelSize % invDir;

	double oldT = alpha_min;
	vT = (((CurrentV+offset) % vol.voxelSize)- ray.startPoint) / Dir;
	
    int badVoxelCount = 0;

    while( (vol.voxelCount[0] > CurrentV[0]) && (0 <= CurrentV[0]) &&
		   (vol.voxelCount[1] > CurrentV[1]) && (0 <= CurrentV[1]) &&
		   (vol.voxelCount[2] > CurrentV[2]) && (0 <= CurrentV[2]) &&  (oldT < 1.0)) //CurrentV is inside the traversal region 
	{
		Voxellncr = rowVector( (vT[0] <= vT[1]) && (vT[0] <= vT[2]) ? 1.0 : 0.0, 
			                   (vT[1] <  vT[0]) && (vT[1] <= vT[2]) ? 1.0 : 0.0, 
							   (vT[2] <  vT[0]) && (vT[2] <  vT[1]) ? 1.0 : 0.0);
        
		double t = vT.minvalue();

		vT.step(Voxellncr, DeltaT);


		T mat =  vol.getVoxel((unsigned int)CurrentV[0], (unsigned int) CurrentV[1], (unsigned int)CurrentV[2]);
		CurrentV.step(Voxellncr, Step);   // Can cause CurrentV[0] to be < 0 - DH 2015-06-11
        
        // Begin - DH 2015-06-11
        // Added this check because (a) CurrentV[0] was found to be < 0 after step(), and (b) voxel value found to be up near 255. - DH
        // Finding this happens once per ray. CurrentV[0] is -1 and mat is 0 - 2017-01-13 DH
        if (CurrentV[0] < 0 || mat > 240)
        {
            badVoxelCount++;
            //     std::cerr << __FUNCTION__ << ": BAD voxel #" << badVoxelCount << "! CurrentV[0]=" << CurrentV[0] << ", mat=" << int(mat) << std::endl;
            //     continue; // IGNORE THIS VOXEL AND MOVE ON - DH 2015-06-11
        }
        // End - DH 2015-06-11

		// Examine volume if not in slice mode or if volume is within slice bounds    // DH 08/29/2014
        if (!view_slice || (CurrentV[2] > lower_bound && CurrentV[2] < upper_bound))
		{
            func(lengthTable, mat, (std::min(1.0, t) - oldT)*length); // invoke the specified lambda function
            
            //lengthTable[mat] = lengthTable[mat] + *length; //Don't always count the whole voxel.
		    oldT = t;
		}                                                                             // DH  08/29/2014
	}
	return true;
 }
