#include "BooneMaterialTable.h"
#include <iostream>

std::vector<BooneCoefficientTable*> BooneMaterialTable::coeffTables(Material::NONE);

/// <summary>
/// Initializes a new instance of the <see cref="BooneMaterialTable"/> class.
/// </summary>
BooneMaterialTable::BooneMaterialTable(void)
{
}


/// <summary>
/// Finalizes an instance of the <see cref="BooneMaterialTable"/> class.
/// </summary>
BooneMaterialTable::~BooneMaterialTable(void)
{
}


/// <summary>
/// Setups the table.
/// </summary>
/// <param name="mat">The mat.</param>
void BooneMaterialTable::setupTable(Material mat)
{

	switch (mat)
	{
		case Mo_spectrum_extrapolated:
			coeffTables[mat] = new BooneCoefficientTable(spectrum_Mo_extrapolatedTable);
			break;
		case Rh_spectrum_extrapolated2:
			coeffTables[mat] = new BooneCoefficientTable(spectrum_Rh_extrapolatedTable);
			break;
		case W_new_pol_withTAILS:
			coeffTables[mat] = new BooneCoefficientTable(spectrum_W_newPol_withTailsTable);
			break;
		case W_spectrum_TASMIP:
			coeffTables[mat] = new BooneCoefficientTable(spectrum_W_TASMIP_Table);
			break;
        //DH//case MonoEnergetic:
        //DH//    coeffTables[mat] = new BooneCoefficientTable(Mono_energeticTable);
		default:
			break;
	};

}


#if 0 //DH//
/// <summary>
/// Creates a mono-energetic table corresponding to the specified kVp.
/// </summary>
/// <param name="kVp">The (XML-)specified kVp.</param>
/// <returns></returns>
void BooneMaterialTable::createMonoEnergeticTable(float kVp)
{
    // populate with zeroes, except for entry corresponding to specified kVp
    int ndx = 0;
    for(float f=1.0f; f<149.0f; f+=0.5f)
    {
        Mono_energeticTable[ndx][0] = f;
        Mono_energeticTable[ndx][1] = ((f != kVp) ? 0.0f : 10.0e6);
        Mono_energeticTable[ndx][2] = 0.0;
        Mono_energeticTable[ndx][3] = 0.0;
        Mono_energeticTable[ndx][4] = 0.0;
        ndx++;
    }  
}
#endif //DH//

/// <summary>
/// Gets the material table.
/// </summary>
/// <param name="mat">The mat.</param>
/// <returns></returns>
BooneCoefficientTable& BooneMaterialTable::getMaterialTable(Material mat)//DH//, unsigned short kvP)
{
	if (!coeffTables[mat])
    {
        //DH//createMonoEnergeticTable(float(kvP));
		setupTable(mat);
    }
	return *(coeffTables[mat]);
}

