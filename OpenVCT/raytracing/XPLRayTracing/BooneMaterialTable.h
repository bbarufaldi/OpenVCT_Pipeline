// BooneMaterialTable.h

#pragma once

#include "BooneCoefficientTable.h"

class BooneMaterialTable
{
public:

    enum Material
	{
        // enumeration values correspond to SpectrumID in configuration XML file
		Mo_spectrum_extrapolated=0,
		Rh_spectrum_extrapolated2 = 1, 
		W_new_pol_withTAILS       = 2, 
		W_spectrum_TASMIP         = 3,
        NONE                      = 4
        //DH//MonoEnergetic             = 4,
		//DH//NONE                      = 5
	};
    
	BooneMaterialTable();
	BooneCoefficientTable& getMaterialTable(Material mat); //DH//, unsigned short kVp = 32); // if kVp not supplied, this default value will be used
	~BooneMaterialTable();

private:
	
	static const double spectrum_Mo_extrapolatedTable[][5];
	static const double spectrum_Rh_extrapolatedTable[][5];
	static const double spectrum_W_TASMIP_Table[][5];
	static const double spectrum_W_newPol_withTailsTable[][5];

    //DH//double Mono_energeticTable[140][5];

	static std::vector<BooneCoefficientTable*> coeffTables;

    //DH//void createMonoEnergeticTable(float kVp);
	void setupTable(Material mat);
};
