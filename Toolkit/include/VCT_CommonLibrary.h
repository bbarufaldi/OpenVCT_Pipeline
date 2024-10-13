// VCT_CommonLibrary.h

#pragma once

#ifndef NOMINMAX
#define NOMINMAX // required to prevent preprocessor definitions of min and max
#endif

#include "VCT_Logger.h"

#if defined(_MSC_VER)
    #include "dirent.h" // Copyright (C) 2006-2012 Toni Ronkko ( Filesystem library (C++17) generally not widespread at this time
    #include <windows.h>
    #include <tchar.h>
    #include <cstdio>
    #include <strsafe.h>
    #pragma comment(lib, "User32.lib")
#else // Linux & Mac
    #include </usr/include/dirent.h>
    #include <sys/stat.h>
#endif

#if __cplusplus > 199711L
   #include <chrono>
   #include <thread>
#endif

#include "pugixml.hpp"

// For OpenGL error message function, CheckGlErrors()
#if defined(_MSC_VER)
   #include <gl/gl.h>
   #include <gl/glext.h>
#elif defined(__linux__)
   #include <GL/gl.h>
#elif defined(__APPLE__)
   #include <OpenGL/gl.h>
#endif

#include <fstream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <ctime>
#include <cmath>
#include <cerrno>
#include <cstdio>
#include <map>

#pragma region Documentation
/// <summary> The Open Virtual Clinical Trials namespace. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
namespace vct
{

const float CURRENT_VCT_VERSION(0.1f);
const std::string WHITESPACE(" \n\r\t");


#pragma region Documentation
/// <summary> Common Library class for XML, Zip, and general utilities. </summary>
/// <remarks>D. Higginbotham, 2017-04-21. </remarks>
#pragma endregion
class CommonLibrary
{
public:

    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    CommonLibrary() : verbose(false), log(nullptr)
    {}


    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    CommonLibrary(const CommonLibrary &rhs)
        : verbose(rhs.verbose),
          log(rhs.log)
    {
        pugi_doc.reset(rhs.pugi_doc);
        curr_node = rhs.curr_node;
    }


    #pragma region Documentation
    /// <summary>Destructor: terminate I/O, deallocates memory, etc. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    ~CommonLibrary()
    {
        // Close log if it has been opted-for
        if (log != nullptr)
        {
            log->timestamp();
            log->setSeverityLevel(vct::Logger::LOG_NONE);
            if (logstream.is_open()) logstream.close();
        }
    }


    #pragma region Documentation
    /// <summary>Enable or disable verbose status messaging. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="state">When true enables verbose mode, otherwise disables verbose mode. </parm>
    #pragma endregion
    void setVerbose(bool state) { verbose = state; }


    #pragma region Documentation
    /// <summary>Enable logging to the specified file and set accepted log severity. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="file">A file name where log information is to be placed. </parm>
    /// <parm name="severity">Indicate the types of messages to log (see VCT_Logger.h). </parm>
    #pragma endregion
    void setLogger(std::string file, vct::Logger::Severity severity)
    {
        // Instantiate Logger
        logstream.open(file);
        if (logstream)
        {
            log = new vct::Logger(logstream);
            if (log != nullptr)
            {
                log->timestamp();
                log->setSeverityLevel(severity);
            }
        }
    }


    #pragma region Documentation
    /// <summary>Sleep for the specified number of milliseconds. </summary>
    /// <remarks>D. Higginbotham, 2020-03-19. </remarks>
    /// <parm name="milliseconds">The number of milliseconds to sleep. </parm>
    #pragma endregion
    void sleep(int milliseconds)
    {
        #if __cplusplus > 199711L
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        #else
            Sleep(milliseconds); // VS2015 apparently doesn't yet support chrono or thread
        #endif
    }


    #pragma region Documentation
    /// <summary>Mark start time for a timed task. </summary>
    /// <remarks>D. Higginbotham, 2020-08-03. </remarks>
    #pragma endregion
    void start_timer()
    {
        #if __cplusplus > 199711L
            timer_start = std::chrono::steady_clock::now();
        #else
            timer_start = clock(); // VS2015 apparently doesn't yet support chrono or thread
        #endif
    }


    #pragma region Documentation
    /// <summary>Find elapsed time for a timed task (must have called start_timer() first). </summary>
    /// <remarks>D. Higginbotham, 2020-08-03. </remarks>
    /// <return>Elapsed time in milliseconds. </return>
    #pragma endregion
    unsigned int get_elapsed_time()
    {
        unsigned int retval = 0u;
        #if __cplusplus > 199711L
            auto timer_end = std::chrono::steady_clock::now();
            retval = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count());
        #else
           auto timer_end = clock();
           retval = static_cast<unsigned int>((timer_end - timer_start) / (CLOCKS_PER_SEC * 0.001));
        #endif
        return retval;
    }


    #pragma region Documentation
    /// <summary>Mark start time for a timed task. </summary>
    /// <remarks>D. Higginbotham, 2020-08-03. </remarks>
    #pragma endregion
    void start_timer(unsigned int timer_number)
    {
        #if __cplusplus > 199711L
            auto start = std::chrono::steady_clock::now();
            timer_start_map.insert(std::make_pair(timer_number, start));
        #else
            auto start = clock();
            timer_start_map.insert(std::make_pair(timer_number, start)); // VS2015 apparently doesn't yet support chrono or thread
        #endif
    }


