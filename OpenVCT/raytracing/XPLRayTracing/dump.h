// dump.h

#pragma once

#include <fstream>
#include <iostream>
#include <vector>


void dumpWeights(std::vector<float> &weights)
{
    std::ofstream ofs("DEBUG_OCL_Weights.txt");
    ofs << "Material Weights, in Order, Including Material Repeats\n";
    for (auto wt : weights)
    {
        ofs << wt << "\n";
    }
    ofs.close();
}


void dumpDensities(std::vector<float> &densities)
{
    std::ofstream ofs("DEBUG_OCL_Densities.txt");
    ofs << "Material Densities, in Order, Including Material Repeats\n";
    for (auto den : densities)
    {
        ofs << den << "\n";
    }
    ofs.close();
}


void dumpAttenuations(std::vector<float> &attenuations, int kVp_x2_m1, std::vector<int> &uniqueMats)
{
    std::ofstream ofs("DEBUG_OCL_Attenuations.xml");

    ofs << "<!-- number of unique mats: " << uniqueMats.size() << ", kVp_x2_m1: " << kVp_x2_m1 << "-->\n\n";

    for (int m = 0; m < uniqueMats.size(); ++m)
    {
        ofs << "  <ns1:MaterialAttenuation>\n";
        ofs << "    <ns1:Name>NIST_" << uniqueMats[m] << "_X.txt</ns1:Name>\n";
        ofs << "    <ns1:Z>" << uniqueMats[m] << "</ns1:Z>\n";
        ofs << "    <ns1:Density>X.XX</ns1:Density>\n";

        for (int k = 0; k < kVp_x2_m1; ++k)
        {
            ofs << "    <ns1:Attentuation" << k << ">" << attenuations[(m * kVp_x2_m1) + k] << "</ns1:Attentuation" << k << ">\n";
        }

        ofs << "  </ns1:MaterialAttenuation>\n";
    }

    ofs.close();
}



void dumplabMatOffs(std::vector<int> &labMatOffs) // 2018-11-19 DH * // 12 offset of first of materials for this material label    2018-11-12 DH *
{
    std::ofstream ofs("DEBUG_OCL_labMatOffs.txt");
    ofs << "Label/Material Offsets\n";
    for (auto off : labMatOffs)
    {
        ofs << off << "\n";
    }
    ofs.close();
}


void dumpnumLabMats(std::vector<int> &numLabMats) // 2018-11-19 DH * // 13 number of materials represented by this material label  2018-11-12 DH *
{
    std::ofstream ofs("DEBUG_OCL_numLabMats.txt");
    ofs << "Numbers of Materials Within Each Label\n";
    for (auto mat : numLabMats)
    {
        ofs << mat << "\n";
    }
    ofs.close();
}


void dumpmatZValue(std::vector<int> &matZValues) // 2018-11-19 DH * // 14 ordinal list of material Z values used, including repeats 2018-11-16 DH *
{
    std::ofstream ofs("DEBUG_OCL_matZValues.txt");
    ofs << "All MaterialZ Values, in Order, Including Repeats\n";
    for (auto mat : matZValues)
    {
        ofs << mat << "\n";
    }
    ofs.close();
}


void dumpuniqueMatZs(std::vector<int> &uniqueMatZs) // 2018-11-19 DH * // 15 list of unique material Z values used, excluding repeats  2018-11-16 DH *
{
    std::ofstream ofs("DEBUG_OCL_uniqueMatZs.txt");
    ofs << "Unique MaterialZ Values, in Order, Excluding Repeats\n";
    for (auto mat : uniqueMatZs)
    {
        ofs << mat << "\n";
    }
    ofs.close();
}


void dumpSpectrum(unsigned short kVp, unsigned short spectrumID, std::vector<float> &spectrum) // 2020-07-30 DH * 
{
    std::ofstream ofs("DEBUG_OCL_Spectrum.txt");
    ofs << "2-dimensional Spectrum array\n";
    ofs << "kVp is " << kVp << ", spectrumID (aka Material mat) is " << spectrumID << "\n";
    ofs << "For insight see these functions:\n\tBooneMaterialTable::setupTable\n\tBooneCoefficientTable::getSpectrum\n\n";
    int i=0;
    while(i<spectrum.size())
    {
        ofs << std::fixed << std::setprecision(1) << std::setw(5)  << spectrum[i]   << "\t" 
            << std::fixed << std::setprecision(1) << std::setw(11) << spectrum[i+1] << "\n";
        i = i + 2;
    }
    ofs.flush();
    ofs.close();
}


