
#include <cmath>
#include <cfloat>
#include <cstdlib>
#include "vector3.h"

template <class T> const T& max ( const T& a, const T& b ) {
  return (b<a)?a:b;     // or: return comp(b,a)?a:b; for the comp version
}
template <class T> const T& min ( const T& a, const T& b ) {
  return (b>a)?a:b;     // or: return comp(b,a)?a:b; for the comp version
}


#pragma region Documentation
/// <summary>	round function. </summary>
///
/// <remarks>	Joe Chui, 10/12/2011. </remarks>
///
/// <param name="v">	The v. </param>
///
/// <returns>	rounded value </returns>
/// <warning>   conflicts with C++11 round function in VS2013 and must be conditionally compiled-out</warning>
#pragma endregion

#if defined(_MSC_VER) && (_MSC_VER <= 1700)
int round(double v)
{
  return int (v+ 0.5); 
}
#endif

#pragma region Documentation
/// <summary>	[] casting operator. </summary>
///
/// <remarks>	Joe Chui, 10/12/2011. </remarks>
/// <remarks>	Dave Higginbotham, 08/105/2014. </remarks>
///
/// <param name="index">	Zero-based index of the column. </param>
///
/// <returns>	The value of the element. </returns>
#pragma endregion

double columnVector::operator [](int index)
{

	return v[index];
}

double columnVector::operator * (const columnVector &a)
{
	return v[0] * a.v[0] + v[1] * a.v[1] + v[2] * a.v[2];
}

columnVector columnVector::operator * (double a)
{
	return columnVector(a*v[0], a*v[1], a*v[2]);
}
columnVector columnVector::square()
{
	return columnVector(v[0]*v[0], v[1]*v[1], v[2]*v[2]);

}

columnVector columnVector::operator - (const columnVector &a)
{
	return columnVector(v[0]-a.v[0], v[1]-a.v[1], v[2]-a.v[2]); 

}
columnVector columnVector::operator + (const columnVector &a)
{
	return columnVector(v[0]+a.v[0], v[1]+a.v[1], v[2]+a.v[2]); 
}
columnVector columnVector::operator / (double s)
{
	return columnVector(v[0]/s, v[1]/s, v[2]/s); 
}
columnVector columnVector::operator / (const columnVector &a)
{
	return columnVector(v[0]/a.v[0], v[1]/a.v[1], v[2]/a.v[2]); 
}
columnVector columnVector::cross(const columnVector &a, const columnVector &b)
{
	return columnVector(a.v[1]*b.v[2]-a.v[2]*b.v[1], a.v[2]*b.v[0]-a.v[0]*b.v[2], a.v[0]*b.v[1]-a.v[1]*b.v[0]);
}
columnVector::columnVector(double x, double y, double z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

columnVector::columnVector()
{

}



double columnVector::sum(const columnVector &a)
{
	return a.v[0]+a.v[1]+a.v[2];
}
void columnVector::set(double a, double b, double c)
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
}

rowVector columnVector::transpose()
{
	return rowVector(v[0], v[1], v[2]);
}


columnVector columnVector::abs()
{
	return columnVector(::abs(v[0]), ::abs(v[1]), ::abs(v[2]));
}

double columnVector::sum()
{
	return v[0] + v[1] + v[2];

}

void columnVector::set(char index, double value)
{
	v[index] = value;
}

columnVector columnVector::log()
{

	return columnVector(::log(v[0]), ::log(v[1]), ::log(v[2]));

}

double rowVector::operator [](int index)
{
	return v[index];

}

double rowVector::operator * (const rowVector &a)
{
	return v[0] * a.v[0] + v[1] * a.v[1] + v[2] * a.v[2];
}
rowVector rowVector::operator * (matrix m)
{
	rowVector c = rowVector(::operator*(*this, m[0]), ::operator*(*this, m[1]), ::operator*(*this, m[2]));
	return c;
}

