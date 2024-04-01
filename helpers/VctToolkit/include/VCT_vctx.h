// VCT_vctx.h

#pragma once

#include "VCT_CommonLibrary.h"

#include "VCT_Trial.h"
#include "VCT_Subject.h"
#include "VCT_Phantom.h"
#include "VCT_LesionMask.h"
#include "VCT_Lesion.h"
#include "VCT_PrivateData.h"

#include "ZipFile.h"
#include "ZipArchive.h"
#include "ZipArchiveEntry.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdio>

#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

const std::string EXTENSION(".vctx");
const std::string PRIVATEDIR("Private/");


#pragma region Documentation
/// <summary> High level interface class for Open Virtual Clinical Trials. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class Vctx
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Vctx()
        : verbose(false),
          leave_temp(false),
          okay_to_use_temp_folder(false),
		  created_phantom(false),
          have_uncompressed_path(false),
          extension(EXTENSION),
          trial(nullptr),
          subject(nullptr),
          phantom(nullptr),
          clib(nullptr)
    {
        lesionMaskMap.clear();
        lesionMap.clear();
        imageFileMap.clear();
        private_data.clear();
        clib = new CommonLibrary;

        #if defined(_MSC_VER)
            std::string suffix("/AppData/Local/Temp/");
            char *userprofile = getenv("USERPROFILE");
            if (verbose) std::cout << "UserProfile is " << userprofile << std::endl;
            std::string userpro(userprofile);
            clib->swapall(userpro, '\\', "/");
            //TEMPDIR = userpro + suffix;
            TEMPDIR = "./";
            if (verbose) std::cout << "TEMPDIR is " << TEMPDIR << std::endl;
        #else
            TEMPDIR = "./";
        #endif
    }
    
    
    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    Vctx(const Vctx &rhs)
        : verbose(rhs.verbose),
          leave_temp(rhs.leave_temp),
          okay_to_use_temp_folder(rhs.okay_to_use_temp_folder),
		  created_phantom(rhs.created_phantom),
          have_uncompressed_path(rhs.have_uncompressed_path),
          phantom_name(rhs.phantom_name),
          phantom_filename(rhs.phantom_name),
          extension(rhs.extension),
          trial(nullptr),
          subject(nullptr),
          phantom(nullptr),
          clib(nullptr),
          xml_topdir(rhs.xml_topdir),
          tmp_xml_path(rhs.tmp_xml_path),
          tmp_xml_private_name(rhs.tmp_xml_private_name),
          vctx_file_in(rhs.vctx_file_in),
          vctx_file_out(rhs.vctx_file_out),
          abs_phantomXml(rhs.abs_phantomXml),
          abs_phantomDat(rhs.abs_phantomDat),
          rel_phantomXml(rhs.rel_phantomXml),
          rel_phantomDat(rhs.rel_phantomDat),
          abs_private_path_in(rhs.abs_private_path_in),
          abs_private_path_out(rhs.abs_private_path_out)  
    {
        if (rhs.trial != nullptr)    trial   = new vct::Trial(*rhs.trial);
        if (rhs.subject != nullptr)  subject = new vct::Subject(*rhs.subject);
        if (rhs.phantom != nullptr)  phantom = new vct::Phantom(*rhs.phantom);
        if (rhs.clib != nullptr)     clib    = new vct::CommonLibrary(*rhs.clib);
    
        lesionMaskMap = rhs.lesionMaskMap;
        lesionMap     = rhs.lesionMap;
        imageFileMap  = rhs.imageFileMap;
        private_data  = rhs.private_data;

        abs_private_entities = rhs.abs_private_entities;
        rel_private_entities = rhs.rel_private_entities;
        abs_privateXML = rhs.abs_privateXML;
        rel_privateXML = rhs.rel_privateXML;
        zippedEntities = rhs.zippedEntities;
        del_paths = rhs.del_paths;
    }

    #pragma region Documentation
    /// <summary>Delete temporary files and directories. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void DeletePaths()
    {
        if (clib != nullptr && del_paths.size() > 0 && leave_temp == false)
        {
            // Remove private files
            for (auto pf : abs_privateXML)
            {
                #if defined(_MSC_VER)
                clib->swapall(pf, '/', "\\");
                #endif
                int result = remove(pf.c_str());
                if (verbose && result != 0)
                {
                    std::cerr << "Couldn't remove " << pf << std::endl;
                }
                else
                {
                    del_paths.clear();
                }
            }

            // Recursively remove paths
            int bad_count = 0;
            for(auto dp : del_paths)
            {
                #if defined(_MSC_VER)
                clib->swapall(dp, '/', "\\");
                #endif
                if (verbose) std::cout << "Line " << __LINE__ << ": Calling clib->deleteDirectory("
                                       << dp << ");" << std::endl;
                bool status = clib->deleteDirectory(dp);
                if (status)
                {
                    if (verbose) std::cout << "path: " << dp << " is successfully deleted"
                                           << std::endl;
                }
                else
                {
                    bad_count++;
                    std::cerr << "* Warning: Couldn't delete path \"" << dp << "\"" << std::endl;
                }
            }
            if (bad_count == 0) del_paths.clear();
        }
    }
    
    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    ~Vctx()
    {
        if (verbose) std::cout << "leave_temp = " << (leave_temp?"True":"False") << std::endl;
        if (clib != nullptr && del_paths.size() > 0 && leave_temp == false)
        {
            // Remove private files
            for (auto pf : abs_privateXML)
            {
                #if defined(_MSC_VER)
                clib->swapall(pf, '/', "\\");
                #endif
                int result = remove(pf.c_str());
                if (verbose && result != 0) std::cerr << "Couldn't remove " << pf << std::endl;
            }

            // Recursively remove paths
            for(auto dp : del_paths)
            {
                #if defined(_MSC_VER)
                clib->swapall(dp, '/', "\\");
                #endif
                
                if (verbose) std::cout << "Line " << __LINE__ << ": Calling clib->deleteDirectory("
                                       << dp << ");" << std::endl;
                bool status = clib->deleteDirectory(dp);
                if (status)
                {
                    if (verbose) std::cout << "path: " << dp << " is successfully deleted" 
                                           << std::endl;
                }
                else
                {
                    std::cerr << "* Error: Couldn't delete path \"" << dp << "\"" << std::endl;
                }
            }

            delete clib;
            clib = nullptr;
        }
        if (created_phantom && phantom != nullptr)
        {
            delete phantom;
            phantom = nullptr;
        }
    }


    #pragma region Documentation
    /// <summary>Enable or disable verbose status messaging. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="state">When true enables verbose mode, otherwise disables verbose mode. </parm>
    #pragma endregion
    void setVerbose(bool state)          { verbose = state; }


    #pragma region Documentation
    /// <summary>Enable or disable flag to leave phantom in temporary folder. </summary>
    /// <remarks>D. Higginbotham, 2017-09-25. </remarks>
    /// <parm name="state">When true prevents removal of phantom from temporary folder when done. </parm>
    #pragma endregion
    void setLeaveTemp(bool state)        { leave_temp = state; }


    #pragma region Documentation
    /// <summary>Enable or disable flag to use phantom left in temporary folder. </summary>
    /// <remarks>D. Higginbotham, 2017-09-25. </remarks>
    /// <parm name="state">When true permits use of phantom left in its temporary folder. </parm>
    #pragma endregion
    void setOkayToUseTemp(bool state)    { okay_to_use_temp_folder = state; }


    #pragma region Documentation
    /// <summary>Set the internal phantom filename. </summary>
    /// <remarks>D. Higginbotham, 2019-01-25. </remarks>
    /// <parm name="name">New internal phantom filename. </parm>
    #pragma endregion    
    void setPhantomName(std::string name) { phantom_name = name; }


    #pragma region Documentation
    /// <summary>Set the VCT Trial object. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="trial_ptr">Address of a valid VCT Trial object. </parm>
    #pragma endregion    
    void setTrial(Trial *trial_ptr)      { trial = trial_ptr; }
    
    
    #pragma region Documentation
    /// <summary>Set the VCT Subject object. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="sub_ptr">Address of a valid VCT Subject object. </parm>
    #pragma endregion  
    void setSubject(Subject *sub_ptr)    { subject = sub_ptr; }
    
    
    #pragma region Documentation
    /// <summary>Set the VCT Phantom object. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ph_ptr">Address of a valid VCT Phantom object. </parm>
    #pragma endregion  
    void setPhantom(Phantom *ph_ptr)     { phantom = ph_ptr; }


    #pragma region Documentation
    /// <summary>Retrieve flag to set verbose status messaging. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>Boolean which when true, enables verbose mode, otherwise disables verbose mode. </return>
    #pragma endregion
    bool getVerbose()                    { return verbose; }


    #pragma region Documentation
    /// <summary>Retrieve flag to leave phantom in temporary folder. </summary>
    /// <remarks>D. Higginbotham, 2017-09-25. </remarks>
    /// <return>Boolean which when true, prevents removal of phantom from temporary folder when done. </return>
    #pragma endregion
    bool getLeaveTemp()                  { return leave_temp; }


    #pragma region Documentation
    /// <summary>Retrieve flag to use phantom left in temporary folder. </summary>
    /// <remarks>D. Higginbotham, 2017-09-25. </remarks>
    /// <return>Boolean which when true permits use of phantom left in its temporary folder. </return>
    #pragma endregion
    bool getOkayToUseTemp(bool state)    { return okay_to_use_temp_folder; }


    #pragma region Documentation
    /// <summary>Retrieve the internal phantom filename. </summary>
    /// <remarks>D. Higginbotham, 2019-01-25. </remarks>
    /// <return>New internal phantom filename. </return>
    #pragma endregion    
    std::string getPhantomName()         { return phantom_name; }

    
    #pragma region Documentation
    /// <summary>Get a pointer to the VCT Trial object. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>Address of a valid VCT Trial object. </return>
    #pragma endregion  
    vct::Trial *getTrial()               { return trial;   }
    
    
    #pragma region Documentation
    /// <summary>Get a pointer to the VCT Subject object. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>Address of a valid VCT Subject object. </return>
    #pragma endregion 
    vct::Subject *getSubject()           { return subject; }
    
    
    #pragma region Documentation
    /// <summary>Get a pointer to the VCT Phantom object. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>Address of a valid VCT Phantom object. </return>
    #pragma endregion
    vct::Phantom *getPhantom()           { return phantom; }
    
    
    #pragma region Documentation
    /// <summary>Get the temporary path for a temporarily uncompressed VCT Phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>Path to the temporarily uncompressed VCT Phantom. </return>
    #pragma endregion
    std::string getTempDir()             { return TEMPDIR; }
    
    
    #pragma region Documentation
    /// <summary>Get the temporary path for a VCT input Phantom's Private folder. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>Path to the temporarily uncompressed VCT Phantom's Private folder. </return>
    #pragma endregion
    std::string getPrivateDirIn()        { return abs_private_path_in; }
    
    
    #pragma region Documentation
    /// <summary>Get the temporary path for a VCT output Phantom's Private folder. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <return>Path to the temporarily uncompressed VCT Phantom's Private folder. </return>
    #pragma endregion
    std::string getPrivateDirOut()       { return abs_private_path_out; }


    #pragma region Documentation
    /// <summary>Add a new instance of private data. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="pd">Address of a valid instance of VCT Private data. </parm>
    #pragma endregion  
    void addPrivateData(PrivateData *pd)
    { 
        if (pd != nullptr)
        {
            private_data.insert(std::make_pair(pd->getProgramName(), pd)); 
        }
    }

    
    #pragma region Documentation
    /// <summary>Clear all (reset) private data references. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion  
    void clearPrivateData()
    {
        private_data.clear();
    }
    

    #pragma region Documentation
    /// <summary>Retrieve an instance of private data by name. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="name">Name of the private instance. </parm>
    /// <return>The address of the specified instance of private data (or nullptr if not found). </return>
    #pragma endregion  
    PrivateData *getPrivateData(std::string name)
    {
        PrivateData *retval = nullptr;
        auto iter = private_data.find(name);
        if (iter != private_data.end())
        {
            retval = iter->second;
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Read the specified phantom vctx file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="archive">Name of the phantom vctx file. </parm>
    /// <return>True if the phantom is successfully uncompressed and read (otherwise false). </return>
    #pragma endregion  
    bool readVctx(std::string archive)
    {
        bool retval = false;

        phantom_name = archive;      // This will be overwritten by the internally specified phantom name
        phantom_filename = archive;  // This is the operating system filename

        std::string path_only = TEMPDIR;
        if (verbose)
        {
            std::cout << __FUNCTION__ << ": phantom_name (temporarily) is \"" << phantom_name << "\"\n";
            std::cout << __FUNCTION__ << ": phantom_filename is \"" << phantom_filename << "\"\n";
            std::cout << __FUNCTION__ << ": uncompressed path is \"" << path_only << "\"" << std::endl;
        }

        // Temporarily strip off file extension so filename matches temporary folder 
        std::string phname_no_ext(phantom_name);
        std::string::size_type pos = phname_no_ext.rfind(EXTENSION);
        if (pos != std::string::npos) phname_no_ext = phname_no_ext.substr(0, pos);
        if (verbose)
        {
            std::cout << __FUNCTION__ << ", line " << __LINE__ << ": phname_no_ext is \"" << phname_no_ext << "\"" << std::endl;
        }

        // Check if phantom exists in its temporary folder
        if (okay_to_use_temp_folder)
        {
            bool isPath(false);
        
            std::string temp_folder = TEMPDIR + phname_no_ext;
            if (verbose)
            {
                std::cout << __FUNCTION__ << ", line " << __LINE__
                          << ": temp_folder is \"" << temp_folder << "\"" 
                          << std::endl;
            }
            
            if (clib->testPath(temp_folder, isPath) && isPath)
            {
                have_uncompressed_path = discoverTemp(temp_folder, phname_no_ext);
                if (verbose)
                {
                    std::cout << __FUNCTION__ << ", line " << __LINE__ << ": temp_folder is \"" << temp_folder
                        << "\", phname_no_ext is \"" << phname_no_ext << "\", have_uncompressed_path is "
                        << have_uncompressed_path << std::endl;
                }
            }

            // Note: have_uncompressed_path is set within the uncompressZip function when successful
            if (!have_uncompressed_path)
            {
                // Overwrite internal phantom name
                //std::string phantom_name = uncompressZip(path_only, archive);
                phantom_name = uncompressZip(path_only, archive);
                if (verbose)
                {
                    std::cout << __FUNCTION__ << ", line " << __LINE__
                              << ": path_only: " << path_only
                              << ", uncompressZip returned phantom name \"" 
                              << phantom_name << "\"" << std::endl;
                }
            }
        }
        else
        {
            // Uncompress into temporary location if not yet done
            phantom_name = uncompressZip(path_only, archive);
            if (verbose) 
            {
                std::cout << "Line " << __LINE__ 
                          << ": NOT OKAY TO USE TEMP FOLDER, path_only: " 
                          << path_only << ", archive: " << archive
                          << std::endl;
            }
        }


        // Need to read directory structure to discover what there is
        // ..at a minimum there will be a set of phantom files
        if (have_uncompressed_path)
        {
            // Ensure that specified path is preferred for temporary files
            phname_no_ext = phantom_name;
            auto pos = phname_no_ext.rfind(extension);
            if (pos != std::string::npos) phname_no_ext = phname_no_ext.substr(0, pos);
            if (verbose)
            {
                std::cout << "Line " << __LINE__ 
                          << ": phname_no_ext is \"" << phname_no_ext << "\"" 
                          << ", path_only: " << path_only << std::endl;
            }
            
            pos = path_only.rfind(EXTENSION);
            if (pos != std::string::npos) path_only = path_only.substr(0, pos);
            pos = path_only.find_last_of("\\/");  if (verbose) std::cout << "* pos: " << pos << " *\n";
            if (pos == std::string::npos || pos < path_only.size()-1)  path_only = path_only + "/";
            
            if (verbose) std::cout << "\n* path_only: " << path_only 
                                   << ", phname_no_ext: " << phname_no_ext << " *\n";
            
            std::string del_path = path_only + phname_no_ext; // phantom_name;
            del_paths.push_back(del_path);
            abs_private_path_in = del_path + PRIVATEDIR;
            
            std::string last_char = del_path.substr(del_path.length()-1);            // DH 2020-07-19
            if (last_char != "/") abs_private_path_in = del_path + "/" + PRIVATEDIR; // DH 2020-07-19            
            
            if (verbose) 
            {
                std::cout << __FUNCTION__ << ", line " << __LINE__ 
                          << ", path_only: " << path_only << ", del_path: " << del_path
                          << ", abs_private_path = " << abs_private_path_in << std::endl;
            }

            std::string phantomXml = del_path + "/Phantom.xml";
            std::string phantomDat = del_path + "/Phantom.dat";

            if (verbose) std::cout << "Attempting to load the phantom: \"" << phantomXml << "\"" << std::endl;
            
            if (clib->loadXML(phantomXml))
            {
                if (phantom == nullptr)
                {
                    if (verbose) std::cout << "allocating phantom" << std::endl;
                    phantom = new vct::Phantom;
                }
                if (phantom != nullptr)
                {
					created_phantom = true;
                    phantom->setCommonLib(clib);
                    phantom->readXML();
                    phantom->readPhantomData(phantomDat);   // Read the voxel array
                    retval = true;
                }
            } else std::cerr << __FUNCTION__ << ": * Error: Could not load phantom XML file \"" << phantomXml << "\"" << std::endl;
        } else std::cerr << __FUNCTION__ << ": * Error: Don't have uncompressed path" <<  std::endl;

        return retval;
    }

    
    #pragma region Documentation
    /// <summary>Write the specified phantom files to the specified vctx file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="archive">Name of the phantom vctx file (incl ".vctx"). </parm>
    /// <parm name="path">Where to write the phantom (path). </parm>
    /// <return>True if the phantom is successfully uncompressed and read (otherwise false). </return>
    #pragma endregion  
    void writeVctx(std::string archive, std::string path)
    {
        abs_private_path_out = TEMPDIR + archive + "/" + PRIVATEDIR;

        if (phantom == nullptr)  
        {  
            std::cerr << "NO PHANTOM EXISTS" << std::endl; return; 
        }

        // At this point the original phantom is in the folder: C:\users\<username>\AppData\Local\Temp\<input_phantom_name>
        // If possible, need to rename that folder to the output phantom name, now.

        if (phantom != nullptr)
        {
            xml_topdir = archive;//path;
            tmp_xml_path = TEMPDIR + archive;//path;

            // Delete this path if it already exists
            bool isAPath;
            bool exists = clib->testPath(tmp_xml_path, isAPath);
            if (!exists || !isAPath)
            {
                // Create the path for this session
                if (!isAPath)
                {
                    if (verbose) 
                    {
                        std::cout << "Attempting to remove temporary file \"" 
                                  << tmp_xml_path << "\"" << std::endl;
                    }
                    remove(tmp_xml_path.c_str());
                }
                exists = clib->createPath(tmp_xml_path);
            }
                    
            rel_phantomXml = archive + "/Phantom.xml"; //path + "/Phantom.xml";
            rel_phantomDat = archive + "/Phantom.dat";//path + "/Phantom.dat";

            abs_phantomXml = TEMPDIR + rel_phantomXml;
            abs_phantomDat = TEMPDIR + rel_phantomDat;       
            
            if (verbose) 
            {
                std::cout << __FUNCTION__ << ", line " << __LINE__ 
                          << ": path: " << path << ", TEMPDIR: " << TEMPDIR
                          << ", abs_phantomXml: " << abs_phantomXml
                          << ", abs_phantomDat: " << abs_phantomDat
                          << std::endl;
            }
                              
            if (clib->initializeXML("VCT_Phantom"))
            {
                phantom->setCommonLib(clib); // prepare to write
                phantom->writePhantomXML();  // create the XML document structure

                clib->saveXML(abs_phantomXml);             // Write the actual XML file out

                phantom->writePhantomData(abs_phantomDat); // Write the binary data file out

                // Write any private XML data
                writeVctxPrivate(archive);//(path);

                // Create the Zip File
                if (archive.find(extension) == std::string::npos) archive += extension;
                vctx_file_out = path;//archive;

                // Don't create zip file if leaving the phantom in its temporary folder
                if (verbose) std::cout << __FUNCTION__ << ": leave_temp flag is " << (leave_temp?"True":"False") << std::endl;
                if (!leave_temp) buildZipFile();

                clib->concludeXML();
            }
            else
            {
                std::cerr << "writeXML(VCT_PHANTOM) failed" << std::endl;
            }
        }
        else
        {
            std::cerr << "phantom is null" << std::endl;
        }
    }

	
    #pragma region Documentation
    /// <summary>Write the private data files associated with the current phantom. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="path">Where to write the phantom (path). </parm>
    #pragma endregion  
    void writeVctxPrivate(std::string path)
	{
        auto pos = tmp_xml_path.find_last_of("\\/");
        if (verbose) std::cout << "pos: " << pos << "\n";
        if (pos == std::string::npos || pos < tmp_xml_path.size() - 1) tmp_xml_path = tmp_xml_path + "/";
        pos = path.find_last_of("\\/");
        if (pos == std::string::npos || pos == 0) path = path + "/";
        
        if (verbose) 
        {
            std::cout << __FUNCTION__ << ", line " << __LINE__ 
                      << ": path: " << path << ", tmp_xml_path: " << tmp_xml_path << '\n'
                      << ", abs_private_path_out: " << abs_private_path_out 
                      << std::endl;
        }

        std::string vctx_private_path = tmp_xml_path + PRIVATEDIR;
        
        if (verbose) std::cout << "path is \"" << path << "\"" << std::endl;
        if (verbose) std::cout << "tmp_xml_path is \"" << tmp_xml_path << "\"" << std::endl;
        if (verbose) std::cout << "vctx_private_path is \"" << vctx_private_path << "\"" << std::endl;

        // Create this path if it doesn't already exist
		bool isAPath;
		bool exists = clib->testPath(vctx_private_path, isAPath);
		if (!exists)
		{
            if (verbose) std::cout << "\nCreating private directory\n" << std::endl;
			exists = clib->createPath(vctx_private_path);
		}

        if (exists) // TODO: What does "exists" indicate exactly?
        {
            if (verbose)
            {
                std::cout << __LINE__ << ": PRIVATEDIR: " << PRIVATEDIR
                          << ", TEMPDIR: " << TEMPDIR << ", private_data.size(): " 
                          << private_data.size() << std::endl;
            }
		    for(auto iter = private_data.begin(); iter != private_data.end(); iter++)
		    {
                tmp_xml_private_name = iter->first;

                // Store paths
                std::string rpath = path + PRIVATEDIR + tmp_xml_private_name + ".xml";
                std::string apath = TEMPDIR + rpath; 
                rel_privateXML.push_back(rpath);
                abs_privateXML.push_back(apath);

                if (verbose) std::cout << "Rel Path is " << rpath << std::endl;
                if (verbose) std::cout << "Abs Path is " << apath << std::endl;
            
			    if (clib->initializeXML("name"))  // reset document head / write head node
			    {
                    PrivateData *pd = iter->second;
				    pd->setCommonLib(clib);         // prepare to write
				    pd->writeXML();                 // create the XML document structure
				    clib->saveXML(apath);           // Write the actual XML file out 
			    } 
                else std::cerr << __FUNCTION__ << ": ERROR: failed to initializeXML with \"name\"" << std::endl;       
			}            
        } 
        else std::cerr << __FUNCTION__ << ": warning: couldn't create private path \""
                       << vctx_private_path << "\"" << std::endl;
	}
    

    #pragma region Documentation
    /// <summary>Add a Lesion Mask. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">Lesion ID. </parm>
    /// <parm name="lmp">Address of a valid Lesion Mask. </parm>
    #pragma endregion  
    void addLesionMask(int id, LesionMask *lmp)
    { 
        lesionMaskMap.insert(std::make_pair(id, lmp)); 
    }


    #pragma region Documentation
    /// <summary>Retrieve a Lesion Mask. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">Lesion ID. </parm>
    /// <return>Address of the requested valid Lesion Mask (or nullptr if not found). </return>
    #pragma endregion  
    LesionMask *getLesionMask(int id) 
    { 
        LesionMask *retval = nullptr;
        auto it = lesionMaskMap.find(id); 
        if (it != lesionMaskMap.end())
        { 
            retval = it->second; 
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Add a Lesion. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">Lesion ID. </parm>
    /// <parm name="lp">Address of a valid Lesion. </parm>
    #pragma endregion  
    void addLesion(int id, Lesion *lp) 
    { 
        lesionMap.insert(std::make_pair(id, lp)); 
    }        

    
    #pragma region Documentation
    /// <summary>Retrieve a Lesion. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">Lesion ID. </parm>
    /// <return>Address of the requested valid Lesion (or nullptr if not found). </return>
    #pragma endregion  
    Lesion *getLesion(int id)  
    { 
        Lesion *retval = nullptr;
        auto it = lesionMap.find(id); 
        if (it != lesionMap.end()) 
        { 
            retval = it->second; 
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Add an image file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">Lesion ID. </parm>
    /// <parm name="filename">Image filename. </parm>
    #pragma endregion  
    void addImageFile(int id, std::string filename) 
    { 
        imageFileMap.insert(std::make_pair(id, filename));
    }


    #pragma region Documentation
    /// <summary>Retrieve an image file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="id">Image ID. </parm>
    /// <return>Address of the requested valid image file (or nullptr if not found). </return>
    #pragma endregion  
    std::string getImageFile(int id)
    { 
        std::string retval;
        auto it = imageFileMap.find(id);
        if (it != imageFileMap.end()) 
        { 
            retval = it->second; 
        }
        return retval;
    }

    // Temporary Folder Discovery


    #pragma region Documentation
    /// <summary>Reuse phantom discovered in its temporary folder. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="search_dir">Path to the temporary folder. </parm>
    /// <parm name="sub_dir">Name of the vctx file (alone). </parm>
    /// <return>The phantom name (or nullptr if not found). </return>
    #pragma endregion  
    bool discoverTemp(std::string search_dir, std::string sub_dir)
    {
        bool retval = true;
        
        #if defined(_MSC_VER)

            WIN32_FIND_DATA fdFile;
            HANDLE hFind = NULL;
            std::string srch_path(search_dir);

            // Specify a file mask. *.* = We want everything!
            srch_path += "/*.*";
            if (verbose) std::cout << "srch_path = " << srch_path << std::endl;

            #ifdef UNICODE
            if ((hFind = FindFirstFile(LPCWSTR(srch_path.c_str()), &fdFile)) != INVALID_HANDLE_VALUE)
            #else        
            if ((hFind = FindFirstFile(srch_path.c_str(), &fdFile)) != INVALID_HANDLE_VALUE)
            #endif
            {
                do
                {
                    #ifdef UNICODE
                    char narrow[128];
                    wcstombs(narrow, fdFile.cFileName, 128);
                    std::string fname(narrow);
                    #else
                    std::string fname(fdFile.cFileName);
                    #endif
                    if (fname != "." && fname != "..")
                    {
                        // Is the entity a File or Folder?
                        if (fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
                        {
                            // Build up our file path using the passed in [sDir] and the file/foldername we just found:
                            srch_path = search_dir + "/" + fname;
                            if (verbose) std::cout << "Found subfolder " << fname << std::endl;

                            if (sub_dir.size() > 0) fname = sub_dir + "/" + fname;
                            discoverTemp(srch_path, fname);
                        }
                        else
                        {
                            if (sub_dir.size() > 0) fname = sub_dir + "/" + fname;
                            zippedEntities.push_back(fname);

                            // Add any private entities to private data collection to be carried forward
                            std::string::size_type pos2 = fname.find("/Phantom.");
                            if (pos2 == std::string::npos)
                            {
                                rel_private_entities.push_back(fname);
                                abs_private_entities.push_back(TEMPDIR + fname);

                                if (verbose) std::cout << "\nADDING " << fname << "(" << TEMPDIR+fname << ") to private paths" << std::endl;
                            }
                            if (verbose) std::cout << "\nfound entry \"" << fname << "\"\n" << std::endl;
                        }
                    }
                }
                while(FindNextFile(hFind, &fdFile)); // Find the next file.

                FindClose(hFind);
                retval = true;
            }
            else
            {
                std::cerr << "Path not found: " << search_dir << "\n";
                retval = false;
            }
            
        #else
            //TODO: insert non-visual studio code here
        #endif
        return retval;
    }


    // Zip File interaction
    

    #pragma region Documentation
    /// <summary>Uncompress the specified vctx file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="path_only">Path to the required vctx file. </parm>
    /// <parm name="archive">Name of the vctx file (alone). </parm>
    /// <return>The phantom name (or nullptr if not found). </return>
    #pragma endregion  
    std::string uncompressZip(std::string path_only, std::string archive)
    {
        std::string specd_phantom_name; // specified phantom name

        // RIT path
        ZipArchive::Ptr ziparchive = ZipFile::Open(archive);
        size_t entries = ziparchive->GetEntriesCount();
        if (verbose) 
        {
            std::cout << "\nLine " << __LINE__
                      << ": archive: " << archive 
                      << ", path_only: " << path_only 
                      << ". There are " << entries << " entries" 
                      << std::endl;
        }

        std::istream *decompress_stream = nullptr;

        std::string entryName;
        for (size_t i = 0; i<entries; ++i)
        {
            auto entry = ziparchive->GetEntry(int(i));
            entryName = entry->GetFullName();
            zippedEntities.push_back(entryName);

            if (verbose) std::cout << "Line " << __LINE__
                                       << ": found entry \"" << entryName << "\"\n" << std::endl;

            if (i == 0)
            {
                specd_phantom_name = entryName;
                size_t pos = specd_phantom_name.find_first_of("\\/");
                if (pos != std::string::npos) 
                {
                    specd_phantom_name = specd_phantom_name.substr(0, pos);
                }
                if (verbose)
                {
                    std::cout << "Line " << __LINE__
                              << ": Specified phantom name: \"" << specd_phantom_name << "\"" 
                              << ", TEMPDIR: \"" << TEMPDIR << "\"" 
                              << ", entryName: " << entryName << std::endl;
                }
            }

                            
            // Add any private entities to private data collection to be carried forward
            #if defined(_MSC_VER)
            std::string::size_type pos2 = entryName.find("/Phantom.");
            #else
            std::string::size_type pos2 = entryName.find("\\Phantom.");
            #endif
            if (pos2 == std::string::npos)
            {
                rel_private_entities.push_back(entryName);
                size_t pos = TEMPDIR.find_last_of("\\/");
                if (pos == std::string::npos || pos == 0)
                {
                    TEMPDIR = TEMPDIR + "/";
                    if (verbose)
                    {
                        std::cout << "\nLine " << __LINE__ << ": TEMPDIR is now " << TEMPDIR << std::endl; 
                    }
                }
                abs_private_entities.push_back(TEMPDIR + entryName);

                if (verbose)
                {
                    std::cout << "\nLine " << __LINE__
                              << ": ADDING " << entryName 
                              << "(" << TEMPDIR+entryName << ") to private paths" << std::endl;
                }
            }

            // Create any subfolders which may need to be created
            bool isFolder = false;
            std::string lastpath;
            std::string::size_type lastpos = 0;
            std::string::size_type pos = entryName.find_first_of("/\\", lastpos);
            while(pos != std::string::npos)
            {
                std::string subfolder = entryName.substr(lastpos, pos - lastpos);
                std::string fullpath = TEMPDIR + lastpath + subfolder;

                if (verbose) std::cout << "Line " << __LINE__
                                       << ": Found subfolder " << subfolder 
                                       << ". Testing fullpath \"" << fullpath << "\"" 
                                       << std::endl;

                isFolder = false;
                if (!clib->testPath(fullpath, isFolder))
                {
                    if (verbose) std::cout << "\t...Creating path " << fullpath << std::endl;
                    clib->createPath(fullpath); 
                    if (verbose) std::cout << "\t...back from creating path." << std::endl;
                }
                else if (verbose) std::cout << "...path already exists." << std::endl;

                lastpos = pos + 1;
                lastpath += subfolder + "/";
                pos = entryName.find_first_of("/\\", lastpos);
            }

            std::string outfile(path_only);
            pos = outfile.find_last_of("\\/");
            if (pos == std::string::npos || pos < 1) outfile = outfile + "/";
            outfile = outfile + entryName; // TODO: Test last char of outfile and 1st char of entryName
            std::string temp = clib->getPathOnly(outfile);
            
            if (verbose)
            {
                std::cout << __FILE__ << ", " << __FUNCTION__
                          << ", Line " << __LINE__ 
                          << ", path_only: " << path_only << ", entryName: " << entryName
                          << ", outfile: " << outfile
                          << ", temp: " << temp << std::endl;
            }

            bool isPath = entry->IsDirectory();
            if (isPath)
            {
                if (verbose) std::cout << "Creating path \"" << temp << "\"" << std::endl;
                clib->createPath(temp);
            }
            else
            {
                if (verbose)
                {
                    std::cout << __FILE__ << ", " << __FUNCTION__
                              << ", Line " << __LINE__ << ": About to call ZipFile::ExtractFile" 
                              << ", Archive: " << archive << ", entryName: " << entryName
                              << ", outfile: " << outfile << std::endl;
                }
                ZipFile::ExtractFile(archive, entryName.c_str(), outfile.c_str());
                if (verbose) std::cout << "Back from extracing File" << std::endl;
            }
        }

        have_uncompressed_path = true;
                
        return specd_phantom_name;
    }


    #pragma region Documentation
    /// <summary>Build the vctx file. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion  
    void buildZipFile()
    {         
        if (verbose)
        {
            std::cout << __FUNCTION__ << ": vctx_file_out: " << vctx_file_out << std::endl;
        }
        // It is critically important that the actual archive file must NOT already exist when saving
        remove(vctx_file_out.c_str());

        ZipArchive::Ptr archive = ZipFile::Open(vctx_file_out);

        if (archive == nullptr)
        {
            // Cannot save with this filename. Change it
            std::string::size_type pos = vctx_file_out.find(EXTENSION);
            if (pos != std::string::npos)
            {
                std::string newfile = vctx_file_out.substr(0, pos-1);
                newfile += "_1";
                newfile += EXTENSION;

                archive = ZipFile::Open(newfile);
                if (archive != nullptr)
                {
                    std::cout << "Couldn't save file as \"" << vctx_file_out << "\", saving as \"" << newfile << "\" instead." << std::endl;
                }
                else
                {
                    std::cerr << "* Error: Cannot save file \"" << vctx_file_out << "\" *" << std::endl;
                    return;
                }
            }
        }

         ZipArchiveEntry::Ptr entry1 = archive->CreateEntry(rel_phantomXml.c_str());
         ZipArchiveEntry::Ptr entry2 = archive->CreateEntry(rel_phantomDat.c_str());
         
         if (entry1 == nullptr || entry2 == nullptr)
         {
             std::cerr << "\n* ERROR: Cannot save \"" << vctx_file_out << "\"" 
                       << "\n  Is file open in another application?"
                       << "\n  If so, close and try again. *\n" 
                       << std::endl;
             return;
         }

         std::ifstream contentStream1(abs_phantomXml.c_str());
         std::ifstream contentStream2(abs_phantomDat.c_str(), std::ios::binary);

         DeflateMethod::Ptr ctx = DeflateMethod::Create();
         ctx->SetCompressionLevel(DeflateMethod::CompressionLevel::L1);

         entry1->SetCompressionStream(contentStream1, ctx, ZipArchiveEntry::CompressionMode::Immediate);
         entry2->SetCompressionStream(contentStream2, ctx, ZipArchiveEntry::CompressionMode::Immediate);


         std::vector<ZipArchiveEntry::Ptr> entries;
         std::vector<std::ifstream*>        streams;
         std::vector<DeflateMethod::Ptr>   methods;

         
         for(int i = 0; i<abs_privateXML.size(); ++i)
         {
             ZipArchiveEntry::Ptr entry3 = archive->CreateEntry(rel_privateXML[i].c_str());
             entries.push_back(entry3);

             std::ifstream *contentStream3 = new std::ifstream(abs_privateXML[i].c_str());
             streams.push_back(contentStream3);

             DeflateMethod::Ptr ctx = DeflateMethod::Create();
             ctx->SetCompressionLevel(DeflateMethod::CompressionLevel::L1);
             methods.push_back(ctx);

             entry3->SetCompressionStream(*contentStream3, ctx, ZipArchiveEntry::CompressionMode::Immediate);    
         }

         // Propagate all pre-existing private entities forward
         for(int i=0; i<rel_private_entities.size(); ++i)
         {
             std::string new_rel_private(rel_private_entities[i]);
             std::string::size_type pos = new_rel_private.find("/Private");
             
             if (verbose)
             {
                std::cout << __FUNCTION__ << ", line " << __LINE__ 
                          << ": new_rel_private: " << new_rel_private
                          << ", pos: " << pos                          
                          << std::endl;                
             }

             if (pos != std::string::npos)
             {
                 new_rel_private = xml_topdir + new_rel_private.substr(pos);
             }
             else continue; // DH 2020-04-15

             std::string filename;
             pos = abs_private_entities[i].find("/Private/");
             if (pos != std::string::npos)
             {
                 filename = abs_private_entities[i].substr(pos+9);
             }

             std::string new_abs_private(abs_private_path_out); // Needs NEW private path
             pos = new_abs_private.find("Private/");
             if (pos != std::string::npos) 
             {                
                new_abs_private = new_abs_private.substr(0,pos+8) + filename;
             }
             
             if (verbose)
             {
                std::cout << __FUNCTION__ << ": copying existing private file " 
                          << abs_private_entities[i] << " to " << new_abs_private 
                          << "\nfilename: " << filename 
                          << std::endl;                
             }
             
             clib->copyFile(abs_private_entities[i], new_abs_private); // verified that the copy is happening correctly.

             ZipArchiveEntry::Ptr entry = archive->CreateEntry(new_rel_private.c_str());  // rel_private_entities[i].c_str());
             std::ifstream contentStream(new_abs_private);            
             entry->SetCompressionStream(contentStream, ctx, ZipArchiveEntry::CompressionMode::Immediate);
         }
         
         std::ofstream archiveToSave;
         archiveToSave.open(vctx_file_out, std::ios::binary);
         archive->WriteToStream(archiveToSave);

         archiveToSave.flush();
         archiveToSave.close();

         // Close the streams
         for(auto s : streams)
         {
             delete s;
         }

         if (verbose) std::cout << __FUNCTION__ << ": Sleeping for 100 msec..." << std::endl;
         clib->sleep(100);

         // Ideally, the temporary directory used to construct the zip file would be deleted here
         // ...windows doesn't allow it after a few milliseconds if the same application created it
         // Windows message: "The process cannot access the file because it is being used by another process."
         if (verbose) std::cout << "Adding del_path: " << tmp_xml_path << std::endl;
         del_paths.push_back(tmp_xml_path);
         /***
         #if !defined(_MSC_VER)
         for(auto dp : del_paths)
         {
            std::cout << "Line " << __LINE__ << ": Calling clib->deleteDirectory("
                      << dp << ");" << std::endl;
            clib->deleteDirectory(dp);
         }
         #endif 
         ***/         
    }

private:

    bool verbose;                     /// verbose flag
    bool leave_temp;                  /// leave phantom in temporary folder when done
    bool okay_to_use_temp_folder;     /// flag to use (read) phantom left in temporary folder

    std::string TEMPDIR;              /// base location where phantom vctx files are create and (un)compressed
	bool created_phantom;             /// flag: phantom was created by vctx instance
    bool have_uncompressed_path;      /// flag: has uncompression path been established?

    std::string phantom_name;         /// internal phantom name
    std::string phantom_filename;     /// external phantom filename
    std::string extension;            /// file extension
    std::string xml_topdir;           /// partial path to xml file
    std::string tmp_xml_path;         /// full path to xml file
    std::string tmp_xml_private_name; /// private data xml filename
    std::string vctx_file_in;         /// path to vctx input file
    std::string vctx_file_out;        /// path to vctx output file

    std::string abs_phantomXml;       /// absolute path to phantom.xml file
    std::string abs_phantomDat;       /// absolute path to phantom.dat file

    std::string rel_phantomXml;       /// relative path to phantom.xml file
    std::string rel_phantomDat;       /// relative path to phantom.dat file

    std::vector<std::string> abs_private_entities; /// collection of absolute private data file paths
    std::vector<std::string> rel_private_entities; /// collection of relative private data file paths

    std::vector<std::string> abs_privateXML;       /// absolute path to private data file
    std::vector<std::string> rel_privateXML;       /// relative path to private data file

    std::string abs_private_path_in;  /// absolute path to private folder
    std::string abs_private_path_out; /// absolute path to private folder

    CommonLibrary *clib;              /// pointer to the current common library instance
    Trial         *trial;             /// pointer to the phantom trial instance
    Subject       *subject;           /// pointer to the phantom subject instance
    Phantom       *phantom;           /// pointer to the phantom instance

    std::map<std::string, PrivateData *> private_data; /// container of private data pointer instances, references by name

    std::map<int, LesionMask*> lesionMaskMap; /// container of pointers oflesion mask instances, referenced by id
    std::map<int, Lesion*>     lesionMap;     /// container of pointers of lesion intances, referenced by id
    std::map<int, std::string> imageFileMap;  /// container of image file paths, referenced by id

    std::vector<std::string> zippedEntities;  /// vctx file manifest
    std::vector<std::string> del_paths;       /// recursively delete these paths when done

};

} // namespace vct