void dumpFilter(std::vector<float> &filter) // 2020-07-31 DH *
{
    std::ofstream ofs("DEBUG_OCL_Filter.txt");
    ofs << "Filter vector:\n\n";
    for (auto f : filter)
    {
        ofs << "\t" << f << "\n";
    }
    ofs.close();
}


void dumpPhysics(int kVp_x2_m1,                        // 0
                 const float *tbl,                     // 1 LengthBuffer. If nullptr, don't process length
                 std::vector<float> &densities,        // 2
                 std::vector<float> &weights,          // 3
                 std::vector<float> &attenuationTable, // 4
                 int numMatLbls,                       // 5
                 std::vector<float> &spectrum,         // 6
                 const int spRowCount,                 // 7 no longer used in kernl, replaced by 'count'
                 const std::vector<float> &filter,     // 8 
                 const float multiplier,               // 9
                 std::vector<float> &signalBuffer,     // 10
                 const int count,                      // 11 detector positions size
                 const std::vector<int> &labMatOffs,   // 12
                 const std::vector<int> &numLabMats,   // 13
                 const int row,                        // value is usually produced via get_global_id
                 const std::vector<int> &matZ)         // also not actually in physics kernel argument list
{
    std::ofstream ofs("DEBUG_OCL_Physics.txt");
    ofs << "Physics Kernel Processing\n\nnumMatLbls: " << numMatLbls << ", kVp_x2_m1: " << kVp_x2_m1 << "\n";
    ofs << "spRowCount: " << spRowCount << ", count: " << count << "\n";
    ofs << "       row: " << row << ", multiplier: " << std::fixed << multiplier << "\n";
    //size_t row = get_global_id(0);
    //if (row > count)
    //{
    //    return; // typically occurs (not an error)
    //}

    double averageValue = 0.0;
    double length = 75.0; // note: in the real code length will often be 0.0 (whenever the ray does not encounter the current material k)
    double density = 0.0;
    double weight = 1.0;
    
    int index = 0;
    int smi = 0;
    
    try
    { 
        // Doing only one ray - out of 4 corners - for this detector element
        for(int m=0; m<4; ++m)
        {
            // Declare and Initialize SumUT
            double sumUT[256];  // 100 is kVp*2 for mammography spectra w/0.5 kV steps
            for(int ndx=0; ndx<256; ndx++)  sumUT[ndx] = 0.0f;  // initialize the array

            // Calculate the sum over the materials in the x-ray path of
            // mass_attn * density * thickness for each energy bin
            // FIX - should also loop over the set of materials for composite materials
            if (tbl == nullptr) m = 2; // calc for m=2 only if no lengthTable
            ofs << "\nFor debug m=" << m << ":\n";

            smi = 0; // sequential material index

            for(int k=0; k<numMatLbls; k++) // Cycle thru all labels
            {
                // If tbl not provided, limit functionality
                if (tbl != nullptr)
                {
                    length = tbl[ (((row * numMatLbls) + k) * 4) + m ]; // length table corresponds to labels, not materials
                }

                ofs << "\n\tLabel (k=" << k << ") of numMatLbls=" << numMatLbls << ", has (numLabMats[" << k << "]=" 
                    << numLabMats.at(k) << ") materials\n";

                for(int j=0; j<numLabMats.at(k); ++j)  // Cycle thru all materials represented by this label
                {
                    if (length > 0.0)  // Don't process if the current label does not occur in this ray
                    {
                        ofs << "\n\t\tLabel\'s material (mat j=" << j << ", Z=" << matZ.at(smi)
                            << ") is at offset (labMatOffs[smi=" << smi << "]): " << labMatOffs.at(smi) << "\n\n";
                        ofs << "\t\t\tlength table index ((((row * numMatLbls) + k) * 4) + m) is " <<  (((row * numMatLbls) + k) * 4) + m << "\n\n";

                        density = densities.at(smi); // use the sequential material index not the offset
                        weight = weights.at(smi);    // use the sequential material index not the offset

                        ofs << "\t\t\tdensities[smi=" << smi << "] is " << density << "\n";
                        ofs << "\t\t\tweights[smi=" << smi << "]   is " << weight << "\n";

                        //  Loop over each energy bin (actually lets just dump the first 4 values)
                        int offset = labMatOffs.at(smi);   // Find index into attenuation table
                        double ldw = length * density * weight; // not used in this debug-only function

                        ofs << "\t\t\tdebug length = " << length << ", so ldw (length*density*weight) = " << ldw << "\n";                     
                        ofs << "\n\t\t\tFirst 4 attenuationTable values of matZ=" << matZ.at(smi) << ":\n\n";

                        for(int ndx=0; ndx<kVp_x2_m1; ndx++)
                        {
                            //  MISTAKE ---  FIX THIS
                            //  Spectra in half kV steps
                            //  Attn coefs in kV steps

                            sumUT[ndx] = sumUT[ndx] + attenuationTable[(offset * kVp_x2_m1) + ndx] * ldw; // not used in this debug-only function
                            if (tbl == nullptr && ndx > 3) break; // just print the first 4 attenuation table values
                            ofs << "\t\t\t\tattenuationTable[((offset=" << offset << ") * kVp_x2_m1) + ndx=" << ndx << "] = " 
                                << attenuationTable.at(index) << "\n";

                        } // next energy bin

                    } // if ray is intersecting a voxel with the current label value

                    smi = smi + 1; // increment sequential material index regardless of length

                } // next material represented by this label // 2018-11-13 DH *

            } // next k (labels found in the length table)

            std::ofstream s_ofs("DEBUG_OCL_SumIUT.txt");
            // loop over energy bins calculating
            // Io * area^2 * attn_of_filter * E * exp(-mu t)
            double sumIUT=0.0f;
            for (int spec=0; spec<kVp_x2_m1; spec++)
            {
                double temp1 = spectrum[spec*2+1];
                double temp2 = exp(-1.0f * sumUT[spec]);

                s_ofs << "\nspec=" << spec << ", spectrum[spec*2+1=" << spec*2+1 << "] = " << std::fixed
                      << spectrum[spec*2+1] << "\n";
                s_ofs << std::fixed << "temp1 = " << temp1 << ", sumUT[spec=" << spec << "] = " << sumUT[spec]
                      << ", temp2 = " << temp2 << "\n";
                s_ofs << "filter[spec=" << spec << "] = " << std::scientific << filter[spec] << std::fixed
                      << ", spectrum[spec*2=" << spec*2 << "] = " << spectrum[spec*2] 
                      << ", multiplier = " << multiplier << "\n";
                s_ofs << "(temp1 * filter[spec] * temp2 * spectrum[spec*2] * multiplier) = " << std::scientific
                      << (temp1 * filter[spec] * temp2 * spectrum[spec*2] * multiplier) << "\n";
                s_ofs << "sumIUT before adding = " << sumIUT << "\n";

                sumIUT = sumIUT + (temp1 * filter[spec] * temp2 * spectrum[spec*2] * multiplier);

                s_ofs << "sumIUT after adding = " << sumIUT << "\n";
            }

            s_ofs << "\n\nsumIUT=" << sumIUT << "\n";
            ofs << "\n\n\tsumIUT=" << sumIUT << "\n";
            // average over the 4 rays per pixel used in the Siddon method
            averageValue = averageValue + 0.25 * sumIUT;
            s_ofs << "averageValue (averageValue = averageValue + 0.25*sumIUT)=" << averageValue << "\n\n";
            s_ofs.close();

            ofs << "\taverageValue (averageValue = averageValue + 0.25*sumIUT)=" << averageValue << "\n";
            /// not used in this debug-only function
            // Clear out this element from the lengthTable
            ///for (int kk=0; kk<numMatLbls; kk++)
            ///{
            ///tbl[ (((row * numMatLbls) + kk) * 4) + m ] = 0.0f;
            ///}
            if (tbl == nullptr) break; // calculate for one m only if no lengthTable
        } // next m
        
        //signalBuffer[row] = averageValue;//sumIUT; // critical
    }
    catch(std::out_of_range const& exc) 
    {
        std::cerr << exc.what() << std::endl;
    }
    
    ofs.close();
        
}


