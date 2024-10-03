// vector3.cpp

#include "vector3.h"
#include <cmath>
#include <cfloat>
#include <iostream>


#pragma region Documentation
/// <summary>   round function. </summary>
/// <remarks>   Joe Chui, 10/12/2011. </remarks>
/// <param name="v">    The v. </param>
/// <returns>   rounded value </returns>
#pragma endregion

#if defined(_MSC_VER) && (_MSC_VER < 1910) // VS2015 and earlier
namespace std
{
    float max(const float &a, const float &b)
    {
        return (b < a) ? a : b;
    }
    float min(const float &a, const float &b)
    {
        return (b > a) ? a : b;
    }
}
#endif
#if defined(MSC_VER) && (_MSC_VER < 1800)// if Visual Studio 2012 and earlier
int round(float v)
{
  return int (v+ 0.5); 
}
#endif

#pragma region Documentation
/// <summary>Print the data contents of this class.</summary>
/// <remarks>D. Higginbotham 2020-04-30.</remarks>
#pragma endregion
void columnVector::dump()
{
    std::cout << v[0] << '\t' << v[1] << '\t' << v[2] << std::endl;
}


#pragma region Documentation
/// <summary>[] casting operator. </summary>
/// <remarks>Joe Chui, 10/12/2011. </remarks>
/// <param name="index">Zero-based index of the column. </param>
/// <returns>The value of the element. </returns>
#pragma endregion

float columnVector::operator[](int index)
{

    return v[index];
}

float columnVector::operator * (columnVector &a)
{
    return v[0] * a.v[0] + v[1] * a.v[1] + v[2] * a.v[2];
}

columnVector columnVector::operator * (float a)
{
    return columnVector(a*v[0], a*v[1], a*v[2]);
}
columnVector columnVector::square()
{
    return columnVector(v[0]*v[0], v[1]*v[1], v[2]*v[2]);

}

