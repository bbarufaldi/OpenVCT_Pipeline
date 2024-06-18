// VCT_Insertion.h

#pragma once

#include "VCT_CommonLibrary.h"
#include "VCT_Software.h"
#include "VCT_Lesion.h"
#include "VCT_VOI.h"
#include "VCT_LesionMask.h"
#include <map>


#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{


#pragma region Documentation
/// <summary> Insertion class for Open Virtual Clinical Trials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class Insertion
{
public:
    
    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Insertion()
        : program_name(""),
          sw_version(""),
          phantom_name_in(""),
          phantom_name_out("")
    {
          lesion_map.clear();
          voi_map.clear();
    }
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Insertion(const Insertion &rhs)
        : program_name(rhs.program_name),
          sw_version(rhs.sw_version),
          phantom_name_in(rhs.phantom_name_in),
          phantom_name_out(rhs.phantom_name_out)
    {              
          lesion_map = rhs.lesion_map;
          voi_map = rhs.voi_map;
    }    
    
    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    virtual ~Insertion()
    {}	
    

    // Manipulators (used to set component values)


    #pragma region Documentation
    /// <summary>Set the insertion's program name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="name">The program name. </parm>
    #pragma endregion
    void setProgramName(std::string name)   { program_name = name; }


    #pragma region Documentation
    /// <summary>Set the insertion's Software Version. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="vers">The software version. </parm>
    #pragma endregion
    void setSWVersion(std::string vers)   { sw_version = vers; }


    #pragma region Documentation
    /// <summary>Set the input phantom filename. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="phin">The input phantom filename. </parm>
    #pragma endregion
    void setInputPhantom(std::string phin)   { phantom_name_in = phin; }


    #pragma region Documentation
    /// <summary>Set the output phantom filename. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="phout">The output phantom filename. </parm>
    #pragma endregion
    void setOutputPhantom(std::string phout)  { phantom_name_out = phout; }


    #pragma region Documentation
    /// <summary>Clear the VOI collection. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void clearVois()                { voi_map.clear(); }


    #pragma region Documentation
    /// <summary>Add a lesion to this insertion body. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="voi">An instance of Volume of Interest. </parm>
    #pragma endregion
    void addVoi(VOI voi)
    { 
        voi_map.insert(std::make_pair(voi.getVoiId(), voi));
    }


    #pragma region Documentation
    /// <summary>Clear the Lesion collection. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void clearLesions()             { lesion_map.clear(); }


    #pragma region Documentation
    /// <summary>Add a lesion to this insertion body. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="les">An instance of vct::Lesion. </parm>
    #pragma endregion
    void addLesion(Lesion les)
    { 
		int map_size = static_cast<int>(lesion_map.size());
        lesion_map.insert(std::make_pair(map_size, les)); 
    }

    
    // Accessors (used to retrieve component values)

    
    #pragma region Documentation
    /// <summary>Retrieve the insertion's program name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The program name. </return>
    #pragma endregion
    std::string getProgramName()    { return program_name; }


    #pragma region Documentation
    /// <summary>Retrieve the insertion's Software Version. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The software version. </return>
    #pragma endregion
    std::string getSWVersion()      { return sw_version; }


    #pragma region Documentation
    /// <summary>Retrieve the input phantom filename. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The input phantom filename. </return>
    #pragma endregion
    std::string getInputPhantom()   { return phantom_name_in; }


    #pragma region Documentation
    /// <summary>Retrieve the output phantom filename. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The output phantom filename. </return>
    #pragma endregion
    std::string getOutputPhantom()  { return phantom_name_out; }


    // XML file Interaction 


    #pragma region Documentation
    /// <summary>Write this insertion's section to the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->writeNodeValueXML("Program_Name", program_name);
            clib->writeNodeValueXML("SW_Version", sw_version);
            clib->writeNodeValueXML("Phantom_Name_In", phantom_name_in);
            clib->writeNodeValueXML("Phantom_Name_Out", phantom_name_out);

            std::cout << "About to write VOI xml. voi_map contains " << voi_map.size() 
                      << " vois" << std::endl;


            if (clib->addASectionXML("VOIs"))
            {
                for(auto &&iter : voi_map)
                {
                    clib->addASectionXML("VOI");
                    iter.second.writeXML(clib);
                }   
                clib->concludeSectionXML();     
            }
            
            if (clib->addASectionXML("Lesions"))
            {
                for(auto &&iter : lesion_map)
                {
                    clib->addASectionXML("Lesion");
                    iter.second.writeXML(clib);
                }
                clib->concludeSectionXML();     
            }
            clib->concludeSectionXML();  
        }
    }

    
     #pragma region Documentation
    /// <summary>Read the insertion section from the current XML doc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
			if (clib->findNodeXML("VOIs"))
            {
			    if (clib->firstChildNodeXML() == true)
			    {
				    VOI voi;
                    voi.readXML(clib);
				    voi_map.insert(std::make_pair(voi.getVoiId(), voi));
			    }

			    while(clib->nextNodeXML())
                {
                    VOI voi;
                    voi.readXML(clib);
				    voi_map.insert(std::make_pair(voi.getVoiId(), voi));
                }        
                clib->concludeSectionXML();
            }

			if (clib->findNodeXML("Lesions"))
            {
			    if (clib->firstChildNodeXML() == true)
			    {
				    Lesion lesion;
                    lesion.readXML(clib);
					int map_size = static_cast<int>(lesion_map.size());
                    lesion.setLesionId(map_size);
				    lesion_map.insert(std::make_pair(map_size, lesion));
			    }

			    while(clib->nextNodeXML())
                {
                    Lesion lesion;
                    lesion.readXML(clib);
					int map_size = static_cast<int>(lesion_map.size());
                    lesion.setLesionId(map_size);
				    lesion_map.insert(std::make_pair(map_size, lesion));
                }        
                clib->concludeSectionXML();
            }
            clib->concludeSectionXML();
        }
    }
    

private:

    std::string program_name;
    std::string sw_version;
    std::string phantom_name_in;
    std::string phantom_name_out;

    // Lesion specific

    std::map<int, Lesion> lesion_map;
    std::map<int, VOI> voi_map;
};

}
