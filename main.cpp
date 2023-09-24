#include <stdio.h>
#include <string.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_thread.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"
#include "file_dialog/ImGuiFileDialog.h"

#include "lib/tinyxml2.h"

#include "src/parser.h"

using namespace std;
using namespace tinyxml2;
namespace fs = std::filesystem;

volatile float progress = 0;

struct LevelInfo {
	string level_id;
	string filename;
	string title;
	string description;
};

void SaveInfoTxt(string map_folder, string level_name, string level_desc) {
	string info_path = map_folder + "info.txt";
	FILE* info_file = fopen(info_path.c_str(), "w");
	if (info_file == NULL) {
		printf("[ERROR] Could not create info.txt\n");
		return;
	}
	fprintf(info_file, "name = %s\n", level_name.c_str());
	fprintf(info_file, "author = Tuxedo Labs\n");
	fprintf(info_file, "description = %s\n", level_desc.c_str());
	fprintf(info_file, "tags = Map, Asset\n");
	fclose(info_file);
}

int DecompileMap(void* param) {
	ConverterParams* data = (ConverterParams*)param;

	fs::create_directories(data->map_folder);
	fs::create_directories(data->map_folder + (data->legacy_format ? "custom" : "vox"));
	SaveInfoTxt(data->map_folder, data->level_name, data->level_desc);

	string preview_image = "preview\\" + data->level_id + ".jpg";
	if (fs::exists(preview_image) && !fs::exists(data->map_folder + "preview.jpg"))
		fs::copy(preview_image, data->map_folder + "preview.jpg");

	string script_folder = data->game_folder + "data\\level\\" + data->level_id + "\\script";
	if (fs::exists(script_folder) && !fs::exists(data->map_folder + "main\\script")) {
		fs::create_directories(data->map_folder + "main");
		fs::copy(script_folder, data->map_folder + "main\\script", fs::copy_options::recursive);
	}

	ParseFile(*data);
	return 0;
}

int FakeProgressBar(void* param) {
	(void)param;
	progress = 0.01f;
	while (progress < 0.98) {
		this_thread::sleep_for(chrono::seconds(1));
		progress += ((float)rand() / RAND_MAX) / 10.0f;
	}
	return 0;
}

vector<LevelInfo> LoadLevels() {
	vector<LevelInfo> levels;
	const char* sandbox[][4] = {
		{"lee", "lee_sandbox", "Lee Chemicals Sandbox", "Operated by the Lee family for three generations. Lawrence Lee Junior showed a promising start, but developed a weakness for fast cash. He is now a well known name in the criminal underworld."},
		{"marina", "marina_sandbox", "West Point Marina Sandbox", "The oldest marina in Löckelle municipality. It features an industrial part and a separate section for leisure activities."},
		{"mansion", "mansion_sandbox", "Villa Gordon Sandbox", "The home of mall manager and racing enthusiast Gordon Woo, his daughter Freya and fiancee Enid Coumans. An exclusive mansion with a private race track in the backyard."},
		{"caveisland", "caveisland_sandbox", "Hollowrock Island Sandbox", "Formerly an old fishing hamlet, a few years ago Hollowrock Island was transformed into an energy drink research facility."},
		{"mall", "mall_sandbox", "The Evertides Mall Sandbox", "An upscale shopping center by the waterfront managed by Gordon Woo. Also in the area is Löckelle municipality folk museum."},
		{"frustrum", "frustrum_sandbox", "Frustrum Sandbox", "Tiny village of Frustrum along the Löckelle river, featuring an upscale hotel, a nightclub and a gas station."},
		{"hub_carib", "hub_carib_sandbox", "Muratori Beach Sandbox", "A tropical beach in the beautiful Muratori Islands. Go for a swim in the ocean or just relax on the beach."},
		{"carib", "carib_sandbox", "Isla Estocastica Sandbox", "A tucked away tropical island in the Muratoris where the secret BlueTide ingredient is being synthesised and shipped to Löckelle."},
		{"factory", "factory_sandbox", "Quilez Security Sandbox", "A high-tech manufacturing and research facility built on the steep cliffs of the north shore. Quilez Security has been a long time market leader in wired alarm boxes, but is pivoting into autonomous security robots."},
		{"cullington", "cullington_sandbox", "Cullington Sandbox", "Cliffside home town of Tracy and Löckelle Teardown Services HQ"},
	};
	int sandbox_count = sizeof(sandbox) / sizeof(sandbox[0]);

	for (int i = 0; i < sandbox_count; i++) {
		LevelInfo info = { sandbox[i][0], sandbox[i][1], sandbox[i][2], sandbox[i][3] };
		levels.push_back(info);
	}

	const char* gamemodes[][3] = {
		{"fetch", "Fetch", "Pick up as many targets as possible and get to your escape vehicle before the time runs out."},
		{"hunted", "Hunted", "Pick up as many targets as possible from randomized positions. Avoid the guard helicopter."},
		{"mayhem", "Mayhem", "Destroy as much as possible in 60 seconds. Be careful during preparation because the timer starts when 1000 voxels have been destroyed."},
	};
	int gamemodes_count = sizeof(gamemodes) / sizeof(gamemodes[0]);

	const char* maps[][2] {
		{"lee", "Lee Chemicals"},
		{"marina", "West Point Marina"},
		{"mansion", "Villa Gordon"},
		{"caveisland", "Hollowrock Island"},
		{"mall", "The Evertides Mall"},
		{"frustrum", "Frustrum"},
		{"factory", "Quilez Security"},
		{"carib", "Isla Estocastica"},
	};
	int maps_count = sizeof(maps) / sizeof(maps[0]);

	for (int j = 0; j < maps_count; j++)
		for (int i = 0; i < gamemodes_count; i++) {
			LevelInfo info = { maps[j][0], "ch_" + string(maps[j][0]) + "_" + string(gamemodes[i][0]), maps[j][1] + string(" ") + gamemodes[i][1], gamemodes[i][2] };
			levels.push_back(info);
		}

	int hub_version = 1;
	int hub_carib_version = 1;
	string hub_description = "Family owned demolition company and your home base. Through the computer terminal you can read messages, accept missions and upgrade you tools.";
	for (int v = 0; v <= 46; v++) {
		string filename = "hub" + to_string(v);
		if (v <= 16) {
			string title = "Hub Part 1 v" + to_string(hub_version);
			LevelInfo info = { "hub", filename, title, hub_description };
			levels.push_back(info);
			hub_version++;
		}
		if ((v >= 20 && v <= 24) || (v >= 40 && v <= 46)) {
			string title = "Hub Part 2 v" + to_string(hub_version);
			LevelInfo info = { "hub", filename, title, hub_description };
			levels.push_back(info);
			hub_version++;
		}
		if (v >= 30 && v <= 34) {
			string title = "Hub Caribbean v" + to_string(hub_carib_version);
			LevelInfo info = { "hub_carib", filename, title, hub_description };
			levels.push_back(info);
			hub_carib_version++;
		}
	}

	const char* missions[][4] = {
		{ "mall", "mall_intro", "The old building problem", "An old building is blocking Gordon's plans for a new wing at the mall. Help him demolish it." },
		{ "mall", "mall_foodcourt", "Covert chaos", "Scavenge the area and prepare for demolition without causing any damage to the food court. Rest in the escape vehicle when ready. Demolish at least half of the food court during the fireworks and leave the scene." },
		{ "mall", "mall_shipping", "The shipping logs", "Bring back the shipping logs for Parisa." },
		{ "mall", "mall_decorations", "Ornament ordeal", "Steal at least four christmas decorations for Gordon's Woonderland. Carry them to your van. Watch out for security robots that will trigger the alarm if you are detected." },
		{ "mall", "mall_radiolink", "Connecting the dots", "Download communication data by simultaneously connecting multiple transmission antennas to the surveillance van. Make sure each antenna is within range and has free line of sight." },
		{ "lee", "lee_computers", "The Lee computers", "Parisa wants access to Lee's customer registry. Pick up three computers at the site for her." },
		{ "lee", "lee_login", "The login devices", "Pick up three login devices for Parisa. They are protected by an alarm system. Security arrives 60 seconds after alarm is triggered. Make sure to plan ahead." },
		{ "lee", "lee_safe", "Heavy lifting", "The GPS decryption key is in Lee's safe. Move the safe to your escape vehicle. If possible, also get the other safe and pick up all key cabinets." },
		{ "lee", "lee_tower", "The tower", "Gordon wants to get back at Lee by making his iconic tower shorter. Remove the upper part of the tower." },
		{ "lee", "lee_powerplant", "Power outage", "Sabotage Lee Chemical's power supply system. Place the large bomb within the marked area by the dam turbines and detonate. Take out switchgear stations and transformers." },
		{ "lee", "lee_flooding", "Flooding", "Steal Lee's bookkeeping documents for Parisa." },
		{ "lee_woonderland", "lee_woonderland", "Malice in Woonderland", "Help locked up Lee sabotage Gordon's Woonderland. Demolish the rides so they are below the marked threshold. Breaking a neon sign will trigger the alarm." },
		{ "marina", "marina_demolish", "Making space", "Make room for Gordon's new yacht at the Marina by demolishing the outermost cabin. Destroy proof of ownership by dumping safes in the ocean." },
		{ "marina", "marina_gps", "The GPS devices", "Steal GPS devices from Lee's boats and get the log files from harbor office." },
		{ "marina", "marina_cars", "Classic cars", "Get the two classic cars for Gordon. Drive them to the marked area at the back of the truck. If possible, also pick up spare parts and vehicle registration documents." },
		{ "marina", "marina_tools", "Tool up", "Anton from Wolfe Construction needs new tools for an upcoming job. Help him get them from the marina" },
		{ "marina", "marina_art_back", "Art return", "Lee has hidden four paintings at the marina. Help the insurance company retrieve them. Avoid setting off the fire alarm." },
		{ "marina", "mansion_pool", "The car wash", "Dump at least three of Gordon's expensive cars in water" },
		{ "mansion", "mansion_art", "Fine arts", "Steal at least four paintings from Gordon's art collection for Lee" },
		{ "mansion", "mansion_fraud", "Insurance fraud", "Help Gordon steal at least three of his own cars to help him with the insurance payout." },
		{ "mansion", "mansion_safe", "A wet affair", "Destroy Gordon's insurance papers by dumping his brand new safes in water. The safes feature a moisture alarm that triggers in contact with water or rain." },
		{ "mansion", "mansion_race", "The speed deal", "Help Lee annoy Gordon by beating the track record on his private race track." },
		{ "caveisland", "caveisland_computers", "The BlueTide computers", "Steal computers for Parisa to investigate Lee's payments from BlueTide" },
		{ "caveisland", "caveisland_propane", "Motivational reminder", "Destroy Mr Amanatides propane tanks for Gillian to demonstrate the true value of proper insurance." },
		{ "caveisland", "caveisland_dishes", "An assortment of dishes", "Download communication data from three satellite dishes and at least two communication terminals. The island is protected by an armed  guard helicopter that arrives shortly after hacking the first target." },
		{ "caveisland", "caveisland_ingredients", "The secret ingredients", "Help Parisa understand why BlueTide is so addictive. Pick up samples of the secret ingredients. Each sample is stored in a secure safe that can only be opened with explosives. Watch out for security robots." },
		{ "caveisland", "caveisland_roboclear", "Droid dismount", "Clear the area for Parisa's big raid. Neutralize security robots by dumping them into the ocean." },
		{ "frustrum", "frustrum_chase", "The chase", "The security helicopter caught you while escaping from Lee Chemicals. Get through the flooded village of Frustrum, reach the speedboat at the far end and escape through the tunnel." },
		{ "frustrum", "frustrum_tornado", "The BlueTide shortage", "Move BlueTide kegs to the escape vehicle to help Mr Amanatides restock his stores. Stay away from tornados." },
		{ "frustrum", "frustrum_vehicle", "Truckload of trouble", "A military truck got stuck in the Frustrum canal. Mr Amanatides needs it for his secret project. Bring the vehicle parts to the escape vehicle." },
		{ "frustrum", "frustrum_pawnshop", "The pawn shop", "Steal valuable items in Frustrum for Anton Wolfe's new pawn shop while everyone is shopping at the Evertides closing down sale." },
		{ "factory", "factory_espionage", "Roborazzi", "Mr Amanatides needs help with industrial espionage to get a discount on security robots. Take pictures of the new prototypes in Quilez robot research lab." },
		{ "factory", "factory_tools", "The Quilez tools", "Secure vaults are being installed at Quilez Security. Anton Wolfe wants the construction tools." },
		{ "factory", "factory_robot", "The droid abduction", "Droid prototypes are kept in the secure vaults. Use the heavy laser cutter to melt the vault doors. Bring at least one droid prototype to the escape boat." },
		{ "factory", "factory_explosive", "Handle with care", "Bring at least three intact nitroglycerin containers to the escape vehicle. Avoid guard robots." },
		{ "carib", "carib_alarm", "The alarm system", "Hack the two main security terminals and at least two communication stations within 60 seconds to shut down the alarm system." },
		{ "carib", "carib_barrels", "Moving the goods", "Collect evidence for Parisa. Move at least three of the heavy barrels to the escape boat." },
		{ "carib", "carib_destroy", "Havoc in paradise", "Scavenge the area for tools. Destroy at least four of the targets. Avoid the guard helicopter." },
		{ "carib", "carib_yacht", "Elena's revenge", "One of the workers has spotted your activities. She is having problems with her boss and wants revenge. Help her sink his luxury yacht into the ocean" },
		{ "cullington", "cullington_bomb", "The final diversion", "Save Tracy and Cullington by making sure Mr Amanatides Truxterminator falls into the ocean without exploding." },
	};
	int missions_count = sizeof(missions) / sizeof(missions[0]);

	for (int i = 0; i < missions_count; i++) {
		LevelInfo info = { missions[i][0], missions[i][1], missions[i][2], missions[i][3] };
		levels.push_back(info);
	}

	LevelInfo info;
	info = { "about", "about", "Credits", "" };
	levels.push_back(info);
	info = { "lee", "ending10", "Lee Chemicals Part 1 Ending", "" };
	levels.push_back(info);
	info = { "hub", "ending20", "Hub Part 2 Ending", "" };
	levels.push_back(info);
	info = { "mansion", "ending21", "Villa Gordon Part 2 Ending", "" };
	levels.push_back(info);
	info = { "marina", "ending22", "Marina Part 2 Ending", "" };
	levels.push_back(info);
	info = { "", "quicksave", "Last Saved Level", "" };
	levels.push_back(info);
	info = { "lee", "test", "Performance Test", "" };
	levels.push_back(info);

	return levels;
}

