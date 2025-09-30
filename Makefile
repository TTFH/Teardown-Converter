TARGET = release/teardown-converter

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -O3 -g
CXXFLAGS += -Iimgui -I.
CXXFLAGS += -Wno-missing-field-initializers
CXXFLAGS += `pkg-config --cflags glfw3`
LIBS = `pkg-config --libs glfw3 --static` -lz

SOURCES = main.cpp glad/glad.c lib/tinyxml2.cpp
SOURCES += src/binary_reader.cpp src/entity.cpp src/levels.cpp src/lua_table.cpp
SOURCES += src/math_utils.cpp src/misc_utils.cpp src/parser.cpp src/scene.cpp
SOURCES += src/vox_writer.cpp src/write_scene.cpp src/xml_writer.cpp src/zlib_utils.cpp
SOURCES += imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
SOURCES += imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp
SOURCES += file_dialog/ImGuiFileDialog.cpp

OBJDIR = obj
OBJS = $(SOURCES:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS := $(addprefix $(OBJDIR)/, $(notdir $(OBJS)))

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Linux"
	CXXFLAGS += -Wno-format-security -Wno-unused-result -Wno-unknown-pragmas
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lopengl32 -limm32 -luuid -lole32 -static icon.res
endif

ifeq ($(UNAME_S), Darwin)
	ECHO_MESSAGE = "MacOS"
endif

.PHONY: all clean rebuild

all: $(TARGET)
	@echo Build complete for $(ECHO_MESSAGE)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: src/%.cpp src/%.h | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: lib/%.cpp lib/%.h | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: glad/%.c glad/%.h | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: imgui/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: imgui/backend/%.cpp imgui/backend/%.h | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: file_dialog/%.cpp file_dialog/%.h | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

rebuild: clean all

clean:
	rm -f $(TARGET) $(OBJDIR)/*.o
