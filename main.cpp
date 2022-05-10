#include <stdio.h>
#include <string.h>
#include <map>
#include <list>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <utility>
#include <experimental/filesystem>

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_thread.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "file_dialog/ImGuiFileDialog.h"

#include "parser.h"

using namespace std;
using namespace std::experimental::filesystem;

float progress = 0;

struct LevelInfo {
	string level_id;
	string filename;
	string title;
	string description;
};

struct ConverterParams {
	string bin_path;
	string map_folder;
	string game_folder;

	string level_id;
	string level_name;
	string level_desc;
};

void SaveInfoTxt(string map_folder, string level_name, string level_desc) {
	string info_path = map_folder + "info.txt";
	FILE* info_file = fopen(info_path.c_str(), "w");
	if (info_file == NULL) {
		printf("ERROR: Could not create info.txt\n");
		return;
	}
	fprintf(info_file, "name = %s\n", level_name.c_str());
	fprintf(info_file, "author = Tuxedo Labs\n");
	fprintf(info_file, "description = %s\n", level_desc.c_str());
	fprintf(info_file, "tags = Map Asset\n");
	fclose(info_file);
}

int DecompileMap(void* param) {
	ConverterParams* data = (ConverterParams*)param;

	create_directories(data->map_folder);
	create_directories(data->map_folder + "vox");
	create_directories(data->map_folder + "compounds");
	SaveInfoTxt(data->map_folder, data->level_name, data->level_desc);

	string preview_image = "preview\\" + data->level_id + ".jpg";
	if (exists(preview_image))
		copy(preview_image, data->map_folder + "preview.jpg");

	string script_folder = data->game_folder + "data\\level\\" + data->level_id + "\\script";
	if (exists(script_folder))
		copy(script_folder, data->map_folder + "script");

	printf("Preview image: %s\n", preview_image.c_str());
	printf("Script Folder: %s\n", script_folder.c_str());

	ParseFile(data->bin_path.c_str(), data->map_folder);
	return 0;
}

int FakeProgressBar(void* param) {
	(void)param;
	progress = 0.01;
	while (progress < 0.98) {
		this_thread::sleep_for(chrono::seconds(1));
		progress += ((double)rand() / RAND_MAX) / 10.0;
	}
	return 1;
}

