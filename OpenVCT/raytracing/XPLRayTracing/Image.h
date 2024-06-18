// Image.h

#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <mutex>
#include <functional>
#include <cmath>
#include <cstdlib> // for exit()

template <class T> class Image
{   
public:

    T getPixel(unsigned short x, unsigned short y);
    void setPixel(unsigned short x, unsigned short y, T value);
    Image(unsigned short width, unsigned short height);
    ~Image(void);
    
#if 0
    void saveRawAsXPL(std::string pathname);
    void saveLogAsXPL(std::string pathname);
    void saveRawAsRaw(std::string pathname);
    void saveLogAsRaw(std::string pathname);
    void saveRawAsRawNoScale(std::string pathname);
    void saveLogAsRawNoScale(std::string pathname);
#endif
    void transform(unsigned short *out, std::function<int(int x, int y, int w, int h)> func); 

    void setUseScaledValue(bool val)              { useScaledValue = val; }
    void setMinGrayscaleValue(unsigned short val) { minGrayScale = val; }
#if 0
    void setPixelSpace(size_t num, size_t hiBit);
#endif
private: 

    std::vector<T> data;
    unsigned short width, height;
    T max, min;
    bool minmaxset;

    bool useScaledValue;
    unsigned short minGrayScale;
    size_t numPixels;
    size_t highBit;
    T mask;
};



template <class T> T Image<T>::getPixel(unsigned short x, unsigned short y)
{
    return data[x+y*width];
}


template <class T> void Image<T>::setPixel(unsigned short x, unsigned short y, T value )
{
    if (!minmaxset)
    {
        min = max = value;
        minmaxset = true;
    }
    if (max < value) max = value;
    if (min > value) min = value;

    data[x+y*width] = value;
}



template <class T> Image<T>::Image(unsigned short x, unsigned short y) 
      : width(x), height(y), minmaxset(false), 
        useScaledValue(false), minGrayScale(50), numPixels(14), highBit(13)
{
    data.resize(width*height, 0);
    mask = (1 << numPixels) - 1;   
}


template <class T> Image<T>::~Image() 
{}


#if 0

template<typename T>
std::ostream &operator <<(std::ostream &os, const std::vector<T> &v) 
{
   using namespace std;
   copy(v.begin(), v.end(), ostream_iterator<T>(os));
   return os;
}


template <class T> void Image<T>::saveRawAsXPL(std::string pathname)
{
    std::ofstream outputFile(pathname, std::fstream::out | std::fstream::binary);
    outputFile << width << " " << height << "\n";


    unsigned char *p = (unsigned char *)&(data[0]);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            T v = data[width *y + x];
            unsigned short scaledValue = ((v - min) / (max + 1 - min))*(1 << 16);
            char *c = reinterpret_cast<char *>(&scaledValue);
            outputFile.write(c, 2);
        }
    }
    outputFile.close();
}


template <class T> void Image<T>::saveRawAsRaw(std::string pathname)
{
    std::ofstream outputFile(pathname, std::fstream::out | std::fstream::binary);
    unsigned char *p = (unsigned char *)&(data[0]);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            T v = data[width *y + x];
            unsigned short scaledValue = static_cast<unsigned short>(((v - min*0.9) / (max*1.1 - min*0.9))*(1 << 16));
            char *c = reinterpret_cast<char *>(&scaledValue);
            outputFile.write(c, 2);
        }
    }
    outputFile.close();
}
#endif

template <class T> void Image<T>::transform(unsigned short *out, std::function<int(int x, int y, int w, int h)> func)
{
    // rotate image 90 degrees to the right
    int index;
    int total_size = width * height; // for use in test

    T post_min = max;
    T post_max = 0;

    int debug_line_count = 0;

    for (int y = 0; y<height; y++)
    {
       for (int x = 0; x<width; x++)
       {
            T v = data[(width * y) + x];
            unsigned short scaledValue = static_cast<unsigned short>(((v-min*0.9)/(max*1.1-min*0.9))*(1<<16));

            index = func(x, y, width, height);

            // Store pixel in (new) location
            if (useScaledValue)
            {
               out[index] = scaledValue;
            }
            else
            {
                v = v + minGrayScale;          // v = v + 50. 

                if (v <= minGrayScale)         // if (v <= 50.)
                {
                    out[index] = minGrayScale; //    out[index] = 50;
                }
                else if (v > mask) // 2^14-1   // else if (v > 16383)
                {
                    out[index] = static_cast<unsigned short>(mask);
                }
                else 
                {
                    out[index] = static_cast<unsigned short>(v);
                }

                if (out[index] < post_min) post_min = out[index];
                if (out[index] > post_max) post_max = out[index];
            }
        }
    }

    int tmp = width;
    width   = height;
    height  = tmp;
}

