// BreastPhantomDeformer_linux.cpp

#include "BreastPhantomDeformer.h"
#include "../VolumeDeformer/VolumeDeformer.h"
#include "ReadWritePhantom.h"
#include "VCT_CommonLibrary.h"
#include "VCT_Logger.h"


#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


// Global Variables

#if defined (_MSC_VER)

    #include <shellapi.h>
    #include "guicon.h"
    #include <gl/GL.h>
    #include <gl/glext.h>
    #include <gl/wglext.h>

    #define MAX_LOADSTRING    100
    #define WM_PROCESS_DEFORM WM_USER + 1

    HINSTANCE hInst;					 // current instance
    TCHAR szTitle[MAX_LOADSTRING];		 // The title bar text
    TCHAR szWindowClass[MAX_LOADSTRING]; // the main window class name
    HWND hWnd;
    LPTSTR *szArglist;
    int argCount = 5;

    // Forward function declarations
    ATOM             MyRegisterClass(HINSTANCE hInstance);
    bool             InitInstance(HINSTANCE, int);
    LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    void             ProcessDeformation(HWND);

#elif defined(__linux__)

    #include <X11/Xlib.h>
    #include <X11/Xutil.h>

    #define GL_GLEXT_PROTOTYPES
    #define GLX_GLXEXT_PROTOTYPES

    #include <GL/gl.h>
    #include <GL/glx.h>

    const int WIN_XPOS    = 256;
    const int WIN_YPOS    = 64;
    const int WIN_XRES    = 800;//320;//500;
    const int WIN_YRES    = 100;//320;//500;
    const int NUM_SAMPLES = 4;

    Display *display;
    int     screen;
    int     depth;
    Window  win, root_win;
    XVisualInfo          *visinfo;
    XSetWindowAttributes winAttr;

    // Forward function declarations
    bool InitWindow(vct::Logger &log);
    void processXEvents(Atom wm_protocols, Atom wm_delete_window);

#endif

static bool verbose = false;
vct::CommonLibrary clib;
int step = 0;

ReadWritePhantom *rwp;  // Virtual Clinical Trials Interface class TODO: Consider making this static