/*
map<string, string> LoadLevels() {
	const char* sandbox[][2] = {
		{"lee_sandbox", "Lee Chemicals Sandbox"},
		{"marina_sandbox", "Marina Sandbox"},
		{"mansion_sandbox", "Villa Gordon Sandbox"},
		{"caveisland_sandbox", "Hollowrock Sandbox"},
		{"mall_sandbox", "Evertides Sandbox"},
		{"frustrum_sandbox", "Frustrum Sandbox"},
		{"carib_sandbox", "Isla Estocastica Sandbox"},
		{"factory_sandbox", "Quilez Security Sandbox"},
		{"cullington_sandbox", "Cullington Sandbox"},
	};
	int sandbox_count = sizeof(sandbox) / sizeof(sandbox[0]);

	const char* challenges[][2] = {
		{"ch_carib_fetch", "Isla Estocastica Fetch"},
		{"ch_carib_hunted", "Isla Estocastica Hunded"},
		{"ch_carib_mayhem", "Isla Estocastica Mayhem"},
		{"ch_caveisland_fetch", "Hollowrock Fetch"},
		{"ch_caveisland_hunted", "Hollowrock Hunded"},
		{"ch_caveisland_mayhem", "Hollowrock Mayhem"},
		{"ch_factory_fetch", "Quilez Security Fetch"},
		{"ch_factory_hunted", "Quilez Security Hunded"},
		{"ch_factory_mayhem", "Quilez Security Mayhem"},
		{"ch_frustrum_fetch", "Frustrum Fetch"},
		{"ch_frustrum_hunted", "Frustrum Hunded"},
		{"ch_frustrum_mayhem", "Frustrum Mayhem"},
		{"ch_lee_fetch", "Lee Chemicals Fetch"},
		{"ch_lee_hunted", "Lee Chemicals Hunded"},
		{"ch_lee_mayhem", "Lee Chemicals Mayhem"},
		{"ch_mall_fetch", "Evertides Fetch"},
		{"ch_mall_hunted", "Evertides Hunded"},
		{"ch_mall_mayhem", "Evertides Mayhem"},
		{"ch_mansion_fetch", "Villa Gordon Fetch"},
		{"ch_mansion_hunted", "Villa Gordon Hunded"},
		{"ch_mansion_mayhem", "Villa Gordon Mayhem"},
		{"ch_marina_fetch", "Marina Fetch"},
		{"ch_marina_hunted", "Marina Hunded"},
		{"ch_marina_mayhem", "Marina Mayhem"},
	};
	int challenges_count = sizeof(challenges) / sizeof(challenges[0]);

	const char* missions[][2] = {
		{"mall_intro", "The old building problem"},
		{"mall_foodcourt", "Covert chaos"},
		{"mall_shipping", "The shipping logs"},
		{"mall_decorations", "Ornament ordeal"},
		{"mall_radiolink", "Connecting the dots"},
		{"lee_computers", "The Lee computers"},
		{"lee_login", "The login devices"},
		{"lee_safe", "Heavy lifting"},
		{"lee_tower", "The tower"},
		{"lee_powerplant", "Power outage"},
		{"lee_flooding", "Flooding"},
		{"lee_woonderland", "Malice in Woonderland"},
		{"marina_demolish", "Making space"},
		{"marina_gps", "The GPS devices"},
		{"marina_cars", "Classic cars"},
		{"marina_tools", "Tool up"},
		{"marina_art_back", "Art return"},
		{"mansion_pool", "The car wash"},
		{"mansion_art", "Fine arts"},
		{"mansion_fraud", "Insurance fraud"},
		{"mansion_safe", "A wet affair"},
		{"mansion_race", "The speed deal"},
		{"caveisland_computers", "The BlueTide computers"},
		{"caveisland_propane", "Motivational reminder"},
		{"caveisland_dishes", "An assortment of dishes"},
		{"caveisland_ingredients", "The secret ingredients"},
		{"caveisland_roboclear", "Droid dismount"},
		{"frustrum_chase", "The chase"},
		{"frustrum_tornado", "The BlueTide shortage"},
		{"frustrum_vehicle", "Truckload of trouble"},
		{"frustrum_pawnshop", "The pawn shop"},
		{"factory_espionage", "Roborazzi"},
		{"factory_tools", "The Quilez tools"},
		{"factory_robot", "The droid abduction"},
		{"factory_explosive", "Handle with care"},
		{"carib_alarm", "The alarm system"},
		{"carib_barrels", "Moving the goods"},
		{"carib_destroy", "Havoc in paradise"},
		{"carib_yacht", "Elena's revenge"},
		{"cullington_bomb", "The final diversion"}
	};
	int missions_count = sizeof(missions) / sizeof(missions[0]);

	map<string, string> levels;
	levels["quicksave"] = "Last Saved Level";

	for (int i = 0; i < sandbox_count; i++)
		levels[sandbox[i][0]] = sandbox[i][1];
	for (int i = 0; i < missions_count; i++)
		levels[missions[i][0]] = missions[i][1];
	for (int i = 0; i < challenges_count; i++)
		levels[challenges[i][0]] = challenges[i][1];

	for (int v = 0; v <= 46; v++) {
		string key = "hub" + to_string(v);
		if (v <= 16) {
			string value = "Hub Part 1 v" + to_string(v);
			levels[key] = value;
		}
		if ((v >= 20 && v <= 24) || (v >= 40 && v <= 46)) {
			string value = "Hub Part 2 v" + to_string(v);
			levels[key] = value;
		}
		if (v >= 30 && v <= 34) {
			string value = "Hub Caribbean v" + to_string(v);
			levels[key] = value;
		}
	}
	levels["about"] = "About Showcase";
	levels["ending10"] = "Lee Chemicals Part 1 Ending";
	levels["ending20"] = "Hub Part 2 Ending";
	levels["ending21"] = "Villa Gordon Part 2 Ending";
	levels["ending22"] = "Marina Part 2 Ending";

	//printf("Loaded Levels: %lld / 112\n", levels.size() - 1);
	return levels;
}
*/
vector<LevelInfo> LoadLevels() {
	vector<LevelInfo> levels;
/*	{"", "", "Fetch", "Pick up as many targets as possible and get to your escape vehicle before the time runs out."},
	{"", "", "Hunted", "Pick up as many targets as possible from randomized positions. Avoid the guard helicopter."},
	{"", "", "Mayhem", "Destroy as much as possible in 60 seconds. Be careful during preparation because the timer starts when 1000 voxels have been destroyed."},

	{"about", "about", "About", ""},
	{"hub", "", "Löckelle Teardown Services", "Löckelle Teardown Services", "Family owned demolition company and your home base. Through the computer terminal you can read messages, accept missions and upgrade you tools."},
	{"hub_carib", "", "Löckelle Teardown Services", "Family owned demolition company and your home base. Through the computer terminal you can read messages, accept missions and upgrade you tools."},
	{"lee_woonderland", "lee_woonderland", "Malice in Woonderland", "Help locked up Lee sabotage Gordon's Woonderland. Demolish the rides so they are below the marked threshold. Breaking a neon sign will trigger the alarm."},
*/
	const char* sandbox[][4] = {
		{"lee", "lee_sandbox", "Lee Chemicals Sandbox", "Operated by the Lee family for three generations. Lawrence Lee Junior showed a promising start, but developed a weakness for fast cash. He is now a well known name in the criminal underworld."},
		{"marina", "marina_sandbox", "West Point Marina Sandbox", "The oldest marina in Löckelle municipality. It features an industrial part and a separate section for leisure activities."},
		{"mansion", "mansion_sandbox", "Villa Gordon Sandbox", "The home of mall manager and racing enthusiast Gordon Woo, his daughter Freya and fiancee Enid Coumans. An exclusive mansion with a private race track in the backyard."},
		{"caveisland", "caveisland_sandbox", "Hollowrock Island Sandbox", "Formerly an old fishing hamlet, a few years ago Hollowrock Island was transformed into an energy drink research facility."},
		{"mall", "mall_sandbox", "The Evertides Mall Sandbox", "An upscale shopping center by the waterfront managed by Gordon Woo. Also in the area is Löckelle municipality folk museum."},
		{"frustrum", "frustrum_sandbox", "Frustrum Sandbox", "Tiny village of Frustrum along the Löckelle river, featuring an upscale hotel, a nightclub and a gas station."},
		{"factory", "factory_sandbox", "Quilez Security Sandbox", "A high-tech manufacturing and research facility built on the steep cliffs of the north shore. Quilez Security has been a long time market leader in wired alarm boxes, but is pivoting into autonomous security robots."},
		{"carib", "carib_sandbox", "Isla Estocastica Sandbox", "A tucked away tropical island in the Muratoris where the secret BlueTide ingredient is being synthesised and shipped to Löckelle."},
		{"cullington", "cullington_sandbox", "Cullington Sandbox", "Cliffside home town of Tracy and Löckelle Teardown Services HQ"},
	};
	int sandbox_count = sizeof(sandbox) / sizeof(sandbox[0]);

	for (int i = 0; i < sandbox_count; i++) {
		LevelInfo info = { sandbox[i][0], sandbox[i][1], sandbox[i][2], sandbox[i][3] };
		levels.push_back(info);
	}

	return levels;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
	_setmaxstdio(2048);
#endif
	if (argc == 2) {
		#ifdef _WIN32
			ParseFile(argv[1], "test\\");
		#else
			ParseFile(argv[1], "test/");
		#endif
		return 0;
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		printf("Error SDL: %s\n", SDL_GetError());
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window* window = SDL_CreateWindow("Teardown Converter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 520, window_flags);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL2_Init();
	ImVec4 clear_color = ImVec4(0.27f, 0.57f, 0.72f, 1.00f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

	ImGuiWindowFlags dialog_flags = 0;
    //dialog_flags |= ImGuiWindowFlags_NoMove;
    //dialog_flags |= ImGuiWindowFlags_NoResize;

	// Needs to be char[] for InputText to work
#ifdef _WIN32
	char quicksave_folder[256] = "C:\\Users\\User\\AppData\\Local\\Teardown";
	char mods_folder[256] = "C:\\Users\\User\\Documents\\Teardown\\mods";
	char game_folder[256] = "E:\\JUEGOS\\SteamLibrary\\steamapps\\common\\Teardown";
#else
	char quicksave_folder[256] = "test";
	char mods_folder[256] = "test";
	char game_folder[256] = "test";
#endif

	bool disable_convert = false;
	bool remove_snow = false;
	bool use_mega_prop_pack = false;

	ConverterParams* paths = new ConverterParams();
	SDL_Thread* parse_thread = NULL;
	SDL_Thread* progress_thread = NULL;

	vector<LevelInfo> levels = LoadLevels();
	vector<LevelInfo>::iterator selected_level_it = levels.begin();

	bool done = false;
	while (!done) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				done = true;
		}

		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
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
				ImGuiFileDialog::Instance()->OpenDialog("DirDialogQS", "Select Quicksave folder", nullptr, ".");
			if (ImGuiFileDialog::Instance()->Display("DirDialogQS")) {
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
				ImGuiFileDialog::Instance()->OpenDialog("DirDialogMF", "Select Mods folder", nullptr, ".");
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
				ImGuiFileDialog::Instance()->OpenDialog("DirDialogGP", "Select Game folder", nullptr, ".");
			if (ImGuiFileDialog::Instance()->Display("DirDialogGP")) {
				if (ImGuiFileDialog::Instance()->IsOk())
					strcpy(game_folder, ImGuiFileDialog::Instance()->GetCurrentPath().c_str());
				ImGuiFileDialog::Instance()->Close();
			}

			ImGui::Dummy(ImVec2(0, 20));
			ImGui::Spacing();
			ImGui::SameLine(64);
			ImGui::Text("File Name");
			ImGui::SameLine(364);
			ImGui::Text("Level Name");
			if (ImGui::BeginListBox("##listbox", ImVec2(-FLT_MIN, 10 * ImGui::GetTextLineHeightWithSpacing()))) {
				for (vector<LevelInfo>::iterator it = levels.begin(); it != levels.end(); it++) {
					bool is_selected = selected_level_it == it;
					if (ImGui::Selectable(it->filename.c_str(), is_selected) && !disable_convert)
						selected_level_it = it;
					ImGui::SameLine(300);
					ImGui::Text(it->title.c_str());

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}
			ImGui::Dummy(ImVec2(0, 10));

			ImGui::Text("Selected Level:");
			ImGui::SameLine();
			ImGui::Text(selected_level_it->title.c_str());
			ImGui::Dummy(ImVec2(0, 10));

			ImGui::Checkbox("Remove Snow", &remove_snow);
			ImGui::Checkbox("Use Mega Prop Pack", &use_mega_prop_pack);
			ImGui::Dummy(ImVec2(0, 5));

			if (disable_convert) {
				if (progress > 1) {
					progress = 1;
					disable_convert = false;
				}
			}

			const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
        	const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
			if (progress > 0 && progress < 1)
        		ImGui::BufferingBar("##buffer_bar", progress, ImVec2(600, 8), bg, col);

			ImGui::Dummy(ImVec2(0, 10));
			bool disabled = disable_convert;
			if (disabled)
				ImGui::BeginDisabled();
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.3f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.3f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.3f, 0.8f, 0.8f));

			ImGui::Spacing();
			ImGui::SameLine(ImGui::GetWindowSize().x / 2 - 62);
			if (ImGui::Button("CONVERT", ImVec2(64, 25))) {
				progress = 0;
				disable_convert = true;
				if (selected_level_it->filename == "quicksave") {
					paths->bin_path = quicksave_folder;
					paths->bin_path += "\\";
				} else {
					paths->bin_path = game_folder;
					paths->bin_path += "\\data\\bin\\";
				}
				paths->bin_path += selected_level_it->filename;

				string tdbin = paths->bin_path + ".tdbin";
				FILE* already_decompressed = fopen(tdbin.c_str(), "rb");
				if (already_decompressed != NULL) {
					printf("A decompressed file was found for the current level.\n");
					paths->bin_path += ".tdbin";
					fclose(already_decompressed);
				} else
					paths->bin_path += ".bin";

				paths->map_folder = mods_folder;
				paths->map_folder += "\\" + selected_level_it->filename + "\\";

				paths->game_folder = game_folder;
				paths->game_folder += "\\";

				paths->level_id = selected_level_it->level_id;
				paths->level_name = selected_level_it->title;
				paths->level_desc = selected_level_it->description;

				parse_thread = SDL_CreateThread(DecompileMap, "decompile_thread", (void*)paths);
				progress_thread = SDL_CreateThread(FakeProgressBar, "progress_thread", NULL);
			}
			ImGui::PopStyleColor(3);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.9f, 0.9f));

			ImGui::SameLine(ImGui::GetWindowSize().x / 2 + 30);
			if (ImGui::Button("Close", ImVec2(64, 25)))
				done = true;
			ImGui::PopStyleColor(3);

			if (disabled)
				ImGui::EndDisabled();

			ImGui::End();
		}

		ImGui::Render();
		glViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	if (parse_thread != NULL)
		SDL_WaitThread(parse_thread, NULL);
	if (progress_thread != NULL)
		SDL_WaitThread(progress_thread, NULL);

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
