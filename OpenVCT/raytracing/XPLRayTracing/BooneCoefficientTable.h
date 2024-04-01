// BooneCoefficientTable.h

#pragma once

#include "BooneCoefficients.h"
#include <vector>
#include "vector3.h"

class BooneCoefficientTable
{
public:

	unsigned int getCoefficientCount();
	const BooneCoefficients& getCoefficients(int index);
	void getSpectrum(int kVp, std::vector<rowVector>& Spectrum);

	BooneCoefficientTable(void);
	BooneCoefficientTable(const double list[][5]);
	~BooneCoefficientTable(void);

private:
    
	std::vector<BooneCoefficients> table;
};

