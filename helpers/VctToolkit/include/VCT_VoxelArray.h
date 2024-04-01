// VCT_VoxelArray.yh

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_IndexTable.h"

#include <cstddef>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

#pragma region Documentation
/// <summary>The voxel data type used for the phantom. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum VOXEL_TYPE
{
    UNK_VOXEL_TYPE = 0,
    V_INT8         = 1,
    V_UINT8        = 2,
    V_INT16        = 3,
    V_UINT16       = 4,
    V_INT32        = 5,
    V_UINT32       = 6,
    V_INT64        = 7,
    V_UINT64       = 8
};

#pragma region Documentation
/// <summary>The voxel index order used for the phantom. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum VOXEL_ORDER
{
    UNK_VXL_ORDER = 0,
    X_Y_Z         = 1,
    X_Z_Y         = 2,
    Y_X_Z         = 3,
    Y_Z_X         = 4,
    Z_X_Y         = 5,
    Z_Y_X         = 6
};

#pragma region Documentation
/// <summary>The endianess of the binary data used for the phantom. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
enum ENDIANESS
{
    VCT_UNK_ENDIAN    = 0,
    VCT_LITTLE_ENDIAN = 1,
    VCT_MIDDLE_ENDIAN = 2,
    VCT_BIG_ENDIAN    = 3
};


#pragma region Documentation
/// <summary> VoxelArray class for Open Virtual Clinical Trials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class VoxelArray
{
public:

    #pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vxl_num">The dimensions of the 3D voxel body (count/deltas). </parm>
    /// <parm name="vxl_siz">The 3D size of a a voxel in the voxel body. </parm>
    /// <parm name="vxl_ord">The order of x, y, z dimensions in the vodel body. </parm>
    /// <parm name="type">The voxel body data type. </parm>
    /// <parm name="bits_vxl">The bits per voxel. </parm>
    /// <parm name="endi">The endianess of the binary voxel body data. </parm>
    /// <parm name="cosines">The direction cosines (3x3) matrix as a linear array. </parm>
    #pragma endregion
    VoxelArray(unsigned int vxl_num[3], float vxl_siz[3], vct::VOXEL_ORDER vxl_ord, 
               vct::VOXEL_TYPE type, int bits_vxl, vct::ENDIANESS endi, 
               float cosines[9])
        : varray_uid("unk UID"),
          voxel_order(vxl_ord),
          voxel_type(type),
          bits_per_voxel(bits_vxl),
          endian(endi),
          total_bytes(0L),
          deallocate(false),
          voxels(nullptr)
    {
        for(int i=0; i<3; ++i)
        {
            voxel_num[i]  = vxl_num[i];
            voxel_size[i] = vxl_siz[i];
        }
        for(int j=0; j<9; ++j)
        {
            dir_cosines[j] = cosines[j];
        }
    }

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    VoxelArray()
        : varray_uid("unk UID"),
          voxel_order(UNK_VXL_ORDER),
          voxel_type(UNK_VOXEL_TYPE),
          bits_per_voxel(8),
          endian(VCT_UNK_ENDIAN),
          total_bytes(0L),
          deallocate(false),
          voxels(nullptr)
    {
        for(int i=0; i<3; ++i)
        {
            voxel_num[i]  = 0;
            voxel_size[i] = 0.0f;
        }
        for(int j=0; j<9; ++j)
        {
            dir_cosines[j] = 0.0f;
        }
    }
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    VoxelArray(const VoxelArray &rhs)
        : varray_uid(rhs.varray_uid),
          voxel_order(rhs.voxel_order),
          voxel_type(rhs.voxel_type),
          bits_per_voxel(rhs.bits_per_voxel),
          endian(rhs.endian),
          total_bytes(rhs.total_bytes),
          deallocate(rhs.deallocate),
          voxels(nullptr)
    {
        voxel_num[0] = rhs.voxel_num[0];
        voxel_num[1] = rhs.voxel_num[1];
        voxel_num[2] = rhs.voxel_num[2];

        voxel_size[0] = rhs.voxel_size[0];
        voxel_size[1] = rhs.voxel_size[1];
        voxel_size[2] = rhs.voxel_size[2];

        dir_cosines[0] = rhs.dir_cosines[0];
        dir_cosines[1] = rhs.dir_cosines[1];
        dir_cosines[2] = rhs.dir_cosines[2];

        dir_cosines[3] = rhs.dir_cosines[3];
        dir_cosines[4] = rhs.dir_cosines[4];
        dir_cosines[5] = rhs.dir_cosines[5];

        dir_cosines[6] = rhs.dir_cosines[6];
        dir_cosines[7] = rhs.dir_cosines[7];
        dir_cosines[8] = rhs.dir_cosines[8];
        
        if (rhs.voxels != nullptr && total_bytes > 0)
        {
            voxels = new unsigned char[total_bytes];
            if (voxels != nullptr) memcpy(voxels, rhs.voxels, total_bytes);
        }
    }
    
    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~VoxelArray()
    {
        if (deallocate == true && voxels != nullptr)
        {
            delete [] voxels;
            voxels = nullptr;
        }
    }


    // Manipulators (used to set component values)

    
    #pragma region Documentation
    /// <summary>Set the voxel array UID. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="uid">The voxel array UID. </parm>
    #pragma endregion
    void setVoxelArrayUID(std::string uid)    { varray_uid = uid; }
    

    #pragma region Documentation
    /// <summary>Set the voxel body's 3D dimensions (counts/delta). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vnum">The voxel body's 3D dimensions (counts). </parm>
    #pragma endregion
    void setVoxelNum(long long vnum[3])
    { 
        voxel_num[0] = vnum[0]; 
        voxel_num[1] = vnum[1]; 
        voxel_num[2] = vnum[2]; 
        total_bytes = vnum[0] * vnum[1] * vnum[2]; 
    }
    
    
    #pragma region Documentation
    /// <summary>Set the voxel body's 3D dimensions (counts/delta). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="first">The first dimension of the voxel body counts. </parm>
    /// <parm name="second">The second dimension of the voxel body counts. </parm>
    /// <parm name="third">The third dimension of the voxel body counts. </parm>
    #pragma endregion
    void setVoxelNum(long long first, 
                     long long second, 
                     long long third)
    { 
        voxel_num[0] = first; 
        voxel_num[1] = second; 
        voxel_num[2] = third; 
    }   
    
       
    #pragma region Documentation
    /// <summary>Set the size of a voxel body's voxel in 3D mm. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vsize">An array containing the voxel body dimensions. </parm>
    #pragma endregion
    void setVoxelSize_mm(float vsize[3])      
    { 
        voxel_size[0] = vsize[0]; 
        voxel_size[1] = vsize[1]; 
        voxel_size[2] = vsize[2]; 
    }
    
    
    #pragma region Documentation
    /// <summary>Set the size of a voxel body's voxel in 3D mm. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="first">The 1st dimension of the voxel body (mm). </parm>
    /// <parm name="second">The 2nd dimension of the voxel body (mm). </parm>
    /// <parm name="third">The 3rd dimension of the voxel body (mm). </parm>
    #pragma endregion
    void setVoxelSize_mm(float first, 
                         float second, 
                         float third)
    { 
        voxel_size[0] = first; 
        voxel_size[1] = second; 
        voxel_size[2] = third; 
    }

       
    #pragma region Documentation
    /// <summary>Set the order of a voxel body's voxel 3D dimensions. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vord">An enumeration describing x/y/z order. </parm>
    #pragma endregion
    void setVoxelOrder(vct::VOXEL_ORDER vord) { voxel_order = vord; }
    
    
    #pragma region Documentation
    /// <summary>Set the order of a voxel body's voxel 3D dimensions. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="order">A string corresponding to the voxel order enumeration. </parm>
    #pragma endregion
    void setVoxelOrder(std::string order)     { voxel_order = interpretVoxelOrder(order); }

    
    #pragma region Documentation
    /// <summary>Set the voxel body's voxel data type. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vtype">A voxel data type enumeration. </parm>
    #pragma endregion
    void setVoxelType(vct::VOXEL_TYPE vtype)  { voxel_type = vtype; } 
    
    
    #pragma region Documentation
    /// <summary>Set the voxel body's voxel data type. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vtype">A string corresponding to the voxel data type enumeration. </parm>
    #pragma endregion
    void setVoxelType(std::string vtype)      { voxel_type = interpretVoxelType(vtype); }    

    
    #pragma region Documentation
    /// <summary>Set the bits-per-voxel used to represent the voxel body's voxels. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="bpv">The bits per voxels. </parm>
    #pragma endregion
    void setBitsPerVoxel(int bpv)             { bits_per_voxel = bpv; }
    
    
    #pragma region Documentation
    /// <summary>Set the endianess of the voxel body's binary data. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="end">An endianess enumeration. </parm>
    #pragma endregion    
    void setEndian(ENDIANESS end)             { endian = end; }

 
    #pragma region Documentation
    /// <summary>Set the endianess of the voxel body's binary data. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="end">A string corresponding to an endianess enumeration. </parm>
    #pragma endregion
    void setEndian(std::string end)           { endian = interpretEndian(end); }
    
    
    #pragma region Documentation
    /// <summary>Set the direction cosines matrix of the voxel body's binary data. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="dircos">An array containing the 3x3 direction cosines matrix. </parm>
    #pragma endregion
    void setDirectionCosines(float dircos[9]) 
    { 
        for(int i=0; i<9; ++i) dir_cosines[i] = dircos[i]; 
    }
    
    
    #pragma region Documentation
    /// <summary>Set the total size (in bytes) of the voxel body's binary data. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ts">The total size of the voxel body in bytes. </parm>
    #pragma endregion
    void setTotalSize(long long ts)           { total_bytes = ts; }

    
    #pragma region Documentation
    /// <summary>Set the voxel body's binary data. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vxls">A pointer to an existing (and valid) contiguous voxel body. </parm>
    #pragma endregion
    void setVoxels(unsigned char *vxls)       { voxels = vxls; deallocate = false; }

    
    #pragma region Documentation
    /// <summary>Replace - free the previous allocation and save the specified allocation of - voxels . </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vxls">A pointer to an existing (and valid) contiguous voxel body. </parm>
    #pragma endregion
    void replaceVoxels(unsigned char *vxls)   
    { 
        if (voxels != nullptr) delete [] voxels; voxels = vxls; 
    }


    // Accessors (used to retrieve component values)
    
    
    #pragma region Documentation
    /// <summary>Retrieve the voxel array UID. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The voxel array UID. </return>
    #pragma endregion
    std::string getVoxelArrayUID() { return varray_uid; }

    
    #pragma region Documentation
    /// <summary>Retrieve the dimensions of the voxel body's binary data. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="a">A reference to a variable to receive the first dimension. </parm>
    /// <parm name="b">A reference to a variable to receive the second dimension. </parm>
    /// <parm name="c">A reference to a variable to receive the third dimension. </parm>
    #pragma endregion
    void getVoxelNum(long long &a, long long &b, long long &c) 
    { 
        a = voxel_num[0]; 
        b = voxel_num[1]; 
        c = voxel_num[2]; 
    }

                                 
    #pragma region Documentation
    /// <summary>Retrieve the size (in 3D) of a voxel in the voxel body. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="a">A reference to a variable to receive the voxel size in the first dimension. </parm>
    /// <parm name="b">A reference to a variable to receive the voxel size second dimension. </parm>
    /// <parm name="c">A reference to a variable to receive the voxel size third dimension. </parm>
    #pragma endregion
    void getVoxelSize_mm(float &a, float &b, float &c) 
    { 
        a = voxel_size[0]; 
        b = voxel_size[1]; 
        c = voxel_size[2]; 
    }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the order of voxel dimensions within the voxel body. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>An enumeration instance specifying the voxel order. </return>
    #pragma endregion
    vct::VOXEL_ORDER getVoxelOrder()                { return voxel_order; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the voxel body's voxel data type. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>An enumeration instance specifying the voxel data type. </return>
    #pragma endregion
    vct::VOXEL_TYPE  getVoxelType()                 { return voxel_type; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the voxel body's bits-per-voxel. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The bits-per-voxel. </return>
    #pragma endregion
    int getBitsPerVoxel()                           { return bits_per_voxel; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the voxel body's binary data endianess. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>An enumeration instance specifying the binary data endianess. </return>
    #pragma endregion
    vct::ENDIANESS getEndian()                      { return endian; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the voxel body's direction cosines matrix. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="dc">A 9-element array to receive the direction cosines matrix. </parm>
    #pragma endregion
    void getDirectionCosines(float *dc)
    { 
        memcpy(dc, dir_cosines, 9 * sizeof(float));
    }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the voxel body's total size (in bytes). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The total size of the binary voxel body data in bytes. </return>
    #pragma endregion
    long long getTotalSize()                        { return total_bytes; }
    
    
    #pragma region Documentation
    /// <summary>Retrieve the voxel body's binary data. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>A (type-agnostic) pointer to the binary data. </return>
    #pragma endregion
    void *getVoxels()                               { return voxels; }
    

    #pragma region Documentation
    /// <summary>Retrieve a count of glandular voxels and count of non-air voxels. </summary>
    /// <remarks>D. Higginbotham, 2017-07-07. </remarks>
    /// <parm name="it">A reference to the corresponding index table. </parm>
    /// <parm name="non_air">The number of non-air voxels in this phantom. </parm>
    /// <return>The count of glandular voxels (weight-adjusted) and the count of non-air voxels. </return>
    #pragma endregion
    float countGlandular(IndexTable &it, size_t &non_air)
    {
        const int NIST_GLANDULAR = 205;

        non_air = 0; // total number of phantom voxels which are not air
        float glandular_count = 0.0f;

        // Query the Index Table for all labels containing glandularity
        std::map<int, float> glandular_labels = it.findLabelsWithThisMaterial(NIST_GLANDULAR);

        size_t total = total_bytes * (8 / bits_per_voxel);

        for(size_t i = 0; i<total; ++i)
        {
            int v = voxels[i];
            if (v != 0) non_air++;

            // Check labels containing glandular material
            for(auto &lbl : glandular_labels)
            {
                // Increment the count by this label's glandular material's weight
                if (v == lbl.first) glandular_count += lbl.second;
            }
        }

        return glandular_count;
    }


    // XML IO Functions


    #pragma region Documentation
    /// <summary>Read the voxel body section of the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->getNodeValueXML("Voxel_Array_UID", varray_uid);

            if (clib->findNodeXML("VOXEL_NUM"))
            {
                clib->getNodeValueXML("X", voxel_num[0]);
                clib->getNodeValueXML("Y", voxel_num[1]);
                clib->getNodeValueXML("Z", voxel_num[2]);
                clib->concludeSectionXML(); // return above Voxel_Num
            }

            if (clib->findNodeXML("VOXEL_SIZE_MM"))
            {
                clib->getNodeValueXML("X", voxel_size[0]);
                clib->getNodeValueXML("Y", voxel_size[1]);
                clib->getNodeValueXML("Z", voxel_size[2]);
                clib->concludeSectionXML(); // return above Voxel_Size
            }

            // Read Voxel Array Order
            std::string order;
            if (clib->getNodeValueXML("VOXEL_ARRAY_ORDER", order))
            {
                voxel_order = interpretVoxelOrder(order);
            }

            // Read Voxel Type
            std::string type;
            if (clib->getNodeValueXML("VOXEL_TYPE", type))
            {
                voxel_type = interpretVoxelType(type);
            }

            clib->getNodeValueXML("Number_Bits", bits_per_voxel);

            // Read endianess
            std::string end_ness; 
            if (clib->getNodeValueXML("Endian", end_ness))
            {
                endian = interpretEndian(end_ness);
            }

            // Read Direction Cosines
            if (clib->findNodeXML("Direction_Cosines"))
            {
                //clib->getNodeValueXML("Value", dir_cosines[0]);
                clib->getFirstChildNodeValueXML(dir_cosines[0]);
                clib->getNextNodeValueXML(dir_cosines[1]);
                clib->getNextNodeValueXML(dir_cosines[2]);
                clib->getNextNodeValueXML(dir_cosines[3]);
                clib->getNextNodeValueXML(dir_cosines[4]);
                clib->getNextNodeValueXML(dir_cosines[5]);
                clib->getNextNodeValueXML(dir_cosines[6]);
                clib->getNextNodeValueXML(dir_cosines[7]);
                clib->getNextNodeValueXML(dir_cosines[8]);
                clib->concludeSectionXML(); // return from sibling values
                clib->concludeSectionXML(); // return from Direction_Cosines
            }
        }
    }


    #pragma region Documentation
    /// <summary>Write this voxel body's section to the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->writeNodeValueXML("Voxel_Array_UID", varray_uid);
            if (clib->addASectionXML("VOXEL_NUM"))
            {
                clib->writeNodeValueXML("X", voxel_num[0]);
                clib->writeNodeValueXML("Y", voxel_num[1]);
                clib->writeNodeValueXML("Z", voxel_num[2]);
                clib->concludeSectionXML(); // return above Voxel_Num
            }

            if (clib->addASectionXML("VOXEL_SIZE_MM"))
            {
                clib->writeNodeValueXML("X", voxel_size[0]);
                clib->writeNodeValueXML("Y", voxel_size[1]);
                clib->writeNodeValueXML("Z", voxel_size[2]);
                clib->concludeSectionXML(); // return above Voxel_Size
            }

            // Write Voxel Array Order
            std::string order = interpretVoxelOrder(voxel_order);
            clib->writeNodeValueXML("VOXEL_ARRAY_ORDER", order);

            // Write Voxel Type
            std::string type = interpretVoxelType(voxel_type);
            clib->writeNodeValueXML("VOXEL_TYPE", type);

            clib->writeNodeValueXML("Number_Bits", bits_per_voxel);

            // Write endianess
            std::string end_ness = interpretEndian(endian);
            clib->writeNodeValueXML("Endian", end_ness);

            // Write Direction Cosines
            if (clib->addASectionXML("Direction_Cosines"))
            {
                clib->writeNodeValueXML("Value", dir_cosines[0]);
                clib->writeNodeValueXML("Value", dir_cosines[1]);
                clib->writeNodeValueXML("Value", dir_cosines[2]);
                clib->writeNodeValueXML("Value", dir_cosines[3]);
                clib->writeNodeValueXML("Value", dir_cosines[4]);
                clib->writeNodeValueXML("Value", dir_cosines[5]);
                clib->writeNodeValueXML("Value", dir_cosines[6]);
                clib->writeNodeValueXML("Value", dir_cosines[7]);
                clib->writeNodeValueXML("Value", dir_cosines[8]);
                clib->concludeSectionXML(); // return Direction_Cosines
            }
        }
    }


    #pragma region Documentation
    /// <summary>Read the voxel body's binary data from the specified location. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    /// <parm name="path">The file path to the binary data. </parm>
    #pragma endregion
    void readPhantomData(CommonLibrary *clib, std::string path)
    {
        if (clib != nullptr && path.size() > 0)
        {
            unsigned int vbytes = 1;
            switch(voxel_type)
            {
                case UNK_VOXEL_TYPE:  vbytes = 1;    break;
                case V_INT8:          vbytes = 1;    break;
                case V_UINT8:         vbytes = 1;    break;
                case V_INT16:         vbytes = 2;    break;
                case V_UINT16:        vbytes = 2;    break;
                case V_INT32:         vbytes = 4;    break;
                case V_UINT32:        vbytes = 4;    break;
                case V_INT64:         vbytes = 8;    break;
                case V_UINT64:        vbytes = 8;    break;
            }

            total_bytes = voxel_num[0] * voxel_num[1] * voxel_num[2] * vbytes;
            if (total_bytes > 0L)
            {
                voxels = new unsigned char[total_bytes];
                if (voxels != nullptr)
                {
                    deallocate = true; // deallocate this memory upon exit
                    std::ifstream ifs(path, std::fstream::binary);
                    if (ifs)
                    {
                        clib->inputByChunks(ifs, voxels, total_bytes);
                        ifs.close();
                    }
                } 
                else
                {
                    std::cerr << "\n* ERROR: Couldn't allocate " << total_bytes << " bytes)\n" 
                              << std::endl;
                }
            }
        }
    }


    // Write out the Phantom Body (binary data)
    #pragma region Documentation
    /// <summary>Write this voxel body's binary data to the specified location. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    /// <parm name="path">The file path to the binary data. </parm>
    #pragma endregion
    void writePhantomData(CommonLibrary *clib, std::string path)
    {
        if (clib != nullptr && path.size() > 0)
        {
            unsigned int vbytes = 1;
            switch(voxel_type)
            {
                case UNK_VOXEL_TYPE:  vbytes = 1;    break;
                case V_INT8:          vbytes = 1;    break;
                case V_UINT8:         vbytes = 1;    break;
                case V_INT16:         vbytes = 2;    break;
                case V_UINT16:        vbytes = 2;    break;
                case V_INT32:         vbytes = 4;    break;
                case V_UINT32:        vbytes = 4;    break;
                case V_INT64:         vbytes = 8;    break;
                case V_UINT64:        vbytes = 8;    break;
            }

            total_bytes = voxel_num[0] * voxel_num[1] * voxel_num[2] * vbytes;

            if (voxels != nullptr && total_bytes > 0L)
            {
                std::ofstream ofs(path, std::fstream::binary);
                if (ofs)
                {
                    clib->outputByChunks(ofs, voxels, total_bytes);
                    ofs.flush();
                    ofs.close();
                }
            }
        }
    }


    #pragma region Documentation
    /// <summary>Convert a string to the corresponding voxel body voxel order enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="order">A string corresponding to a voxel body order enumeration. </parm>
    /// <return>The corresponding voxel order enumeration (or unknown voxel order if not found). </return>
    #pragma endregion  
    VOXEL_ORDER interpretVoxelOrder(std::string order)
    {
        std::transform(order.begin(), order.end(), order.begin(), toupper); // convert to upper case

        VOXEL_ORDER retval = UNK_VXL_ORDER;
        if      (order == "X | Y | Z")  retval = vct::X_Y_Z;
        else if (order == "X | Z | Y")  retval = vct::X_Z_Y;
        else if (order == "Y | X | Z")  retval = vct::Y_X_Z;
        else if (order == "Y | Z | X")  retval = vct::Y_Z_X;
        else if (order == "Z | X | Y")  retval = vct::Z_X_Y;
        else if (order == "Z | Y | X")  retval = vct::Z_Y_X;
        return retval;
    }
    
    
    #pragma region Documentation
    /// <summary>Convert a voxel body voxel order enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="order">Voxel body voxel order enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretVoxelOrder(VOXEL_ORDER order)
    {
        std::string retval("UNK_VXL_ORDER");        
        if      (order == vct::X_Y_Z)    retval = "X | Y | Z";
        else if (order == vct::X_Z_Y)    retval = "X | Z | Y";
        else if (order == vct::Y_X_Z)    retval = "Y | X | Z";
        else if (order == vct::Y_Z_X)    retval = "Y | Z | X";
        else if (order == vct::Z_X_Y)    retval = "Z | X | Y";
        else if (order == vct::Z_Y_X)    retval = "Z | Y | X";
        return retval;
    }
    
    
    #pragma region Documentation
    /// <summary>Convert a string to the corresponding voxel data type enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="type">A string corresponding to a voxel data type enumeration. </parm>
    /// <return>The corresponding voxel data type enumeration (or unknown data type if not found). </return>
    #pragma endregion  
    VOXEL_TYPE interpretVoxelType(std::string type)
    {
        std::transform(type.begin(), type.end(), type.begin(), toupper); // convert to upper case

        VOXEL_TYPE retval = UNK_VOXEL_TYPE;
        if      (type == "INT8")    retval = V_INT8;
        else if (type == "UINT8")   retval = V_UINT8;
        else if (type == "INT16")   retval = V_INT16;
        else if (type == "UINT16")  retval = V_UINT16;
        else if (type == "INT32")   retval = V_INT32;
        else if (type == "UINT32")  retval = V_UINT32;
        else if (type == "INT64")   retval = V_INT64;
        else if (type == "UINT64")  retval = V_UINT64;
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a voxel body voxel data type enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="type">Voxel data type enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretVoxelType(VOXEL_TYPE type)
    {
        std::string retval("UNK_VOXEL_TYPE");
        if      (type == V_INT8)    retval = "V_INT8";
        else if (type == V_UINT8)   retval = "V_UINT8";
        else if (type == V_INT16)   retval = "V_INT16";
        else if (type == V_UINT16)  retval = "V_UINT16";
        else if (type == V_INT32)   retval = "V_INT32";
        else if (type == V_UINT32)  retval = "V_UINT32";
        else if (type == V_INT64)   retval = "V_INT64";
        else if (type == V_UINT64)  retval = "V_UINT64";
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a string to the corresponding binary endianess enumeration. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="end">A string corresponding to a binary endianess enumeration. </parm>
    /// <return>The corresponding binary endianess enumeration (or unknown binary endianess if not found). </return>
    #pragma endregion  
   ENDIANESS interpretEndian(std::string end)
    {
        std::transform(end.begin(), end.end(), end.begin(), toupper); // convert to upper case

        ENDIANESS retval = VCT_UNK_ENDIAN;
        if      (end == "LITTLE_ENDIAN" || end == "LITTLE ENDIAN") retval = VCT_LITTLE_ENDIAN;
        else if (end == "MIDDLE_ENDIAN" || end == "MIDDLE ENDIAN") retval = VCT_MIDDLE_ENDIAN;
        else if (end == "BIG_ENDIAN"    || end == "BIG ENDIAN")    retval = VCT_BIG_ENDIAN;
        return retval;
    }

    #pragma region Documentation
    /// <summary>Convert a voxel body binary endianess enumeration instance to a string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="end">Voxel body binary endianess enumeration instance. </parm>
    /// <return>The corresponding string representation. </return>
    #pragma endregion  
    std::string interpretEndian(ENDIANESS end)
    {
        std::string retval("UNK_ENDIAN");
        if      (end == VCT_LITTLE_ENDIAN) retval = "LITTLE_ENDIAN";
        else if (end == VCT_MIDDLE_ENDIAN) retval = "MIDDLE_ENDIAN";
        else if (end == VCT_BIG_ENDIAN)    retval = "BIG_ENDIAN";
        return retval;
    }

private:

    std::string   varray_uid;      /// Voxel Array UID
    long long     voxel_num[3];    /// The count of voxels in each of 3 dimensions
    float         voxel_size[3];   /// The size of a single voxel in 3 dimensions (in meters)
    VOXEL_ORDER   voxel_order;     /// The order of the x, y, and z coordinates in the voxel body
    VOXEL_TYPE    voxel_type;      /// The data type used for the binary voxel body
    int           bits_per_voxel;  /// The number of bits used to represent a single voxel
    ENDIANESS     endian;          /// The endianess of the voxel body
    float         dir_cosines[9];  /// 3x3 direction cosines matrix for rotating the voxel body
    long long     total_bytes;     /// The size of the binary voxel body in bytes
    bool          deallocate;      /// A flag indicating whether voxel memory should be deallocaed upon exit
    unsigned char *voxels;         /// A pointer to the binary voxels
};

}