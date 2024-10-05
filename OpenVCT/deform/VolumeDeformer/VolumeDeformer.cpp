// VolumeDeformer.cpp

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#endif

#include "VolumeDeformer.h"
#include "VCT_CommonLibrary.h"
#include "VCT_Logger.h"
#include "VCT_Deformation.h"
#include "ReadWritePhantom.h"
#include "pugixml.hpp"

#include <cfloat>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

// #version 440 was original version in legacy code
// #version is updated to match OpenGL driver when compiled

//Fragment shader
const char* VolumeDeformer::srcFragShader = "\
    #version 420 \n\
    uniform usampler3D sampler3d;\
    smooth in vec3  triTexture;\
    out uint outColor;\
    void main (void)\
    {\
        uint texel = texture(sampler3d, triTexture).x;\
        outColor = texel;\
        if (outColor == 0) discard;\
    }";


// Vertex shader 
const char*  VolumeDeformer::srcVertShader = "\
    #version 420 \n\
    in vec3 myVertex;\
    in vec3 myUV;\
    out vec3 tetraTexture;\
    void main(void)\
    {\
        gl_Position = vec4(myVertex, 1.0);\
        tetraTexture = myUV;\
    }";

/*
The Geometry shader considers a special occasion where an end point lying on the plane. 
In this case, the end point is added just once. 
EndPtAdded and AddEndPoint are used to  keep track of this condition. 
It is possible more than 1 point are on the plane. However, this is not considered by the shader.
Only the first element of the offset vector is used. This is the coefficient that adjusts the Z value,
to accommodate bricking (breaking a large 3D texture into manageable chunks).

ADM  -  have to multiple x and y by 2.0, but z by 1.0.   We don't fully understand why, but it works.
This requires that the x and y have input offsets of 0.5 (see calling routine).  In so doing, the 
actual offset is -1 after multiplication, so the data is renormalized from [0,1] to [-1,1].
*/
const char*  VolumeDeformer::srcGeoShader = "\
    #version 420 \n\
    layout (lines_adjacency) in;\
    layout (triangle_strip, max_vertices = 4) out;\
    in vec3 tetraTexture[4];\
    smooth out vec3 triTexture;\
    uniform vec4 normalPlane;\
    uniform vec4 pointPlane;\
    uniform vec4 offset;\
    void main(void)\
    {\
        int count =0;\
        int EndPtAdded = 0;\
        for (int i=0; i<4;i++)\
        {\
            for (int j=i+1; j<4; j++)\
            {\
                vec4 v1 = gl_in[i].gl_Position;\
                vec4 v2 = gl_in[j].gl_Position;\
                float dotp = dot(normalPlane, v2-v1);\
                if (dotp != 0.0)\
                {\
                    int AddEndPt = 0;\
                    float u = dot(normalPlane, pointPlane - v1)/dotp;\
                    if ((EndPtAdded == 0) && ((u == 1.0) || (u==0.0)))\
                    {\
                        AddEndPt = 1;\
                        EndPtAdded = 1;\
                    }\
                    if ((AddEndPt == 1) || ((u < 1.0) && (u > 0.0))) \
                    {\
                        triTexture    = u*tetraTexture[j] + (1.0 - u) * tetraTexture[i];\
                        gl_Position   = v1 + u*(v2-v1);\
                        gl_Position.x = 2.0*(gl_Position.x - offset.x);\
                        gl_Position.y = 2.0*(gl_Position.y - offset.y);\
                        triTexture.z  = 1.0*(triTexture.z - offset.z);\
                        EmitVertex ();\
                        count++;\
                    }\
                }\
            }\
        }\
        if (count > 0) EndPrimitive ();\
        \
    }";



#pragma region Documentation
/// <summary>(default) Constructor.</summary>
/// <remarks>Joe Chui, 10/12/2011.</remarks>
#pragma endregion

VolumeDeformer::VolumeDeformer()
    : 
	  #if defined(_MSC_VER)
	     hRC2(0),
      #endif
      vxlCntIn_x(0.0f), vxlCntIn_y(0.0f), vxlCntIn_z(0.0f),
      voxelSize_x(0.0f), voxelSize_y(0.0f), voxelSize_z(0.0f),
      verbose(false)
{
    hasValidRC = false;

    max[0] = max[1] = max[2] = max[3] = max[4] = max[5] = FLT_MIN;
    min[0] = min[1] = min[2] = min[3] = min[4] = min[5] = FLT_MAX;

    glBindBuffer = NULL;
    glGetUniformLocation=NULL;
    glUniformMatrix4fv = NULL;
    glUniform4f = NULL;
    glEnableVertexAttribArray = NULL;
    glVertexAttribPointer = NULL;

    clib.setLogger("VolumeDeformer.log", vct::Logger::LOG_INFO);
}


#pragma region Documentation
/// <summary>Store the uncompressed phantom's voxel counts.</summary>
/// <remarks>2017-07-01.</remarks>
/// <param name="vxlCntX">Voxel count (extent) in X direction.</param>
/// <param name="vxlCntY">Voxel count (extent) in Y direction.</param>
/// <param name="vxlCntZ">Voxel count (extent) in Z direction.</param>
#pragma endregion

void VolumeDeformer::setPhantomVoxelCounts(float vxlCntX, float vxlCntY, float vxlCntZ)    // from phantom header
{
    vxlCntIn_x = vxlCntX;
    vxlCntIn_y = vxlCntY;
    vxlCntIn_z = vxlCntZ;
}


#pragma region Documentation
/// <summary>Store the uncompressed phantom's voxel size(s).</summary>
/// <remarks>2017-07-01.</remarks>
/// <param name="vsizeX">Voxel size in X direction, in meters.</param>
/// <param name="vsizeY">Voxel size in Y direction, in meters.</param>
/// <param name="vSizeZ">Voxel size in Z direction, in meters.</param>
#pragma endregion

void VolumeDeformer::setPhantomVoxelSizes(float vsizeX, float vsizeY, float vSizeZ) // in meters, from command line (or default)
{
    voxelSize_x = vsizeX;
    voxelSize_y = vsizeY;
    voxelSize_z = vSizeZ;
}



#if defined(_MSC_VER)

#pragma region Documentation
/// <summary>Set up Microsoft Windows OS windows for OpenGL rendering.</summary>
/// <remarks>Joe Chui, 10/12/2011.</remarks>
/// <param name="pdlog">Active log file stream.</param>
/// <param name="originalMesh">Filename of undeformed mesh (.feb).</param>
/// <param name="deformedMesh">Filename of deformed mesh (.log).</param>
/// <param name="handle">Handle to 1st background window, used to create WGL context.</param>
/// <param name="handle2">Handle to 2nd background window, used to create ARB context.</param>
/// <return>Boolean indicating success or failure.</return>
#pragma endregion
bool VolumeDeformer::Process(vct::Logger &pdlog, std::string originalMesh, std::string deformedMesh, HWND handle, HWND handle2)

#elif defined(__linux__)

#pragma region Documentation
/// <summary>Set up Linux X Windows for OpenGL rendering.</summary>
/// <remarks>D. Higginbotham, 2020-08-03.</remarks>
/// <param name="pdlog">Active log file stream.</param>
/// <param name="originalMesh">Filename of undeformed mesh (.feb).</param>
/// <param name="deformedMesh">Filename of deformed mesh (.log).</param>
/// <param name="win1">Handle to 1st background window, used to create GLX context.</param>
/// <param name="win2">Handle to 2nd background window, used to create ARB context.</param>
/// <return>Boolean indicating success or failure.</return>
#pragma endregion
bool VolumeDeformer::Process(vct::Logger &pdlog, std::string originalMesh, std::string deformedMesh, Display *display, int screen, Window win1, Window win2)
    
