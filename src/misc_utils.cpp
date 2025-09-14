#include <math.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <filesystem>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Lmcons.h>
#include <shlobj.h>
#include <knownfolders.h>
#endif

#include "misc_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

namespace fs = filesystem;

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

string GetUsername() {
#ifdef _WIN32
	char username[UNLEN + 1];
    DWORD username_len = sizeof(username);
	GetUserNameA(username, &username_len);
	return string(username);
#else
	return string(getenv("USER"));
#endif
}

static string WideToUtf8(const wstring& wstr) {
	if (wstr.empty()) return string();
	int length = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
	string result(length, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), result.data(), length, nullptr, nullptr);
	return result;
}

string GetMyDocuments() {
#ifdef _WIN32
	PWSTR path = nullptr;
	SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
	string documents_path = WideToUtf8(path);
	CoTaskMemFree(path);
	return documents_path;
#else
	return string(getenv("HOME")) + "/Documents";
#endif
}

string GetAppDataLocal() {
#ifdef _WIN32
	PWSTR path = nullptr;
	SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path);
	string appdata_path = WideToUtf8(path);
	CoTaskMemFree(path);
	return appdata_path;
#else
	return string(getenv("HOME")) + "/.local/share";
#endif
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

string FloatToString(float value) {
	if (fabs(value) < 0.001) value = 0;
	stringstream ss;
	ss << fixed << setprecision(3) << value;
	string str = ss.str();
	if (str.find('.') != string::npos) {
		str = str.substr(0, str.find_last_not_of('0') + 1);
		if (str.find('.') == str.size() - 1)
			str = str.substr(0, str.size() - 1);
	}
	return str;
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
