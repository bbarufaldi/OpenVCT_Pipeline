#include "Ray.h"


/// <summary>
/// Initializes a new instance of the <see cref="Ray"/> class.
/// </summary>
/// <param name="startVec">The start vec.</param>
/// <param name="endVec">The end vec.</param>
Ray::Ray(rowVector &startVec, rowVector &endVec):startPoint(startVec), endPoint(endVec)
{

}

/// <summary>
/// Finalizes an instance of the <see cref="Ray"/> class.
/// </summary>
Ray::~Ray(void)
{

}

/// <summary>
/// Evaluates the specified t.
/// </summary>
/// <param name="t">The t.</param>
/// <returns></returns>
rowVector Ray::Evaluate(double t)
{
	return (1-t)*startPoint + t*endPoint;
}