#endif
{
    bool retval = false;
    
    #if defined(_MSC_VER)    

        // Insert something from the VolumeVisualizer constructor here
        hWnd = handle;
        hDC2 = NULL;
        hDC = GetDC(hWnd);           // get the device context for window
        hRC = wglCreateContext(hDC); // create rendering context

        if (handle2) hDC2 = GetDC(handle2);

        if (verbose)
        {
            std::cout  << __FUNCTION__ << ": Setting up OpenGL.\n" << std::endl;
            pdlog.info << __FUNCTION__ << ": Setting up OpenGL.\n";
        }

        // Define pixel descriptor (double buffer, support ogl, draw window, version, color and depth bit)
        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
        pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion   = 1;
        pfd.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW; // (single-buffer, by default)
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int nPixelFormat = ChoosePixelFormat(hDC, &pfd);
        if (nPixelFormat == 0)
        {
            std::cerr   << __FUNCTION__ << ": Could not choose pixel format for windows" << std::endl;
            pdlog.error << __FUNCTION__ << ": Could not choose pixel format for windows\n";
            retval = false; 
            return retval;
        }

        // Set the pixel format for both window device contexts
        BOOL result = SetPixelFormat(hDC, nPixelFormat, &pfd);
        if (hDC2)  result = SetPixelFormat(hDC2, nPixelFormat, &pfd);
        if (!result)
        {
            std::cerr   << __FUNCTION__ << ": Could not set up pixel format for windows" << std::endl;
            pdlog.error << __FUNCTION__ << ": Could not set up pixel format for windows\n";
            retval = false; 
            return retval;
        }

        HGLRC tempContext = wglCreateContext(hDC);
        wglMakeCurrent(hDC, tempContext);

        int attribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4, //3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 2,
            WGL_CONTEXT_FLAGS_ARB, 0,
            WGL_CONTEXT_PROFILE_MASK_ARB , WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
            0
        };

        // Context ARB: "Architecture Review Board" (windows): ogl > 3.X
        PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;
        wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

        if (wglCreateContextAttribsARB)        
        {
            if (verbose)
            {
                std::cout  << "Creating 3.2 context." << std::endl;
                pdlog.info << "Creating 3.2 context.\n";
            }

            hRC = wglCreateContextAttribsARB(hDC,0, attribs);
            wglMakeCurrent(NULL,NULL);
            wglDeleteContext(tempContext);
            wglMakeCurrent(hDC, hRC);

            if (hRC2)
            {
                if (verbose)
                {
                    std::cout  << "Sharing context" << std::endl;
                    pdlog.info << "Sharing context\n";
                }

                hRC2 = wglCreateContextAttribsARB(hDC2,0, attribs);
                wglShareLists(hRC, hRC2);
            }
        }
        else
        {   
            std::cerr   << "It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)" << std::endl;
            pdlog.error << "It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)\n";
            retval = false; 
            return retval;
        }   

        hasValidRC = true;
    
    
    #endif
    
    
    // Use the GL3 way to get the OpenGL version number
    const GLubyte *GLVersionString; 
    int OpenGLVersion[2];
    glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
    glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);
    float oglVersion = OpenGLVersion[0] + 0.1f * OpenGLVersion[1];

    GLVersionString = glGetString(GL_SHADING_LANGUAGE_VERSION);
    float glslVersion;
    std::string glvs((char *)GLVersionString);
    std::istringstream iss(glvs);
    iss >> glslVersion;

    std::cout << "\n* OpenGL version " << OpenGLVersion[0] << "." << OpenGLVersion[1] 
              << ", version string: \"" << GLVersionString << "\" *\n" << std::endl;
    pdlog.info << "* OpenGL version " << OpenGLVersion[0] << "." << OpenGLVersion[1] 
               << ", version string: \"" << GLVersionString << "\" *\n";
    
    // Acquire the OpenGL version of this computing environment
    std::ostringstream oss;
    oss << OpenGLVersion[0] << OpenGLVersion[1] << '0';
    std::string version_number(oss.str());
    
    // Update the shaders to reflect the OpenGL version
    std::string frag_shader(srcFragShader);
    size_t pos = frag_shader.find("#version ");
    if (pos != std::string::npos)
    {
        if (glslVersion == oglVersion)
        {
            frag_shader = frag_shader.substr(0, pos+9) + version_number + frag_shader.substr(pos+12);
        }
        else
        {
            frag_shader = frag_shader.substr(0, pos-1) + "// " + frag_shader.substr(pos);
        }
        srcFragShader = reinterpret_cast<const char *>(frag_shader.c_str());
        pdlog.info << "frag_shader version changed:\n" << frag_shader.substr(0,20) << ".\n";
    }

    std::string vert_shader(srcVertShader);
    pos = vert_shader.find("#version ");
    if (pos != std::string::npos)
    {
        if (glslVersion == oglVersion)
        {
            vert_shader = vert_shader.substr(0, pos+9) + version_number + vert_shader.substr(pos+12);
        }
        else
        {
            vert_shader = vert_shader.substr(0, pos-1) + "// " + vert_shader.substr(pos);
        }
        srcVertShader = reinterpret_cast<const char *>(vert_shader.c_str());
        pdlog.info << "VertShader version set to:\n" << vert_shader.substr(0,20) << ".\n";
    }

    std::string geo_shader(srcGeoShader);
    pos = geo_shader.find("#version ");
    if (pos != std::string::npos)
    {
        if (glslVersion == oglVersion)
        {
            geo_shader = geo_shader.substr(0, pos+9) + version_number + geo_shader.substr(pos+12);
        }
        else
        {
            geo_shader = geo_shader.substr(0, pos-1) + "// " + geo_shader.substr(pos);
        }
        srcGeoShader = reinterpret_cast<const char *>(geo_shader.c_str());
        pdlog.info << "GeoShader version set to:\n" << geo_shader.substr(0,20) << ".\n";
    }

    #if defined(_MSC_VER)
    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
    #elif defined(__linux__)
    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) glXGetProcAddress((const GLubyte *)"glGenFramebuffers");
    #endif
    
    pdlog.info << "Address of glGenFramebuffers process is: " << (uint64_t) glGenFramebuffers << "\n"; // DEBUG DH 2020-09-02
    
    glGenFramebuffers(1, &frameBuffer);
    if (frameBuffer == 0) 
    {
        std::cout << "Using the default Framebuffer." << std::endl;
        pdlog.info << "Using the default Framebuffer.\n";
    }

    
    #if defined(_MSC_VER)
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
    #elif defined(__linux__)
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) glXGetProcAddress((const GLubyte *)"glBindFramebuffer");
    #endif
    
    pdlog.info << "Address of glBindFramebuffer process is: " << (uint64_t) glBindFramebuffer << "\n"; // DEBUG DH 2020-09-02
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer); 

    BuildGLSLProgram(pdlog);

    // Read Both Mesh Files 
    readSourceMeshFEBio(pdlog, originalMesh, elements, nodes); // .log mesh file
    readDefNodesFEBio(pdlog, deformedMesh, nodes);             // .feb mesh file

    #if defined(_MSC_VER)
    PFNGLGENBUFFERSPROC glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
    #elif defined(__linux__)
    PFNGLGENBUFFERSPROC glGenBuffers = (PFNGLGENBUFFERSPROC) glXGetProcAddress((const GLubyte *)"glGenBuffers");
    #endif
    
    pdlog.info << "Address of glGenBuffers process is: " << (uint64_t) glGenBuffers << "\n"; // DEBUG DH 2020-09-02
    
    glGenBuffers(2, vbo32);
    
    vertexStride = 6 * sizeof(GLfloat); // 3 floats for the pos, 3 for the UVs 
    
    #if defined(_MSC_VER)   
    PFNGLBUFFERDATAPROC glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
    #elif defined(__linux__)
    PFNGLBUFFERDATAPROC glBufferData = (PFNGLBUFFERDATAPROC) glXGetProcAddress((const GLubyte *)"glBufferData");
    #endif
    
    pdlog.info << "Address of glBufferData process is: " << (uint64_t) glBufferData << "\n"; // DEBUG DH 2020-09-02

    #if defined(_MSC_VER)   
    glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
    #elif defined(__linux__)
    glBindBuffer = (PFNGLBINDBUFFERPROC) glXGetProcAddress((const GLubyte *)"glBindBuffer");
    #endif
    
    pdlog.info << "Address of glBindBuffer process is: " << (uint64_t) glBindBuffer << "\n"; // DEBUG DH 2020-09-02

    // Bind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, vbo32[0]);

    // Set the buffer's data: Nodes (vbo32[0]) are 3D Vertices; deformed are "myVertex" & predeformed are "myUV" in vertex shader 
    glBufferData(GL_ARRAY_BUFFER, nodes.size() *  sizeof(GLfloat) , &nodes[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo32[1]);

    // Set the buffer's data: elements (vbo32[1]) are Node indicies 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() *  sizeof(GLuint) , &elements[0], GL_STATIC_DRAW);

    return true;
}


#pragma region Documentation
/// <summary>Build the shaders program with the shader functions (fragment, vertex, & geometry).</summary>
/// <remarks>Joe Chui, 10/12/2011.</remarks>
/// <param name="pdlog">Active log file stream.</param>
#pragma endregion

void VolumeDeformer::BuildGLSLProgram(vct::Logger &pdlog)
{
    std::cout << "Building shaders program..." << std::endl;
    std::cout << "version oglsl " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Verify shader creation
    #if defined(_MSC_VER)
    PFNGLCREATESHADERPROC glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
    #elif defined(__linux__)
    PFNGLCREATESHADERPROC glCreateShader = (PFNGLCREATESHADERPROC) glXGetProcAddress((const GLubyte *)"glCreateShader");
    #endif
    
    pdlog.info << "Address of glCreateShader process is: " << (uint64_t) glCreateShader << "\n"; // DEBUG DH 2020-09-02
    
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    #if defined(_MSC_VER)
    PFNGLSHADERSOURCEPROC glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
    #elif defined(__linux__)
    PFNGLSHADERSOURCEPROC glShaderSource = (PFNGLSHADERSOURCEPROC) glXGetProcAddress((const GLubyte *)"glShaderSource");
    #endif
    
    pdlog.info << "Address of glShaderSource process is: " << (uint64_t) glShaderSource << "\n"; // DEBUG DH 2020-09-02
    
    // Load the source code into it
    glShaderSource(fragShader, 1, (const char**)&srcFragShader, NULL);

    #if defined(_MSC_VER)
    PFNGLCOMPILESHADERPROC glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
    #elif defined(__linux__)
    PFNGLCOMPILESHADERPROC glCompileShader = (PFNGLCOMPILESHADERPROC) glXGetProcAddress((const GLubyte *)"glCompileShader");
    #endif
    
    pdlog.info << "Address of glCompileShader process is: " << (uint64_t) glCompileShader << "\n"; // DEBUG DH 2020-09-02
    
    // Compile the source code
    glCompileShader(fragShader);

    #if defined(_MSC_VER)
    PFNGLGETSHADERIVPROC glGetShaderiv = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
    #elif defined(__linux__)
    PFNGLGETSHADERIVPROC glGetShaderiv = (PFNGLGETSHADERIVPROC) glXGetProcAddress((const GLubyte *)"glGetShaderiv");
    #endif
    
    pdlog.info << "Address of glGetShaderiv process is: " << (uint64_t) glGetShaderiv << "\n"; // DEBUG DH 2020-09-02
    
    // Check if compilation succeeded
    GLint shader_compiled;
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &shader_compiled);

    #if defined(_MSC_VER)
    PFNGLGETPROGRAMINFOLOGPROC glGetShaderInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
    #elif defined(__linux__)
    PFNGLGETPROGRAMINFOLOGPROC glGetShaderInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) glXGetProcAddress((const GLubyte *)"glGetShaderInfoLog");
    #endif
    
    pdlog.info << "Address of glGetShaderInfoLog process is: " << (uint64_t) glGetShaderInfoLog << "\n"; // DEBUG DH 2020-09-02
    
    // Print out any OpenGL errors
    clib.CheckGlErrors("frag shader");

    if (!shader_compiled)
    {
        std::cout << "Error: fragment shader was not compiled" << std::endl;
        pdlog.error << "Deformer Fragment Shader did not compile!\n";

        // An error happened, first retrieve the length of the log message
        int i32InfoLogLength, i32CharsWritten;
        glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

        // Allocate enough space for the message and retrieve it
        char* info_log = new char[i32InfoLogLength];

        glGetShaderInfoLog(fragShader, i32InfoLogLength, &i32CharsWritten, info_log);

        std::cout << "Failed to compile Deformer Fragment Shader" << std::endl;
        pdlog.info << "Failed to compile Deformer Fragment Shader\n";

        return;
    }

    if (verbose)
    {
        std::cout << "Fragment deformer shader compiled" << std::endl;
        pdlog.info << "Deformer Fragment Shader compiled\n";
    }
    
    // Load the vertex shader in the same way
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char**)&srcVertShader, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shader_compiled);
    
    // Print out any OpenGL errors
    clib.CheckGlErrors("vertex shader");
    
    if (!shader_compiled)
    {
        std::cerr << "Error: vertex shader was not compiled" << std::endl;
        pdlog.error << "Deformer Vertex Shader did not compile!\n";

        int i32InfoLogLength, i32CharsWritten;

        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
        char* info_log = new char[i32InfoLogLength];

        glGetShaderInfoLog(vertexShader, i32InfoLogLength, &i32CharsWritten, info_log);
        char* msg = new char[i32InfoLogLength+256];
        
        strcpy(msg, "Failed to compile Deformer Vertex Shader: ");
        strcat(msg, info_log);

        std::cout << msg << info_log << std::endl;
        pdlog.info << msg << info_log << "\n";
        
        delete [] msg;
        delete [] info_log;
        return;
    }
    
    if (verbose)
    {
        std::cout << "Vertex deformation shader compiled" << std::endl;
        pdlog.info << "Deformer Vertex Shader compiled.\n";
    }
    
    // Load the geometry shader in the same way 
    geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometryShader, 1, (const char**)&srcGeoShader, NULL);
    glCompileShader(geometryShader);
    glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &shader_compiled);
    
    // Print out any OpenGL errors
    clib.CheckGlErrors("geometry shader");
    
    if (!shader_compiled)
    {
        std::cerr << "Error: vertex shader was not compiled" << std::endl;
        pdlog.error << "Deformer Geometry Shader did not compile!\n";

        int i32InfoLogLength, i32CharsWritten;
        glGetShaderiv(geometryShader, GL_INFO_LOG_LENGTH, &i32InfoLogLength);

        char* log_info = new char[i32InfoLogLength];
        glGetShaderInfoLog(geometryShader, i32InfoLogLength, &i32CharsWritten, log_info);
        
        char* msg = new char[i32InfoLogLength+256];
        strcpy(msg, "Failed to compile Deformer Geometry Shader: ");
        strcat(msg, log_info);

        std::cout << msg << log_info << std::endl;
        pdlog.info << msg << log_info << "\n";

        delete [] msg;
        delete [] log_info;
        return;
    }

    if (verbose)
    {
        std::cout << "Deformer Geometry Shader compiled" << std::endl;
        pdlog.info << "Deformer Geometry Shader compiled.\n";
    }

    #if defined(_MSC_VER)
    PFNGLCREATEPROGRAMPROC glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
    #elif defined(__linux__)
    PFNGLCREATEPROGRAMPROC glCreateProgram = (PFNGLCREATEPROGRAMPROC) glXGetProcAddress((const GLubyte *)"glCreateProgram");
    #endif
    
    pdlog.info << "Address of glCreateProgram process is: " << (uint64_t) glCreateProgram << "\n"; // DEBUG DH 2020-09-02

    // Create the shader program
    programObject = glCreateProgram();
    
    // Print out any OpenGL errors
    clib.CheckGlErrors("glCreateProgram");

    #if defined(_MSC_VER)
    PFNGLATTACHSHADERPROC glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
    #elif defined(__linux__)
    PFNGLATTACHSHADERPROC glAttachShader = (PFNGLATTACHSHADERPROC) glXGetProcAddress((const GLubyte *)"glAttachShader");
    #endif
    
    pdlog.info << "Address of glAttachShader process is: " << (uint64_t) glAttachShader << "\n"; // DEBUG DH 2020-09-02

    // Attach the fragment and vertex shaders to it
    glAttachShader(programObject, fragShader);
    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, geometryShader);

    #if defined(_MSC_VER)
    PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) wglGetProcAddress("glBindAttribLocation"); 
    #elif defined(__linux__)
    PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) glXGetProcAddress((const GLubyte *)"glBindAttribLocation");
    #endif
    
    pdlog.info << "Address of glBindAttribLocation process is: " << (uint64_t) glBindAttribLocation << "\n"; // DEBUG DH 2020-09-02


    // Bind the custom vertex attribute vbo32[0], "myVertex" (nodes) to location VERTEX_ARRAY. It's a Lines_Adjacency Array Buffer.
    glBindAttribLocation(programObject, 0, "myVertex");
    
    // Print out any OpenGL errors
    clib.CheckGlErrors("glBindAttribLocation, myVertex");

    // Bind the custom vertex attribute vbo32[1], "myUV" (elements) to location TEXCOORD_ARRAY. It's a Triangle_Strip Element Array Buffer
    glBindAttribLocation(programObject, 1, "myUV");
    
    // Print out any OpenGL errors
    clib.CheckGlErrors("glBindAttribLocation, myUV");

    #if defined(_MSC_VER)
    PFNGLLINKPROGRAMPROC glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
    #elif defined(__linux__)
    PFNGLLINKPROGRAMPROC glLinkProgram = (PFNGLLINKPROGRAMPROC) glXGetProcAddress((const GLubyte *)"glLinkProgram");
    #endif
    
    pdlog.info << "Address of glLinkProgram process is: " << (uint64_t) glLinkProgram << "\n"; // DEBUG DH 2020-09-02

    // Link the program
    glLinkProgram(programObject);
    
    // Print out any OpenGL errors
    clib.CheckGlErrors("glLinkProgram");

    // Check if linking succeeded in the same way we checked for compilation success
    GLint linked;
    #if defined(_MSC_VER)
    PFNGLGETPROGRAMIVPROC glGetProgramiv = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
    #elif defined(__linux__)
    PFNGLGETPROGRAMIVPROC glGetProgramiv = (PFNGLGETPROGRAMIVPROC) glXGetProcAddress((const GLubyte *)"glGetProgramiv");
    #endif
    
    pdlog.info << "Address of glGetProgramiv process is: " << (uint64_t) glGetProgramiv << "\n"; // DEBUG DH 2020-09-02
    
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
    
    // Print out any OpenGL errors
    clib.CheckGlErrors("glGetProgramiv");

    if (!linked)
    {
        std::cerr << "Error: shadersprogram did not link" << std::endl;
        pdlog.error << "Deformer Shaders program did not link!\n";

        int i32InfoLogLength, i32CharsWritten;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &i32InfoLogLength);
        char* info_log = new char[i32InfoLogLength];

        #if defined(_MSC_VER)
        PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
        #elif defined(__linux__)
        PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) glXGetProcAddress((const GLubyte *)"glGetProgramInfoLog");
        #endif
    
        pdlog.info << "Address of glGetProgramInfoLog process is: " << (uint64_t) glGetProgramInfoLog << "\n"; // DEBUG DH 2020-09-02
        
        glGetProgramInfoLog(programObject, i32InfoLogLength, &i32CharsWritten, info_log);       
        char* msg = new char[i32InfoLogLength+256];

        strcpy(msg, "Failed to link program: ");
        strcat(msg, info_log);

        std::cout << msg << info_log << std::endl;
        pdlog.info << msg << info_log << "\n";

        delete [] msg;
        delete [] info_log;
        return;
    }

    if (verbose)
    {
        std::cout  << "Deformation shaders have been successfully linked" << std::endl;
        pdlog.info << "Deformation shaders have been successfully linked\n";
    }

    #if defined(_MSC_VER)
    PFNGLUSEPROGRAMPROC glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
    #elif defined(__linux__)
    PFNGLUSEPROGRAMPROC glUseProgram = (PFNGLUSEPROGRAMPROC) glXGetProcAddress((const GLubyte *)"glUseProgram");
    #endif
    
    pdlog.info << "Address of glUseProgram process is: " << (uint64_t) glUseProgram << "\n"; // DEBUG DH 2020-09-02

    // Use (invoke) the created shader program
    glUseProgram(programObject);

    // Print out any OpenGL errors
    clib.CheckGlErrors("glUseProgram");
}


#pragma region Documentation
/// <summary>
/// Read the source mesh FE bio (.feb file).
/// Read orig node coords, materials and elements (indicies).
/// Orig node coords are placed in last 3 elements of node.
/// </summary>
/// <remarks>Joe Chui, 10/12/2011. </remarks>
/// <param name="pdlog">Active log file stream.</param>
/// <param name="filePath">The file path.</param>
/// <param name="elements">The elements.</param>
/// <param name="nodes">The nodes.</param>
#pragma endregion

void VolumeDeformer::readSourceMeshFEBio(vct::Logger &pdlog, std::string filePath, std::vector<GLuint> &elements, std::vector<GLfloat> &nodes)
{
    clib.start_timer(1); // time entire function
    
    if (verbose)
    {
        std::cout << "Reading original nodes" << std::endl;
        pdlog.info << "Reading original nodes\n";
    }

    pugi::xml_document doc;
    if (doc.load_file(filePath.c_str()))
    {
        
        if (verbose)
        {
            std::cout << filePath.c_str() << std::endl;
            pdlog.info << filePath << "\n";
        }

        pugi::xml_node tools = doc.child("febio_spec").child("Geometry").child("Nodes");        

        if (verbose)
        {
            std::cout  << "found" << std::endl;
            pdlog.info << "found\n";
            pdlog.info << __FUNCTION__ << ": voxelSize is: " << voxelSize_x << ", and voxel counts are: " 
                       << vxlCntIn_x << " " << vxlCntIn_y << " " << vxlCntIn_z << "\n";
        }
        
        float mind=FLT_MAX, minc=FLT_MAX, minb=FLT_MAX, maxd=FLT_MIN, maxc=FLT_MIN, maxb=FLT_MIN;

        for(pugi::xml_node tool=tools.first_child(); tool; tool=tool.next_sibling())
        {
            const pugi::char_t * line = tool.first_child().value();
            std::string comma;
            float b, c, d;

            if (std::stringstream(line) >> b >> comma >> c >> comma >> d)
            {
                // Push dummy entries for the deformed mesh... will fill later from log file
                nodes.push_back(0.0);
                nodes.push_back(0.0);  
                nodes.push_back(0.0);

                // Push undeformed mesh.  Keep it in units of meters
                nodes.push_back(d);                    
                nodes.push_back(c);  
                nodes.push_back(b);  

                // Handle where the breast expands or compresses:
                // d, which is the X direction, holds the origin at the chest wall, which doesn't go anywhere (the breast squishes out in the nipple direction only)
                // c, which is the Y direction, holds the origin at the center of the breast. The breast is compressed in this direction.
                // b, which is the Z direction, holds the origin at the center of the breast. The breast squishes out in the positive and negative Z directions.

                if (d < mind) mind = d;    if (d > maxd) maxd = d;
                if (c < minc) minc = c;    if (c > maxc) maxc = c; 
                if (b < minb) minb = b;    if (b > maxb) maxb = b;
            }
            else if (nodes.size() > 0)
            {
                break;
            }
        }

        if (verbose)
        {
            std::cout << "Range of Node locations - undeformed with compression plates (in mm)"  << "\n"  
                      << __FUNCTION__  << "\n"
                      << "\tx ranges from " << mind << " to " << maxd << "\n"
                      << "\ty ranges from " << minc << " to " << maxc << "\n"
                      << "\tz ranges from " << minb << " to " << maxb << "\n";
        }
        pdlog.info << "Range of Node locations - undeformed with compression plates (in mm)"  << "\n"  
                   << __FUNCTION__  << "\n"
                   << "\tx ranges from " << mind << " to " << maxd << "\n"
                   << "\ty ranges from " << minc << " to " << maxc << "\n"
                   << "\tz ranges from " << minb << " to " << maxb << "\n";

        // Now use the "rigid body" Material associated with tetrahedrons to screen out the breast support and compression paddle

        tools = doc.child("febio_spec").child("Material");
        std::map<short, bool> materialMap;

        // Read the Material section of the feb XML file and find entries for which the type is "rigid body". (in c++ only "rigid" appears).
        // ..then build a map, materialMap, containing the corresponding ID attribute; e.g., 3 and 4 in ph200u_601_DPerc20.feb.

        for (pugi::xml_node tool=tools.first_child(); tool; tool=tool.next_sibling())
        {
            bool nonRigid = true; 
            pugi::xml_attribute attr = tool.attribute("type"); // line is: <material id="3" name="Plate 1" type="rigid body">
            std::string typeName(attr.value());
            short id;
            std::stringstream(tool.attribute("id").value()) >> id;
            materialMap.insert(std::pair<short, bool>(id, !typeName.find("rigid"))); // 0 if not rigid, 1 if rigid
        }
        
        for(auto it=materialMap.begin(); it != materialMap.end(); it++)
        {
            if (verbose)std::cout  << "MaterialMap item: " << it->first << ", " << it->second << std::endl;
            pdlog.info << "MaterialMap item: " << it->first << ", " << it->second << "\n";
        }

        tools = doc.child("febio_spec").child("Geometry").child("Elements");    
        elementCount=0;

        mind=FLT_MAX; minc=FLT_MAX; minb=FLT_MAX; maxd=FLT_MIN; maxc=FLT_MIN; maxb=FLT_MIN;
        float dval;  float cval;  float bval;

        for (pugi::xml_node tool=tools.first_child(); tool; tool=tool.next_sibling())
        {
            const pugi::char_t * line = tool.first_child().value();
            GLuint b, c, d, e;
            std::string comma;

            if (std::stringstream(line) >> c >> comma >> b >> comma >> e >> comma >> d)
            {
                pugi::xml_attribute attr = tool.attribute("mat");
                int mat;
                std::stringstream(attr.value()) >> mat;

                if (!materialMap.at(mat)) // if not rigid
                {
                    // Note that NODES include the vertices which make up the breast support and compression paddle
                    // ..ELEMENTS ignore (omit) the breast support and compression paddle because they are "rigid"

                    elements.push_back(b-1);

                    // Test whether this node extends the min-max envelope for the set of non-rigid nodes 
                    dval = nodes[(b-1) * 6 + 3];  cval = nodes[(b-1)*6 + 4];  bval = nodes[(b-1)*6+5];
                    if (dval < mind) mind = dval;    if (dval > maxd) maxd = dval;
                    if (cval < minc) minc = cval;    if (cval > maxc) maxc = cval;
                    if (bval < minb) minb = bval;    if (bval > maxb) maxb = bval;

                    elements.push_back(c-1);

                    // Test whether this node extends the min-max envelope for the set of non-rigid nodes 
                    dval = nodes[(c-1) * 6 + 3];  cval = nodes[(c-1)*6 + 4];  bval = nodes[(c-1)*6+5];
                    if (dval < mind) mind = dval;    if (dval > maxd) maxd = dval;
                    if (cval < minc) minc = cval;    if (cval > maxc) maxc = cval;
                    if (bval < minb) minb = bval;    if (bval > maxb) maxb = bval;

                    elements.push_back(d-1);

                    // Test whether this node extends the min-max envelope for the set of non-rigid nodes 
                    dval = nodes[(d-1) * 6 + 3];  cval = nodes[(d-1)*6 + 4];  bval = nodes[(d-1)*6+5];
                    if (dval < mind) mind = dval;    if (dval > maxd) maxd = dval;
                    if (cval < minc) minc = cval;    if (cval > maxc) maxc = cval;
                    if (bval < minb) minb = bval;    if (bval > maxb) maxb = bval;

                    elements.push_back(e-1);

                    // Test whether this nodes extends the min-max envelope for the set of non-rigid nodes 
                    dval = nodes[(e-1) * 6 + 3];  cval = nodes[(e-1)*6 + 4];  bval = nodes[(e-1)*6+5];
                    if (dval < mind) mind = dval;    if (dval > maxd) maxd = dval;
                    if (cval < minc) minc = cval;    if (cval > maxc) maxc = cval;
                    if (bval < minb) minb = bval;    if (bval > maxb) maxb = bval;
                    elementCount++;
                }
            }
            else if (elements.size() > 0)
            {
                break;
            }
        }

        if (verbose) 
        {
            std::cout  << "Element count " << elements.size() << std::endl;
            pdlog.info << "Element count " << elements.size() << "\n";
            
            std::cout << "Range of Node locations - undeformed without compression plates (in mm)"  << "\n"  
                      << __FUNCTION__  << "\n"                               
                      << "\tx ranges from " << mind << " to " << maxd << "\n" 
                      << "\ty ranges from " << minc << " to " << maxc << "\n" 
                      << "\tz ranges from " << minb << " to " << maxb << "\n";
        
            pdlog.info << "Range of Node locations - undeformed without compression plates (in mm)"  << "\n" 
                       << __FUNCTION__  << "\n"                              
                       << "\tx ranges from " << mind << " to " << maxd << "\n"
                       << "\ty ranges from " << minc << " to " << maxc << "\n"
                       << "\tz ranges from " << minb << " to " << maxb << "\n";
        }
    }
    else
    {
        std::cerr   << "Can't load the .feb file \"" << filePath << "\". Exiting program" << std::endl;
        pdlog.fatal << "Can't load the .feb file \"" << filePath << "\". Exiting program\n" ;
        exit(EXIT_FAILURE);
    }
    

    pdlog.info << "Entire readSourceMeshFEBio() function took " << clib.get_elapsed_time(1) << " ms.\n";
}


