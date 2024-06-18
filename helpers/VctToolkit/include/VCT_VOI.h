// VCT_VOI.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_Date.h"
#include "VCT_Time.h"
#include "VCT_Organization.h"
#include "VCT_Software.h"
#include "VCT_Station.h"
#include <algorithm>
#include <map>


#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{


#pragma region Documentation
/// <summary> Volume of Interest class for Open Virtual Clinical Trials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class VOI
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    VOI()
        : voi_id(-1),                   // unique id within the phantom
          voi_unique_id("unknown_uid"), // overall unique id (serial number)
          center_x_vxl(0.0f), // centroid x (displacment from phantom origin - in voxels)
          center_y_vxl(0.0f), // centroid y (displacment from phantom origin - in voxels)
          center_z_vxl(0.0f), // centroid z (displacment from phantom origin - in voxels)

          width_vxls(0.0f),   // size in x (in voxels)
          height_vxls(0.0f),  // size in y (in voxels)
          depth_vxls(0.0f),   // size in z (in voxels)

          has_lesion(false)   // true if the VOI has a lesion
    {}
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    VOI(const VOI &rhs)
        : voi_id(rhs.voi_id),
          voi_unique_id(rhs.voi_unique_id),
          center_x_vxl(rhs.center_x_vxl),
          center_y_vxl(rhs.center_y_vxl),
          center_z_vxl(rhs.center_z_vxl),
          width_vxls(rhs.width_vxls),  
          height_vxls(rhs.height_vxls), 
          depth_vxls(rhs.depth_vxls),
          has_lesion(rhs.has_lesion)  
    {}
          


    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~VOI()
    {}
    

    // Manipulators (used to set component values)


    #pragma region Documentation
    /// <summary>Set the insertion's VOI ID. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">The VOI ID unique to (within) this phantom. </parm>
    #pragma endregion
    void setVoiId(int id)                { voi_id = id; }


    #pragma region Documentation
    /// <summary>Set the insertion's unique id (serial number). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">The VOI ID serial number (external to this phantom). </parm>
    #pragma endregion
    void setVoiUniqueId(std::string id)  { voi_unique_id = id; }


    #pragma region Documentation
    /// <summary>Set coordinates of the center of the VOI. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="x">The VOI center's x coordinate (mm). </parm>
    /// <parm name="y">The VOI center's y coordinate (mm). </parm>
    /// <parm name="z">The VOI center's z coordinate (mm). </parm>
    #pragma endregion
    void setCenterVxls(float x, float y, float z) { center_x_vxl = x; center_y_vxl = y; center_z_vxl = z; }


    #pragma region Documentation
    /// <summary>Set VOI dimensions. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="w">The VOI's width (X) in mm. </parm>
    /// <parm name="d">The VOI's height (Y) in mm. </parm>
    /// <parm name="h">The VOI's depth (Z) in mm. </parm>
    #pragma endregion
    void setBoundingBoxSize(float w, float d, float h) { width_vxls = w; height_vxls = d; depth_vxls = h; }


    #pragma region Documentation
    /// <summary>Set whether the VOI contains a lesion. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="hl">True if VOI has a lesion (otherwise false). </parm>
    #pragma endregion
    void setHasLesion(bool hl)      { has_lesion = hl; }

    
    // Accessors (used to retrieve component values)

    
    #pragma region Documentation
    /// <summary>Retrieve the insertion's input phantom filename. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The VOI ID unique to (within) this phantom. </return>
    #pragma endregion
    int              getVoiId()      { return voi_id; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's unique id (serial number). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The VOI ID serial number (external to this phantom). </return>
    #pragma endregion
    std::string     getVoiUniqueId() { return voi_unique_id; }


    #pragma region Documentation
    /// <summary>Retrieve coordinates of the center of the VOI. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="x">The VOI center's x coordinate (mm). </parm>
    /// <parm name="y">The VOI center's y coordinate (mm). </parm>
    /// <parm name="z">The VOI center's z coordinate (mm). </parm>
    #pragma endregion
    void getCenterVxls(float &x, float &y, float &z) { x = center_x_vxl; y = center_y_vxl; z = center_z_vxl; }


    #pragma region Documentation
    /// <summary>Retrieve VOI dimensions. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="w">The VOI's width (X) in mm. </parm>
    /// <parm name="h">The VOI's height (Y) in mm. </parm>
    /// <parm name="d">The VOI's depth (Z) in mm. </parm>
    #pragma endregion
    void getBoundingBoxSize(float &w, float &h, float &d) { w = width_vxls; h = height_vxls; d = depth_vxls; }


    #pragma region Documentation
    /// <summary>Retrieve whether the VOI contains a lesion. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>True if VOI has a lesion (otherwise false). </return>
    #pragma endregion
    bool getHasLesion()         { return has_lesion; }


       // XML Interaction routines


    #pragma region Documentation
    /// <summary>Write this insertion's section to the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        clib->writeNodeValueXML("VOI_Id", voi_id);
        clib->writeNodeValueXML("VOI_Unique_Id", voi_unique_id);

        if (clib->addASectionXML("VOI_Center"))
        {
            clib->writeNodeValueXML("X", center_x_vxl);
            clib->writeNodeValueXML("Y", center_y_vxl);
            clib->writeNodeValueXML("Z", center_z_vxl);
            clib->concludeSectionXML();
        }

        if (clib->addASectionXML("Dimensions"))
        {
            clib->writeNodeValueXML("Width", width_vxls);
            clib->writeNodeValueXML("Height", height_vxls);
            clib->writeNodeValueXML("Depth", depth_vxls);
            clib->concludeSectionXML();
        }

        clib->writeNodeValueXML("Has_Lesion", (has_lesion?"true":"false"));
        
        clib->concludeSectionXML();
    }

    
    #pragma region Documentation
    /// <summary>Read the insertion section from the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        clib->getNodeValueXML("VOI_ID", voi_id);
        clib->getNodeValueXML("VOI_UID", voi_unique_id);

        if (clib->findNodeXML("VOI_Center"))
        {
            clib->getNodeValueXML("X", center_x_vxl);
            clib->getNodeValueXML("Y", center_y_vxl);
            clib->getNodeValueXML("Z", center_z_vxl);
            clib->concludeSectionXML();
        } else std::cerr << "Couldn't find VOIT_Center section" << std::endl;

        if (clib->findNodeXML("Dimensions"))
        {
            clib->getNodeValueXML("Width",  width_vxls);
            clib->getNodeValueXML("Height", height_vxls);
            clib->getNodeValueXML("Depth",  depth_vxls);
            clib->concludeSectionXML();
        } else std::cerr << "Couldn't find Dimensions section" << std::endl;
        
        std::string hl("false");
        clib->getNodeValueXML("Has_Lesion", hl);
        has_lesion = (hl == "True" || hl == "true");

        /*
        std::cout << "\t VOI_ID " << voi_id << ":\n"
                  << "\t\t VOID UID:   " << voi_unique_id << "\n"
                  << "\t\t center X:   " << center_x_vxl << "\n"
                  << "\t\t center Y:   " << center_y_vxl << "\n"
                  << "\t\t center Z:   " << center_z_vxl << "\n"
                  << "\t\t Width:      " << width_vxls << "\n"
                  << "\t\t Height:     " << height_vxls << "\n"
                  << "\t\t Depth:      " << depth_vxls << "\n"
                  << "\t\t has_lesion: " << (has_lesion?"true":"false") << "\n";
        */
    }
    
private:

    int         voi_id;         // unique id within the phantom
    std::string voi_unique_id;  // overall unique id (serial number)

    float center_x_vxl; // centroid x (displacment from phantom origin - in voxels)
    float center_y_vxl; // centroid y (displacment from phantom origin - in voxels)
    float center_z_vxl; // centroid z (displacment from phantom origin - in voxels)

    float width_vxls;   // size in x (in voxels)
    float height_vxls;  // size in x (in voxels)
    float depth_vxls;   // size in x (in voxels)

    bool  has_lesion;   // true if the VOI is a sphere
};

}
