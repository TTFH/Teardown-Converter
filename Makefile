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
SOURCES += src/entity.cpp src/lua_table.cpp src/math_utils.cpp src/parser.cpp src/scene.cpp src/vox_writer.cpp src/write_scene.cpp src/xml_writer.cpp src/zlib_utils.cpp lib/tinyxml2.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backend/imgui_impl_sdl.cpp $(IMGUI_DIR)/backend/imgui_impl_opengl2.cpp
SOURCES += file_dialog/ImGuiFileDialog.cpp

OBJS = $(addprefix obj/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -DNDEBUG -O2
CXXFLAGS += -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backend -Ifile_dialog -Ilib
LIBS = -lz -lstdc++fs

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Linux"
	CXXFLAGS += -Wno-unused-parameter -Wno-unused-result -Wno-format-security
	CXXFLAGS += `sdl2-config --cflags`
	LIBS += -lGL -ldl `sdl2-config --libs`
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	CXXFLAGS += `pkg-config --cflags sdl2` -static
	LIBS += -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2` -mconsole icon.res
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
.PHONY: all clean

$(ODIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(ODIR)/%.o: src/%.cpp src/%.h
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
