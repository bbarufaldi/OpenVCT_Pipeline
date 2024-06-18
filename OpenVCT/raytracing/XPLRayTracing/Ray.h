// Ray.h

#pragma once

#include "vector3.h"

class Ray
{
public:
	Ray(rowVector &startVec, rowVector &endVec);
	Ray();
	rowVector Evaluate(double t);
	~Ray(void);
	rowVector startPoint;
	rowVector endPoint;

	double getLength() { return (endPoint-startPoint).length(); }
};