// Program entry point (main)
#if defined(_MSC_VER)
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
#elif defined(__linux__)
int main(int argc, char *argv[])
#endif
{
	// Instantiate Logger
    std::ofstream ofs_log("BreastPhantomDeformer.log");
    vct::Logger logger(ofs_log);
    logger.timestamp();
    logger.setSeverityLevel(vct::Logger::LOG_INFO);
    logger.info << "Just instantiated logger\n";

    #if defined(_MSC_VER)

	    nCmdShow    = SW_SHOWMINIMIZED;
	    int argc    = __argc;
	    char **argv = __argv;
        
	    MSG msg;
	    HACCEL hAccelTable;

		// Initialize global strings
		LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
		LoadString(hInstance, IDC_BREASTPHANTOMDEFORMER, szWindowClass, MAX_LOADSTRING);
        MyRegisterClass(hInstance); // Local Function (this file)

        // Perform application initialization:

		if (!InitInstance(hInstance, nCmdShow))
        {
            logger.fatal << "init instance failed\n";
            return FALSE;
        }

        hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_BREASTPHANTOMDEFORMER));

    #elif defined(__linux__)

        // Perform application initialization:
        /*if (!InitWindow(logger))
        {
            std::cerr << "InitWindow failed" << std::endl;
            logger.fatal << "InitWindow failed\n";
            return -1;
        }*/

    #endif

    if (argc > 1)
    {
        logger.info << "arg count=" << argc << "\n";
        for(int i=0; i<argc; ++i)
        {
            std::string arg(argv[i]);
            clib.trimMultiQuotes(arg);
            logger.info << "arg[" << i << "]=" << arg << "\n";
        }
    }

    if (argc < 3)
    {
        #if defined(_MSC_VER)
            // Bring this window to the front
            HWND window_handle = GetConsoleWindow(); // windows only
            //SetWindowPos(window_handle, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW); // Cause Window to rise in front of others
			SetWindowPos(window_handle, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW); // Cause Window to rise in front of others
        #endif

        std::cerr << "\nUsage:\n\n"
                  << "   BreastPhantomDeformer -xml_input <xml_input_file>\n\n"
                  << "   where:\n\n"
                  << "      - the xml_input_file specifies the deformation arguments - see program documentation for more.\n"
                  << std::endl;
        return 0;
    }
    else
    {
        std::string arg2(argv[1]);
        if (arg2 == "-xml_input")
        {
            logger.info << "Getting program parameters from xml_input file\n";
            rwp = new ReadWritePhantom;
            rwp->readConfigFile();

            // Read XML input file
            std::string xml_input_file(argv[2]);
            clib.trimMultiQuotes(xml_input_file);
            logger.info << "Input XML File is " << xml_input_file << "\n";
            rwp->readInputXML(logger, xml_input_file);

		    step = 1; // to ensure deformation occurs only after both render areas are set up

            ofs_log.close();

            // Start timer 1 (for whole program elapsed time)
            clib.start_timer(1);
            
            // Enter Window Event Processing Loop

            #if defined(_MSC_VER)

                InvalidateRect(hWnd, NULL, true); // Cause Windows to process events

                // Main message loop:
                while (GetMessage(&msg, NULL, 0, 0))
                {
                    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

            #else

                std::cout << "\n" << __LINE__ << std::endl;

                // Register to receive window close events (the "X" window manager button)
                Atom wm_protocols     = XInternAtom(display, "WM_PROTOCOLS"    , False);
                Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);

                std::cout << "\n" << __LINE__ << std::endl;
                XSetWMProtocols(display, win, &wm_delete_window, True);
                clib.start_timer(); // this timer is used to periodically check X events

                std::cout << "\n" << __LINE__ << std::endl;

                while(1)
                {
                    // Redraw window (after it's mapped)
                    ///if (displayed) displayCB();

                    // Check X events every 1/10 second
                    if (clib.get_elapsed_time() > 100)
                    {
                        std::cout << "\n" << __LINE__ << std::endl;
                        logger.info << "Invoking processXEvents\n";
                        processXEvents(wm_protocols, wm_delete_window);
                        clib.start_timer();
                    }
                }

            #endif

        }
    }

    clib.sleep(1);

    // Note: the program does not exit from here
	return 0;
}