rowVector rowVector::operator * (double a)
{
	return rowVector(a*v[0], a*v[1], a*v[2]);
}
rowVector rowVector::square()
{
	return rowVector(v[0]*v[0], v[1]*v[1], v[2]*v[2]);
}

rowVector rowVector::operator - (const rowVector &a)
{	
	return rowVector(v[0]-a.v[0], v[1]-a.v[1], v[2]-a.v[2]); 

}

rowVector rowVector::operator / (const rowVector &a)
{	
	return rowVector(v[0]/a.v[0], v[1]/a.v[1], v[2]/a.v[2]); 

}

rowVector rowVector::operator % (const rowVector &a)
{	
	return rowVector(v[0]*a.v[0], v[1]*a.v[1], v[2]*a.v[2]); 

}
double rowVector::minvalue()
{
	return min(v[0], min(v[1], v[2]));
}

void rowVector::step(const rowVector & a, const rowVector& b)
{
	v[0] =v[0]+ a.v[0]*b.v[0];
	v[1] =v[1]+ a.v[1]*b.v[1];
	v[2] =v[2]+ a.v[2]*b.v[2];


}


rowVector rowVector::operator + (const rowVector& a)
{
	return rowVector(v[0]+a.v[0], v[1]+a.v[1], v[2]+a.v[2]); 

}
rowVector rowVector::operator / (double s)
{
	return rowVector(v[0]/s, v[1]/s, v[2]/s); 
}
rowVector rowVector::cross(const rowVector &a, const rowVector &b)
{
	return rowVector(a.v[1]*b.v[2]-a.v[2]*b.v[1], a.v[2]*b.v[0]-a.v[0]*b.v[2], a.v[0]*b.v[1]-a.v[1]*b.v[0]);
}
rowVector::rowVector(double x, double y, double z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

rowVector::rowVector()
{

}
double rowVector::sum(const rowVector &a)
{
	return a.v[0] + a.v[1] + a.v[2];
}

double rowVector::length()
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2]* v[2]) ;
}



void rowVector::set(double a, double b, double c)
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
}

columnVector rowVector::transpose()
{
	return columnVector(v[0], v[1], v[2]);
}


rowVector rowVector::abs()
{
	return rowVector(::abs(v[0]), ::abs(v[1]), ::abs(v[2]));
}
double rowVector::sum()
{
	return v[0] + v[1] + v[2];
}

#pragma region Documentation
/// <summary>	Constructor. </summary>
///
/// <remarks>	Joe Chui, 10/12/2011. </remarks>
///
/// <param name="x">	[in,out] The x coordinate. </param>
/// <param name="y">	[in,out] The y coordinate. </param>
/// <param name="z">	[in,out] The z coordinate. </param>
#pragma endregion
matrix::matrix(const rowVector &x, const rowVector &y, const rowVector &z, const rowVector &w)
{
	v[0] = x.v[0];
	v[3] = x.v[1];
	v[6] = x.v[2];
	
	v[1] = y.v[0];
	v[4] = y.v[1];
	v[7] = y.v[2];
	
	v[2] = z.v[0];
	v[5] = z.v[1];
	v[8] = z.v[2];
	
	v[2] = w.v[0];
	v[5] = w.v[1];
	v[8] = w.v[2];



}

#pragma region Documentation
/// <summary>	Constructor. </summary>
///
/// <remarks>	Joe Chui, 10/12/2011. </remarks>
///
/// <param name="x">	The x coordinate. </param>
/// <param name="y">	The y coordinate. </param>
/// <param name="z">	The z coordinate. </param>
#pragma endregion

matrix::matrix(double x, double y, double z)
{
	v[0] = x;
	v[4] = y;
	v[8] = z;
	v[1] = v[2] = v[3] = v[5] =v[6] = v[7] = v[9] = v[10] = v[11] = 0.0;

}

#pragma region Documentation
/// <summary>	[] casting operator. </summary>
///
/// <remarks>	Joe Chui, 10/12/2011. </remarks>
///
/// <param name="index">	Zero-based index of the. </param>
///
/// <returns>	The result of the operation. </returns>
#pragma endregion

columnVector matrix::operator [](int index)
{
	return columnVector(v[index * 3], v[index * 3+1], v[index * 3+2]);
}

#pragma region Documentation
/// <summary>	* casting operator. </summary>
///
/// <remarks>	Joe Chui, 10/12/2011. </remarks>
///
/// <param name="v1">	[in] The columnVector to be multipled . </param>
///
/// <returns>	The result of the operation. </returns>
#pragma endregion

columnVector matrix::operator * (const columnVector &v1)
{
	columnVector c;
	double e = v1.v[0], f = v1.v[1], g = v1.v[2];
	c.set(0, v[0] * e + v[3] * f + v[6] * g + v[9]);  
	c.set(1, v[1] * e + v[4] * f + v[7] * g + v[10]);  
	c.set(2, v[2] * e + v[5] * f + v[8] * g + v[11]);  

	return c;
}

matrix matrix::operator * (const matrix &b)
{
	matrix m(1.0, 0.0, 0.0);
	for (int i=0; i<12; i++)
	{
		int row = i%3;
		int col = i/3;
		m.v[i] = v[row] * b.v[col*3] + v[row+3] * b.v[col*3+1] + v[row+6] * b.v[col*3+2] ;
	}
	return m;
}

matrix matrix::transpose()
{
	matrix m(1.0, 1.0, 1.0);
	m.v[0] = v[0];
	m.v[1] = v[3];
	m.v[2] = v[6];
	m.v[3] = v[1];
	m.v[4] = v[4];
	m.v[5] = v[7];
	m.v[6] = v[2];
	m.v[7] = v[5];
	m.v[8] = v[8];
	return m;
}

matrix matrix::operator - (const matrix &a)
{
	matrix m(1.0, 1.0, 1.0);
	for (int i=0; i<12; i++)
		m.v[i] = v[i]-a.v[i];
	return m;
}

rowVector matrix::row(int row)
{
	return rowVector(v[row], v[3+row], v[6+row]);
}

const double epsilon= 1.0e-5;

matrix matrix::inverse()
{
    // Invert a 3x3 using cofactors.  This is faster than using a generic
    // Gaussian elimination because of the loop overhead of such a method.
 
    matrix inverse(1.0, 1.0, 1.0);
 
    // Compute the adjoint.
    inverse.v[0] = v[4]*v[8] - v[5]*v[7];
    inverse.v[1] = v[2]*v[7] - v[1]*v[8];
    inverse.v[2] = v[1]*v[5] - v[2]*v[4];
    inverse.v[3] = v[5]*v[6] - v[3]*v[8];
    inverse.v[4] = v[0]*v[8] - v[2]*v[6];
    inverse.v[5] = v[2]*v[3] - v[0]*v[5];
    inverse.v[6] = v[3]*v[7] - v[4]*v[6];
    inverse.v[7] = v[1]*v[6] - v[0]*v[7];
    inverse.v[8] = v[0]*v[4] - v[1]*v[3];
 
    double det = v[0]*inverse.v[0] + v[1]*inverse.v[3] +
        v[2]*inverse.v[6];
 
    if (::abs(det) > epsilon)
    {
        double  invDet = ((double)1)/det;
        inverse.v[0] *= invDet;
        inverse.v[1] *= invDet;
        inverse.v[2] *= invDet;
        inverse.v[3] *= invDet;
        inverse.v[4] *= invDet;
        inverse.v[5] *= invDet;
        inverse.v[6] *= invDet;
        inverse.v[7] *= invDet;
        inverse.v[8] *= invDet;
        return inverse;
    }

    return matrix(0.0, 0.0, 0.0);
}

columnVector matrix::diag()
{
	return columnVector(v[0], v[4], v[8]);
}



rowVector operator * (double a, const rowVector &b)
{
	rowVector r = b;
	return r*a;
}
double operator *(const rowVector &r, const columnVector &c)
{

	return r.v[0]*c.v[0] +r.v[1]*c.v[1]+r.v[2]*c.v[2];
}