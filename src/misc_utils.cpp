#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

#include "misc_utils.h"

namespace fs = std::filesystem;

void copy_file(string origin, string destination) {
	if (fs::exists(origin) && !fs::exists(destination))
		fs::copy(origin, destination);
}

void copy_folder(string origin, string destination) {
	if (fs::exists(origin) && !fs::exists(destination))
		fs::copy(origin, destination, fs::copy_options::recursive);
}

void create_folder(string name) {
	if (!fs::exists(name))
		fs::create_directories(name);
}

string GetFilename(const char* path) {
	string filename = path;
	size_t slash = filename.find_last_of("\\/");
	if (slash != string::npos)
		filename = filename.substr(slash + 1);
	size_t dot = filename.find_last_of(".");
	filename = filename.substr(0, dot);
	return filename;
}

GLFWwindow* InitOpenGL(const char* window_title, int width, int height) {
	int major = 4;
	int minor = 6;
#ifdef __linux__
	major = 4;
	minor = 2;
#endif
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	glfwWindowHint(GLFW_SAMPLES, 1); // MSAA
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(width, height, window_title, NULL, NULL);
	if (window == NULL) {
		printf("[GLFW] Failed to initialize OpenGL\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, width, height);
	return window;
}

GLuint LoadTexture(const char* path) {
	GLuint texture_id;
	int width, height, channels;
	uint8_t* data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float clampColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture_id;
}

void SaveImageJPG(string input_image, string output_image) {
	int width, height, channels;
	uint8_t* image = stbi_load(input_image.c_str(), &width, &height, &channels, 0);
	if (image == NULL) {
		printf("[WARNING] Image %s not found.\n", input_image.c_str());
		return;
	}
	stbi_write_jpg(output_image.c_str(), width, height, channels, image, 90);
	stbi_image_free(image);
}