#pragma region Documentation
/// <summary>
/// Read the def nodes FE bio file (.log file).
/// Contains deltas to original node positions. 
/// New positions are stored in the 1st 3 elements of node.
/// Then normalize all of these positions (orig and adjusted).
/// </summary>
/// <remarks>Joe Chui, 10/12/2011. </remarks>
/// <param name="pdlog">Active log file stream.</param>
/// <param name="filePath">The file path.</param>
/// <param name="nodes">The nodes.</param>
#pragma endregion

void VolumeDeformer::readDefNodesFEBio(vct::Logger &pdlog, std::string filePath, std::vector<GLfloat> &nodes)
{
    clib.start_timer(2); // time entire function
    
    if (verbose)
    {
        std::cout << __FUNCTION__ << ": Reading deforming nodes " << filePath << std::endl;
        pdlog.info << __FUNCTION__ << ": Reading deforming nodes " << filePath << "\n";
    }
    
    std::string line;
    std::ifstream nodeFile(filePath);

    if (nodeFile.is_open())
    {
        std::streampos found = 0;

        // Store the position of the last occurrence of "Data = ux;uy;uz",
        // ..which is the last step (iteration). Preceding steps are skipped-over.
        while (!nodeFile.eof())
        {
            getline(nodeFile, line);
            if (std::string::npos != line.find("Data = ux;uy;uz"))
            {
                found = nodeFile.tellg();
            }
        }

        if (verbose)
        {
            std::cout  << "Have advanced to final iteration of 'Data' in the log file... will now read it" << std::endl;
            pdlog.info << "Have advanced to final iteration of 'Data' in the log file... will now read it.\n";
        }

        float mindold=FLT_MAX, mincold=FLT_MAX, minbold=FLT_MAX, maxdold=FLT_MIN, maxcold=FLT_MIN, maxbold=FLT_MIN;
        float mindnew=FLT_MAX, mincnew=FLT_MAX, minbnew=FLT_MAX, maxdnew=FLT_MIN, maxcnew=FLT_MIN, maxbnew=FLT_MIN;
                        
        if (found > 0)
        {
            nodeFile.clear();
            nodeFile.seekg(found, std::ios::beg);
            int index = 0;

            if (verbose)
            {
                pdlog.info << __FUNCTION__ << ": voxelSize is: " << voxelSize_x << ", and voxel counts are: " 
                           << vxlCntIn_x << " " << vxlCntIn_y << " " << vxlCntIn_z << "\n";
            }

            // For each node there are 6 elements of "nodes".
            // ..the first 3 elements will contain the deformed position in X, Y, and Z (assuming b=>Z, c=>Y, and d=>X).
            // ..the last 3 elements contain the uncompressed position in X, Y, and Z.
            
            GLfloat a, b, c, d, newb, newc, newd;
            
            while(!nodeFile.eof())
            {
                getline(nodeFile, line);
                
                if (std::stringstream(line) >> a >> b >> c >> d)
                {
                    //  Read the undeformed mesh elements (in metres)               
                    //  Add the deformation (nodewise distances in metres) to the undeformed node
                    newd = nodes[index*6+3] + d; 
                    newc = nodes[index*6+4] + c; 
                    newb = nodes[index*6+5] + b; 

                    if (def_mode == vct::DEFORM_ML)
                    {
                        //  Assign to the deformed node elements in the array
                        nodes[index*6]   = newd;
                        nodes[index*6+1] = newc;
                        nodes[index*6+2] = newb;
                    }
                    else if (def_mode == vct::DEFORM_CC)
                    {
                        //  Assign to the deformed node elements in the array
                        nodes[index*6]   = nodes[index * 6 + 3] + d;
                        nodes[index*6+1] = nodes[index * 6 + 4] + b;
                        nodes[index*6+2] = nodes[index * 6 + 5] - c;
                    }

                    //  Get mins and maxs of the deformed phantom, with compression plates
                    if (newd < mindnew) mindnew = newd;    if (newd > maxdnew) maxdnew = newd; 
                    if (newc < mincnew) mincnew = newc;    if (newc > maxcnew) maxcnew = newc; 
                    if (newb < minbnew) minbnew = newb;    if (newb > maxbnew) maxbnew = newb;

                    index++;
                }
                else
                {
                    break; // exit the while loop; the rest of the file is irrelevant
                }
            }

            if (verbose)
            {
                std::cout << "Range of Deformation with compression plates (in mm)"  << "\n"  
                          << __FUNCTION__  << "\n"
                          << "\tx ranges from " << mindnew << " to " << maxdnew << "\n"
                          << "\ty ranges from " << mincnew << " to " << maxcnew << "\n"
                          << "\tz ranges from " << minbnew << " to " << maxbnew << "\n";
            }
            pdlog.info << "Range of Deformation with compression plates (in mm)"  << "\n"
                       << __FUNCTION__  << "\n"
                       << "\tx ranges from " << mindnew << " to " << maxdnew << "\n"
                       << "\ty ranges from " << mincnew << " to " << maxcnew << "\n"
                       << "\tz ranges from " << minbnew << " to " << maxbnew << "\n";

            if (verbose) std::cout  << "number of deformed nodes: " << index << std::endl;
            pdlog.info << "number of deformed nodes: " << index << "\n";
        }

        nodeFile.close();

        clib.start_timer(3); // time min/max calculations
        
        if (verbose) std::cout  << "Reading deforming nodes. elements.size()=" << elements.size() << ", nodes.size()=" << nodes.size() << std::endl;
        pdlog.info << "Reading deforming nodes. elements.size()=" << elements.size() << ", nodes.size()=" << nodes.size() << "\n";

        // This is where the min and max values of deformed locations are found, to support normalization (below)

        size_t i;
        for (i=0; i<6; i++)
        {
            min[i] = FLT_MAX;
            max[i] = FLT_MIN;
        }

        for (i=0; i<elements.size();i++)
        {
            if (nodes[elements[i]*6]   > max[0])   max[0] = nodes[elements[i]*6];    // deformed locations
            if (nodes[elements[i]*6]   < min[0])   min[0] = nodes[elements[i]*6];
            if (nodes[elements[i]*6+1] > max[1])   max[1] = nodes[elements[i]*6+1];
            if (nodes[elements[i]*6+1] < min[1])   min[1] = nodes[elements[i]*6+1];
            if (nodes[elements[i]*6+2] > max[2])   max[2] = nodes[elements[i]*6+2];
            if (nodes[elements[i]*6+2] < min[2])   min[2] = nodes[elements[i]*6+2]; 

            if (nodes[elements[i]*6+3] > max[3])   max[3] = nodes[elements[i]*6+3];  // undeformed locations
            if (nodes[elements[i]*6+3] < min[3])   min[3] = nodes[elements[i]*6+3];
            if (nodes[elements[i]*6+4] > max[4])   max[4] = nodes[elements[i]*6+4];
            if (nodes[elements[i]*6+4] < min[4])   min[4] = nodes[elements[i]*6+4];
            if (nodes[elements[i]*6+5] > max[5])   max[5] = nodes[elements[i]*6+5];
            if (nodes[elements[i]*6+5] < min[5])   min[5] = nodes[elements[i]*6+5]; 
        }

        if (verbose) std::cout  << "Done reading deforming nodes" << std::endl;
        pdlog.info << "Done reading deforming nodes\n";

        if (verbose)
        {
            std::cout << __FUNCTION__ << "\n"
                    << "\t min[0]=" << min[0] << ", max[0]=" << max[0] << ", (max[0] - min[0]) = " << (max[0] - min[0]) << "\n" 
                    << "\t min[1]=" << min[1] << ", max[1]=" << max[1] << ", (max[1] - min[1]) = " << (max[1] - min[1]) << "\n"  
                    << "\t min[2]=" << min[2] << ", max[2]=" << max[2] << ", (max[2] - min[2]) = " << (max[2] - min[2]) << "\n"
                    << "\t min[3]=" << min[3] << ", max[3]=" << max[3] << ", (max[3] - min[3]) = " << (max[3] - min[3]) << "\n" 
                    << "\t min[4]=" << min[4] << ", max[4]=" << max[4] << ", (max[4] - min[4]) = " << (max[4] - min[4]) << "\n"  
                    << "\t min[5]=" << min[5] << ", max[5]=" << max[5] << ", (max[5] - min[5]) = " << (max[5] - min[5]) <<  std::endl;
        }
        
        pdlog.info << __FUNCTION__ << "\n"
                   << "\t min[0]=" << min[0] << ",\t max[0]=" << max[0] << ",\t (max[0] - min[0]) = " << (max[0] - min[0]) << "\n"
                   << "\t min[1]=" << min[1] << ",\t max[1]=" << max[1] << ",\t (max[1] - min[1]) = " << (max[1] - min[1]) << "\n" 
                   << "\t min[2]=" << min[2] << ",\t max[2]=" << max[2] << ",\t (max[2] - min[2]) = " << (max[2] - min[2]) << "\n"
                   << "\t min[3]=" << min[3] << ",\t max[3]=" << max[3] << ",\t (max[3] - min[3]) = " << (max[3] - min[3]) << "\n"
                   << "\t min[4]=" << min[4] << ",\t max[4]=" << max[4] << ",\t (max[4] - min[4]) = " << (max[4] - min[4]) << "\n"
                   << "\t min[5]=" << min[5] << ",\t max[5]=" << max[5] << ",\t (max[5] - min[5]) = " << (max[5] - min[5]) << "\n";

        //  Calculate the ratio of the axes, as this gives the size of the new phantom matrix
        def_ratio_x = (max[0] - min[0])/(max[3] - min[3]); 
        def_ratio_y = (max[1] - min[1])/(max[4] - min[4]); 
        def_ratio_z = (max[2] - min[2])/(max[5] - min[5]);
        
        pdlog.info << __FUNCTION__ << ": def_ratio_x: " << def_ratio_x << ", def_ratio_y: " << def_ratio_y 
                   << ", def_ratio_z: " << def_ratio_z << "\n";
        
        // Now normalize the textures between 0 and 1 NOTE: None of these vectors are unit vectors
        float minx0=FLT_MAX, maxx0=FLT_MIN, minx1=FLT_MAX, maxx1=FLT_MIN; // TEMPORARY
        float miny0=FLT_MAX, maxy0=FLT_MIN, miny1=FLT_MAX, maxy1=FLT_MIN; // TEMPORARY
        float minz0=FLT_MAX, maxz0=FLT_MIN, minz1=FLT_MAX, maxz1=FLT_MIN; // TEMPORARY
        float x0, y0, z0, x1, y1, z1; // TEMPORARY
        
        for (i=0; i<nodes.size(); i=i+6)
        {
            x0 = nodes[i]   = (nodes[i  ] - min[0]) / (max[0] - min[0]); 
            y0 = nodes[i+1] = (nodes[i+1] - min[1]) / (max[1] - min[1]);     
            z0 = nodes[i+2] = (nodes[i+2] - min[2]) / (max[2] - min[2]);
            
            x1 = nodes[i+3] = (nodes[i+3] - min[3]) / (max[3] - min[3]);       
            y1 = nodes[i+4] = (nodes[i+4] - min[4]) / (max[4] - min[4]);        
            z1 = nodes[i+5] = (nodes[i+5] - min[5]) / (max[5] - min[5]);
            
            // TEMPORARY
            if (x0 < minx0) minx0 = x0; if (x0 > maxx0) maxx0 = x0;          if (x1 < minx1) minx1 = x1; if (x1 > maxx1) maxx1 = x1;
            if (y0 < miny0) miny0 = y0; if (y0 > maxy0) maxy0 = y0;          if (y1 < miny1) miny1 = y1; if (y1 > maxy1) maxy1 = y1;
            if (z0 < minz0) minz0 = z0; if (z0 > maxz0) maxz0 = z0;          if (z1 < minz1) minz1 = z1; if (z1 > maxz1) maxz1 = z1;
            // TEMPORARY
        }
        pdlog.info << "Min x0: " << minx0 << ", maxx0: " << maxx0 << ", delta: " << maxx0 - minx0 <<  "\n"
                   << "Min y0: " << miny0 << ", maxy0: " << maxy0 << ", delta: " << maxy0 - miny0 << "\n"
                   << "Min z0: " << minz0 << ", maxz0: " << maxz0 << ", delta: " << maxz0 - minz0 << "\n\n"
                   << "Min x1: " << minx1 << ", maxx1: " << maxx1 << ", delta: " << maxx1 - minx1 << "\n"
                   << "Min y1: " << miny1 << ", maxy1: " << maxy1 << ", delta: " << maxy1 - miny1 << "\n"
                   << "Min z1: " << minz1 << ", maxz1: " << maxz1 << ", delta: " << maxz1 - minz1 << "\n";
        
        pdlog.info << "Calculating min/max alone inside readDefNodesFEBio() function took " << clib.get_elapsed_time(3) << " ms.\n";
    }
    else
    {
        std::cerr   << "Can't load the .log file \"" << filePath << "\". Exiting program" << std::endl;
        pdlog.fatal << "Can't load the .log file \"" << filePath << "\". Exiting program\n" ;
        exit(0);
    }
    pdlog.info << "Entire readDefNodesFEBio() function took " << clib.get_elapsed_time(2) << " ms.\n";
}


