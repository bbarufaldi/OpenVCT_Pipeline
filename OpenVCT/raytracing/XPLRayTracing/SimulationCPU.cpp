// SimulationCPU.cpp

#if defined(NO_OPENCL)


#include "SimulationCPU.h"
#include "Image.h"
#include "Ray.h"
#include "RayTracer.h"
#include "BooneMaterialTable.h"
#include <random>
#include <thread>
#include <iostream> // debug

#include "ProgressBar.h"
extern ProgressBar progressBar;


/// <summary>
/// Initializes a new instance of the <see cref="Simulation"/> class.
/// </summary>
/// <parm name="seed">The random number seed.>/parm>
SimulationCPU::SimulationCPU(std::string seed)
    : Simulation(), mSeed(seed)
{}


/// <summary>
/// Initializes a new instance of the <see cref="Simulation"/> class.
/// </summary>
/// <param name="xmlPathName">Name of the XML path.</param>
/// <param name="attentuationPathName">Name of the attentuation path.</param>
/// <param name="filePath">The file path.</param>
/// <parm name="seed">The random number seed.>/parm>
SimulationCPU::SimulationCPU(std::string xmlPathName, std::string attentuationPathName, std::string filePath, std::string seed)
    : Simulation(xmlPathName, attentuationPathName, filePath), mSeed(seed)
{}


/// <summary>
/// Finalizes an instance of the <see cref="Simulation"/> class.
/// </summary>
SimulationCPU::~SimulationCPU()
{}


/// <summary>
/// Simulates the specified acq ID.
/// </summary>
/// <param name="acqID">The acq ID.</param>
/// <param name="outFilePath">The out file path.</param>

