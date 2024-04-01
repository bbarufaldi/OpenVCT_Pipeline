// vector3.h (implementation is in vector.cpp)

#pragma once

class rowVector; // forward declaration

class columnVector
{
private:

public:
	double v[3];
	double operator [](int index);
	double operator * (const columnVector &a);
	columnVector operator * (double a);
	columnVector square();
//	columnVector operator ^ (const columnVector &a);
	columnVector operator - (const columnVector &a);
	columnVector operator + (const columnVector &a);
	columnVector operator / (double s);
	columnVector operator / (const columnVector &a);
	static columnVector cross(const columnVector &a, const columnVector &b);
	columnVector(double x, double y, double z);
	columnVector();
	static double sum(const columnVector &a);
	void set(double a, double b, double c);
	rowVector transpose();
	columnVector abs();
	double sum();
	void set(char index, double value);
	columnVector log();
	
};

class matrix; // forward declaration

class rowVector
{
private:
	
public:
	double v[3];

	double operator [](int index);
	rowVector operator % (const rowVector &a);
	double operator * (const rowVector &a);

	rowVector operator * (double a);
	rowVector operator * ( matrix m);
	rowVector square();
	rowVector operator - (const rowVector &a);
	rowVector operator + (const rowVector &a);
	rowVector operator / (double s);
	rowVector operator / (const rowVector &a);

	static rowVector cross(const rowVector &a, const rowVector &b);
	rowVector(double x, double y, double z);
	rowVector();

	static double sum(const rowVector &a);
	double length();

	void set(double a, double b, double c);
	columnVector transpose();
	rowVector abs();
	double sum();
	double minvalue();

	void step(const rowVector& a, const rowVector& b);
};


class matrix 
{
private:
	
public:
	double v[12];

	matrix(const rowVector &x, const rowVector &y, const rowVector &z, const rowVector &w);
	matrix(double x, double y, double z);
	matrix();

	columnVector operator [](int index);
	columnVector operator * (const columnVector &v);

	matrix operator * (const matrix &m);
	matrix transpose();
	matrix operator - (const matrix &a);

	rowVector row(int row);
	matrix inverse();
	matrix chol();
	columnVector diag();
};


class columnVector8
{
private:
	double v[8];

public:
    columnVector8();

	columnVector8 operator * (double a);
	columnVector8 operator + (double a);
	columnVector8 operator + (const columnVector8 &a);

	double min2();
	double max2();
	void set(int index, double v);
	double operator [] (int index);
};

class matrix8x3 
{

public:
	rowVector v[8];

	matrix8x3();
	matrix8x3(const columnVector &x, const columnVector &y, const columnVector &z);

	const rowVector& operator [](int index);
	columnVector8 operator *(const columnVector &a);
	void set(int index, const rowVector &r);
};

matrix8x3 operator * (double a, const matrix8x3 &m);
columnVector8 operator * (double a, const columnVector8 &b); 
rowVector operator * (double a, const rowVector &b); 
double operator *(const rowVector &r, const columnVector &c);
