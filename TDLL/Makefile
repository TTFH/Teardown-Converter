DLL = winmm.dll
ODIR = obj

SOURCES = winmain.cpp

OBJS = $(addprefix obj/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -Wall -Wextra -Werror -Wpedantic
CXXFLAGS += -s -shared -static -DDEBUGCONSOLE -g
LIBS = -lcomdlg32 -llua5.1

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux)
	CXX = x86_64-w64-mingw32-g++
	ECHO_MESSAGE = "Linux"
endif

ifeq ($(OS), Windows_NT)
	CXX = g++
	ECHO_MESSAGE = "MinGW"
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
.PHONY: all clean

$(ODIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(DLL)
	@echo Build complete for $(ECHO_MESSAGE)

$(DLL): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(DLL) $(OBJS)