#if defined(_MSC_VER)
void ProcessDeformation(HWND handle)
#elif defined(__linux__)
void ProcessDeformation(Window win)
#endif
{

    std::string logfilename("ProcessDeformation_");
    logfilename += rwp->getPhantomNameIn();
    logfilename += ".log";

    std::ofstream ofs_pdlog(logfilename);
    vct::Logger pdlog(ofs_pdlog);
    pdlog.timestamp();
    pdlog.setSeverityLevel(vct::Logger::LOG_INFO);
	pdlog.info << "Init Process Deformation\n";

    std::string original; // input phantom mesh
	std::string deformed; // output phantom mesh

    pdlog.info << "Getting data from rwp (ReadWritePhantom)\n";
    original = rwp->getInputMesh();
    deformed = rwp->getOutputMesh();

    // For phantom voxel counts
    size_t xdim, ydim, zdim;
    float  vxlsize_x, vxlsize_y, vxlsize_z; // voxel size in mm

    //std::cout << "About to call getPhantom. rwp = " << (int *) rwp << std::endl;

    unsigned char *vxls = rwp->getPhantom(pdlog, xdim, ydim, zdim, vxlsize_x, vxlsize_y, vxlsize_z);
    if (vxls != nullptr)
    {
        // Phantom stores voxel sizes in mm.
        if (verbose)
        {
            std::cout << "\n\n" << __FUNCTION__ << ", line " << __LINE__
                      << ": phantom dimensions are " << xdim << " x " << ydim << " x " << zdim
                      << ": voxel sizes are " << vxlsize_x << " x " << vxlsize_y << " x " << vxlsize_z
                      << " mm, voxels are at " << (int *)vxls << std::endl;
        }
        
        pdlog.info << "Input phantom dimensions are " << xdim << " x " << ydim << " x " << zdim << " (vxls)\n";
        pdlog.info << "Voxel sizes are " << vxlsize_x << " x " << vxlsize_y << " x " << vxlsize_z << " (mm)\n";
        pdlog.info << "Voxels are at: " << (int *)vxls << " (addr)\n";

	    VolumeDeformer deformer;
        
        deformer.setVerbose(verbose); // mirror the current verbosity setting in the volumeDeformer
        if (verbose)
        {
            std::cout << __FUNCTION__ << ", line " << __LINE__ << ": just instantiated deformer" << std::endl;
            pdlog.info << __FUNCTION__ << ", line " << __LINE__ << ": just instantiated deformer\n";
        }

        deformer.setMode(rwp->getView());
        deformer.setPhantomVoxelCounts(float(xdim), float(ydim), float(zdim));
	    deformer.setPhantomVoxelSizes(vxlsize_x * 0.001f, vxlsize_y * 0.001f, vxlsize_z * 0.001f);	// mm to meters

        // Read the input and output meshes
        if (verbose)
        {
            std::cout << __FUNCTION__ << ", line " << __LINE__ << ": reading meshes" << std::endl;
            pdlog.info << "Reading meshes\n";
        }

        #if defined(_MSC_VER)
	    deformer.Process(pdlog, original, deformed, handle, 0);	// read meshes
        #elif defined(__linux__)
	    deformer.Process(pdlog, original, deformed, display, screen, win, 0);	// read meshes
        #endif

        // Load GPU memory with voxel data
	    GLuint id = deformer.loadVolume(pdlog, vxls);
	    GLuint t = deformer.setVolume(pdlog);

        // Actually perform the deformation in deformer.draw()
        if (verbose)
        {
            std::cout << "Drawing phantom..." << std::endl;
            pdlog.info << "Drawing phantom...\n";
        }

        // Flush the underlying pdlog stream to ensure log content
        ofs_pdlog.flush();

        // Create the deformed phantom
	    deformer.draw(pdlog, rwp);
        if (verbose)
        {
            std::cout << "...drawing complete" << std::endl;
            pdlog.info << "...drawing complete\n";
        }
    }
    else
    {
        std::cerr << "ERROR: no voxel array found. phantom file \"" << rwp->getPhantomNameIn() 
                  << "\" may be corrupted." << std::endl;
        pdlog.fatal << "No voxel array returned. Phantom file \"" << rwp->getPhantomNameIn() 
                    << "\" may be corrupted or missing.\n";
        exit(EXIT_FAILURE);
    }

    if (verbose) std::cout << "Exiting ProcessDeformation" << std::endl;
    pdlog.info << "Exiting ProcessDeformation\n";

    // Deallocate read/write phantom and cause temporary directory to be removed
    delete rwp;

	#if defined(OPERATOR_CLOSE_AND_EXIT)
        exit(0);
        std::cout << "Completed. Please close this window to quit." << std::endl;
        pdlog.info << "Completed. Please close this window to quit.\n";
        char l[100];
        std::cin >> l;
	#elif defined(CLOSE_AFTER_3_SECONDS)
        std::cout << "Window will close in 3 seconds" << std::endl;
        clib.sleep(3000);
	#endif

    // Calculate program elapsed time
    unsigned int elapsed_sec = clib.get_elapsed_time(1) / 1000;
    unsigned int elapsed_min = elapsed_sec / 60;
    unsigned int remain_sec  = elapsed_sec - (elapsed_min * 60);
    
    std::cout << "Elapsed Time is " << elapsed_sec << " seconds (" 
              << elapsed_min << " min, " << remain_sec << " sec)" << std::endl;
    ofs_pdlog << "Elapsed Time is " << elapsed_sec << " seconds (" 
              << elapsed_min << " min, " << remain_sec << " sec)\n";
    
    ofs_pdlog.close();
}



#if defined(_MSC_VER) // Windows Visual Studio

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	//wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BREASTPHANTOMDEFORMER));
	wcex.hIcon          = NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_BREASTPHANTOMDEFORMER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


//   FUNCTION: InitInstance(HINSTANCE, int)
//   PURPOSE: Saves instance handle and creates main window
//   COMMENTS:
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.

bool InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, 800, 100, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        return false;
    }

    ShowWindow(hWnd, nCmdShow);
    SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE); // Cause Window to hide behind others
    UpdateWindow(hWnd);

    return true;
}


//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//  PURPOSE:  Processes messages for the main window.
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
            
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		if (step == 1)
		{
			ProcessDeformation(hWnd);
			step = 2;
			InvalidateRect(hWnd, NULL, true);
			exit(EXIT_SUCCESS);
        }
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_PROCESS_DEFORM:
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#elif defined(__linux__) //---------------------------------------------------------------------


//   FUNCTION: InitWindow()
//   PURPOSE: Create main window

bool InitWindow(vct::Logger &log)
{

    bool retval = true;

    display  = XOpenDisplay(":0.0");
    screen   = XDefaultScreen(display);
    depth    = DefaultDepth(display, screen);
    root_win = RootWindow(display, screen);

     // Pick an FBconfig and visual
    GLXFBConfig fbconfig = 0;
    {
        static const int Visual_attribs[] =
        {
            GLX_X_RENDERABLE,   True,
            GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,    GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     8,
            GLX_DEPTH_SIZE,     24,
            GLX_STENCIL_SIZE,   8,
            GLX_DOUBLEBUFFER,   True,
            GLX_SAMPLE_BUFFERS, 1,
            GLX_SAMPLES,        4,
            None
        };

        int attribs[100];
        memcpy(attribs, Visual_attribs, sizeof(Visual_attribs));
        int fbcount;
        GLXFBConfig *fbc = glXChooseFBConfig(display, screen, Visual_attribs, &fbcount);

        if (fbc)
        {
            if (fbcount >= 1) fbconfig = fbc[0];
            XFree(fbc);
        }
    }

    if (!fbconfig)
    {
        printf("Failed to get GLXFBConfig\n");
        exit(EXIT_FAILURE);
    }

    visinfo = glXGetVisualFromFBConfig(display, fbconfig);

    if (!visinfo)
    {
        printf("Failed to get XVisualInfo\n");
        exit(EXIT_FAILURE);
    }

    if (verbose) std::cout << "X Visual ID = " << (long int)visinfo->visualid << std::endl;
    log.info << "X Visual ID = " << (long int)visinfo->visualid << "\n";

    winAttr.event_mask = StructureNotifyMask | KeyPressMask;
    winAttr.background_pixmap = None;
    winAttr.background_pixel  = 0;
    winAttr.border_pixel      = 0;
    winAttr.colormap = XCreateColormap(display, root_win, visinfo->visual, AllocNone);

    if (!glXQueryExtension(display, 0, 0))
    {
        std::cerr << "X Server doesn't support GLX extension\n";
        log.info << "X Server doesn't support GLX extension\n";
        retval = false;
    }
    
    // Create the X window
    unsigned int valuemask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;
    int borderwidth = 0;

    log.info << "About to create a XWindow...\n";
    win = XCreateWindow(display, root_win, WIN_XPOS, WIN_YPOS, WIN_XRES, WIN_YRES,
                        borderwidth, depth, InputOutput, visinfo->visual, valuemask, &winAttr);

    XStoreName(display, win, "BreastPhantomDeformer");

    #define  GLX_CONTEXT_MAJOR_VERSION_ARB  0x2091
    #define  GLX_CONTEXT_MINOR_VERSION_ARB  0x2092

    typedef GLXContext (*glXCreateContextAttribsARBProc) (Display*, GLXFBConfig, GLXContext, int, const int*);

    // Verify GL driver supports glXCreateContextAttribsARB()

    // Create an old-style GLX context first, to get the correct function ptr.
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;

    GLXContext ctx_old = glXCreateContext(display, visinfo, 0, True);

    if (!ctx_old)
    {
        std::cerr << __FUNCTION__ << ", line " << __LINE__ << ": Could not even allocate an old-style GL context!" << std::endl;
        log.error << __FUNCTION__ << ", line " << __LINE__ << ": Could not even allocate an old-style GL context!\n";
        exit(EXIT_FAILURE);
    }

    glXMakeCurrent(display, win, ctx_old) ;

    // Verify that GLX implementation supports the new context create call
    if (strstr(glXQueryExtensionsString(display, screen), "GLX_ARB_create_context") != 0)
    {
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddress((const GLubyte *) "glXCreateContextAttribsARB");
    }

    if (!glXCreateContextAttribsARB)
    {
        std::cerr << __FUNCTION__ << ", line " << __LINE__ << ": Can't create new-style GL context" << std::endl;
        log.error << __FUNCTION__ << ", line " << __LINE__ << ": Can't create new-style GL context\n";
        exit(EXIT_FAILURE);
    }

    // Got the pointer. Nuke old context.
    glXMakeCurrent(display, None, 0);
    glXDestroyContext(display, ctx_old);

    // Try to allocate a GL 3.0 not 4.2 COMPATIBILITY context
    static int Context_attribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB,  3, // 4,
        GLX_CONTEXT_MINOR_VERSION_ARB,  0, // 2,
        GLX_CONTEXT_PROFILE_MASK_ARB,   GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
        /*GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB, */
        /*GLX_CONTEXT_FLAGS_ARB,        GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, */
        /*GLX_CONTEXT_FLAGS_ARB,        GLX_CONTEXT_DEBUG_BIT_ARB, */
        None
    };

    GLXContext context = glXCreateContextAttribsARB(display, fbconfig, 0, True, Context_attribs);

    // Forcably wait for any resulting X errors
    XSync(display, False);
    if (!context)
    {
        std::cerr << "Failed to allocate a GL 3.0 context!!!" << std::endl;
        log.error << "Failed to allocate a GL 3.0 context!!!\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Created a GL 3.0 context" << std::endl;
    log.info  << "Created a GL 3.0 context\n";

    // Display the window
    XMapWindow(display, win);

    if (!glXMakeCurrent(display, win, context))
    {
        std::cerr << "glXMakeCurrent failed!" << std::endl;
        log.error << "glXMakeCurrent failed!\n";
        exit(EXIT_FAILURE);
    }

    XLowerWindow(display, win);           // Place window behind all others
    XIconifyWindow(display, win, screen); // Iconify it
    
    return retval;
}


void processXEvents(Atom wm_protocols, Atom wm_delete_window)
{
    int setting_change = 0;
    
    while(XEventsQueued(display, QueuedAfterFlush))
    {
        XEvent event;
        XNextEvent(display, &event);

        if (event.xany.window != win) continue;

        switch(event.type)
        {
            case MapNotify:
            case ConfigureNotify:
            {
                XConfigureEvent cevent = event.xconfigure;
                if (step == 1)
                {
                    ProcessDeformation(win);
                    step = 2;
                    std::cout << "Deformation is completed successfully" << std::endl;
                    XMapWindow(display, win);
                    exit(EXIT_SUCCESS);
                }
                break;
            }
            case KeyPress:
            {
                char chr;
                KeySym symbol;
                XComposeStatus status;
                XLookupString(&event.xkey, &chr, 1, &symbol, &status);
                break;
            }
            case ClientMessage:
            {
                if (event.xclient.message_type == wm_protocols &&
                    (Atom)event.xclient.data.l[0] == wm_delete_window)
                {
                    exit(EXIT_SUCCESS);
                }
                break;
            }
        }
    }
}



#endif