columnVector columnVector::operator - (columnVector &a)
{
    return columnVector(v[0]-a.v[0], v[1]-a.v[1], v[2]-a.v[2]); 

}
columnVector columnVector::operator + (columnVector &a)
{
    return columnVector(v[0]+a.v[0], v[1]+a.v[1], v[2]+a.v[2]); 
}
columnVector columnVector::operator / (float s)
{
    return columnVector(v[0]/s, v[1]/s, v[2]/s); 
}
columnVector columnVector::operator / (columnVector &a)
{
    return columnVector(v[0]/a[0], v[1]/a[1], v[2]/a[2]); 
}
columnVector columnVector::cross(columnVector &a, columnVector &b)
{
    return columnVector(a.v[1]*b.v[2]-a.v[2]*b.v[1], a.v[2]*b.v[0]-a.v[0]*b.v[2], a.v[0]*b.v[1]-a.v[1]*b.v[0]);
}
columnVector::columnVector(float x, float y, float z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

columnVector::columnVector()
{
    v[0] = v[1] = v[2] = 0.0f;
}

float columnVector::sum(columnVector &a)
{
    return a.v[0]+a.v[1]+a.v[2];
}
void columnVector::set(float a, float b, float c)
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

float columnVector::sum()
{
    return v[0] + v[1] + v[2];

}

void columnVector::set(char index, float value)
{
    v[index] = value;
}

columnVector columnVector::log()
{

    return columnVector(::log(v[0]), ::log(v[1]), ::log(v[2]));

}

#pragma region Documentation
/// <summary>Print the data contents of this class.</summary>
/// <remarks>D. Higginbotham 2020-04-30.</remarks>
#pragma endregion
void rowVector::dump()
{
    std::cout << v[0] << '\t' << v[1] << '\t' << v[2] << std::endl;
}

float rowVector::operator [](int index)
{
    return v[index];

}

float rowVector::operator * (rowVector &a)
{
    return v[0] * a.v[0] + v[1] * a.v[1] + v[2] * a.v[2];
}
rowVector rowVector::operator * (matrix &m)
{
    ///rowVector c = rowVector(::operator*(*this, m[0]), ::operator*(*this, m[1]), ::operator*(*this, m[2]));
    columnVector cv1(m[0]);
    columnVector cv2(m[1]);
    columnVector cv3(m[2]);
    rowVector rv = rowVector(::operator*(*this, cv1), ::operator*(*this, cv2), ::operator*(*this, cv3));
    return rv;
}

rowVector rowVector::operator * (float a)
{
    return rowVector(a*v[0], a*v[1], a*v[2]);
}
rowVector rowVector::square()
{
    return rowVector(v[0]*v[0], v[1]*v[1], v[2]*v[2]);
}

rowVector rowVector::operator - (rowVector &a)
{   
    return rowVector(v[0]-a.v[0], v[1]-a.v[1], v[2]-a.v[2]); 

}
rowVector rowVector::operator + (rowVector& a)
{
    return rowVector(v[0]+a.v[0], v[1]+a.v[1], v[2]+a.v[2]); 

}
rowVector rowVector::operator / (float s)
{
    return rowVector(v[0]/s, v[1]/s, v[2]/s); 
}
rowVector rowVector::cross(rowVector &a, rowVector &b)
{
    return rowVector(a.v[1]*b.v[2]-a.v[2]*b.v[1], a.v[2]*b.v[0]-a.v[0]*b.v[2], a.v[0]*b.v[1]-a.v[1]*b.v[0]);
}
rowVector::rowVector(float x, float y, float z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

rowVector::rowVector()
{
    v[0] = v[1] = v[2] = 0.0f;
}

float rowVector::sum(rowVector &a)
{
    return a.v[0] + a.v[1] + a.v[2];
}

void rowVector::set(float a, float b, float c)
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
float rowVector::sum()
{
    return v[0] + v[1] + v[2];
}


#pragma region Documentation
/// <summary>Print the data contents of this class.</summary>
/// <remarks>D. Higginbotham 2020-04-30.</remarks>
#pragma endregion
void matrix::dump()
{
    std::cout << v[0] << '\t' << v[1] << '\t' << v[2] << '\n'
        << v[3] << '\t' << v[4] << '\t' << v[5] << '\n'
        << v[6] << '\t' << v[7] << '\t' << v[8] 
        << std::endl;
}

#pragma region Documentation
/// <summary>   Constructor. </summary>
///
/// <remarks>   Joe Chui, 10/12/2011. </remarks>
///
/// <param name="x">    [in,out] The x coordinate. </param>
/// <param name="y">    [in,out] The y coordinate. </param>
/// <param name="z">    [in,out] The z coordinate. </param>
#pragma endregion
matrix::matrix(rowVector &x, rowVector &y, rowVector &z)
{
    v[0] = x[0];
    v[3] = x[1];
    v[6] = x[2];
    v[1] = y[0];
    v[4] = y[1];
    v[7] = y[2];
    v[2] = z[0];
    v[5] = z[1];
    v[8] = z[2];
}

#pragma region Documentation
/// <summary>   Constructor. </summary>
///
/// <remarks>   D. Higginbotham, 04/10/2020. </remarks>
///
/// <param name="x">    The x coordinate. </param>
/// <param name="y">    The y coordinate. </param>
/// <param name="z">    The z coordinate. </param>
#pragma endregion

matrix::matrix(float x, float y, float z)
{
    v[0] = x;
    v[4] = y;
    v[8] = z;
    v[1] = v[2] = v[3] = v[5] =v[6] = v[7] = 0.0;

}

matrix::matrix()
{
    // Initialize to identity matrix
    v[0] = v[4] = v[8] = 1.0f;
    v[1] = v[2] = v[3] = v[5] =v[6] = v[7] = 0.0;
}

#pragma region Documentation
/// <summary>   [] casting operator. </summary>
///
/// <remarks>   Joe Chui, 10/12/2011. </remarks>
///
/// <param name="index">    Zero-based index of the. </param>
///
/// <returns>   The result of the operation. </returns>
#pragma endregion

columnVector matrix::operator [](int index)
{
    return columnVector(v[index * 3], v[index * 3+1], v[index * 3+2]);
}

#pragma region Documentation
/// <summary>   * casting operator. </summary>
///
/// <remarks>   Joe Chui, 10/12/2011. </remarks>
///
/// <param name="v1">   [in] The columnVector to be multipled . </param>
///
/// <returns>   The result of the operation. </returns>
#pragma endregion

columnVector matrix::operator * (columnVector &v1)
{
    columnVector c;
    float e = v1[0], f = v1[1], g = v1[2];
    c.set(0, v[0] * e + v[3] * f + v[6] * g);  
    c.set(1, v[1] * e + v[4] * f + v[7] * g);  
    c.set(2, v[2] * e + v[5] * f + v[8] * g);  

    return c;
}

matrix matrix::operator * (matrix &b)
{
    matrix m(1.0, 0.0, 0.0);
    for (int i=0; i<9; i++)
    {
        int row = i%3;
        int col = i/3;
        m.v[i] = v[row] * b.v[col*3] + v[row+3] * b.v[col*3+1] + v[row+6] * b.v[col*3+2];
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

matrix matrix::operator - (matrix &a)
{
    matrix m(1.0, 1.0, 1.0);
    for (int i=0; i<9; i++)
        m.v[i] = v[i]-a.v[i];
    return m;
}

rowVector matrix::row(int row)
{
    return rowVector(v[row], v[3+row], v[6+row]);
}

const float epsilon = float(1.0e-5);

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
 
    float det = v[0]*inverse.v[0] + v[1]*inverse.v[3] +
        v[2]*inverse.v[6];
 
    if (::abs(det) > epsilon)
    {
        float  invDet = ((float)1)/det;
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

matrix matrix::chol()
{
    matrix m(1.0, 1.0, 1.0);
    m = *this;
    int i, j, k; 
    float sum=0.0;
    float p[3];
    for (i=0; i< 3;i++)
    {
        for (j=i; j<3; j++)
        {
            for (sum=m.v[i*3+j], k=i-1; k>=0; k--)
                sum -= m.v[i*3+k]*m.v[j*3+k];
            if (i == j)
            {
                if (sum <= 0.0)
                {
                    float dd = 100.0;
                    //throw exception TODO
                }
                p[i] = sqrt(sum);
            }
            else
            {
                m.v[j*3 + i] = sum/p[i];
            }
        }
    }
    m.v[0] = p[0];
    m.v[4] = p[1];
    m.v[8] = p[2];
    m.v[1]=m.v[2]=m.v[5] = 0.0;
    m= m.transpose();
    return m;
}


#pragma region Documentation
/// <summary>Print the data contents of this class.</summary>
/// <remarks>D. Higginbotham 2020-04-30.</remarks>
#pragma endregion
void columnVector8::dump()
{
    std::cout << v[0] << '\t' << v[1] << '\t' << v[2] << '\t'
        << v[3] << '\t' << v[4] << '\t' << v[5] << '\t'
        << v[6] << '\t' << v[7] << std::endl; 
}

columnVector8 columnVector8::operator * (float a)
{
    columnVector8 c;
    c.v[0] = v[0] * a;
    c.v[1] = v[1] * a;
    c.v[2] = v[2] * a;
    c.v[3] = v[3] * a;
    c.v[4] = v[4] * a;
    c.v[5] = v[5] * a;
    c.v[6] = v[6] * a;
    c.v[7] = v[7] * a;

    return c;

}
columnVector8 columnVector8::operator + (float a)
{
    columnVector8 c;
    c.v[0] = v[0] + a;
    c.v[1] = v[1] + a;
    c.v[2] = v[2] + a;
    c.v[3] = v[3] + a;
    c.v[4] = v[4] + a;
    c.v[5] = v[5] + a;
    c.v[6] = v[6] + a;
    c.v[7] = v[7] + a;
    return c;

}
columnVector8 columnVector8::operator + (columnVector8 &a)
{
    columnVector8 c;
    c.v[0] = v[0] + a.v[0];
    c.v[1] = v[1] + a.v[1];
    c.v[2] = v[2] + a.v[2];
    c.v[3] = v[3] + a.v[3];
    c.v[4] = v[4] + a.v[4];
    c.v[5] = v[5] + a.v[5];
    c.v[6] = v[6] + a.v[6];
    c.v[7] = v[7] + a.v[7];
    return c;
    
}

float columnVector8::min()
{
    float c = FLT_MAX;
    for (int i=0; i<8; i++)
        c = ::min(v[i], c);
    return c;

}

float columnVector8::max()
{
    float c = -FLT_MAX;
    for (int i=0; i<8; i++)
        c = ::max(v[i], c);
    return c;
}
void columnVector8::set(int index, float value)
{
    v[index] = value;

}

float columnVector8::operator [](int index)
{
    return v[index];

}



columnVector8::columnVector8()
{
  v[0] =   v[1] =   v[2] =   v[3] =   v[4] =   v[5] =   v[6] =   v[7] = 0.0;

}



#pragma region Documentation
/// <summary>Print the data contents of this class.</summary>
/// <remarks>D. Higginbotham 2020-04-30.</remarks>
#pragma endregion
void matrix8x3::dump()
{
    for(int i=0; i<8; ++i)
    {
        std::cout << v[i][0] << '\t' << v[i][1] << '\t' << v[i][2] 
            << std::endl;
    }
}

rowVector matrix8x3:: operator [](int index)
{
    return v[index];
};

columnVector8 matrix8x3::operator *(columnVector &a)
{
    columnVector8 c;
    for (int i = 0; i<8; i++)
    {
        c.set(i, v[i][0]*a[0] + v[i][1]*a[1] +v[i][2]* a[2]);

    }
    return c;

}

void matrix8x3::set(int index, rowVector &r)
{
    v[index] = r;
}

matrix8x3::matrix8x3()//(int index, rowVector r)
{
  v[0] = rowVector(0.0, 0.0, 0.0); 
  v[1] = rowVector(0.0, 0.0, 0.0); 
  v[2] = rowVector(0.0, 0.0, 0.0); 
  v[3] = rowVector(0.0, 0.0, 0.0); 
  v[4] = rowVector(0.0, 0.0, 0.0); 
  v[5] = rowVector(0.0, 0.0, 0.0); 
  v[6] = rowVector(0.0, 0.0, 0.0); 
  v[7] = rowVector(0.0, 0.0, 0.0); 
}

matrix8x3 operator * (float a, matrix8x3 &b)
{
    matrix8x3 m;
    for (int i=0; i<8; i++)
    {
        ///m.set(i, b[i] *a);
        rowVector rv(b[i] * a);
        m.set(i, rv);
    }
    return m;
}

columnVector8 operator * (float a, columnVector8 &b)
{
    return b*a;
}

rowVector operator * (float a, rowVector &b)
{
    return b*a;
}

float operator *(rowVector &r, columnVector &c)
{
    return r[0]*c[0] +r[1]*c[1]+r[2]*c[2];
}