#pragma region Documentation
/// <summary>Determine the dimensions of the output texture and create it. </summary>
/// <remarks>Joe Chui, 10/12/2011. </remarks>
/// <param name="pdlog">Active log file stream.</param>
/// <param name="volume">A pointer to the uncompressed phantom's voxels.</param>
#pragma endregion

GLuint VolumeDeformer::loadVolume(vct::Logger &pdlog, unsigned char *volume)
{
    // Assign the inputTexture to point the the uncompressed phantom's voxels
    inputTexture = volume;
    inputTexture_size = static_cast<size_t>(vxlCntIn_x * vxlCntIn_y * vxlCntIn_z);

    if (verbose) std::cout  << __FUNCTION__ << ": uncompressed texture size " <<  inputTexture_size << std::endl;
    pdlog.info << __FUNCTION__ << ": uncompressed texture size " <<  inputTexture_size << "\n";

    // Create an input texture for the undeformed phantom
    #if defined(_MSC_VER)
    PFNGLTEXIMAGE3DPROC glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
    #elif defined(__linux__)
    PFNGLTEXIMAGE3DPROC glTexImage3D = (PFNGLTEXIMAGE3DPROC) glXGetProcAddress((const GLubyte *)"glTexImage3D");
    #endif
    
    pdlog.info << "Address of glTexImage3D process is: " << (uint64_t) glTexImage3D << "\n"; // DEBUG DH 2020-09-02

    
    if (glTexImage3D) 
    {
        pdlog.info << __FUNCTION__ << ": glTexImage3D is not null\n";
        if (textureID) glDeleteTextures(1, &textureID);

        glGenTextures(1, &textureID);
        clib.CheckGlErrors(__FUNCTION__); // report any openGL errors

        glBindTexture(GL_TEXTURE_3D, textureID);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        // inputTexture is a buffer containing the uncompressed phantom's voxels
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, 
			         static_cast<GLsizei>(vxlCntIn_x), 
			         static_cast<GLsizei>(vxlCntIn_y), 
			         static_cast<GLsizei>(vxlCntIn_z),
			         0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, inputTexture);
        clib.CheckGlErrors("loadVolume() after 1st glTexImage3D"); // report any openGL errors
        pdlog.info << __FUNCTION__ << ": about to return textureID " << textureID << "\n";
    }
    else
    {
        std::cerr   << __FUNCTION__ << ": glTexImage3D is NULL" << std::endl;
        pdlog.error << __FUNCTION__ << ": glTexImage3D is NULL!\n";
    }

    return textureID;   
}


