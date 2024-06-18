// VCT_Random.h

#pragma once

#include <fstream>
#include <string>

#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

#pragma region Documentation
/// <summary> Common Random Number store and retrieve class. </summary>
/// <remarks>D. Higginbotham, 2020-04-28. </remarks>
#pragma endregion
class Random
{
public:

      #pragma region Documentation
      /// <summary>(default)Constructor: initialize internal states. </summary>
      /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
      #pragma endregion
      Random()
      {}


    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    ~Random()
    {
        // Close random number streams (close is a noop if not already open)
        rand_ofs.close(); // random number storate output stream
        rand_ifs.close(); // random number storate input stream
    }

    #pragma region Documentation
    /// <summary>Set the filename which is opened for random number storage. </summary>
    /// <remarks>D. Higginbotham, 2020-04-28. </remarks>
    /// <remarks>This file accumulates random numbers which are later read in.</remarks>
    /// <parm name="fname">The name of file in which to store random numbers. </parm>
    /// <return>An boolean indication of file open success. </return>
    #pragma endregion
    bool setRandStoreFile(std::string fname)
    {
        if (rand_ofs.is_open()) rand_ofs.close();
        rand_ofs.open(fname);
        rand_ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        return rand_ofs.is_open();
    }


    #pragma region Documentation
    /// <summary>Set the filename which is opened for random number retrieval. </summary>
    /// <remarks>D. Higginbotham, 2020-04-28. </remarks>
    /// <remarks>This file retreives accumulated random numbers in order.</remarks>
    /// <parm name="fname">The name of file from which to retrieve random numbers. </parm>
    /// <return>An boolean indication of file open success. </return>
    #pragma endregion
    bool setRandRetrieveFile(std::string fname)
    {
        if (rand_ifs.is_open()) rand_ifs.close();
        rand_ifs.open(fname);
        rand_ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::istream::eofbit);
        return rand_ifs.is_open();
    }


    #pragma region Documentation
    /// <summary>Retieve the next random number. </summary>
    /// <remarks>D. Higginbotham, 2020-04-28. </remarks>
    /// <remarks>Random numbers are retrieved from pre-constructed file.</remarks>
    /// <return>The next random number (all random numbers are stored as floats).</return>
    /// <throw>A descriptive string if EOF, or other problem, occurs.</throw>
    #pragma endregion
    float nextRandom()
    {
        float retval = -1.0;
        try
        {
           rand_ifs >> retval;
        }
        catch (const std::ifstream::failure &e)
        {
           std::string msg(e.what());
           throw msg;
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Store the next random number. </summary>
    /// <remarks>D. Higginbotham, 2020-04-28. </remarks>
    /// <remarks>Random numbers are stored as floats in a pre-specified file.</remarks>
    /// <throw>A descriptive string if EOF, or other problem, occurs.</throw>
    #pragma endregion
    void addRandom(float number)
    {
        try
        {
           rand_ofs << number << '\n';
        }
        catch (const std::ofstream::failure &e)
        {
           std::string msg(e.what());
           throw msg;
        }
    }

private:

    std::ofstream rand_ofs; /// random number storate output stream
    std::ifstream rand_ifs; /// random number storate input stream

};


} // namespace vct
