// VCT_IndexTable.h

#pragma once


#include "VCT_CommonLibrary.h"
#include <string>
#include <map>
#include <vector>


#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{


#pragma region Documentation
/// <summary> Material class for managing index materials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class Material
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="name">The material's name. </parm>
    /// <parm name="wt">The material's weight (% of this label). </parm>
    /// <parm name="dens">The material's density. </parm>
    /// <parm name="matZ">The material's Z value. </parm>
    #pragma endregion
    Material(std::string name, float wt, float dens, int matZ) 
        : name(name), 
          weight(wt), 
          density(dens), 
          materialZ(matZ)
    {}

    #pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Material() : name("Air"), weight(1.0f), density(0.0012f), materialZ(201)
    {}
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Material(const Material &rhs)
        : name(rhs.name),
          weight(rhs.weight),
          density(rhs.density),
          materialZ(rhs.materialZ)
    {}


    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    ~Material()
    {}
	
	
	// Manipulators (used to set component values)


    #pragma region Documentation
    /// <summary>Set the material's name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="mat_name">The material's name. </parm>
    #pragma endregion
    void setName(std::string mat_name)  { name = mat_name; }


    #pragma region Documentation
    /// <summary>Set the material's weight (% of this label). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="wt">The material's weight (% of this label). </parm>
    #pragma endregion
    void setWeight(float wt)            { weight = wt; }


    #pragma region Documentation
    /// <summary>Set the material's density. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="dens">The material's density. </parm>
    #pragma endregion
    void setDensity(float dens)         { density = dens; }

    
    #pragma region Documentation
    /// <summary>Set the material's Z value. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="matZ">The material's Z value. </parm>
    #pragma endregion
    void setMaterialZ(int matZ)          { materialZ = matZ; }


    // Accessors (used to retrieve component values)

    
    #pragma region Documentation
    /// <summary>Retrieve the material's name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The material's name. </return>
    #pragma endregion
    std::string getName()               { return name; }


    #pragma region Documentation
    /// <summary>Retrieve the material's weight (% of this label). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The material's weight (% of this label). </return>
    #pragma endregion
    float getWeight()                   { return weight; }


    #pragma region Documentation
    /// <summary>Retrieve the material's density. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The material's density. </return>
    #pragma endregion
    float getDensity()                  { return density; }

    
    #pragma region Documentation
    /// <summary>Retrieve the material Z value. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The material's Z value. </return>
    #pragma endregion
    int getMaterialZ()                  { return materialZ; }


    #pragma region Documentation
    /// <summary>Read the XML section pertaining to IndexTable. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->getNodeValueXML("Material_Name", name);
            clib->getNodeValueXML("Material_Weight", weight);
            clib->getNodeValueXML("Material_Density", density);
            clib->getNodeValueXML("Material_MaterialZ", materialZ);
        }
    } 

    
    #pragma region Documentation
    /// <summary>Write this IndexTable's XML section. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clib->writeNodeValueXML("Material_Name", name);
            clib->writeNodeValueXML("Material_Weight", weight);
            clib->writeNodeValueXML("Material_Density", density);
            clib->writeNodeValueXML("Material_MaterialZ", materialZ);
        }
    }


private:

    std::string name;      /// name of this material
    float       weight;    /// percentage of the parent label
    float       density;   /// nist override
    int         materialZ; /// nist material (atomic number)
};