void dumpPhysics_double(int kVp_x2_m1,                   // 0
                 const float *tbl,                       // 1 LengthBuffer. If nullptr, don't process length
                 std::vector<float> &densities,          // 2
                 std::vector<float> &weights,            // 3
                 std::vector<float> &attenuationTable,   // 4
                 int numMatLbls,                         // 5
                 std::vector<float> &spectrum,           // 6
                 const int spRowCount,                   // 7 no longer used in kernl, replaced by 'count'
                 const std::vector<float> &filter,       // 8 
                 const float multiplier,                 // 9
                 std::vector<float> &signalBuffer,       // 10
                 const int count,                        // 11 detector positions size
                 const std::vector<int> &labMatOffs,     // 12
                 const std::vector<int> &numLabMats,     // 13
                 const int row,                          // value is usually produced via get_global_id
                 const std::vector<int> &matZ)           // also not actually in physics kernel argument list
{
    std::ofstream ofs("DEBUG_OCL_Physics_double.txt");
    ofs << "Physics Kernel Processing\n\nnumMatLbls: " << numMatLbls << ", kVp_x2_m1: " << kVp_x2_m1 << "\n";
    ofs << "spRowCount: " << spRowCount << ", count: " << count << "\n";
    ofs << "       row: " << row << ", multiplier: " << std::fixed << multiplier << "\n";
    //size_t row = get_global_id(0);
    //if (row > count)
    //{
    //    return; // typically occurs (not an error)
    //}

    double averageValue = 0.0;
    double length       = 75.0; // note: in the real code length will often be 0.0 (whenever the ray does not encounter the current material k)
    double density      = 0.0;
    double weight       = 1.0;

    // Doing only one ray - out of 4 corners - for this detector element
    for(int m = 0; m < 4; ++m)
    {
        // Declare and Initialize SumUT
        double sumUT[256];  // 100 is kVp*2 for mammography spectra w/0.5 kV steps
        for(int ndx=0; ndx<256; ndx++)  sumUT[ndx] = 0.0;  // initialize the array

        // Calculate the sum over the materials in the x-ray path of
        // mass_attn * density * thickness for each energy bin
        // FIX - should also loop over the set of materials for composite materials
        if (tbl == nullptr) m = 2; // calc for m=2 if no lengthTable
        ofs << "\nFor debug m=" << m << ":\n";

        int smi = 0; // sequential material index

        for(int k=0; k<numMatLbls; k++) // Cycle thru all labels
        {
            /// If tbl not provided, limit functionality
            if (tbl != nullptr)
            {
                length = (double)tbl[ (((row * numMatLbls) + k) * 4) + m ]; // length table corresponds to labels, not materials
            }
            
			ofs << "\n\tLabel (k=" << k << ") of numMatLbls=" << numMatLbls << ", has (numLabMats[" << k << "]=" 
                << numLabMats[k] << ") materials\n";

            for(int j=0; j<numLabMats[k]; ++j)  // Cycle thru all materials represented by this label
            {
                if (length > 0.0)  // Don't process if the current label does not occur in this ray
                {
                    ofs << "\n\t\tLabel\'s material (mat j=" << j << ", Z=" << matZ[smi] 
                        << ") is at offset (labMatOffs[smi=" << smi << "]): " << labMatOffs[smi] << "\n\n";
                    ofs << "\t\t\tlength table index ((((row * numMatLbls) + k) * 4) + m) is " <<  (((row * numMatLbls) + k) * 4) + m << "\n\n";

                    density = (double)densities[smi]; // use the sequential material index not the offset
                    weight  = (double)weights[smi];    // use the sequential material index not the offset

                    ofs << "\t\t\tdensities[smi=" << smi << "] is " << density << "\n";
                    ofs << "\t\t\tweights[smi=" << smi << "]   is " << weight << "\n";

                    //  Loop over each energy bin (actually lets just dump the first 4 values)
                    int offset = labMatOffs[smi];   // Find index into attenuation table
                    double ldw = length * density * weight; // not used in this debug-only function
                    
                    ofs << "\t\t\tdebug length = " << length << ", so ldw (length*density*weight) = " << ldw << "\n";                     
                    ofs << "\n\t\t\tFirst 4 attenuationTable values of matZ=" << matZ[smi] << ":\n\n";
                        
                    for(int ndx=0; ndx<kVp_x2_m1; ndx++)
                    {
                        //  MISTAKE ---  FIX THIS
                        //  Spectra in half kV steps
                        //  Attn coefs in kV steps

                        sumUT[ndx] = sumUT[ndx] + (double)attenuationTable[(offset * kVp_x2_m1) + ndx] * ldw; // not used in this debug-only function
                        if (tbl == nullptr && ndx > 3) break; // just print the first 4 attenuation table values
                        ofs << "\t\t\t\tattenuationTable[((offset=" << offset << ") * kVp_x2_m1) + ndx=" << ndx << "] = " 
                            << attenuationTable[(offset * kVp_x2_m1) + ndx] << "\n";

                    } // next energy bin

                } // if ray is intersecting a voxel with the current label value

                smi = smi + 1; // increment sequential material index regardless of length

            } // next material represented by this label // 2018-11-13 DH *

        } // next k (labels found in the length table)

        std::ofstream s_ofs("DEBUG_OCL_SumIUT_double.txt");
        // loop over energy bins calculating
        // Io * area^2 * attn_of_filter * E * exp(-mu t)
        double sumIUT=0.0;
        for (int spec=0; spec<kVp_x2_m1; spec++)
        {
            double temp1 = spectrum[spec*2+1];
            double temp2 = exp(-1.0 * sumUT[spec]);
            
            s_ofs << "\nspec=" << spec << ", spectrum[spec*2+1=" << spec*2+1 << "] = " << std::fixed
                  << spectrum[spec*2+1] << "\n";
            s_ofs << std::fixed << "temp1 = " << temp1 << ", sumUT[spec=" << spec << "] = " << sumUT[spec]
                  << ", temp2 = " << temp2 << "\n";
            s_ofs << "filter[spec=" << spec << "] = " << std::scientific << filter[spec] << std::fixed
                  << ", spectrum[spec*2=" << spec*2 << "] = " << spectrum[spec*2] 
                  << ", multiplier = " << multiplier << "\n";
            s_ofs << "(temp1 * filter[spec] * temp2 * spectrum[spec*2] * multiplier) = " << std::scientific
                  << (temp1 * filter[spec] * temp2 * spectrum[spec*2] * multiplier) << "\n";
            s_ofs << "sumIUT before adding = " << sumIUT << "\n";
            
            sumIUT = sumIUT + (temp1 * (double)filter[spec] * temp2 * (double)spectrum[spec*2] * (double)multiplier);
            
            s_ofs << "sumIUT after adding = " << sumIUT << "\n";
        }
        
        s_ofs << "\n\nsumIUT=" << sumIUT << "\n";
        ofs << "\n\n\tsumIUT=" << sumIUT << "\n";
        // average over the 4 rays per pixel used in the Siddon method
        averageValue = averageValue + 0.25 * sumIUT;
        s_ofs << "averageValue (averageValue = averageValue + 0.25*sumIUT)=" << averageValue << "\n\n";
        s_ofs.close();
        
        ofs << "\taverageValue (averageValue = averageValue + 0.25*sumIUT)=" << averageValue << "\n";
        /// not used in this debug-only function
        // Clear out this element from the lengthTable
        ///for (int kk=0; kk<numMatLbls; kk++)
        ///{
        ///tbl[ (((row * numMatLbls) + kk) * 4) + m ] = 0.0f;
        ///}
        if (tbl == nullptr) break; // calculate for one m only if no lengthTable
    } // next m

    //signalBuffer[row] = averageValue;//sumIUT; // critical
    
    ofs.close();
}
