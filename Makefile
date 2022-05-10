# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1+
# Fuck Mac
#
# You will need SDL2 (http://www.libsdl.org):
# Linux:
#   apt-get install libsdl2-dev
# MSYS2:
#   pacman -S mingw-w64-i686-SDL2
#
# Oh, and zlib too!
# Figure out how to download that

CXX = g++
EXE = release/teardown-converter
ODIR = obj
IMGUI_DIR = imgui

SOURCES = main.cpp
SOURCES += parser.cpp entity.cpp write_xml.cpp xml_writer.cpp math_utils.cpp lua_table.cpp scene.cpp vox_writer.cpp zlib_inflate.cpp lib/tinyxml2.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backend/imgui_impl_sdl.cpp $(IMGUI_DIR)/backend/imgui_impl_opengl2.cpp
SOURCES += file_dialog/ImGuiFileDialog.cpp

OBJS = $(addprefix obj/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -g
CXXFLAGS += -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backend -Ifile_dialog
CXXFLAGS += -Ilib
LIBS = -lz -lstdc++fs

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	CXXFLAGS += -Wno-unused-parameter -Wno-unused-result -Wno-format-security
	CXXFLAGS += `sdl2-config --cflags`
	LIBS += -lGL -ldl `sdl2-config --libs`
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	CXXFLAGS += `pkg-config --cflags sdl2`
	LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2` -mconsole
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

$(ODIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: lib/%.cpp lib/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(ODIR)/%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: $(IMGUI_DIR)/backend/%.cpp $(IMGUI_DIR)/backend/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: file_dialog/%.cpp file_dialog/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@


all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