#pragma region Documentation
/// <summary>Determine the dimensions of the output texture and create it. </summary>
/// <remarks>Joe Chui, 10/12/2011. </remarks>
/// <param name="pdlog">Active log file stream.</param>
#pragma endregion

GLuint VolumeDeformer::setVolume(vct::Logger &pdlog)
{
    // Find the new (deformed) phantom dimensions by applying a scale factor to each axis
    fxDim = size_t( float(vxlCntIn_x) * def_ratio_x );
    fyDim = size_t( float(vxlCntIn_y) * def_ratio_y ); 
    fzDim = size_t( float(vxlCntIn_z) * def_ratio_z );

    if (verbose) std::cout  << "\n" << __FUNCTION__ << ": Output phantom sizes are " << fxDim << " x " << fyDim << " x " << fzDim << " (vxls)" << std::endl;
    pdlog.info << __FUNCTION__ << ": Output phantom sizes are " << fxDim << " x " << fyDim << " x " << fzDim << " (vxls)\n";

    // Create an output texture
    #if defined(_MSC_VER)
    PFNGLTEXIMAGE3DPROC glTexImage3D = (PFNGLTEXIMAGE3DPROC) wglGetProcAddress("glTexImage3D");
    #elif defined(__linux__)
    PFNGLTEXIMAGE3DPROC glTexImage3D = (PFNGLTEXIMAGE3DPROC) glXGetProcAddress((const GLubyte *)"glTexImage3D");
    #endif
    
    pdlog.info << "Address of glTexImage3D process is: " << (uint64_t) glTexImage3D << "\n"; // DEBUG DH 2020-09-02
    
    if (glTexImage3D)
    {
        pdlog.info << __FUNCTION__ << ": glTexImage3D is not null\n";
        if (deformedTextureID) glDeleteTextures(1, &deformedTextureID);

        glGenTextures(1, &deformedTextureID);
        glBindTexture(GL_TEXTURE_3D, deformedTextureID);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, static_cast<GLsizei>(fxDim), static_cast<GLsizei>(fyDim), 
			         1, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0); // A one-voxel-thick slice of x/y !!
        clib.CheckGlErrors("loadVolume() after 2nd glTexImage3D"); // report any openGL errors
        pdlog.info << __FUNCTION__ << ": about to return deformedTextureID " << deformedTextureID << "\n";
    }
    else
    {
        std::cerr   << __FUNCTION__ << ": Can't find the glTexImage3D process. Exiting program" << std::endl;
        pdlog.fatal << __FUNCTION__ << ": Can't find the glTexImage3D process. Exiting program\n" ;
        exit(EXIT_FAILURE);
    }

    return deformedTextureID;
}



#pragma region Documentation
/// <summary>Draw the deformed phantom, slice by slice, and store it in a binary file (invoked once). </summary>
/// <remarks>Joe Chui, 10/12/2011. </remarks>
/// <param name="pdlog">Active log file stream.</param>
/// <param name="rwp">VCT library interface class instance.</param>
#pragma endregion

