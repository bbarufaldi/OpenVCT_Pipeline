// vector3.h

#pragma once

#include <cmath>

#if defined(_MSC_VER) && (_MSC_VER < 1910) // VS2015 and earlier
namespace std
{
    float max(const float &a, const float &b);
    float min(const float &a, const float &b);  
}
#endif

#pragma region Documentation
/// <summary>Find the object with the greater value.</summary>
/// <remarks>Joe Chui, 10/12/2011.</remarks>
/// <param name="a">One of the items to compare.</param>
/// <param name="b">The other item to compare.</param>
/// <return>The object with the greater value.</return>
#pragma endregion
template <class T> const T& max ( const T& a, const T& b ) 
{
  return (b<a)?a:b;     // or: return comp(b,a)?a:b; for the comp version
}

#pragma region Documentation
/// <summary>Find the object with the lesser value.</summary>
/// <remarks>Joe Chui, 10/12/2011.</remarks>
/// <param name="a">One of the items to compare.</param>
/// <param name="b">The other item to compare.</param>
/// <return>The object with the lesser value.</return>
#pragma endregion
template <class T> const T& min ( const T& a, const T& b ) 
{
  return (b>a)?a:b;     // or: return comp(b,a)?a:b; for the comp version
}


#if defined(MSC_VER) && (_MSC_VER < 1800) // if Visual Studio 2012 and earlier
#pragma region Documentation
/// <summary>Round a floating point number to an integer.</summary>
/// <remarks>Joe Chui, 10/12/2011.</remarks>
/// <param name="v">The floating point value to round.</param>
/// <return>The rounded value as an integer.</return>
#pragma endregion
int round(float v); // this conflicts with C++11
#endif

class rowVector; // fwd reference

#pragma region Documentation
/// <summary>Provide column-oriented vector operations. </summary>
/// <remarks>Joe Chui, 10/12/2011.</remarks>
#pragma endregion
class columnVector
{
private:
    float v[3];

public:

    #pragma region Documentation
    /// <summary>Print the data contents of this class.</summary>
    /// <remarks>D. Higginbotham 2020-04-30.</remarks>
    #pragma endregion
    void dump();

    #pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
    #pragma endregion
    float operator [](int index);



#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    float operator * (columnVector &a);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector operator * (float a);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector square();

    //columnVector operator ^ (columnVector &a);



#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector operator - (columnVector &a);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector operator + (columnVector &a);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector operator / (float s);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector operator / (columnVector &a);



#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    static columnVector cross(columnVector &a, columnVector &b);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector(float x, float y, float z);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector();


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    static float sum(columnVector &a);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    void set(float a, float b, float c);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    rowVector transpose();


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector abs();


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    float sum();


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    void set(char index, float value);


#pragma region Documentation
    /// <summary>Round a floating point number to an integer.</summary>
    /// <remarks>Joe Chui, 10/12/2011.</remarks>
    /// <param name="v">The floating point value to round.</param>
    /// <return>The rounded value as an integer.</return>
#pragma endregion
    columnVector log();
};


class matrix; // fwd reference

class rowVector
{
private:
    float v[3];

public:

    #pragma region Documentation
    /// <summary>Print the data contents of this class.</summary>
    /// <remarks>D. Higginbotham 2020-04-30.</remarks>
    #pragma endregion
    void dump();

    float operator [](int index);
    float operator * (rowVector &a);

    rowVector operator * (float a);
    rowVector operator * (matrix &m);
    rowVector square();
    rowVector operator - (rowVector &a);
    rowVector operator + (rowVector &a);
    rowVector operator / (float s);

    static rowVector cross(rowVector &a, rowVector &b);
    rowVector(float x, float y, float z);
    rowVector();

    static float sum(rowVector &a);
    void set(float a, float b, float c);
    columnVector transpose();
    rowVector abs();
    float sum();
};


class matrix 
{
private:
    float v[9];

public:

    #pragma region Documentation
    /// <summary>Print the data contents of this class.</summary>
    /// <remarks>D. Higginbotham 2020-04-30.</remarks>
    #pragma endregion
    void dump();

    matrix(rowVector &x, rowVector &y, rowVector &z);
    matrix(float x, float y, float z);
    matrix();

    columnVector operator [](int index);
    columnVector operator * (columnVector &v);

    matrix operator * (matrix &m);
    matrix transpose();
    matrix operator - (matrix &a);
    rowVector row(int row);
    matrix inverse();
    matrix chol();
    columnVector diag();
};


class columnVector8
{
private:
    float v[8];

public:

    #pragma region Documentation
    /// <summary>Print the data contents of this class.</summary>
    /// <remarks>D. Higginbotham 2020-04-30.</remarks>
    #pragma endregion
    void dump();

    columnVector8();
    columnVector8 operator * (float a);
    columnVector8 operator + (float a);
    columnVector8 operator + (columnVector8 &a);

    float min();
    float max();

    void set(int index, float v);
    float operator [](int index);
};


class matrix8x3 
{
private:
    rowVector v[8];

public:

    #pragma region Documentation
    /// <summary>Print the data contents of this class.</summary>
    /// <remarks>D. Higginbotham 2020-04-30.</remarks>
    #pragma endregion
    void dump();

    matrix8x3();
    // NOT IMPLMENTED matrix8x3(columnVector &x, columnVector &y, columnVector &z);

    rowVector operator [](int index);
    columnVector8 operator *(columnVector &a);

    void set(int index, rowVector &r);
};


matrix8x3     operator * (float a, matrix8x3 &m);
columnVector8 operator * (float a, columnVector8 &b); 
rowVector     operator * (float a, rowVector &b); 
float         operator *(rowVector &r, columnVector &c);
