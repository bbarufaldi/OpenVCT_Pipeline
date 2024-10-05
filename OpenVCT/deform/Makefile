EXEC = OpenVCT/deform/BreastPhantomDeformer_docker

# Note: On Linux, OpenGL header files are obtained from the system, they are
# ----- not obtained from the OpenGL sibling directory as they are on Windows

CXX = g++
CXXFLAGS = -std=c++11

SRCDIR1 = OpenVCT/deform/VolumeDeformer
OBJDIR1 = $(SRCDIR1)/obj
SRCDIR2 = OpenVCT/deform/BreastPhantomDeformer
OBJDIR2 = $(SRCDIR2)/obj

VCTDIR  = helpers/VctToolkit
INCDIR  = -I. -I$(SRCDIR1) -I$(SRCDIR2) -I$(VCTDIR)/include -I$(VCTDIR)/ZipLib/include

LIBDIR = -L$(VCTDIR)/ZipLib/bin/x64/Release 
LIBS   = -lziplib -llzma -lbzip2 -lzlib -lX11 -lGLX -lGLEW -lGL -pthread

OCLDIR = usr/lib/x86_64-linux-gnu
OCLLIB = OpenCL -lX11 -lGLX -lGLEW -lGL

DEPS := $(wildcard $(SRCDIR2)/*.h $(SRCDIR2)/*.hpp)	 
SOURCES := $(wildcard $(SRCDIR1)/*.cpp $(SRCDIR2)/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

exec: $(OBJECTS) $(DEPS)
	$(CXX) -no-pie $(OBJECTS) $(LIBDIR) $(LIBS) -Wl,-rpath=$(OCLDIR) -l$(OCLLIB) \
    -o $(EXEC) -ldl -v
	
%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $(INCDIR) -o $@ $< 
	
clean: FORCE
	@rm -f $(EXEC) $(OBJECTS)
	
defs: $(DEPS) $(SOURCES)
	@echo Deps: $(DEPS)
	@echo Sources: $(SOURCES)
	@echo Objects: $(OBJECTS)

FORCE:


