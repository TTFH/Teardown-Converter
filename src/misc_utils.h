#ifndef _MISC_UTILS_H
#define _MISC_UTILS_H

#include <string>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

using namespace std;

void copy_file(string origin, string destination);
void copy_folder(string origin, string destination);
void create_folder(string name);

string GetFilename(const char* path);
GLFWwindow* InitOpenGL(const char* window_title, int width, int height);
GLuint LoadTexture(const char* path);
void SaveImageJPG(string input_image, string output_image);

#endif