    #pragma region Documentation
    /// <summary>Find elapsed time for a timed task (must have called start_timer() first). </summary>
    /// <remarks>D. Higginbotham, 2020-08-03. </remarks>
    /// <return>Elapsed time in milliseconds. </return>
    #pragma endregion
    unsigned int get_elapsed_time(unsigned int timer_number)
    {
        unsigned int retval = 0u;
        #if __cplusplus > 199711L
            auto timer_end = std::chrono::steady_clock::now();
            auto iter = timer_start_map.find(timer_number);
            if (iter != timer_start_map.end())
            {
                retval = static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - iter->second).count());
            }
        #else
           auto timer_end = clock();
           auto iter = timer_start_map.find(timer_number);
           if (iter != timer_start_map.end())
           {
               retval = static_cast<unsigned int>((timer_end - iter->second) / (CLOCKS_PER_SEC * 0.001));
           }
        #endif
        return retval;
    }


    // String-trimming utilities

    #pragma region Documentation
    /// <summary>Trim whitespace - newlines, tabs, spaces - from the end of the string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="str">The string which is to be trimmed (immediately affects the supplied string). </parm>
    /// <return>A reference to the trimmed string. </return>
    #pragma endregion
    std::string &trimEnd(std::string &str)
    {
        std::string::size_type tpos = str.find_last_not_of(WHITESPACE);
        return (str = (tpos == std::string::npos) ? "" : str.substr(0,tpos+1));
    }


    #pragma region Documentation
    /// <summary>Trim whitespace - newlines, tabs, spaces - from the end of the string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="str">The string which is to be trimmed (immediately affects the supplied string). </parm>
    /// <return>A reference to the trimmed string. </return>
    #pragma endregion
    std::string &trimFront(std::string &str)
    {
        std::string::size_type tpos = str.find_first_not_of(WHITESPACE);
        return (str = (tpos == std::string::npos) ? "" : str.substr(tpos));
    }


    #pragma region Documentation
    /// <summary>Trim whitespace - newlines, tabs, spaces - from the front of the string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="str">The string which is to be trimmed (immediately affects the supplied string). </parm>
    /// <return>A reference to the trimmed string. </return>
    #pragma endregion
    std::string &trim(std::string &str)
    {
        return trimFront(trimEnd(str));
    }


    #pragma region Documentation
    /// <summary>Trim any and all (double) quote characters from the specified string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="str">The string which is to be trimmed (immediately affects the supplied string). </parm>
    /// <return>A reference to the trimmed string. </return>
    #pragma endregion
    void trimMultiQuotes(std::string &str)
    {
        std::string::size_type st = str.find("\"\"");
        while(st != std::string::npos)
        {
            str.replace(st, 2, "\"");
            st = str.find("\"\"");
        }
    }



    // Regular and Wide String Utilities


    #pragma region Documentation
    /// <summary>Replace all occurrences of a specified character with a different character. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="str">The string to operate upon (unaffected by this function). </parm>
    /// <parm name="from">The character to replace. </parm>
    /// <parm name="to">The substitute character (replacement). </parm>
    #pragma endregion
    void swapall(std::string &str, char from, std::string to)
    {
        size_t pos = str.find(from);
        while(pos != std::string::npos)
        {
            if (verbose) std::cout << "str: " << str << ", pos=" << pos << std::endl;
            if (log != nullptr) log->info << "str: " << str << ", pos=" << pos << '\n';
            str.replace(pos, 1, to);
            pos = str.find(from);
        }
    }


    #pragma region Documentation
    /// <summary>Replace all occurrences of a specified character with a different character. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="wstr">The wide string to operate upon (unaffected by this function). </parm>
    /// <parm name="from">The character to replace. </parm>
    /// <parm name="to">The substitute character (replacement). </parm>
    #pragma endregion
    void swapall(std::wstring &wstr, wchar_t from, std::wstring to)
    {
        size_t pos = wstr.find(from);
        while(pos != std::wstring::npos)
        {
            if (verbose) std::wcout << "wstr: " << wstr << ", pos=" << pos << std::endl;
            if (log != nullptr) log->info << "wstr: " << ws2s(wstr) << ", pos=" << pos << '\n';
            wstr.replace(pos, 1, to);
            pos = wstr.find(from);
        }
    }


    #pragma region Documentation
    /// <summary>Convert a regular, basic string to a wide string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="rstr">The regular, basic string to convert (unaffected by this function). </parm>
    /// <return>Same string content but as a wide string. </return>
    #pragma endregion
    std::wstring s2ws(const std::string &rstr)
    {
        #if defined(_MSC_VER)
            int slen = (int)rstr.length() + 1;
            int wlen = MultiByteToWideChar(CP_ACP, 0, rstr.c_str(), slen, 0, 0);
            wchar_t* buf = new wchar_t[wlen];
            MultiByteToWideChar(CP_ACP, 0, rstr.c_str(), slen, buf, wlen);
            std::wstring wide(buf);
            delete[] buf;
            return wide;
        #else
            return std::wstring(rstr.begin(), rstr.end());
        #endif
    }


    #pragma region Documentation
    /// <summary>Convert a wide string to a regular, basic string. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="wstr">The wide string to convert (unaffected by this function). </parm>
    /// <return>Same wide string content but as a regular, basic string. </return>
    #pragma endregion
    std::string ws2s(const std::wstring &wstr)
    {
        #if defined(_MSC_VER)
           int wlen = (int)wstr.length() + 1;
           int slen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wlen, 0, 0, 0, 0);
           char* buf = new char[slen];
           WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wlen, buf, slen, 0, 0);
           std::string rstr(buf);
           delete[] buf;
           return rstr;
        #else
           return std::string(wstr.begin(), wstr.end());
        #endif
    }


    // Error Message Utilities


    #pragma region Documentation
    /// <summary>Output the string associated with the last OpenCL error.</summary>
    /// <remarks>D. Higginbotham, 2020-04-20. </remarks>
    /// <return>System error message as a string.</return>
    #pragma endregion
    std::string getErrorString(int error)
    {
        std::string msg;
        switch(error)
        {
            // run-time and JIT compiler errors
            case 0:   msg =  "CL_SUCCESS"; break;
            case -1:  msg =  "CL_DEVICE_NOT_FOUND"; break;
            case -2:  msg =  "CL_DEVICE_NOT_AVAILABLE"; break;
            case -3:  msg =  "CL_COMPILER_NOT_AVAILABLE"; break;
            case -4:  msg =  "CL_MEM_OBJECT_ALLOCATION_FAILURE"; break;
            case -5:  msg =  "CL_OUT_OF_RESOURCES"; break;
            case -6:  msg =  "CL_OUT_OF_HOST_MEMORY"; break;
            case -7:  msg =  "CL_PROFILING_INFO_NOT_AVAILABLE"; break;
            case -8:  msg =  "CL_MEM_COPY_OVERLAP"; break;
            case -9:  msg =  "CL_IMAGE_FORMAT_MISMATCH"; break;
            case -10: msg =  "CL_IMAGE_FORMAT_NOT_SUPPORTED"; break;
            case -11: msg =  "CL_BUILD_PROGRAM_FAILURE"; break;
            case -12: msg =  "CL_MAP_FAILURE"; break;
            case -13: msg =  "CL_MISALIGNED_SUB_BUFFER_OFFSET"; break;
            case -14: msg =  "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST"; break;
            case -15: msg =  "CL_COMPILE_PROGRAM_FAILURE"; break;
            case -16: msg =  "CL_LINKER_NOT_AVAILABLE"; break;
            case -17: msg =  "CL_LINK_PROGRAM_FAILURE"; break;
            case -18: msg =  "CL_DEVICE_PARTITION_FAILED"; break;
            case -19: msg =  "CL_KERNEL_ARG_INFO_NOT_AVAILABLE"; break;

            // compile-time errors
            case -30: msg =   "CL_INVALID_VALUE"; break;
            case -31: msg =   "CL_INVALID_DEVICE_TYPE"; break;
            case -32: msg =  "CL_INVALID_PLATFORM"; break;
            case -33: msg =  "CL_INVALID_DEVICE"; break;
            case -34: msg =  "CL_INVALID_CONTEXT"; break;
            case -35: msg =  "CL_INVALID_QUEUE_PROPERTIES"; break;
            case -36: msg =  "CL_INVALID_COMMAND_QUEUE"; break;
            case -37: msg =  "CL_INVALID_HOST_PTR"; break;
            case -38: msg =  "CL_INVALID_MEM_OBJECT"; break;
            case -39: msg =  "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"; break;
            case -40: msg =  "CL_INVALID_IMAGE_SIZE"; break;
            case -41: msg =  "CL_INVALID_SAMPLER"; break;
            case -42: msg =  "CL_INVALID_BINARY"; break;
            case -43: msg =  "CL_INVALID_BUILD_OPTIONS"; break;
            case -44: msg =  "CL_INVALID_PROGRAM"; break;
            case -45: msg =  "CL_INVALID_PROGRAM_EXECUTABLE"; break;
            case -46: msg =  "CL_INVALID_KERNEL_NAME"; break;
            case -47: msg =  "CL_INVALID_KERNEL_DEFINITION"; break;
            case -48: msg =  "CL_INVALID_KERNEL"; break;
            case -49: msg =  "CL_INVALID_ARG_INDEX"; break;
            case -50: msg =  "CL_INVALID_ARG_VALUE"; break;
            case -51: msg =  "CL_INVALID_ARG_SIZE"; break;
            case -52: msg =  "CL_INVALID_KERNEL_ARGS"; break;
            case -53: msg =  "CL_INVALID_WORK_DIMENSION"; break;
            case -54: msg =  "CL_INVALID_WORK_GROUP_SIZE"; break;
            case -55: msg =  "CL_INVALID_WORK_ITEM_SIZE"; break;
            case -56: msg =  "CL_INVALID_GLOBAL_OFFSET"; break;
            case -57: msg =  "CL_INVALID_EVENT_WAIT_LIST"; break;
            case -58: msg =  "CL_INVALID_EVENT"; break;
            case -59: msg =  "CL_INVALID_OPERATION"; break;
            case -60: msg =  "CL_INVALID_GL_OBJECT"; break;
            case -61: msg =  "CL_INVALID_BUFFER_SIZE"; break;
            case -62: msg =  "CL_INVALID_MIP_LEVEL"; break;
            case -63: msg =  "CL_INVALID_GLOBAL_WORK_SIZE"; break;
            case -64: msg =  "CL_INVALID_PROPERTY"; break;
            case -65: msg =  "CL_INVALID_IMAGE_DESCRIPTOR"; break;
            case -66: msg =  "CL_INVALID_COMPILER_OPTIONS"; break;
            case -67: msg =  "CL_INVALID_LINKER_OPTIONS"; break;
            case -68: msg =  "CL_INVALID_DEVICE_PARTITION_COUNT"; break;

            // extension errors
            case -1000: msg =  "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR"; break;
            case -1001: msg =  "CL_PLATFORM_NOT_FOUND_KHR"; break;
            case -1002: msg =  "CL_INVALID_D3D10_DEVICE_KHR"; break;
            case -1003: msg =  "CL_INVALID_D3D10_RESOURCE_KHR"; break;
            case -1004: msg =  "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR"; break;
            case -1005: msg =  "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR"; break;
            default: msg =  "Unknown OpenCL error"; break;
        }
        return msg;
    }

    //#if defined(_WIN32) || defined(_WIN64)
    #if defined(_MSC_VER)

    #pragma region Documentation
    /// <summary>Output the string associated with the last system error.</summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void LastErrorText()
    {
        DWORD dwError = GetLastError();
        std::string msg;

        switch(dwError)
        {
            case NO_ERROR:                  msg = "NO ERROR";                   break;
            case ERROR_DIRECTORY:           msg = "ERROR_DIRECTORY";            break;
            case ERROR_DIR_NOT_EMPTY:       msg = "ERROR_DIR_NOT_EMPTY";        break;
            case ERROR_DIRECTORY_NOT_RM:    msg = "ERROR_DIRECTORY_NOT_RM";     break;
            case ERROR_DIR_NOT_ROOT:        msg = "ERROR_DIR_NOT_ROOT";         break;
            case ERROR_FILE_NOT_FOUND:      msg = "ERROR_FILE_NOT_FOUND";       break;
            case ERROR_ACCESS_DENIED:       msg = "ERROR_ACCESS_DENIED";        break;
            case ERROR_FILE_INVALID:        msg = "ERROR_FILE_INVALID";         break;
            case ERROR_FILE_READ_ONLY:      msg = "ERROR_FILE_READ_ONLY";       break;
            case ERROR_FILE_CHECKED_OUT:    msg = "ERROR_FILE_CHECKED_OUT";     break;
            case ERROR_FILE_HANDLE_REVOKED: msg = "ERROR_FILE_HANDLE_REVOKED";  break;
            case ERROR_FILE_CORRUPT:        msg = "ERROR_FILE_CORRUPT";         break;
            case ERROR_FILEMARK_DETECTED:   msg = "ERROR_FILEMARK_DETECTED";    break;
            case ERROR_FILE_ENCRYPTED:      msg = "ERROR_FILE_ENCRYPTED";       break;
            case ERROR_FILE_EXISTS:         msg = "ERROR_FILE_EXISTS";          break;

            case ERROR_FILENAME_EXCED_RANGE:          msg = "ERROR_FILENAME_EXCED_RANGE";           break;
            case ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED: msg = "ERROR_FILE_LEVEL_TRIM_NOT_SUPPORTED";  break;
            case ERROR_FILE_SHARE_RESOURCE_CONFLICT:  msg = "ERROR_FILE_SHARE_RESOURCE_CONFLICT";   break;
            case ERROR_FILE_IDENTITY_NOT_PERSISTENT:  msg = "ERROR_FILE_IDENTITY_NOT_PERSISTENT";   break;
        }

        if (msg.size() > 0)
        {
            std::cout << msg << std::endl;
            if (log != nullptr) log->error << msg << '\n';
        }
    }

    #else

    #pragma region Documentation
    /// <summary>Output the string associated with the last system error.</summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void LastErrorText()
    {
        std::perror("Error");
    }

    #endif


    #pragma region Documentation
    /// <summary>Check for and report useful text information for any OpenGL errors. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>OpenGL is a state machine in which errors must be proactively queried-for and reported. </remarks>
    /// <parm name="func">The name of the current function (to aid in localizing error reporting). </parm>
    /// <return>OpenGL Error expressed as a string, or no message if no error. </return>
    #pragma endregion
    std::string CheckGlErrors(std::string func)
    {
        std::string error_msg;

        int error;
        while((error = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << func << ": glError " << error << std::endl;
            if (log != nullptr) log->error << func << ": glError " << error << '\n';


            switch(error)
            {
                case GL_NO_ERROR:
                    error_msg = "error has been recorded. The value of this symbolic constant is "
                                "guaranteed to be 0.";
                    break;
                case GL_INVALID_ENUM:
                    error_msg = "An unacceptable value is specified for an enumerated argument. "
                                "The offending command is ignored and has no other side effect than "
                                "to set the error flag.";
                    break;
                case GL_INVALID_VALUE:
                    error_msg = "A numeric argument is out of range. The offending command is ignored "
                                "and has no other side effect than to set the error flag.";
                    break;
                case GL_INVALID_OPERATION:
                    error_msg = "The specified operation is not allowed in the current state. The "
                                "offending command is ignored and has no other side effect than to "
                                "set the error flag.";
                    break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                    error_msg = "The framebuffer object is not complete. The offending command is "
                                "ignored and has no other side effect than to set the error flag.";
                    break;
                case GL_OUT_OF_MEMORY:
                    error_msg = "There is not enough memory left to execute the command. The state "
                                "of the GL is undefined, except for the state of the error flags, "
                                "after this error is recorded.";
                    break;
                case GL_STACK_UNDERFLOW:
                    error_msg = "An attempt has been made to perform an operation that would cause "
                                "an internal stack to underflow.";
                    break;
                case GL_STACK_OVERFLOW:
                    error_msg = "An attempt has been made to perform an operation that would cause "
                                "an internal stack to overflow.";
                    break;
                default:
                    error_msg = "unknown error.";
                    break;
            }

            if (error_msg.size() > 0)
            {
                std::cerr << error_msg << std::endl;
                if (log != nullptr)
                {
                    log->error << error_msg << '\n';
                    logstream.flush();
                }
            }
        }

        return error_msg;
    }


    // Path, Filename and extension utilities


    #pragma region Documentation
    /// <summary>Extract the path-only component from the specified string path (remove filename and extension). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="fullPath">The supplied path (function doesn't change the supplied path). </parm>
    /// <return>The path-only component. </return>
    #pragma endregion
    std::string getPathOnly(std::string &fullPath)
    {
        std::string retval(fullPath);
        std::string::size_type pos = fullPath.find_last_of("/\\");
        if (pos != std::string::npos) retval = fullPath.substr(0, pos);
        return retval;
    }


    #pragma region Documentation
    /// <summary>Extract the filename and extension components from the specified string path (remove path). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="fullPath">The supplied path (function doesn't change the supplied path). </parm>
    /// <return>The filename and extension. </return>
    #pragma endregion
    std::string getFilenameAndExt(std::string &fullPath)
    {
        std::string retval(fullPath);
        std::string::size_type pos = fullPath.find_last_of("/\\");
        if (pos != std::string::npos) retval = fullPath.substr(pos+1);
        return retval;
    }


    #pragma region Documentation
    /// <summary>Extract the filename extension component from the specified string path (remove path and filename). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="fullPath">The supplied path (function doesn't change the supplied path). </parm>
    /// <return>The filename extension (only). </return>
    #pragma endregion
    std::string getFileExtOnly(std::string &fullPath)
    {
        std::string retval(fullPath);
        std::string::size_type pos = fullPath.find_last_of(".");
        if (pos != std::string::npos) retval = fullPath.substr(pos+1);
        return retval;
    }


    // Directory (path) and File I/O Utilities

    #pragma region Documentation
    /// <summary>Test whether the specified path exists and whether the last item is a folder. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="path">The supplied path (function doesn't change the supplied path). </parm>
    /// <parm name="isPath">Set to true if the last path component is a folder. </parm>
    /// <return>True if the specified path exists (otherwise false). </return>
    #pragma endregion
    bool testPath(std::string &path, bool &isPath)
    {
        #if defined(_MSC_VER)
            bool exists = false;
            DIR *myDir = opendir(path.c_str()); // depends on local dirent.h
            struct stat myStat;
            exists = (stat(path.c_str(), &myStat) == 0);
            isPath = (((myStat.st_mode) & S_IFMT) == S_IFDIR);
            if (exists) closedir(myDir);
            if (log != nullptr) log->info << "Path: " << path << (exists ? " exists" : " doesn't exist") << '\n';
            return exists;
        #else
            bool exists = false;
            if (isPath)
            {
                DIR *myDir = opendir(path.c_str());
                if (myDir != nullptr) exists = true;
            }
            else // check for file
            {
                std::ifstream ifs(path);
                if (ifs)
                {
                    exists = true;
                    ifs.close();
                }
            }
            return exists;
        #endif
    }


    #pragma region Documentation
    /// <summary>Create the specified folder. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="path">The path specification of the folder to create. </parm>
    /// <return>True if the folder was successfully created (otherwise false). </return>
    #pragma endregion
    bool createPath(std::string &path)
    {
        // depends on local dirent.h
        bool status = false;

        #if defined(_MSC_VER)

            #if defined(_UNICODE)
                std::wstring stemp = std::wstring(path.begin(), path.end());
                LPCWSTR sw = stemp.c_str();
                status = (CreateDirectory(sw, NULL) != 0);
            #else
                status = (CreateDirectory(path.c_str(), NULL) != 0);
            #endif

            DWORD indicator = GetLastError();
            if (indicator == ERROR_ALREADY_EXISTS) status = true;

        #else

            mode_t mode = 0774;
            const char *cc_path = path.c_str();
            status = (bool)(mkdir(cc_path, mode) == 0);

        #endif

        if (log != nullptr) log->info << "Path: " << path << (status ? " created" : " not created") << '\n';
        return status;
    }


    #pragma region Documentation
    /// <summary>Create the specified folder at the specified location. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="path">The path where the folder is to be created. </parm>
    /// <parm name="folderName">The (simple) name of the folder to be created. </parm>
    /// <return>True if the folder was successfully created (otherwise false). </return>
    #pragma endregion
    bool createTemporaryFolder(std::string path, std::string folderName)
    {
        // depends on local dirent.h
        // Steps:
        // 1. Verify that the path exists
        // 2. If the base path doesn't exist, attempt to create it
        // 3. If the base path exists, create the new, temporary subfolder

        bool status = false;
        bool isPath = false;

        status = testPath(path, isPath);

        if (!status || !isPath) // folder does not exist
        {
            // Attempt to create the path
            status = createPath(path);
        }

        if (status)
        {
            // Create full pathname
            std::string outputDir(path);
            outputDir += "/";
            outputDir += folderName;

            // Output to this folder
            status = createPath(outputDir);
        }
        else if (log != nullptr)
        {
            log->error << "Could not create Temporary Folder: " << path << '/' << folderName << '\n';
        }

        return status;
    }


    //#if defined(_WIN32) || defined(_WIN64)
    #if defined(_MSC_VER)

    #pragma region Documentation
    /// <summary>Recursively delete a directory and its contents (Windows OS). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="directoryname">The path to be deleted. </parm>
    /// <return>True if the directory and it's contents were successfully deleted (otherwise false). </return>
    #pragma endregion
    bool deleteDirectory(std::string &directoryname)
    {
        WIN32_FIND_DATA ffd;
        LARGE_INTEGER filesize;
        TCHAR  tfolder[MAX_PATH]; // folder name for lisitng purposes
        TCHAR  dfolder[MAX_PATH]; // folder name for deletion purposes
        HANDLE handle = INVALID_HANDLE_VALUE;

        swapall(directoryname, '/', "\\"); // unnecessary

        #if defined(_UNICODE) || defined(UNICODE)

        std::wstring wide = s2ws(directoryname);
        StringCchCopy(dfolder, MAX_PATH, wide.c_str()); // copy foldername for later deletion
        StringCchCopy(tfolder, MAX_PATH, wide.c_str()); // copy foldername for listing
        StringCchCat(tfolder, MAX_PATH, TEXT("\\*"));   // req'd for content listing
        #else
        // Copy the string to a buffer and append '\*' to the directory name.
        StringCchCopy(dfolder, MAX_PATH, directoryname.c_str()); // copy foldername for deletion
        StringCchCopy(tfolder, MAX_PATH, directoryname.c_str());
        StringCchCat(tfolder, MAX_PATH, TEXT("\\*"));
        #endif

        if (verbose) _tprintf(TEXT("\nDirectory path is %s.\n"), tfolder);

        // Find the first file in the directory
        if ((handle = FindFirstFile(tfolder, &ffd)) == INVALID_HANDLE_VALUE)
        {
            std::cerr << __FUNCTION__ << ": ERROR: INVALID HANDLE VALUE (" << directoryname << ")" << std::endl;
            if (log != nullptr) log->error << __FUNCTION__ << ": ERROR: INVALID HANDLE VALUE (" << directoryname << ")" << '\n';
            return false;
        }

        // List all the files in the directory with some info about them.
        do
        {
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (verbose) _tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);

                #if defined(_UNICODE) || defined(UNICODE)
                std::wstring wdir(ffd.cFileName);
                if ((wdir != L".") && (wdir != L".."))
                {
                    std::string subdir = ws2s(ffd.cFileName);
                    subdir = directoryname + '\\' + subdir;
                    deleteDirectory(subdir);
                }
                #else
                std::string dir(ffd.cFileName);
                if (dir != "." && dir != "..")
                {
                    std::string subdir(ffd.cFileName);
                    subdir = directoryname + '\\' + subdir;
                    deleteDirectory(subdir);
                }
                #endif
            }
            else
            {
                filesize.LowPart  = ffd.nFileSizeLow;
                filesize.HighPart = ffd.nFileSizeHigh;

                if (verbose) _tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, (long)filesize.QuadPart);

                TCHAR  dfile[MAX_PATH]; // full filename for deletion purposes
                StringCchCopy(dfile, MAX_PATH, dfolder); // copy foldername for deletion
                StringCchCat(dfile, MAX_PATH, TEXT("\\"));
                StringCchCat(dfile, MAX_PATH, ffd.cFileName);

                if (verbose) _tprintf(TEXT("Attempting to DeleteFile %s\n"), dfile);

                FlushFileBuffers(handle);

                BOOL ret = 1;
                DWORD Err = 0x20;
                int count = 0;

                ret = DeleteFile(dfile);
                Err = GetLastError();
                if (Err != 0)
                {
                    if (verbose) std::cerr << "DeleteFile of " << dfile << " return code is 0x" << std::hex << Err << std::endl;
                }

                if (verbose) LastErrorText();

                #if defined(_UNICODE) || defined(UNICODE)
                std::string filename = ws2s(dfile);
                #else
                std::string filename(dfile);
                #endif
                swapall(filename, '/', "\\");
                remove(filename.c_str());
            }
        }
        while (FindNextFile(handle, &ffd) != 0);

        if (GetLastError() != ERROR_NO_MORE_FILES)
        {
            return false;
        }

        FindClose(handle);
        if (verbose) _tprintf(TEXT("Attempting to RemoveDirectory %s\n"), dfolder);
        RemoveDirectory(dfolder);

        if (verbose) LastErrorText();

        return true;;
    }


    #pragma region Documentation
    /// <summary>Delete a single file (Windows OS). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="file">The file to be deleted. </parm>
    #pragma endregion
    BOOL deleteFile(std::string file)
    {
        #if defined(_UNICODE) || defined(UNICODE)
        return DeleteFile(s2ws(file).c_str());
        #else
        return DeleteFile(file.c_str());
        #endif
    }

    #endif


    #if !defined(_MSC_VER)

    #pragma region Documentation
    /// <summary>Recursively delete a directory and its contents (non-Windows OS). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="directoryname">The path to be deleted. </parm>
    /// <return>True if the directory and it's contents were successfully deleted (otherwise false). </return>
    #pragma endregion
    bool deleteDirectory(std::string &directoryname)
    {
        bool retval = true;
        DIR *dir;
        struct dirent *entry;
        //char path[PATH_MAX];

        // TODO: VERIFY THAT directoryname exists 1st. No error msg if it does.
        dir = opendir(directoryname.c_str());
        if (dir == nullptr)
        {
            if (errno == ENOENT)
            {
                retval = true; // already deleted
            }
            else
            {
                perror("Error opendir()");
                retval = false;
            }
        }
        else
        {
            while((entry = readdir(dir)) != nullptr)
            {
                if (verbose) std::cout << "PROCESSING " << entry->d_name << std::endl;
                std::string dir_entry(entry->d_name);
                if (dir_entry != "." && dir_entry != "..")
                {
                    std::string fullpath = directoryname + "/" + dir_entry;
                    if (entry->d_type == DT_DIR)
                    {
                        if (verbose) std::cout << "\tDescending into Folder " << fullpath << std::endl;
                        retval = deleteDirectory(fullpath);
                        if (verbose) std::cout << "\tDELETING Folder " << fullpath << std::endl;
                        int ret = remove(fullpath.c_str()); // delete the file
                        retval = (bool)(ret == 0);
                    }
                    else
                    {
                        if (verbose) std::cout << "\tDELETING FILE " << fullpath << std::endl;
                        int ret = remove(fullpath.c_str()); // delete the file
                        retval = (bool)(ret == 0);
                    }
                }
            }
            int ret = closedir(dir);
            if (verbose) std::cout << "\tDELETING Folder " << directoryname << std::endl;
            ret = remove(directoryname.c_str());
            retval = (bool)(ret == 0);
        }

        return retval;
    }


    #pragma region Documentation
    /// <summary>Delete a single file (non-Windows OS). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="file">The file to be deleted. </parm>
    #pragma endregion
    void deleteFile(std::string file)
    {
        remove(file.c_str()); // delete the file
    }

    #endif // Windows OS vs. non-Windows OS


    #pragma region Documentation
    /// <summary>Copy a file from one location to another. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="from">The source path to be copied. </parm>
    /// <parm name="to">The destination path. </parm>
    /// <return>True if the copy appears to have succeeded (otherwise false). </return>
    #pragma endregion
    bool copyFile(std::string from, std::string to)
    {
        bool retval = false;
        std::ifstream src(from, std::ios::binary);
        std::ofstream dst(to, std::ios::binary);
        dst << src.rdbuf();

        return retval;
    }


    // File-reading and File-writing Utilities


    #pragma region Documentation
    /// <summary>Read a (non-binary) file and return its contents. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="fileName">The file to be read. </parm>
    /// <return>The contents of the file as a string. </return>
    #pragma endregion
    std::string readTextFile(std::string fileName)
    {
        std::string retval;

        // Open file stream
        std::ifstream ifs(fileName);

        // Check for successful file open
        if (ifs.is_open())
        {
            std::string str((std::istreambuf_iterator<char>(ifs)),
                             std::istreambuf_iterator<char>());
            ifs.close();
            retval.assign(str);
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Read a (typically binary) file in 10 chunks. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ifs">A reference to the input stream of an open file. </parm>
    /// <parm name="buf">A pointer to a pre-allocated buffer to receive the file contents. </parm>
    /// <parm name="total_size">The total size (in bytes) of the file contents. </parm>
    #pragma endregion
    void inputByChunks(std::ifstream &ifs, unsigned char *buf, long long total_size)
    {
        long long numparts = 10L;
        long long chunksize = total_size / numparts;
        long long remainder = total_size % numparts;
        unsigned char *ptr = buf;

        if (verbose) std::cout << __FUNCTION__ << ": about to read chunks of size " << chunksize << "." << std::endl;
        if (log != nullptr) log->info << __FUNCTION__ << ": about to read chunks of size " << chunksize << "." << '\n';

        for(long long i=0L; i<numparts && ifs.is_open() && !ifs.eof(); ++i)
        {
            ifs.read(reinterpret_cast<char *>(ptr), chunksize);
            ptr += chunksize;
        }

        if (verbose) std::cout << __FUNCTION__ << ": about to read remainder of size " << remainder << "." << std::endl;
        if (log != nullptr) log->info << __FUNCTION__ << ": about to read remainder of size " << remainder << "." << '\n';

        if (remainder > 0L) ifs.read(reinterpret_cast<char *>(ptr), remainder);
    }


    #pragma region Documentation
    /// <summary>Write a (typically binary) file in 10 chunks. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <parm name="ofs">A reference to the output stream of an open file. </parm>
    /// <parm name="buf">A pointer to a buffer containing the content to write to the file. </parm>
    /// <parm name="total_size">The total size (in bytes) of the buffer contents. </parm>
    #pragma endregion
    void outputByChunks(std::ofstream &ofs, unsigned char *buf, long long total_size)
    {
        long long numparts = 10L;
        long long chunksize = total_size / numparts;
        long long remainder = total_size % numparts;
        unsigned char *ptr = buf;

        if (verbose) std::cout << __FUNCTION__ << ": outputting the image chunks of size " << chunksize <<  "." << std::endl;
        if (log != nullptr) log->info << __FUNCTION__ << ": outputting the image chunks of size " << chunksize <<  "." << '\n';

        for(long long i=0L; i<numparts && ofs.is_open(); ++i)
        {
            ofs.write(reinterpret_cast<char *>(ptr), chunksize);
            ptr += chunksize;
        }

        if (verbose) std::cout << __FUNCTION__ << ": outputting the image remainder of size " << remainder << "." << std::endl;
        if (log != nullptr) log->info << __FUNCTION__ << ": outputting the image remainder of size " << remainder << "." << '\n';

        if (remainder > 0L) ofs.write(reinterpret_cast<char *>(ptr), remainder);
    }


    // XML file Interaction


    #pragma region Documentation
    /// <summary>Open the specified XML file and prepare its content for parsing. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>This function must be called before any reading can occur. </remarks>
    /// <parm name="path">The XML file to be opened and prepared. </parm>
    /// <return>True if the specified XML file was successfully opened and prepared (otherwise false). </return>
    #pragma endregion
    bool loadXML(std::string path)
    {
        pugi::xml_parse_result result = pugi_doc.load_file(path.c_str());
        while(!curr_node.empty()) curr_node.pop(); //  clear the current node stack
        return (result);
    }


    #pragma region Documentation
    /// <summary>Open XML internals and prepare for creating a new XML document. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>This function must be called before any writing can occur. </remarks>
    /// <parm name="head_node_label">The label to be used for the head XML node. </parm>
    /// <return>True if the specified node was successfully created (otherwise false). </return>
    #pragma endregion
    bool initializeXML(std::string head_node_label)
    {
        // Start with a fresh XML doc
        concludeXML();

        // Write the first node of a (new) document tree
        pugi::xml_node ph_node = pugi_doc.append_child(head_node_label.c_str());
        curr_node.push(ph_node);
        return (ph_node != 0);
    }

    #pragma region Documentation
    /// <summary>Close any open XML internals. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    #pragma endregion
    void concludeXML()
    {
        // Start with a fresh XML doc
        while(!curr_node.empty()) curr_node.pop();
        pugi_doc.reset();
    }


    #pragma region Documentation
    /// <summary>Save the current (internal) XML document to the specifed path (including filename). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>An XML document must have been created before calling this function. </remarks>
    /// <parm name="path">The path (location and filename) to save the current (internal) XML document. </parm>
    /// <return>True if the specified file successfully created (otherwise false). </return>
    #pragma endregion
    bool saveXML(std::string path)
    {
        bool retval = pugi_doc.save_file(path.c_str());
        if (verbose) std::cout << "\nVCT_CommonLibrary::saveXML: Status is " << (retval?"GOOD":"BAD") << ", path is " << path << std::endl;
        if (log != nullptr) log->info << "\nSAVING XML! Status is " << (retval?"GOOD":"BAD") << ", path is " << path << '\n';

        // read that file will actually be closed if you open and reclose it.
        std::ifstream ifs(path);
        ifs.close();
        return retval;
    }


    #pragma region Documentation
    /// <summary>Read a value from the specified XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>An XML document must have been loaded and prepared before calling this function. </remarks>
    /// <parm name="name">The name of the field whose value is to be read. </parm>
    /// <parm name="value">A reference to the approprate variable to read the node's value into. </parm>
    /// <return>True if the specified node value was successfully read (otherwise false). </return>
    #pragma endregion
    template <typename T>
    bool getNodeValueXML(std::string name, T &value)
    {
        bool retval = false;
        if (findNodeXML(name)) retval = getValueXML(value);
        return retval;
    }


    #pragma region Documentation
    /// <summary>Read a value from the first child node of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>An XML document must have been loaded and prepared before calling this function. </remarks>
    /// <parm name="value">A reference to the approprate variable to read the node's value into. </parm>
    /// <return>True if the child node value was successfully read (otherwise false). </return>
    #pragma endregion
    template <typename T>
    bool getFirstChildNodeValueXML(T &value)
    {
        bool retval = false;
        if (firstChildNodeXML()) retval = getSiblingValueXML(value);
        return retval;
    }


    #pragma region Documentation
    /// <summary>Read a value from the next child node of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>The first child node should be read before calling this function. </remarks>
    /// <parm name="value">A reference to the approprate variable to read the node's value into. </parm>
    /// <return>True if the next node value was successfully read (otherwise false). </return>
    #pragma endregion
    template <typename T>
    bool getNextNodeValueXML(T &value)
    {
        bool retval = false;
        if (nextNodeXML()) retval = getSiblingValueXML(value);
        return retval;
    }


    #pragma region Documentation
    /// <summary>Write the specified string value to the specified node into the current XML document. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>An XML document must have been created before calling this function. </remarks>
    /// <parm name="name">The name of the field the value is to be written into. </parm>
    /// <parm name="value">The string value to writtend to the XML node. </parm>
    /// <return>True if the node value was successfully written (otherwise false). </return>
    #pragma endregion
    bool writeNodeValueXML(std::string name, std::string value)
    {
        bool retval = false;
        (void) addChild(curr_node.top(), name, value);
        return retval;
    }


    #pragma region Documentation
    /// <summary>Write the specified (non-string) value to the specified node into the current XML document. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>An XML document must have been created before calling this function. </remarks>
    /// <parm name="name">The name of the field the value is to be written into. </parm>
    /// <parm name="value">The (non-string) value to written to the XML node. </parm>
    /// <return>True if the node value was successfully written (otherwise false). </return>
    #pragma endregion
    template <typename T>
    bool writeNodeValueXML(std::string name, T &value)
    {
        bool retval = false;
        (void) addChild(curr_node.top(), name, std::to_string(value));
        return retval;
    }


    #pragma region Documentation
    /// <summary>Search for the specified node within the current XML document. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>An XML document must have been loaded and prepared before calling this function. </remarks>
    /// <remarks>If found the node becomes the current node within the internal XML document structure. </remarks>
    /// <parm name="name">The name of the field to find. </parm>
    /// <return>True if the node is found (otherwise false). </return>
    #pragma endregion
    bool findNodeXML(std::string name)
    {
        bool retval = false;
        pugi::xml_node node;

        if (curr_node.empty())
        {
            node = pugi_doc.child(name.c_str());
        }
        else
        {
            pugi::xml_node topnode = curr_node.top();
            if (topnode) node = topnode.child(name.c_str());
        }

        if (node)
        {
            curr_node.push(node);
            retval = true;
        }

        return retval;
    }


    #pragma region Documentation
    /// <summary>Query - get name of - the current node (top of stack). </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>A current node should be successfully established before calling this function. </remarks>
    /// <return>Name of current node if there is one (otherwise "N/A"). </return>
    #pragma endregion
    std::string queryCurrentNodeXML()
    {
        std::string retval("N/A");

        if (!curr_node.empty())
        {
            pugi::xml_node node = curr_node.top();
            retval = node.name();
            retval += " at pos ";
            std::ostringstream oss; oss << node.offset_debug();
            retval += oss.str();
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Located the first child node of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>An XML document must have been loaded and prepared before calling this function. </remarks>
    /// <return>True if the first child node value was successfully located (otherwise false). </return>
    #pragma endregion
    bool firstChildNodeXML()
    {
        bool retval = false;
        pugi::xml_node node;
        if (!curr_node.empty())
        {
            node = curr_node.top();
            node = node.first_child();
            if (node)
            {
                // Replace with sibling
                curr_node.push(node);
                retval = true;
            }
        }

        return retval;
    }


    #pragma region Documentation
    /// <summary>Located the next child node of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>The first child node should be located before calling this function. </remarks>
    /// <return>True if the next child node value was successfully located (otherwise false). </return>
    #pragma endregion
    bool nextNodeXML()
    {
        bool retval = false;
        pugi::xml_node node;
        if (!curr_node.empty())
        {
            node = curr_node.top();
            node = node.next_sibling();
            if (node)
            {
                // Replace with sibling
                curr_node.pop();
                curr_node.push(node);
                retval = true;
            }
        }

        return retval;
    }


    #pragma region Documentation
    /// <summary>Located the next child node of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>The first child node should be located before calling this function. </remarks>
    /// <parm name="sibling_name">The name of the next sibling. </parm>
    /// <return>True if the next child node value was successfully located (otherwise false). </return>
    #pragma endregion
    bool nextNodeXML(std::string sibling_name)
    {
        bool retval = false;
        pugi::xml_node node;
        if (!curr_node.empty())
        {
            curr_node.pop();

            node = curr_node.top();
            node = node.next_sibling(sibling_name.c_str());
            if (node)
            {
                // Replace with sibling
                curr_node.pop();
                curr_node.push(node);
                retval = true;
            }
        }

        return retval;
    }


    // Lower-level XML-reading functions



    #pragma region Documentation
    /// <summary>Read the string value of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>A current node should be successfully established before calling this function. </remarks>
    /// <parm name="value">A reference to the variable to receive the string value. </parm>
    /// <return>True if the string value was successfully read (otherwise false). </return>
    #pragma endregion
    bool getValueXML(std::string &value)
    {
        bool retval = false;
        if (!curr_node.empty())
        {
            pugi::xml_node node = curr_node.top();
            value = node.first_child().value();
            concludeSectionXML(); // after reading a node's value we are presumed to be done with it
            retval = true;
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Read the (non-string) value of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>A current node should be successfully established before calling this function. </remarks>
    /// <parm name="value">A reference to the variable to receive the (non-string) value. </parm>
    /// <return>True if the (non-string) value was successfully read (otherwise false). </return>
    #pragma endregion
    template <typename T>
    bool getValueXML(T &value)
    {
        bool retval = false;
        if (!curr_node.empty())
        {
            pugi::xml_node node = curr_node.top();
            std::stringstream(node.first_child().value()) >> value;
            concludeSectionXML(); // after reading a node's value we are presumed to be done with it
            retval = true;
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Read the string value of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>A current node should be successfully established before calling this function. </remarks>
    /// <parm name="value">A reference to the variable to receive the string value. </parm>
    /// <return>True if the string value was successfully read (otherwise false). </return>
    #pragma endregion
    bool getSiblingValueXML(std::string &value)
    {
        bool retval = false;
        if (!curr_node.empty())
        {
            pugi::xml_node node = curr_node.top();
            value = node.first_child().value();
            retval = true;
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Read the value from the sibling of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>A current node should be successfully established before calling this function. </remarks>
    /// <parm name="value">A reference to the variable to receive the (non-string) value. </parm>
    /// <return>True if the value was successfully read (otherwise false). </return>
    #pragma endregion
    template <typename T>
    bool getSiblingValueXML(T &value)
    {
        bool retval = false;
        if (!curr_node.empty())
        {
            pugi::xml_node node = curr_node.top();
            std::stringstream(node.first_child().value()) >> value;
            retval = true;
        }
        return retval;
    }


    #pragma region Documentation
    /// <summary>Begin a new section after the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>A current node should be successfully established before calling this function. </remarks>
    /// <parm name="sectionName">The name of the new section to create. </parm>
    /// <return>True if the section was successfully created (otherwise false). </return>
    #pragma endregion
    bool addASectionXML(std::string sectionName)
    {
        bool status = false;
        if (!curr_node.empty())
        {
            pugi::xml_node section_node = curr_node.top().append_child(sectionName.c_str());
            if (section_node)
            {
                curr_node.push(section_node);
                status = true;
            }
        }
        return status;
    }


    #pragma region Documentation
    /// <summary>End the current section and return to previous level in the XML document hierarchy. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>A current node should be successfully established before calling this function. </remarks>
    /// <return>True if the section was successfully exited (otherwise false). </return>
    #pragma endregion
    bool concludeSectionXML()
    {
        bool status = false;

        if (!curr_node.empty())
        {
            curr_node.pop();
            status = true;
        }
        return status;
    }



    #pragma region Documentation
    /// <summary>Add a child of the current XML node. </summary>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>An XML document must have been created before calling this function. </remarks>
    /// <parm name="parent">The name of intended parent of the new node. </parm>
    /// <parm name="childName">The name of the new child node to create. </parm>
    /// <parm name="data">The value to be placed in the new childs (expressed as a string value). </parm>
    /// <return>The new child node (otherwise nullptr). </return>
    #pragma endregion
    pugi::xml_node addChild(pugi::xml_node parent, std::string childName, std::string data)
    {
        pugi::xml_node retval = parent.append_child(childName.c_str());
        if (retval && data.size() > 0)
        {
            retval.append_child(pugi::node_pcdata).set_value(data.c_str());
        }
        return retval;
    }


private:

    bool verbose;                   /// verbose flag
    #if __cplusplus > 199711L
    std::chrono::time_point<std::chrono::steady_clock> timer_start;  /// start time for timed tasks
    std::map<int, std::chrono::time_point<std::chrono::steady_clock> > timer_start_map;   /// start time for timer n for timed tasks
    #else
    unsigned int  timer_start;      /// start time for timed tasks
    std::map<int, unsigned int> timer_start_map; /// start time for timer n for timed tasks
    #endif

    std::ofstream logstream;        /// output filestream for logger
    vct::Logger  *log;              /// pointer to logger

    pugi::xml_document         pugi_doc;  /// pugi xml document instance
    std::stack<pugi::xml_node> curr_node; /// STL stack of current XML nodes
};


} // namespace vct