#if 0

template <class T> void Image<T>::saveRawAsRawNoScale(std::string pathname)
{
    std::ofstream outputFile(pathname, std::fstream::out | std::fstream::binary);
    unsigned char *p = (unsigned char *)&(data[0]);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            T v = data[width *y + x];
            unsigned short scaledValue = v;
            unsigned char *c = (unsigned char *)&scaledValue;
            outputFile.write(reinterpret_cast<char *>(c), 2);
        }
    }
    outputFile.close();
}


template <class T> void Image<T>::saveLogAsXPL(std::string pathname) 
{
    std::ofstream outputFile(pathname, std::fstream::out|std::fstream::binary);
    outputFile << width << " " << height << "\n";
    
    double min2 = min-1;
    double max2 = max+1;
    
    unsigned char *p = (unsigned char *) &(data[0]);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            T v = data[width *y + x];
            double scaledValue = ((v - min2) / (max2 + 1 - min2))*(1 << 16);
            double sumIUTtemp1 = std::log(scaledValue);
            double sumIUTtemp2 = 1 << 16;
            double sumIUTtemp3 = sumIUTtemp1*sumIUTtemp2;
            double sumIUTtemp4 = std::log(1 << 16);
            double sumIUTtemp5 = sumIUTtemp3 / sumIUTtemp4;
            unsigned short fixedPt = sumIUTtemp5;
            unsigned char *c = (unsigned char *)&fixedPt;
            outputFile.write(reinterpret_cast<char *>(c), 2);
        }
    }
    outputFile.close();
}


template <class T> void Image<T>::saveLogAsRaw(std::string pathname) 
{
    std::ofstream outputFile(pathname, std::fstream::out|std::fstream::binary);
    unsigned char *p = (unsigned char *) &(data[0]);

    double min2 = min-1;
    double max2 = max+1;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            T v = data[width *y + x];
            double scaledValue = ((v - min2) / (max2 - min2))*(1 << 16);
            double sumIUTtemp1 = std::log(scaledValue);
            double sumIUTtemp2 = 1 << 16;
            double sumIUTtemp3 = sumIUTtemp1*sumIUTtemp2;
            double sumIUTtemp4 = std::log(1 << 16);
            double sumIUTtemp5 = sumIUTtemp3 / sumIUTtemp4;

            unsigned short fixedPt = (unsigned short)sumIUTtemp5;
            unsigned char *c = (unsigned char *)&fixedPt;
            outputFile.write(reinterpret_cast<char *>(c), 2);
        }
    }
    outputFile.close();
}


template <class T> void Image<T>::saveLogAsRawNoScale(std::string pathname) 
{
    std::ofstream outputFile(pathname, std::fstream::out|std::fstream::binary);
    unsigned char *p = (char *) &(data[0]);

    double min2 = min-1;
    double max2 = max+1;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            T v = data[width *y + x];
            double scaledValue = v;
            double sumIUTtemp1 = std::log(scaledValue);
            double sumIUTtemp2 = 1 << 16;
            double sumIUTtemp3 = sumIUTtemp1*sumIUTtemp2;
            double sumIUTtemp4 = std::log(1 << 16);
            double sumIUTtemp5 = sumIUTtemp3 / sumIUTtemp4;

            unsigned short fixedPt = (unsigned short)sumIUTtemp5;
            unsigned char *c = (unsigned char *)&fixedPt;
            outputFile.write(reinterpret_cast<char *>(c), 2);
        }
    }
    outputFile.close();
}


template <class T> void Image<T>::setPixelSpace(size_t num, size_t hiBit)
{
    numPixels = num;
    highBit = hiBit;

    mask = (1 << numPixels) - 1;
    size_t shift = numPixels - highBit - 1;
    if (shift > 0) mask = mask << shift;
}

#endif