string GetFilename(const char* path) {
	string filename = path;
	size_t pos = filename.find_last_of("\\/");
	if (pos != string::npos)
		filename = filename.substr(pos + 1);
	filename = filename.substr(0, filename.find_last_of("."));
	return filename;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
	_setmaxstdio(2048);
#endif
	if (argc > 1) {
		if (argc == 2) {
			ParseFile({argv[1], GetFilename(argv[1]) + "/", "", "", "", "", false, false, false});
		} else
			printf("CLI Usage: %s quicksave.bin\n", argv[0]);
		return 0;
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
		printf("[ERROR] Failed to init SDL: %s\n", SDL_GetError());
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	int window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
	SDL_Window* window = SDL_CreateWindow("Teardown Converter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 570, window_flags);
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
	dialog_flags |= ImGuiWindowFlags_NoResize;

	// Needs to be char[] for InputText to work
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
	bool use_tdcz = false;
	int game_version = 0;

	ConverterParams* params = new ConverterParams();
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
			ImGui::Dummy(ImVec2(0, 5));

			ImGui::Text("Game Version:    ");
			ImGui::SameLine();
			ImGui::PushItemWidth(80);
			ImGui::Combo("##gameversion", &game_version, " 1.4.0\0 1.3.0\0 1.2.0\0 1.1.0\0 1.0.0\0 0.9.6\0 0.9.5\0 0.9.2\0 0.9.0\0 0.8.0\0 0.7.4\0 0.7.2\0 0.7.1\0 0.7.0\0 0.6.2\0 0.6.1\0 0.5.2\0 0.5.1\0 0.4.6\0 0.4.5\0 0.3.0\0");
			ImGui::PopItemWidth();

			ImGui::Dummy(ImVec2(0, 10));
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
			ImGui::Checkbox("Compress Vox Files", &use_tdcz);
			ImGui::Checkbox("Legacy format", &save_as_legacy);
			ImGui::Dummy(ImVec2(0, 5));

			if (disable_convert) {
				if (progress >= 1) {
					progress = 1;
					disable_convert = false;
				}
			}

			const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
			const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
			if (progress > 0 && progress <= 1)
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
					params->bin_path = quicksave_folder;
					params->bin_path += "\\";
				} else {
					params->bin_path = game_folder;
					params->bin_path += "\\data\\bin\\";
				}
				params->bin_path += selected_level_it->filename;

				string tdbin = params->bin_path + ".tdbin";
				FILE* already_decompressed = fopen(tdbin.c_str(), "rb");
				if (already_decompressed != NULL) {
					printf("A decompressed file was found for the current level.\n");
					params->bin_path += ".tdbin";
					fclose(already_decompressed);
				} else
					params->bin_path += ".bin";

				params->map_folder = mods_folder;
				params->map_folder += "\\" + selected_level_it->filename + "\\";

				params->game_folder = game_folder;
				params->game_folder += "\\";

				params->level_id = selected_level_it->level_id;
				params->level_name = selected_level_it->title;
				params->level_desc = selected_level_it->description;
				params->remove_snow = remove_snow;
				params->compress_vox = use_tdcz;
				params->legacy_format = save_as_legacy;

				parse_thread = SDL_CreateThread(DecompileMap, "decompile_thread", (void*)params);
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

	if (config_root == NULL) {
		config_root = config_file.NewElement("config");
		config_file.InsertFirstChild(config_root);
	}
	config_root->SetAttribute("game_folder", game_folder);
	config_root->SetAttribute("mods_folder", mods_folder);
	config_root->SetAttribute("quicksave_folder", quicksave_folder);
	config_file.SaveFile("config.xml");

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
