// initialize_distribution_parameters.cpp

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cfloat>
#include "vector3.h"

#include "BreastPhantomGenerator.h"

//% 11/26/2010 Initializes distribution parameters and corresponding elements
//% of the structure using Cuiping ideas
//% NOTE: we DO CHECK for minimal separation between seeds!!!
//% Note: new_tree.DISTRIBUTIONS_Sigma are not used anymore
//% 12/11/2010>initialize_distribution_parameters no structures! NOTE: To be
//% executed on CPU, not on GPU, since DSU does not have license for float
//% precision numerical linear algebra. AFTER THAT, we need send GPU the
//% values of distributionMUVectors, etc.
//% 12/22/2010 Computed values needed for faster evaluation of the function
//% using 2nd order Fourier expansion, generates distributionUMatrices, distributionHMatrices
//% VERY IMPORTANT CHANGE!!!! R is now TRANSPOSED, but it is NOT renamed.
//% 12/22/2010 MUREP not initialized and not used
//% 1/13/2011 We check whether the seed corresponds to dense or to fat, and
//% assign label to each seed.
//% 1/17/2011 added HARD-CODED threshold (20%) for number of dense
//% compartments
//% 1/18/2011 sigma (coefficient for distribution of dense tissue) and
//% Percentage (percentage of compartments that are dense) are now inputs
//% 2/5/2011 Saves generated distribution parameters or, if the distribution
//% file name is specified, loads it!
//% 4/30/2011 In this version, after the distribution file is loaded, we
//% still randomly generate speed (while using ratios, etc. from the loaded
//% file). This way, we could simulate 2 phantoms with the same seeds and
//% orientations but different speeds.


        
float BreastPhantomGenerator::dblRand()
{
    //return ((float)rand()/(float)RAND_MAX); 
    float drand = ((float)rand()/(float)RAND_MAX);
    #if defined(STORE_RANDOM)
    vct_rand.addRandom(drand);
    #elif defined(READ_RANDOM)
    drand = vct_rand.nextRandom();
    #endif
    return drand;
}