// DH: Observation: id is always zero (Simulation::simulate() specifies a literal 0 as when invoking this)
bool SimulationCPU::process(signed short acqID, int id, Image<double> *outImage)
{
    if (outImage == nullptr) 
    {
        std::cerr << "outImage is NULL!!" << std::endl;
        return false;
    }

    XRayTube *xraytube = machine->getXRayTube();
	unsigned short kVp = xraytube->getkVP();

	BooneMaterialTable matTable;
	BooneCoefficientTable coeffTable = matTable.getMaterialTable((BooneMaterialTable::Material) machine->getXRayTube()->getSpectrumID());//DH//, kVp);

	std::vector<double> filter(kVp*2-1);
	
	ThreeDDDARayTracer<unsigned char> tracer;
	tracer.get_volume_options(*volume);

	std::vector<double> intersectedLengthTable(256, 0.0);

	std::vector<rowVector> Sp;		
	coeffTable.getSpectrum(kVp, Sp); 

    unsigned short filterMaterialZ = xraytube->getFilterMaterialZ();
    double filterDensity = attenuationTable.getDensity(filterMaterialZ);
    double filterThickness = xraytube->getFilterThickness();

    // FSD is min focal spot distance from detector.
	double FSD = machine->acquisitionGeometry[acqID].focalPosition[2];	
    double FSDsquared = FSD * FSD;
    double invSqAttn = 1.0;
    bool   calcInvSq = useInverseSq;

	std::seed_seq seed (mSeed.begin(), mSeed.end());
	std::mt19937 gen(seed);                          // mersenne_twister
	std::poisson_distribution<> d2(elecNoise);


    // Build Filter table
	for (int spec=0; spec<kVp*2-1; spec++)
	{
		filter[spec] = exp(-1.0*attenuationTable.getAttenuation(filterMaterialZ,spec)*filterThickness*filterDensity);
	}    

	double start, end;
    Detector *detector = machine->getDetector(); 

    // Added 0.5 coef for elementsizeY per PDB 2016-05-31 - DH
	double multiplier = 0.5 * detector->elementsizeX * 0.5 * detector->elementsizeY; 

	double averageValue = 0.0;
	double x, y;
	rowVector ElementLocation;
    double rayX, rayY, rayZ;

	double len;
	unsigned int elementCountX = detector->elementCountX;
	unsigned int elementCountY = detector->elementCountY;

    int count = 0;

	// Iterate over detector elements
	for (unsigned int i = 0; i < elementCountX; ++i) 
	{
		for (unsigned int j = 0; j < elementCountY; ++j)
		{
			averageValue = 0.0;

            for (unsigned int m = 0; m < 4; ++m) // Four rays per square pixel (element)
            {
                x = i + (m / 2); // sequence: 0 0 1 1
                y = j + (m % 2); // sequence: 0 1 0 1

                ElementLocation = machine->acquisitionGeometry[acqID].getDetectorElementLocation(*detector, x, y);

                Ray ray(machine->acquisitionGeometry[acqID].focalPosition, ElementLocation);

                // Calculate the inverse square value (intensity attenuation based on distance)
                if (m == 0 && useInverseSq) // don't recalculate inverse square law value more than once per element
                {
                    rayX = ElementLocation[0] - machine->acquisitionGeometry[acqID].focalPosition[0];
                    rayY = ElementLocation[1] - machine->acquisitionGeometry[acqID].focalPosition[1];
                    rayZ = ElementLocation[2] - machine->acquisitionGeometry[acqID].focalPosition[2];

                    invSqAttn = FSDsquared / ((rayX * rayX) + (rayY * rayY) + (rayZ * rayZ));
                }

                len = ray.getLength();

                // Adjust entire ray for volume offset prior to ray-tracing
                ray.startPoint.v[0] += volume_offsetX;
                ray.startPoint.v[1] += volume_offsetY;
                ray.startPoint.v[2] += volume_offsetZ;

                ray.endPoint.v[0] += volume_offsetX;
                ray.endPoint.v[1] += volume_offsetY;
                ray.endPoint.v[2] += volume_offsetZ;

                bool okaytoPrint = false;

                Volume<unsigned char> &vol = *volume;
                tracer.trace(ray, vol,
                    [](std::vector<double> &lengthTable, unsigned char &voxel, double length) { lengthTable[voxel] = lengthTable[voxel] + length; },
                    intersectedLengthTable,
                    start, end, okaytoPrint);

                intersectedLengthTable[0] = intersectedLengthTable[0] + start + end;

                if (okaytoPrint)
                {
                    std::cout << "Intersected LengthTable: \nStart=" << start << ", End=" << end << ", ray length: " << len << "\n";
                    for (int iii = 0; iii < 6; ++iii) std::cout << intersectedLengthTable[iii] << '\n';
                    okaytoPrint = false;
                }

                std::vector<double> sumUT(kVp * 2 - 1, 0); // size, init'l value

                // Capacity: 256 materials in the intersectedLengthTable
				for (int k=0; k<attenuationTable.getMaterialCount(); ++k) 
				{	  
                    double length = intersectedLengthTable[k]; // get the length of material k

					if ( length != 0.0)  // Don't process if there is none of this material
					{
						std::map<unsigned short, LabelMap>::iterator key = volume->labelMap.find(k);
						if (key != volume->labelMap.end())
						{
							auto &map = key->second.getMaterialMap();
			
							for (size_t i = 0; i < map.size(); ++i)
							{
								Material mat = map[i];
								int realZ = mat.materialZ;

								double density = attenuationTable.getDensity(realZ);
								if (mat.density >= 0.0)
								{
									density = mat.density;
								}
								double weight = mat.weight;


								std::vector<double> *table = attenuationTable.getAttenuationTable(realZ);
								if (table)
								{
									for (int l=0; l<kVp*2-1; ++l)
									{					
										sumUT[l] = sumUT[l] + table->at(l) * length * density * weight;
									}
								}
							}
						}
						else // material not one of the xml-specified label indicies
						{
                            static int count = 0;
                            if (count < 10)
                            {
                                count += 1;
							    std::cerr << "Label " << k << " not found in LabelMap. Check XML file used for config. " 
                                          << " element " << i << ", " << j << ", invSqAttn=" << invSqAttn << "\n";// DH 2014-10-06
                            }
						}
					}
				} // next attenuation table material
                


                // This code corresponds to physics kernel in OpenCL version
				double sumIUT=0.0;
				for (int spec=0; spec<kVp*2-1; ++spec)
				{
                    double temp1 = Sp[spec][1];
					double temp2 = exp(-1.0*sumUT[spec]);
                    sumIUT = sumIUT + temp1 * filter[spec] * temp2 * Sp[spec][0] * multiplier;
				}
			
				averageValue = averageValue + 0.25 * sumIUT; // average over the 4 rays used in the Siddon method
				intersectedLengthTable.assign(256, 0);       // zero out for the next detector element

			} // next m * * *

            double d1_init = averageValue * mAs * propConst;
            if (useScaling)                            
            {
                d1_init = averageValue * normAirKerma;
            }

			std::poisson_distribution<> d1(d1_init);  // mean is averageValue*normAirKerma
            
			double a1 = d1(gen) * gain;   // gain noise (poisson) applied to normAirKerma and averageValue
			double a2 = d2(gen);        // electric noise (poisson)

            std::ofstream ofs("CPU_signal_buffer.txt", std::ios::app);   // 2018-12-05 DH *

            if (useNoise)
			{
                outImage->setPixel(i, j, (a1 + a2) * invSqAttn);  // add noise to the image (base electric noise + k*gain)
			}
			else
			{
                // This isn't completely noiseless - DH 2015-03-09
                if (useScaling)
                {
                    outImage->setPixel(i, j, (averageValue * normAirKerma * gain) * invSqAttn);
                }
                else
                {
                    outImage->setPixel(i, j, (averageValue * mAs*gain * propConst) * invSqAttn);
                }
			}

			intersectedLengthTable.assign(256,0); // initialize table for next iteration

		} // next j

		if (i % 100 == 0)
        {
			progressBar.print(i*100 / machine->getDetector()->elementCountX);
        }

	} // next i (next element along X axis)
    
    return true;
}
#endif //#if defined(NO_OPENCL)
