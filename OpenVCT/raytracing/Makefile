EXEC = OpenVCT/raytracing/XPLProjectionSim_GPU_docker

CXX = g++
CXXFLAGS = -std=c++11

SRCDIR1 = OpenVCT/raytracing/XPLProjectionSimulation
SRCDIR2 = OpenVCT/raytracing/XPLRayTracing

VCTDIR  = helpers/VctToolkit
GDCMDIR = usr/include/gdcm-3.0

INCDIR  = -I. -I$(SRCDIR1) -I$(SRCDIR2) \
          -I$(VCTDIR)/include -I$(VCTDIR)/ZipLib/include \
          -I$(GDCMDIR)

VCTLDIR = -L$(VCTDIR)/ZipLib/bin/x64/Release
VLIBS   = -lziplib -llzma -lbzip2 -lzlib -pthread

GDCM = usr/lib/x86_64-linux-gnu
GDCMLDIR = -L$(GDCM)

GLIBS    = -lgdcmCommon -lgdcmDICT -lgdcmDSED \
           -lgdcmIOD -lgdcmjpeg8 -lgdcmjpeg12 -lgdcmjpeg16 -lgdcmMEXD \
           -lgdcmMSFF -lgdcmCommon -lgdcmDICT -lgdcmDSED \
           -lgdcmIOD -lgdcmjpeg8 -lgdcmjpeg12 -lgdcmjpeg16 

OCLDIR = usr/lib/x86_64-linux-gnu
OCLDIR1  = -L$(OCLDIR)
OLIB    = -lOpenCL

DEPS := $(wildcard $(SRCDIR2)/*.h $(SRCDIR2)/*.hpp)	 
SOURCES := $(wildcard $(SRCDIR1)/*.cpp $(SRCDIR2)/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

exec: $(OBJECTS) $(DEPS)
	$(CXX) -no-pie -o $(EXEC) $(OBJECTS) $(VCTLDIR) $(VLIBS) $(GDCMLDIR) $(GLIBS) \
	$(OCLDIR1) $(OLIB) -ldl 
	
%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $(INCDIR) -o $@ $< 
	
clean: FORCE
	@rm -f $(EXEC) $(OBJECTS)
	
defs: $(DEPS)
	@echo $(DEPS)
	@echo $(SOURCES)
	@echo $(OBJECTS)

FORCE:





