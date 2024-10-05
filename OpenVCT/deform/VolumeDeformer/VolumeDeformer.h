// VolumeDeformer.h

#pragma once

#if defined(_MSC_VER)
   #include <Windows.h>
   #include <gl/GL.h>
   #include <gl/glext.h>
   #include <gl/wglext.h>
#else
   #include <X11/Xlib.h>
   #include <X11/Xutil.h>
   #define GL_GLEXT_PROTOTYPES
   #define GLX_GLXEXT_PROTOTYPES
   #include <GL/gl.h>
   #include <GL/glx.h>
#endif

#include <string>
#include <vector>

#include "VCT_Logger.h"
#include "VCT_CommonLibrary.h"
#include "VCT_Deformation.h"

class ReadWritePhantom; // fwd reference

class VolumeDeformer
{
public:

    VolumeDeformer();  // default constructor

    // Manipulators (setters)
    void   setPhantomVoxelCounts(float vxlCntX, float vxlCntY, float vxlCntZ); // in voxels, from phantom header
    void   setPhantomVoxelSizes(float vsizeX, float vsizeY, float vSizeZ);     // in meters, from command line (or default)
    GLuint setVolume(vct::Logger &pdlog); 
    void   setMode(vct::DEFORMATION_MODE mode) { def_mode = mode; }
    void   setVerbose(bool state) { verbose = state; }

    // Other object methods
    GLuint loadVolume(vct::Logger &pdlog, unsigned char *volume);
    #if defined(_MSC_VER)
       bool Process(vct::Logger &pdlog, std::string originalMesh, std::string deformedMesh, HWND handle, HWND handle2);
    #else       
       bool Process(vct::Logger &pdlog, std::string originalMesh, std::string deformedMesh, Display *display, int screen, Window win1, Window win2);
    #endif
	void draw(vct::Logger &pdlog, ReadWritePhantom *rwp);

    // Destructor
    virtual ~VolumeDeformer(void);    


protected:

    void BuildGLSLProgram(vct::Logger &pdlog);
	void readDefNodesFEBio(vct::Logger &pdlog, std::string filePath, std::vector<GLfloat> &nodes);
	void readSourceMeshFEBio(vct::Logger &pdlog, std::string filePath, std::vector<GLuint> &elements, std::vector<GLfloat> &nodes);


private:
    
    vct::CommonLibrary clib;
    vct::DEFORMATION_MODE def_mode;

    // 3D Image Texture IDs
	GLuint  textureID;            // 3D texture ID for the undeformed volume (input)
	GLuint  deformedTextureID;    // 3D texture ID for the deformed volume (output)
	bool hasValidRC;
    
    #if defined(_MSC_VER)

	   HGLRC hRC, hRC2; // rendering context
	   HDC   hDC, hDC2; // device context
	   HWND  hWnd;      // window handle passed from client
    
    #else

       Display              *display;
       Window               win;
       GLXContext           ctx1, ctx2;
       Colormap             cmap;
       XSetWindowAttributes swa;
       XVisualInfo          *visinfo;
       GLXFBConfig          fbconfig;
       GLXFBConfig          bestFbc;
    
    #endif
    
    GLuint frameBuffer;
	GLsizei elementCount; 

	std::vector<GLuint> elements;
	std::vector<GLfloat> nodes;

    size_t fxDim, fyDim, fzDim;                  // output size of the volume?
    float vxlCntIn_x, vxlCntIn_y, vxlCntIn_z;    // from phantom header
    float voxelSize_x, voxelSize_y, voxelSize_z; // in meters, from command line (or default)
    float def_ratio_x, def_ratio_y, def_ratio_z; // ratio of number of voxels in deformed/undeformed grid

	short viewWidth, viewHeight; // current viewport size

	GLuint fragShader,    vertexShader,   geometryShader; // GL names for shaders
    GLuint programObject, vertexStride,   vbo32[2];       // GL names for program and VBO.
	
	static const char* srcFragShader; // fragment shader
	static const char* srcVertShader; // vertex shader
	static const char* srcGeoShader;  // geometry shader

	PFNGLBINDBUFFERPROC              glBindBuffer;
	PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
	PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;
	PFNGLUNIFORM4FPROC               glUniform4f;
	PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
	PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;

	float max[6], min[6];

    // Voxel bodies (3D textures)
    unsigned char             *inputTexture;
    size_t                     inputTexture_size;
    std::vector<unsigned char> outputTexture;
    
    bool verbose;
};