void BreastPhantomGenerator::initialize_distribution_parameters(
                            int number_distributions, float a, float b,
                            float cprim, float csec, 
                            float minspeed,float maxspeed,
                            float minratio, float maxratio, 
                            float xlow0, float xhigh0, 
                            float ylow0, float yhigh0, 
                            float zhigh0, float zlow0,
                            float mindistseeds, float sigma, float Percentage, 
                            std::string DistributionLoadFile, 
                            std::string DistributionSaveFile, 
                            float alpha1, float beta1, float alpha2, float beta2, 
                            bool isGaussian)
{

    distributionA1InverseMatrices.clear(); 
    distributionA2InverseMatrices.clear();
    distributionA3InverseMatrices.clear();
    distributionHMatrices.clear();

    distributionSigmaInverseMatrices.clear();
    distributionSigmaInverseMUVectors.clear();
    distributionLabels.clear();
    distributionLogSqrtSigmaLogPriors.clear();
    distributionMUVectors.clear();

    distributionPriors.clear();
    distributionRMatrices.clear();
    distributionUMatrices.clear();  
    
    if (!DistributionLoadFile.empty()) 
    {   
        std::ifstream inputStream(DistributionLoadFile);
        if (inputStream.is_open())
        {
            while (!inputStream.eof())
            {
                float x, y, z;
                double speed, ratio1, ratio2;
                short label;
                std::string line;
                std::getline(inputStream, line);
                std::vector <double> Speeds;
                std::vector <double> Ratios1, Ratios2;

                if (std::stringstream(line) >> x >> y >> z >> label >> speed >> ratio1 >> ratio2)
                {
                    rowVector mucandidate(x, y, z);

                    distributionMUVectors.push_back(mucandidate);
                    distributionLabels.push_back(label);
                    Speeds.push_back(speed);
                    Ratios1.push_back(ratio1);
                    Ratios2.push_back(ratio2);
    
                    double kaprim = speed;
                    double kbprim = speed * ratio1;
                    double kcprim = speed * ratio2;

                    matrix Vtransposed = EllipsoidOrientation(rowVector(0, 0, 0), a, cprim, csec, b, mucandidate);
                    /*ORIG...
                    distributionSigmaInverseMatrices.push_back(Vtransposed.transpose() * matrix(1.0f / float(kaprim * kaprim),
                                                               1.0f / float(kbprim * kbprim), 
                                                               1.0f / float(kcprim * kcprim)) * Vtransposed);
                    ...ORIG */
                    float inv_kaprim_sqd = 1.0f / float(kaprim * kaprim);
                    float inv_kbprim_sqd = 1.0f / float(kbprim * kbprim);
                    float inv_kcprim_sqd = 1.0f / float(kcprim * kcprim);
                    
                    matrix m_invkprimsqd(inv_kaprim_sqd, inv_kbprim_sqd, inv_kcprim_sqd);
                    distributionSigmaInverseMatrices.push_back(Vtransposed.transpose() * m_invkprimsqd * Vtransposed);        
                    //DHnow matrix prim_matrix(1.0f / float(kaprim * kaprim), 1.0f / float(kbprim * kbprim),1.0f / float(kcprim * kcprim));
                    //DHnow distributionSigmaInverseMatrices.push_back(Vtransposed.transpose() * prim_matrix * Vtransposed);
                }
            }

            inputStream.close();
            mDistributionCount = (int)distributionMUVectors.size();

            for (int i=0; i<mDistributionCount; i++)
            {
                distributionPriors.push_back(1.0f / float(mDistributionCount));
                distributionRMatrices.push_back(distributionSigmaInverseMatrices[i].chol()); // %If we could find to represent cholesky using eigenvalues and eigenvectors, then this is not needed   
                distributionLogSqrtSigmaLogPriors.push_back(distributionRMatrices[i].transpose().diag().log().sum()+std::log(distributionPriors[i])); //%12/22/2010 to replace DISTRIBUTIONS_LOGSQRTDETSIGMA and DISTRIBUTIONS_LOGPRIOR
                
                // Orig (1): distributionSigmaInverseMUVectors.push_back(distributionSigmaInverseMatrices[i]*distributionMUVectors[i].transpose()); //%11/13/2010 ffor distr_max_box_4
                // Replace (1)
                columnVector dist_MU_vec_xpose(distributionMUVectors[i].transpose());
                distributionSigmaInverseMUVectors.push_back(distributionSigmaInverseMatrices[i]*dist_MU_vec_xpose); //%11/13/2010 ffor distr_max_box_4

                rowVector one_zero_zero(1, 0, 0);
                rowVector zero_one_zero(0, 1, 0);
                rowVector zero_zero_one(0, 0, 1);
                
                rowVector row0_dist_sigma_inv(distributionSigmaInverseMatrices[i].row(0));
                rowVector row1_dist_sigma_inv(distributionSigmaInverseMatrices[i].row(1));
                rowVector row2_dist_sigma_inv(distributionSigmaInverseMatrices[i].row(2));
                
                // Orig (2): distributionA1InverseMatrices.push_back((matrix(rowVector(1, 0, 0), distributionSigmaInverseMatrices[i].row(1), distributionSigmaInverseMatrices[i].row(2))).inverse());
                // Replace (2):                
                matrix m_1_0_0(one_zero_zero, row1_dist_sigma_inv, row2_dist_sigma_inv);
                distributionA1InverseMatrices.push_back(m_1_0_0.inverse());             
                
                // Orig (3): distributionA2InverseMatrices.push_back((matrix(distributionSigmaInverseMatrices[i].row(0), rowVector(0, 1, 0), distributionSigmaInverseMatrices[i].row(2))).inverse());
                // Replace (3):                
                matrix m_0_1_0(row0_dist_sigma_inv, zero_one_zero, row2_dist_sigma_inv);
                distributionA2InverseMatrices.push_back(m_0_1_0.inverse());
                
                // Orig (4): distributionA3InverseMatrices.push_back((matrix( distributionSigmaInverseMatrices[i].row(0), distributionSigmaInverseMatrices[i].row(1), rowVector(0, 0, 1))).inverse());
                matrix m_0_0_1(row0_dist_sigma_inv, row1_dist_sigma_inv, zero_zero_one);
                distributionA3InverseMatrices.push_back(m_0_0_1.inverse());

                int counter = 0; //% 12/22/2010 to hopefuly calculate the function in vertices faster
                matrix8x3     m;
                columnVector8 c;
        
                for (int s1=-1; s1<=1; s1=s1+2)
                {
                    for (int s2=-1; s2<=1; s2=s2+2)
                    {
                        for (int s3=-1; s3<=1; s3=s3+2)
                        {
                            
                            //ORIG:m.set(counter, -0.5f * rowVector(float(s1), float(s2), float(s3)) * distributionSigmaInverseMatrices[i]);
                            //ORIG:c.set(counter, -0.125f * rowVector(float(s1), float(s2), float(s3)) * distributionSigmaInverseMatrices[i] * columnVector(float(s1), float(s2), float(s3)));
                            
                            float fs1 = float(s1);
                            float fs2 = float(s2);
                            float fs3 = float(s3);
                            
                            rowVector rv_s1s2s3(fs1, fs2, fs3);
                            rowVector rv_m_s1s2s3(-0.5f * rv_s1s2s3);
                            rowVector rv_c_s1s2s3( -0.125f * rv_s1s2s3);
                            
                            matrix distSigInvMat(distributionSigmaInverseMatrices[i]);
                            
                            rowVector rv_temp_m(rv_s1s2s3 * distSigInvMat);
                            
                            m.set(counter, rv_temp_m);
                            
                            columnVector cv_s1s2s3(fs1, fs2, fs3);
                            rowVector    rv_temp_crv(rv_c_s1s2s3 * distSigInvMat);
                            float        temp_cF(rv_temp_crv * cv_s1s2s3);
                            
                            c.set(counter, temp_cF);
                            
                            counter = counter + 1;
                        }
                    }
                }

                distributionUMatrices.push_back(m);
                distributionHMatrices.push_back(c);
            }
            
            return;
        }
    }
    


    mDistributionCount=number_distributions;
    int counter=0;
    std::vector<rowVector> mu;
    while (counter<number_distributions)
    {
        rowVector mucandidate(dblRand()*(xhigh0-xlow0)+xlow0, dblRand()*(yhigh0-ylow0)+ylow0, dblRand()*(zhigh0-zlow0)+zlow0);
        float minsum = FLT_MAX;
        for (int i=0; i<mu.size(); i++)
        {
            float sum = ((mucandidate - mu[i]).square()).sum();
            if (sum < minsum)
            {
                minsum = sum;
            }
        }

        if (counter==0||sqrt(minsum)>=mindistseeds)// %Checks minimal distance of seeds
        {
            //% Does not check for the first seed!
            if (mucandidate[2]>0)
            {
                if ((mucandidate[0]*mucandidate[0])/(a*a)+(mucandidate[1]*mucandidate[1])/(b*b)+ (mucandidate[2] * mucandidate[2])/(cprim*cprim)<=1)// %we use < and equal here TALK TO PEDJA
                {
                    counter=counter+1;
                    distributionMUVectors.push_back(mucandidate);
                    mu.push_back(mucandidate); //%used to check distances!
                }
            }
            else // %below plane x3=0, second ellipsoid applies
            {
                if ((mucandidate[0]*mucandidate[0])/(a*a)+(mucandidate[1]*mucandidate[1])/(b*b)+ (mucandidate[2] * mucandidate[2])/(csec*csec)<=1)// %we use < and equal here TALK TO PEDJA
                {
                    counter=counter+1;
                    distributionMUVectors.push_back(mucandidate);
                    mu.push_back(mucandidate); //%used to check distances!
                }
            }
        }
    }


    // 1/13/2011 Assignment of labels to the seeds  
    std::vector<float> prob_dense(number_distributions);
    float prob_sum =0.0;

    for (int counter=0; counter < number_distributions; counter++)
    {
        if (isGaussian)
        {
            prob_dense[counter]=probability_dense(mu[counter],a,b,cprim,csec,sigma);
        }
        else
        {
            prob_dense[counter]=probability_denseBeta(mu[counter],a,b,cprim,csec,sigma,  alpha1,  beta1,  alpha2,  beta2);
        }
        prob_sum = prob_sum + prob_dense[counter];
    }

    // Percentage is now an input
    float norm_coefficient=Percentage/100*number_distributions/prob_sum; // 1/17/2011
    for (int counter=0; counter < number_distributions; counter++)
    {
        if (dblRand()<=prob_dense[counter]*norm_coefficient) // norm_coefficient
        {
            distributionLabels.push_back(5); // utilize label 5 for dense compartments
        }
        else
        {
            distributionLabels.push_back(3); // utilize label 3 for fat compartments
        }
    }

    std::vector <float> Speeds(number_distributions);
    float maxSpeed = -FLT_MAX;
    for (int i = 0; i<number_distributions; i++)
    {
        float speed = dblRand() * (maxspeed - minspeed) + minspeed;
        Speeds[i] = speed;
        if (speed > maxSpeed)
        {
            maxSpeed = speed;
        }
    }

    for (int i = 0; i<number_distributions; i++)
    {
        Speeds[i] = Speeds[i]/maxSpeed;
    }

    std::vector <float> Ratios1(number_distributions), Ratios2(number_distributions);

    for (int i = 0; i<number_distributions; i++)
    {
        Ratios1[i]=dblRand()*(maxratio-minratio)+minratio;
        Ratios2[i]=dblRand()*(maxratio-minratio)+minratio;
    }

    for (int i= 0; i < mDistributionCount; i++)
    {
        float kaprim = Speeds[i];
        float kbprim = Speeds[i]*Ratios1[i];
        float kcprim = Speeds[i]*Ratios2[i];

        matrix Vtransposed = EllipsoidOrientation(rowVector(0, 0, 0),a,cprim,csec,b,distributionMUVectors[i]);
        ///ORIG: distributionSigmaInverseMatrices.push_back(Vtransposed.transpose() * matrix(1/(kaprim*kaprim),1/(kbprim*kbprim),1/(kcprim * kcprim))*Vtransposed);
        matrix mat_ka_kb_kc = matrix( 1.0f/(kaprim*kaprim),  1.0f/(kbprim*kbprim),  1.0f/(kcprim * kcprim) );
        distributionSigmaInverseMatrices.push_back(Vtransposed.transpose() * mat_ka_kb_kc * Vtransposed);
        //FIX IT LATER
    }

    for (int i=0; i<mDistributionCount; i++)
    {
        distributionPriors.push_back(1.0f/float(mDistributionCount));
    }

    // INITIALIZATION (could be put into initialization code above)

    

    // TODO distributionRMatrices=cell(1,mDistributionCount); 12/11/2010 CHECK!
    for (int i=0; i<mDistributionCount; i++)
    {
        matrix dis_sig_inv_mat(distributionSigmaInverseMatrices[i]); // eliminate redundant index calc & retrieve (performance)
        
        ///ORIG: distributionRMatrices.push_back(distributionSigmaInverseMatrices[i].chol()); // %If we could find to represent cholesky using eigenvalues and eigenvectors, then this is not needed
        distributionRMatrices.push_back(dis_sig_inv_mat.chol()); // orig compiled, changed to take advantage of dis_sig_inv_mat matrix (performance gain?)
      
        distributionLogSqrtSigmaLogPriors.push_back(distributionRMatrices[i].transpose().diag().log().sum()+log(distributionPriors[i])); //%12/22/2010 to replace DISTRIBUTIONS_LOGSQRTDETSIGMA and DISTRIBUTIONS_LOGPRIOR
        
        ///ORIG: distributionSigmaInverseMUVectors.push_back(distributionSigmaInverseMatrices[i]*distributionMUVectors[i].transpose()); //%11/13/2010 ffor distr_max_box_4
        columnVector dist_mu_vec_xpose = distributionMUVectors[i].transpose();
        distributionSigmaInverseMUVectors.push_back(dis_sig_inv_mat * dist_mu_vec_xpose);        
        
        ///ORIG: distributionA1InverseMatrices.push_back((matrix(rowVector(1, 0, 0), distributionSigmaInverseMatrices[i].row(1), distributionSigmaInverseMatrices[i].row(2))).inverse());
        ///ORIG: distributionA2InverseMatrices.push_back((matrix(distributionSigmaInverseMatrices[i].row(0), rowVector(0, 1, 0), distributionSigmaInverseMatrices[i].row(2))).inverse());
        ///ORIG: distributionA3InverseMatrices.push_back((matrix( distributionSigmaInverseMatrices[i].row(0), distributionSigmaInverseMatrices[i].row(1), rowVector(0, 0, 1))).inverse());      
               
        rowVector r0_dist_sig_inv_mat(dis_sig_inv_mat.row(0));
        rowVector r1_dist_sig_inv_mat(dis_sig_inv_mat.row(1));
        rowVector r2_dist_sig_inv_mat(dis_sig_inv_mat.row(2));
        
        rowVector rv_1_0_0 = rowVector(1, 0, 0);
        rowVector rv_0_1_0 = rowVector(0, 1, 0);
        rowVector rv_0_0_1 = rowVector(0, 0, 1);
        
        matrix    m_100_r1_r2(rv_1_0_0, r1_dist_sig_inv_mat, r2_dist_sig_inv_mat);
        matrix    m_r0_010_r2(r0_dist_sig_inv_mat, rv_0_1_0, r2_dist_sig_inv_mat);
        matrix    m_r0_r1_001(r0_dist_sig_inv_mat, r1_dist_sig_inv_mat, rv_0_0_1);
        
        distributionA1InverseMatrices.push_back(m_100_r1_r2.inverse());        
        distributionA2InverseMatrices.push_back(m_r0_010_r2.inverse());
        distributionA3InverseMatrices.push_back(m_r0_r1_001.inverse());

        int counter=0; // hopefuly calculate the function in vertices faster
        matrix8x3 m;
        columnVector8 c;
        
        for (int s1=-1; s1<=1; s1=s1+2)
        {
            for (int s2=-1; s2<=1; s2=s2+2)
            {
                for (int s3=-1; s3<=1; s3=s3+2)
                {
                    float fs1 = float(s1);
                    float fs2 = float(s2);
                    float fs3 = float(s3);
                    
                    ///ORIG: m.set(counter, -0.5 * rowVector(float(s1), float(s2), float(s3)) * distributionSigmaInverseMatrices[i]);
                    rowVector rv_s1s2s3(fs1, fs2, fs3);
                    matrix dist_sig_inv_mat = distributionSigmaInverseMatrices[i];
                    rowVector temp1(-0.5f * rv_s1s2s3);
                    rowVector temp2(temp1 * dist_sig_inv_mat);
                    m.set(counter, temp2);
                    
                    ///ORIG: c.set(counter, -0.125 * rowVector(float(s1), float(s2), float(s3)) * distributionSigmaInverseMatrices[i] * columnVector(float(s1), float(s2), float(s3)));
                    columnVector cv_s1s2s3(fs1, fs2, fs3);
                    rowVector temp3(-0.125f * rv_s1s2s3);
                    rowVector temp4(temp3 * dist_sig_inv_mat);
                    float temp5(temp4 * cv_s1s2s3);
                    c.set(counter, temp5);
                    
                    counter = counter + 1;      
                }
            }
        }

        distributionUMatrices.push_back(m);
        distributionHMatrices.push_back(c);
    }

    if (!DistributionSaveFile.empty()) 
    {   
        std::ofstream outputStream(DistributionSaveFile);
        if (outputStream.is_open())
        {
            rowVector mucand;
            short label;
            double speed, ratio1, ratio2;

            for(int ndx=0; ndx<number_distributions; ++ndx)
            {
                mucand = distributionMUVectors[ndx];
                label  = distributionLabels[ndx];
                speed  = Speeds[ndx];
                ratio1 = Ratios1[ndx];             
                ratio2 = Ratios2[ndx];
                
                // outputStream << mucand[0] << mucand[1] << mucand[2] << label << speed << ratio1 << ratio2 <<'\n';
                // Fixed 2021-01-15 per Predrag's report
                outputStream << mucand[0] << " " << mucand[1] << " " << mucand[2] << " " 
                             << label << " " << speed << " " << ratio1 << " " << ratio2 <<'\n';

            }
            
            outputStream.close();
        }
    }

}