#pragma region Documentation
/// <summary> Label class for managing labels (indicies) and their materials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class Label
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Label() : label_id(-1)
    {}

    #pragma region Documentation
    /// <summary>(parametric)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">The Label's ID. </parm>
    /// <parm name="mat">A single Material to add. </parm>
    #pragma endregion
    Label(int id, Material mat) 
        : label_id(id)          
    {
        addMaterial(mat);
    }
        
        
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Label(const Label &rhs)
        : label_id(rhs.label_id)
    {
        materials = rhs.materials;
    }
    
    
	#pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    ~Label()
    {
        materials.clear();
    }


    #pragma region Documentation
    /// <summary>Clear the Label's materials collection. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void clearMaterials()           { materials.clear(); }

    
    // Manipulators (used to set component values)


    #pragma region Documentation
    /// <summary>Set the Label's ID. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">The Label's ID. </parm>
    #pragma endregion
    void setId(int id)              { label_id = id; }

    
    #pragma region Documentation
    /// <summary>Add a Material to this Label. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="mat">The Material to add. </parm>
    #pragma endregion
    void addMaterial(Material mat)  { materials.push_back(mat); }

    
    // Accessors (used to retrieve component values)

    
    #pragma region Documentation
    /// <summary>Retrieve this Label's ID. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The label's ID. </return>
    #pragma endregion
    int getId()                     { return label_id; }

    
    #pragma region Documentation
    /// <summary>Retrieve the number of this Label's materials. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The number of materials represented by this label. </return>
    #pragma endregion
    int getNumMaterials()           { return (int) materials.size(); }

    
    #pragma region Documentation
    /// <summary>Retrieve the specified material from this Label. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ndx">The index of the material to retrieve (indicies begin at 0).</parm>
    /// <return>The number of materials represented by this label. </return>
    Material getMaterial(int ndx)  
    { 
        Material retval;
        if (ndx >= 0 && ndx < materials.size())
        {
            retval = materials[ndx];
        }
        return retval;
    }



    #pragma region Documentation
    /// <summary>Read the XML section pertaining to this Label. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            clearMaterials();
			int num_mats;

            clib->getNodeValueXML("Index_ID", label_id);
			clib->getNodeValueXML("Number_Of_Materials", num_mats);
            if (clib->findNodeXML("Materials"))
            {
                Material mat;
			
                if (clib->findNodeXML("Material"))
                {
                    mat.readXML(clib);
					//std::cout << "\tMaterial found. \tname:" << mat.getName() << ", density:" << mat.getDensity()
					//          << ", wt:" << mat.getWeight() << "Z:" << mat.getMaterialZ() << std::endl;
                    addMaterial(mat);
                }
				
				while(clib->nextNodeXML())
                {
                    mat.readXML(clib);
					//std::cout << "\tMaterial found. \tname:" << mat.getName() << ", density:" << mat.getDensity()
					//          << ", wt:" << mat.getWeight() << "Z:" << mat.getMaterialZ() << std::endl;
                    addMaterial(mat);
                }
				clib->concludeSectionXML();
            }
        }
    } 

    
    #pragma region Documentation
    /// <summary>Write this IndexTable's XML section. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            int num_materials = (int)materials.size();
            clib->addASectionXML("Index");
            clib->writeNodeValueXML("Index_ID", label_id);
            clib->writeNodeValueXML("Number_Of_Materials", num_materials);
            clib->addASectionXML("Materials");
            for(auto &&iter : materials)
            {
                clib->addASectionXML("Material");
                iter.writeXML(clib);
                clib->concludeSectionXML();
            }
            clib->concludeSectionXML();            
            clib->concludeSectionXML();
        }
    }

private:

    int   label_id;                  /// Index value (voxel value in a voxel array phantom)
    std::vector<Material> materials; /// collection of corresponding materials
};


