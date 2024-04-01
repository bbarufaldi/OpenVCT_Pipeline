// BooneCoefficientTable.cpp

#include "BooneCoefficientTable.h"
#include <vector>
#include <cmath>
#include <fstream>


/// <summary>
/// Gets the coefficient count.
/// </summary>
/// <returns></returns>
unsigned int BooneCoefficientTable::getCoefficientCount()
{
	return static_cast<unsigned int>(table.size());
}

/// <summary>
/// Gets the coefficients.
/// </summary>
/// <param name="index">The index.</param>
/// <returns></returns>
const BooneCoefficients& BooneCoefficientTable::getCoefficients(int index)
{
	return table[index];
}

/// <summary>
/// Gets the spectrum.
/// </summary>
/// <param name="KVP">The KVP.</param>
/// <param name="Spectrum">The spectrum.</param>
void BooneCoefficientTable::getSpectrum(int KVP, std::vector<rowVector>& Spectrum)
{
	Spectrum.resize(KVP*2-1);
	
	for (int i=0;  i< KVP*2-1; i++)
	{
	    Spectrum[i].v[0] = getCoefficients(i).id;
	    Spectrum[i].v[1] = abs(getCoefficients(i).a0 +
                               KVP * getCoefficients(i).a1 +
                               KVP * KVP * getCoefficients(i).a2 +
                               KVP * KVP * KVP * getCoefficients(i).a3);
	}

	return;
}
	// S_unat(ii) = abs(a0(ii+1) + a1(ii+1) * kvp + a2(ii+1) * kvp^2 + a3(ii+1) * kvp^3);


/// <summary>
/// Initializes a new instance of the <see cref="BooneCoefficientTable"/> class.
/// </summary>
/// <param name="list">The list.</param>
BooneCoefficientTable::BooneCoefficientTable(const double list[][5])
{
	int currentIndex=0;
	while (list[currentIndex][0]>0.0)
	{
		this->table.push_back(BooneCoefficients(list[currentIndex][0], list[currentIndex][1], list[currentIndex][2], list[currentIndex][3],list[currentIndex][4]));
		currentIndex++;
	}
}


/// <summary>
/// Initializes a new instance of the <see cref="BooneCoefficientTable"/> class.
/// </summary>
BooneCoefficientTable::BooneCoefficientTable(void)
{
}


/// <summary>
/// Finalizes an instance of the <see cref="BooneCoefficientTable"/> class.
/// </summary>
BooneCoefficientTable::~BooneCoefficientTable(void)
{
}
