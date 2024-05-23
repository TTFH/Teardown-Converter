CXX = g++
EXE = release/teardown-converter
ODIR = obj
IMGUI_DIR = imgui

SOURCES = main.cpp glad/glad.c
SOURCES += src/entity.cpp src/lua_table.cpp src/math_utils.cpp src/parser.cpp src/scene.cpp src/vox_writer.cpp src/write_scene.cpp src/xml_writer.cpp src/zlib_utils.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += file_dialog/ImGuiFileDialog.cpp lib/tinyxml2.cpp

OBJS = $(addprefix obj/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)

CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -O3 -g
CXXFLAGS += -std=c++17 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backend -Ifile_dialog -Ilib
CXXFLAGS += -Wno-missing-field-initializers
LIBS = -lz -lstdc++fs -lpthread

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Linux"
	CXXFLAGS += -Wno-format-security -Wno-unused-result -Wno-unknown-pragmas
	CXXFLAGS += `pkg-config --cflags glfw3`
	LIBS += -lglfw `pkg-config --static --libs glfw3`
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "Windows"
	CXXFLAGS += `pkg-config --cflags glfw3`
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32 -static icon.res
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

$(ODIR)/%.o: glad/%.c glad/%.h
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