#pragma region Documentation
/// <summary> IndexTable class for managing labels (indicies) and their materials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class IndexTable
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    IndexTable() : unique_id("unknown uid"), data_source("NIST"), max_value(0U)
    {}
        
        
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    IndexTable(const IndexTable &rhs)
        : unique_id(rhs.unique_id),
          max_value(rhs.max_value),
          data_source(rhs.data_source)
    {
       labelMap = rhs.labelMap;
    }
    
    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    ~IndexTable()
    {
        data_source.clear();
    }

    
    // Manipulators (used to set component values)


    #pragma region Documentation
    /// <summary>(Re)initialize - zero-out - the collection of indicies. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void clear() { labelMap.clear(); }


    #pragma region Documentation
    /// <summary>Add a label to the index table. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="label">A valid Label. </parm>
    #pragma endregion
    void addLabel(Label label)
    { 
        unsigned int index = label.getId();
        if (index > max_value) max_value = index;
        labelMap.insert(std::make_pair(index, label)); 
    }


    #pragma region Documentation
    /// <summary>Set the index table UID. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="uid">The index table UID. </parm>
    #pragma endregion
    void setUID(std::string uid) { unique_id = uid; }


    #pragma region Documentation
    /// <summary>Set the data source. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="source">The data source. </parm>
    #pragma endregion
    void setDataSource(std::string source) { data_source = source; }


    // Accessors (used to retrieve component values)
    

    #pragma region Documentation
    /// <summary>Retrieve the index table UID. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The index table UID. </return>
    #pragma endregion
    std::string getUID() { return unique_id; }
    

    #pragma region Documentation
    /// <summary>Retrieve the largest index number. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The largest index number. </return>
    #pragma endregion
    int getLargestIndex() { return max_value; }

    
    #pragma region Documentation
    /// <summary>Retrieve the data source. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>The data source. </return>
    #pragma endregion
    std::string getDataSource()             { return data_source; }

    
    #pragma region Documentation
    /// <summary>Retrieve the specified label (by ID). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">The id of the label to retrieve.</parm>
    /// <return>The corresponding label if it exists (otherwise and empty Label). </return>
    #pragma endregion
    Label getLabel(int id)
    {
        Label retval;
        auto iter = labelMap.find(id);
        if (iter != labelMap.end())
        {
            retval = iter->second;
        }
        return retval;
    }
    

    #pragma region Documentation
    /// <summary>Retrieve the labels which contain the specified material. </summary>
    /// <remarks>D. Higginbotham, 2017-07-06. </remarks>
    /// <parm name="mat">The material to search for.</parm>
    /// <return>The label id and material weight of each label containing the specified material. </return>
    #pragma endregion
    std::map<int, float> findLabelsWithThisMaterial(int mat)
    {
        std::map<int, float> retdata;

        for(auto &lbl : labelMap)
        {
            for(int i=0; i<lbl.second.getNumMaterials(); ++i)
            {
                Material m = lbl.second.getMaterial(i);
                if (m.getMaterialZ() == mat)
                {
                    retdata.insert(std::make_pair(lbl.first, m.getWeight()));
                }
            }
        }

        return retdata;
    }


    // XML file Interaction 


    #pragma region Documentation
    /// <summary>Read the XML section pertaining to IndexTable. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void readXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            size_t num_indicies = labelMap.size();
            clib->getNodeValueXML("Index_Table_UID", unique_id);
            clib->getNodeValueXML("Maximum_Index", max_value);
            clib->getNodeValueXML("Number_of_Indicies", num_indicies);

			if (clib->findNodeXML("Index"))
			{
				Label label;
                label.readXML(clib);
				labelMap.insert(std::make_pair(label.getId(), label));
			}

			while(clib->nextNodeXML("Index"))
            {
                Label label;
                label.readXML(clib);
				labelMap.insert(std::make_pair(label.getId(), label));
            }
            clib->concludeSectionXML();
            clib->getNodeValueXML("Data_Source", data_source);
        }
    } 

    
    #pragma region Documentation
    /// <summary>Write this IndexTable's XML section. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="clib">CommonLibrary pointer (for current context). </parm>
    #pragma endregion
    void writeXML(CommonLibrary *clib)
    {
        if (clib != nullptr)
        {
            size_t num_indicies = labelMap.size();
            clib->writeNodeValueXML("Index_Table_UID", unique_id);
            clib->writeNodeValueXML("Maximum_Index", max_value);
            clib->writeNodeValueXML("Number_of_Indicies", num_indicies);
            for(auto iter : labelMap)
            {
                iter.second.writeXML(clib);
            }
            clib->writeNodeValueXML("Data_Source", data_source);
        }
    }

private:

    std::string          unique_id;   /// index table unique identifier
    std::map<int, Label> labelMap;    /// container of indicies (labels) and their associated data
    unsigned int         max_value;   /// largest index number
    std::string          data_source; /// data source; e.g., NIST
};

}
