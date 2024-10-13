EXEC = OpenVCT/anatomy/BreastPhantomGenerator_docker

# Note: This version of the BreastPhantomGenerator builds and runs on both
# ----- Windows and on Linux. It uses OpenCL but it does not use OpenGL. It
#       uses the float data type in the kernel programs instead of double
#       because it has been found to be faster, with no measurable difference
#       in phantoms it produces.

CXX = g++
CXXFLAGS = -std=c++11
SRCDIR = OpenVCT/anatomy/BreastPhantomGenerator
OBJDIR = $(SRCDIR)/obj
VCTDIR  = Toolkit

INCDIR = -I. -I$(SRCDIR) -I$(VCTDIR)/include -I$(VCTDIR)/ZipLib/include
LIBDIR = -L$(VCTDIR)/ZipLib/bin/x64/Release 
LIBS   = -lziplib -llzma -lbzip2 -lzlib -pthread
OCLDIR = usr/lib/x86_64-linux-gnu
OCLLIB = OpenCL

DEPS := $(wildcard $(SRCDIR)/*.h $(SRCDIR)/*.hpp)	 
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))

exec: $(OBJECTS) $(DEPS)
	$(CXX) -no-pie $(OBJECTS) $(LIBDIR) $(LIBS) -Wl,-rpath=$(OCLDIR) -Wall -l$(OCLLIB) \
    -o $(EXEC) -ldl 
	
%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $(INCDIR) -o $@ $< 
	
clean: FORCE
	@rm -f $(EXEC) $(OBJECTS)
	
defs: $(DEPS) $(SOURCES)
	@echo Deps: $(DEPS)
	@echo Sources: $(SOURCES)
	@echo Objects: $(OBJECTS)

FORCE:
