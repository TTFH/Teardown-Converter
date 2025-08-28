#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <atomic>
#include <chrono>
#include <string>
#include <vector>

#include "src/parser.h"
#include "src/levels.h"
#include "src/misc_utils.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/backend/imgui_impl_glfw.h"
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui/backend/imgui_impl_opengl3.h"
#include "file_dialog/ImGuiFileDialog.h"

#include "lib/tinyxml2.h"

using namespace std;
using namespace tinyxml2;

atomic<float> progress = 0;

void* DecompileMap(void* param) {
	ConverterParams* data = (ConverterParams*)param;

	create_folder(data->map_folder);
	if (!data->legacy_format) {
		string input_image = "preview/" + data->level_id;
		if (data->dlc_id == "space")
			input_image += "_dlc3";
		input_image += ".png";
		string output_image = data->map_folder + "preview.jpg";
		SaveImageJPG(input_image, output_image);
		SaveInfoTxt(data->map_folder, data->level_name, data->level_desc);
	}
	if (data->legacy_format) {
		create_folder(data->map_folder + "custom");
		copy_folder(data->script_folder, data->map_folder + "custom/script");
	} else {
		create_folder(data->map_folder + "vox");
		copy_folder(data->script_folder + data->level_id, data->map_folder + "main");
		if (!data->dlc_id.empty())
			copy_folder(data->script_folder + "script", data->map_folder + "script");
	}
	ParseFile(*data);
	return NULL;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
	_setmaxstdio(2048);
#endif
	if (argc > 1) {
		if (argc == 2) {
			ConverterParams params;
			params.bin_path = argv[1];
			params.map_folder = GetFilename(argv[1]) + "/";
			ParseFile(params);
			SaveInfoTxt(params.map_folder, "Converted", "Converted map");
			return 0;
		} else
			printf("CLI Usage: %s quicksave.bin\n", argv[0]);
	}

	GLFWwindow* window = InitOpenGL("Teardown Converter", 700, 600);
	ImVec4 clear_color = ImVec4(0.27f, 0.57f, 0.72f, 1.00f);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGuiWindowFlags dialog_flags = ImGuiWindowFlags_None;
	dialog_flags |= ImGuiWindowFlags_NoResize;

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);

#ifdef _WIN32
	char quicksave_folder[256] = "C:\\Users\\User\\AppData\\Local\\Teardown";
	char mods_folder[256] = "C:\\Users\\User\\Documents\\Teardown\\mods";
	char game_folder[256] = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Teardown";
	//const char* quicksave_folder_legacy = "C:\\Users\\user\\Documents\\Teardown";
	//const char* mods_folder_legacy = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Teardown\\create";
#else
	char quicksave_folder[256] = "quicksave_folder";
	char mods_folder[256] = "mods_folder";
	char game_folder[256] = "game_folder";
#endif

	XMLDocument config_file;
	XMLElement* config_root = NULL;
	if (config_file.LoadFile("config.xml") == XML_SUCCESS) {
		config_root = config_file.RootElement();
		strcpy(quicksave_folder, config_root->FindAttribute("quicksave_folder")->Value());
		strcpy(mods_folder, config_root->FindAttribute("mods_folder")->Value());
		strcpy(game_folder, config_root->FindAttribute("game_folder")->Value());
	}

	bool disable_convert = false;
	bool save_as_legacy = false;
	bool remove_snow = false;
	bool no_voxbox = false;
	bool use_tdcz = false;
	int game_version = 0;

	string selected_preview = "";
	GLuint preview_texture = 0;

	pthread_t parse_thread;
	ConverterParams* params = new ConverterParams();

	string selected_category = "Sandbox";
	vector<string> categories = { "Sandbox", "Challenges", "Hub", "Missions", "Art Vandals", "Time Campers", "Folkrace", "The Greenwash Gambit", "Relic Hunters", "Others" };
	vector<LevelInfo> levels = LoadLevels(selected_category);
	vector<LevelInfo>::iterator selected_level = levels.begin();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("Convert TDBIN file", NULL, dialog_flags);

			ImGui::Text("Quicksave folder:");
			ImGui::SameLine();
			ImGui::PushItemWidth(350);
			ImGui::InputText("##qsfolder", quicksave_folder, IM_ARRAYSIZE(quicksave_folder));
			ImGui::PopItemWidth();
			ImGui::SameLine();

			if (ImGui::Button("Select Folder##1"))
				ImGuiFileDialog::Instance()->OpenDialog("DirDialogQF", "Select Quicksave folder", ".");
			if (ImGuiFileDialog::Instance()->Display("DirDialogQF")) {
				if (ImGuiFileDialog::Instance()->IsOk())
					strcpy(quicksave_folder, ImGuiFileDialog::Instance()->GetCurrentPath().c_str());
				ImGuiFileDialog::Instance()->Close();
			}

			ImGui::Text("Mods folder:     ");
			ImGui::SameLine();
			ImGui::PushItemWidth(350);
			ImGui::InputText("##modsfolder", mods_folder, IM_ARRAYSIZE(mods_folder));
			ImGui::PopItemWidth();
			ImGui::SameLine();

			if (ImGui::Button("Select Folder##2"))
				ImGuiFileDialog::Instance()->OpenDialog("DirDialogMF", "Select Mods folder", ".");
			if (ImGuiFileDialog::Instance()->Display("DirDialogMF")) {
				if (ImGuiFileDialog::Instance()->IsOk())
					strcpy(mods_folder, ImGuiFileDialog::Instance()->GetCurrentPath().c_str());
				ImGuiFileDialog::Instance()->Close();
			}

			ImGui::Text("Game folder:     ");
			ImGui::SameLine();
			ImGui::PushItemWidth(350);
			ImGui::InputText("##gamefolder", game_folder, IM_ARRAYSIZE(game_folder));
			ImGui::PopItemWidth();
			ImGui::SameLine();

			if (ImGui::Button("Select Folder##3"))
				ImGuiFileDialog::Instance()->OpenDialog("DirDialogGF", "Select Game folder", ".");
			if (ImGuiFileDialog::Instance()->Display("DirDialogGF")) {
				if (ImGuiFileDialog::Instance()->IsOk())
					strcpy(game_folder, ImGuiFileDialog::Instance()->GetCurrentPath().c_str());
				ImGuiFileDialog::Instance()->Close();
			}
			ImGui::Dummy(ImVec2(0, 5));

			ImGui::Text("Filter maps:     ");
			ImGui::SameLine();

			ImGui::PushItemWidth(350);
			if (ImGui::BeginCombo("##combo", selected_category.c_str())) {
				for (vector<string>::iterator it = categories.begin(); it != categories.end(); it++) {
					bool is_selected = selected_category == *it;
					if (ImGui::Selectable(it->c_str(), is_selected)) {
						selected_category = *it;
						levels = LoadLevels(selected_category);
						selected_level = levels.begin();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();

			ImGui::SameLine();
			ImGui::PushItemWidth(80);
			ImGui::Combo("##gameversion", &game_version, " 1.7.0\0 1.6.3\0 1.6.0\0 1.5.4\0");
			ImGui::PopItemWidth();

			ImGui::Spacing();
			ImGui::SameLine(64);
			ImGui::Text("File Name");
			ImGui::SameLine(364);
			ImGui::Text("Level Name");
			if (ImGui::BeginListBox("##listbox", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing() + 5))) {
				for (vector<LevelInfo>::iterator it = levels.begin(); it != levels.end(); it++) {
					bool is_selected = selected_level == it;
					if (ImGui::Selectable(it->filename.c_str(), is_selected) && !disable_convert) {
						selected_level = it;
					}
					ImGui::SameLine(300);
					if (it->title.empty())
						ImGui::Text(it->level_id.c_str());
					else
						ImGui::Text(it->title.c_str());
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}
			ImGui::Dummy(ImVec2(0, 10));

			ImGui::Text("Selected Level:");
			ImGui::SameLine();
			ImGui::Text(selected_level->title.c_str());
			ImGui::TextWrapped(selected_level->description.c_str());
			ImGui::Dummy(ImVec2(0, 10));

			// Avoid conflicts with file names
			string preview_name = selected_level->level_id;
			if (selected_category == "The Greenwash Gambit")
				preview_name = preview_name + "_dlc3";

			if (selected_preview != preview_name) {
				selected_preview = preview_name;
				string texture_path = "preview/" + selected_preview + ".png";
				preview_texture = LoadTexture(texture_path.c_str());
			}
			if (preview_texture != 0)
				ImGui::Image((void*)(uintptr_t)preview_texture, ImVec2(175, 100));

			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Checkbox("Remove snow", &remove_snow);
			ImGui::Checkbox("Legacy format", &save_as_legacy);
			ImGui::Checkbox("Do not use voxboxes", &no_voxbox);
			ImGui::Checkbox("Compress .vox files (very slow)", &use_tdcz);
			ImGui::EndGroup();
			ImGui::Dummy(ImVec2(0, 5));

			if (disable_convert && progress >= 1) {
				progress = 1;
				disable_convert = false;
			}

			const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
			const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
			if (disable_convert)
				ImGui::BufferingBar("##buffer_bar", progress, ImVec2(600, 8), bg, col);

			ImGui::Dummy(ImVec2(0, 10));
			bool disabled = disable_convert;
			if (disabled)
				ImGui::BeginDisabled();
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.3f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.3f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.3f, 0.8f, 0.8f));

			ImGui::Spacing();
			ImGui::SameLine(ImGui::GetWindowSize().x / 2 - 72);
			if (ImGui::Button("CONVERT", ImVec2(72, 32))) {
				progress = 0;
				disable_convert = true;

				params->level_id = selected_level->level_id;
				params->level_name = selected_level->title;
				params->level_desc = selected_level->description;

				if (selected_category == "Art Vandals")
					params->dlc_id = "artvandals";
				else if (selected_category == "Time Campers")
					params->dlc_id = "wildwestheist";
				else if (selected_category == "Folkrace")
					params->dlc_id = "folkrace";
				else if (selected_category == "The Greenwash Gambit")
					params->dlc_id = "space";
				else if (selected_category == "Relic Hunters")
					params->dlc_id = "relichunters";

				if (!params->dlc_id.empty()) {
					params->script_folder = game_folder;
					params->script_folder += "/dlcs/" + params->dlc_id + "/";
				} else {
					params->script_folder = game_folder;
					params->script_folder += "/data/level/";
				}

				if (selected_level->filename == "quicksave") {
					params->bin_path = quicksave_folder;
					params->bin_path += "/";
				} else if (!params->dlc_id.empty()) {
					params->bin_path = game_folder;
					params->bin_path += "/dlcs/" + params->dlc_id + "/bin/";
				} else {
					params->bin_path = game_folder;
					params->bin_path += "/data/bin/";
				}
				params->bin_path += selected_level->filename;

				string tdbin = params->bin_path + ".tdbin";
				FILE* already_decompressed = fopen(tdbin.c_str(), "rb");
				if (already_decompressed != NULL) {
					printf("A decompressed file was found for the current level.\n");
					params->bin_path += ".tdbin";
					fclose(already_decompressed);
				} else
					params->bin_path += ".bin";

				params->map_folder = mods_folder;
				params->map_folder += "/";
				if (!save_as_legacy)
					params->map_folder += selected_level->filename + "/";

				params->use_voxbox = !no_voxbox;
				params->remove_snow = remove_snow;
				params->compress_vox = use_tdcz;
				params->legacy_format = save_as_legacy;

				pthread_create(&parse_thread, NULL, DecompileMap, params);
			}
			ImGui::PopStyleColor(3);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.9f, 0.9f));

			ImGui::SameLine(ImGui::GetWindowSize().x / 2 + 36);
			if (ImGui::Button("Close", ImVec2(72, 32)))
				glfwSetWindowShouldClose(window, true);
			ImGui::PopStyleColor(3);

			if (disabled)
				ImGui::EndDisabled();
			ImGui::End();
		}

		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	pthread_join(parse_thread, NULL);
	delete params;

	if (config_root == NULL) {
		config_root = config_file.NewElement("config");
		config_file.InsertFirstChild(config_root);
	}
	config_root->SetAttribute("game_folder", game_folder);
	config_root->SetAttribute("mods_folder", mods_folder);
	config_root->SetAttribute("quicksave_folder", quicksave_folder);
	config_file.SaveFile("config.xml");

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