void VolumeDeformer::draw(vct::Logger &pdlog, ReadWritePhantom *rwp)
{
    if (verbose) std::cout  << __FUNCTION__ << ": Start Deforming" << std::endl;
    pdlog.info << __FUNCTION__ << ": Start Deforming\n";
    
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    #if defined(_MSC_VER)
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
    #elif defined(__linux__)
    PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) glXGetProcAddress((const GLubyte *)"glBindFramebuffer");
    #endif
    
    pdlog.info << "Address of glBindFramebuffer process is: " << (uint64_t) glBindFramebuffer << "\n"; // DEBUG DH 2020-09-02
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer); 

    if (!frameBuffer) 
    {
        std::cerr   << "Framebuffer is NULL" << std::endl;
        pdlog.error << "Framebuffer is NULL\n";
        exit(EXIT_FAILURE);
    }

    #if defined(_MSC_VER)
    PFNGLUSEPROGRAMPROC glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
    #elif defined(__linux__)
    PFNGLUSEPROGRAMPROC glUseProgram = (PFNGLUSEPROGRAMPROC) glXGetProcAddress((const GLubyte *)"glUseProgram");
    #endif
    
    pdlog.info << "Address of glUseProgram process is: " << (uint64_t) glUseProgram << "\n"; // DEBUG DH 2020-09-02
    glUseProgram(programObject);

    glBindTexture(GL_TEXTURE_3D, textureID);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glBlendFunc(GL_ONE, GL_ONE);
    
    // Bind the projection model view matrix (PMVMatrix) to the associated uniform variable in the shader
    
    #if defined(_MSC_VER)
    
        if (!glUniformMatrix4fv)
        {
             glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");            
             glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC)     wglGetProcAddress("glVertexAttribPointer");            
             glUniform4f               = (PFNGLUNIFORM4FPROC)               wglGetProcAddress("glUniform4f");            
             glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC)      wglGetProcAddress("glGetUniformLocation");            
    
             pdlog.info << "Address of glEnableVertexAttribArray process is: " << (uint64_t) glEnableVertexAttribArray << "\n"; // DEBUG DH 2020-09-02     
             pdlog.info << "Address of glVertexAttribPointer process is: "     << (uint64_t) glVertexAttribPointer << "\n";     // DEBUG DH 2020-09-02    
             pdlog.info << "Address of glUniform4f process is: "               << (uint64_t) glUniform4f << "\n";               // DEBUG DH 2020-09-02            
             pdlog.info << "Address of glGetUniformLocation process is: "      << (uint64_t) glGetUniformLocation << "\n";      // DEBUG DH 2020-09-02            
        }

        PFNGLUNIFORM1IPROC glUniform1i = (PFNGLUNIFORM1IPROC) wglGetProcAddress("glUniform1i");
    
        pdlog.info << "Address of glUniform1i process is: " << (uint64_t) glUniform1i << "\n"; // DEBUG DH 2020-09-02

        glGetUniformLocation =(PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation"); 
    
        pdlog.info << "Address of glGetUniformLocation process is: " << (uint64_t) glGetUniformLocation << "\n"; // DEBUG DH 2020-09-02
    
    
    #elif defined(__linux__)
    
        if (!glUniformMatrix4fv)
        {
             glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) glXGetProcAddress((const GLubyte *)"glEnableVertexAttribArray");
             glVertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC)     glXGetProcAddress((const GLubyte *)"glVertexAttribPointer");
             glUniform4f               = (PFNGLUNIFORM4FPROC)               glXGetProcAddress((const GLubyte *)"glUniform4f");
             glGetUniformLocation      = (PFNGLGETUNIFORMLOCATIONPROC)      glXGetProcAddress((const GLubyte *)"glGetUniformLocation");           
    
             pdlog.info << "Address of glEnableVertexAttribArray process is: " << (uint64_t) glEnableVertexAttribArray << "\n"; // DEBUG DH 2020-09-02     
             pdlog.info << "Address of glVertexAttribPointer process is: "     << (uint64_t) glVertexAttribPointer << "\n";     // DEBUG DH 2020-09-02    
             pdlog.info << "Address of glUniform4f process is: "               << (uint64_t) glUniform4f << "\n";               // DEBUG DH 2020-09-02            
             pdlog.info << "Address of glGetUniformLocation process is: "      << (uint64_t) glGetUniformLocation << "\n";      // DEBUG DH 2020-09-02
            
        }

        PFNGLUNIFORM1IPROC glUniform1i = (PFNGLUNIFORM1IPROC) glXGetProcAddress((const GLubyte *)"glUniform1i");
        glGetUniformLocation =(PFNGLGETUNIFORMLOCATIONPROC) glXGetProcAddress((const GLubyte *)"glGetUniformLocation");
    
        pdlog.info << "Address of glUniform1i process is: " << (uint64_t) glUniform1i << "\n"; // DEBUG DH 2020-09-02
        pdlog.info << "Address of glGetUniformLocation process is: " << (uint64_t) glGetUniformLocation << "\n"; // DEBUG DH 2020-09-02
    
    #endif

    // Set the sampler2D variable to the first texture unit
    glUniform1i(glGetUniformLocation(programObject, "sampler3d"), 0);

    // First, get the location of each uniform variable in the shader using its name
    int iNormalPlane = glGetUniformLocation(programObject, "normalPlane");
    int iPointPlane  = glGetUniformLocation(programObject, "pointPlane");
    int iOffset      = glGetUniformLocation(programObject, "offset");

    // Get a pointer to the glFramebufferTextureLayer function in the OpenGL driver for this system
    #if defined(_MSC_VER)
    PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC) wglGetProcAddress("glFramebufferTextureLayer");
    #elif defined(__linux)
    PFNGLFRAMEBUFFERTEXTURELAYERPROC glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC) glXGetProcAddress((const GLubyte *)"glFramebufferTextureLayer");
    #endif
    
    pdlog.info << "Address of glFramebufferTextureLayer process is: " << (uint64_t) glFramebufferTextureLayer << "\n"; // DEBUG DH 2020-09-02

    // Draw a triangle. Refer to HelloTriangle or IntroducingPVRShell for a detailed explanation.

    // Bind the 1st VBO ( and , Nodes - a LINES_ADJACENCY, ARRAY_BUFFER)
    glBindBuffer(GL_ARRAY_BUFFER, vbo32[0]);

    // Define the index location of each set of VertexArray data for set #0 (the 1st 3 floats of each 6 float set)
    glEnableVertexAttribArray(0); //"myVertex" (deformed nodes)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexStride, 0); // first 3: coords after deformation "myVertex"

    // Define the index location of each set of VertexArray data for set #1 (the last 3 floats of each 6 float set)
    glEnableVertexAttribArray(1); //"MyUV" (predeformed nodes)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexStride, (void*) (3 * sizeof(GLfloat))); // last 3: coords before deformation "MyUV"
    
    
    // Bind the 2nd VBO (Elements - a TRIANGLE_STRIP, ELEMENT_ARRAY_BUFFER)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo32[1]);
    
    
    #if defined(_MSC_VER)
    PFNGLDRAWBUFFERSPROC glDrawBuffers = (PFNGLDRAWBUFFERSPROC) wglGetProcAddress("glDrawBuffers");
    #elif defined(__linux__)
    PFNGLDRAWBUFFERSPROC glDrawBuffers = (PFNGLDRAWBUFFERSPROC) glXGetProcAddress((const GLubyte *)"glDrawBuffers");
    #endif
    
    pdlog.info << "Address of glDrawBuffers process is: " << (uint64_t) glDrawBuffers << "\n"; // DEBUG DH 2020-09-02

    // Create the output texture buffer and size it to hold one slice (this is just a buffer)
    std::vector<unsigned char> outputTexture(fxDim * fyDim, 1);

    // Use the ReadWritePhantom object to output the deformed phantom
    std::ofstream &outputStream = rwp->prepareOutputPhantom(static_cast<int>(fxDim), static_cast<int>(fyDim), static_cast<int>(fzDim));

    // Define the normal and offset planes (these don't change) <--DH: 2020-09-10 Shaved 2 mins off exec time by moving these here!
    glUniform4f(iNormalPlane, 0.0f, 0.0f, 1.0f, 0.0f);
    glUniform4f(iOffset, 0.5, 0.5, 0.0, 0.0); // offset by 1/2 of texture in x and y. Unnecessary in z
    
        
    // Render and store the deformed phantom one x/y slice at a time
    for(int i=0; i<fzDim; ++i)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, deformedTextureID, 0, 0);

        // Set the list of draw buffers.
        GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glBlendFunc(GL_ONE, GL_ONE);
        glViewport(0, 0, static_cast<GLsizei>(fxDim), static_cast<GLsizei>(fyDim));

        glClearColor(0.0, 0.0, 0.0, 0.0); // zeroes are required here to match air=0

        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
    
        // Define the point plane - this changes every iteration
        glUniform4f(iPointPlane, 0.0f, 0.0f, (i+0.5f)/fzDim, 1.0f); // + 0.5 is to round conversion to float <-why not just cast it? DH

        if (i == 0) // do this only once; the 1st time; otherwise performance will suffer
        {
            if (verbose)
            {
                std::cout << "Range of Deformation with compression plates (in mm)\n"  
                          << __FUNCTION__  << "\n"
                          << "\tx ranges from " << min[0] << " to " << max[0] << "\n"
                          << "\ty ranges from " << min[1] << " to " << max[1] << "\n"
                          << "\tz ranges from " << min[2] << " to " << max[2] << "\n";
            }
            pdlog.info << "Range of Deformation with compression plates (in mm)\n"
                       << __FUNCTION__  << "\n"
                       << "\tx ranges from " << min[0] << " to " << max[0] << "\n"
                       << "\ty ranges from " << min[1] << " to " << max[1] << "\n"
                       << "\tz ranges from " << min[2] << " to " << max[2] << "\n";;
        }
        
        glBindTexture(GL_TEXTURE_3D, textureID); // make input "texture" current (voxel body)
        glDrawElements(GL_LINES_ADJACENCY, elementCount*4, GL_UNSIGNED_INT, 0);

        glReadPixels(0, 0, static_cast<GLsizei>(fxDim), static_cast<GLsizei>(fyDim), 
			         GL_RED_INTEGER, GL_UNSIGNED_BYTE, &outputTexture[0]);

        glFlush();  

        clib.outputByChunks(outputStream, &outputTexture[0], (long long)outputTexture.size());

        // Display status every n slices (as a health indicator)
        if (verbose)
        {
            if (i % 30 == 0) // n is 30
            {
                std::cout << "slice " << i << " out of " << fzDim << std::endl; 
                pdlog.info << "slice " << i << " out of " << fzDim << "\n";
            }
        }
    }

    outputStream.close();

    rwp->initVoxelArray();
    rwp->finishOutputPhantom();

    // Unbind the VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
}


#pragma region Documentation
/// <summary>Clean up and prepare for program termination (destructor). </summary>
/// <remarks>Joe Chui, 10/12/2011. </remarks>
#pragma endregion

// Destructor
VolumeDeformer::~VolumeDeformer()
{
    if (textureID)  glDeleteTextures(1, &textureID);
    
    #if defined(_MSC_VER)
    PFNGLDETACHSHADERPROC glDetachShader  =(PFNGLDETACHSHADERPROC) ::wglGetProcAddress("glDetachShader");
    #elif defined(__linux__)
    PFNGLDETACHSHADERPROC glDetachShader  =(PFNGLDETACHSHADERPROC) ::glXGetProcAddress((const GLubyte *)"glDetachShader");
    #endif

    if (glDetachShader)
    {
        glDetachShader(programObject, fragShader);
        glDetachShader(programObject, vertexShader);
    }
    
    #if defined(_MSC_VER)
    PFNGLDELETESHADERPROC glDeleteShader = (PFNGLDELETESHADERPROC) ::wglGetProcAddress("glDeleteShader");
    #elif defined(__linux__)
    PFNGLDELETESHADERPROC glDeleteShader = (PFNGLDELETESHADERPROC) ::glXGetProcAddress((const GLubyte *)"glDeleteShader");
    #endif
    
    if (glDeleteShader)
    {
        glDeleteShader(fragShader);
        glDeleteShader(vertexShader);
    }

    #if defined(_MSC_VER)
    PFNGLDELETEPROGRAMPROC glDeleteProgram = (PFNGLDELETEPROGRAMPROC) ::wglGetProcAddress("glDeleteProgram");
    #elif defined(__linux__)
    PFNGLDELETEPROGRAMPROC glDeleteProgram = (PFNGLDELETEPROGRAMPROC) ::glXGetProcAddress((const GLubyte *)"glDeleteProgram");
    #endif
    
    if (glDeleteProgram) glDeleteProgram(programObject);    
    
    #if defined(_MSC_VER)
    PFNGLDELETEBUFFERSPROC glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) ::wglGetProcAddress("glDeleteBuffers");
    #elif defined(__linux__)
    PFNGLDELETEBUFFERSPROC glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) ::glXGetProcAddress((const GLubyte *)"glDeleteBuffers");
    #endif
    
    if (glDeleteBuffers) glDeleteBuffers(2, vbo32); 
}
