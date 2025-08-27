#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <atomic>
#include <chrono>
#include <string>
#include <vector>
#include <filesystem>

#include "src/parser.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/backend/imgui_impl_glfw.h"
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui/backend/imgui_impl_opengl3.h"
#include "file_dialog/ImGuiFileDialog.h"

#include "lib/tinyxml2.h"
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

using namespace std;
using namespace tinyxml2;
namespace fs = std::filesystem;

atomic<float> progress = 0;

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

void copy_file(string origin, string destination) {
	if (fs::exists(origin) && !fs::exists(destination))
		fs::copy(origin, destination);
}

void copy_folder(string origin, string destination) {
	if (fs::exists(origin) && !fs::exists(destination))
		fs::copy(origin, destination, fs::copy_options::recursive);
}

void SavePreviewImage(string map_folder, string level_id) {
	// Convert to JPEG
	string input_image = "preview/" + level_id + ".png";
	string output_path = map_folder + "preview.jpg";
	int width, height, channels;
	uint8_t* image = stbi_load(input_image.c_str(), &width, &height, &channels, 0);
	if (image == NULL) {
		printf("[WARNING] Could not load preview image.\n");
		return;
	}
	stbi_write_jpg(output_path.c_str(), width, height, channels, image, 90);
	stbi_image_free(image);
}

void* DecompileMap(void* param) {
	ConverterParams* data = (ConverterParams*)param;

	fs::create_directories(data->map_folder);
	if (!data->legacy_format) {
		// TODO: fix name collision
		SavePreviewImage(data->map_folder, data->level_id);
		SaveInfoTxt(data->map_folder, data->level_name, data->level_desc);
	}
	if (data->legacy_format) {
		fs::create_directories(data->map_folder + "custom");
		copy_folder(data->script_folder, data->map_folder + "custom/script");
	} else {
		fs::create_directories(data->map_folder + "vox");
		copy_folder(data->script_folder + data->level_id, data->map_folder + "main");
		if (!data->dlc_id.empty())
			copy_folder(data->script_folder + "script", data->map_folder + "script");
	}
	ParseFile(*data);
	return NULL;
}

vector<LevelInfo> LoadLevels(string filter) {
	vector<LevelInfo> levels;
	if (filter == "Sandbox") {
		const char* sandbox[][4] = {
			{"lee", "lee_sandbox", "Lee Chemicals Sandbox", "Operated by the Lee family for three generations. Lawrence Lee Junior showed a promising start, but developed a weakness for fast cash. He is now a well known name in the criminal underworld."},
			{"marina", "marina_sandbox", "West Point Marina Sandbox", "The oldest marina in Löckelle municipality. It features an industrial part and a separate section for leisure activities."},
			{"mansion", "mansion_sandbox", "Villa Gordon Sandbox", "The home of mall manager and racing enthusiast Gordon Woo, his daughter Freya and fiancee Enid Coumans. An exclusive mansion with a private track in the backyard."},
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
	} else if (filter == "Hub") {
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
	} else if (filter == "Missions") {
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
			{ "mansion", "mansion_race", "The speed deal", "Help Lee annoy Gordon by beating the track record on his private track." },
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
	} else if (filter == "Challenges") {
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
			{"carib", "Isla Estocastica"},
			{"factory", "Quilez Security"},
		};
		int maps_count = sizeof(maps) / sizeof(maps[0]);

		for (int j = 0; j < maps_count; j++)
			for (int i = 0; i < gamemodes_count; i++) {
				LevelInfo info = { maps[j][0], "ch_" + string(maps[j][0]) + "_" + string(gamemodes[i][0]), maps[j][1] + string(" ") + gamemodes[i][1], gamemodes[i][2] };
				levels.push_back(info);
			}
	} else if (filter == "Art Vandals") {
		const char* dlc_missions[][4] = {
			// Sandbox
			{ "tillaggaryd", "museum_sandbox", "Sandbox", "Sandbox mode, go have some fun!" },
			// Missions
			{ "tillaggaryd", "intro", "Art Vandals", "School's out for summer but no time to work on your tan when you play as Freya Woo helping your stepmum Enid in her rowdy plans." },
			{ "tillaggaryd", "paperboy", "Paper girl", "Prepare your paper route then wait for todays papers in the car and start delivering." },
			{ "tillaggaryd", "art_heist", "Her prized prizes", "Steal Kerstin Stråbäck's art prizes since Enid is jealous." },
			{ "tillaggaryd", "museum_heist", "Art Vandals", "Steal Kerstin's art from the museum since Enid found out they are up for new prizes. Bring the heavy targets to your car." },
			{ "tillaggaryd", "museum_cam_heist", "The photo shoot", "Steal the security cameras from the museum." },
			{ "tillaggaryd", "museum_destroy", "Out with a bang", "Destroy the museum since it's getting re-branded as Kerstin Stråbäck's Art Museum."},
		};
		int dlc_missions_count = sizeof(dlc_missions) / sizeof(dlc_missions[0]);

		for (int i = 0; i < dlc_missions_count; i++) {
			LevelInfo info = { dlc_missions[i][0], dlc_missions[i][1], dlc_missions[i][2], dlc_missions[i][3] };
			levels.push_back(info);
		}
		for (int i = 0; i < 7; i++) {
			string hub_id = "hub";
			if (i > 0) hub_id += to_string(i - 1);
			LevelInfo info = { "mansion", hub_id, "The Woo Mansion v" + to_string(i + 1), "Check your phone for messages." };
			levels.push_back(info);
		}
	} else if (filter == "Time Campers") {
		const char* dlc_missions[][4] = {
			// Sandbox
			{ "hub_wildwest", "hub", "Western Camp Sandbox", "The western camp." },
			{ "town", "town_sandbox", "Combustown Sandbox", "The old town and the mine." },
			{ "ravine", "ravine_sandbox", "Mineral Ravine Sandbox", "This is the ravine level, it is a big ravine, there's also a curch here." },
			// Missions
			{ "town", "town_heist", "Stocking Up", "Steal supplies for your camp, then go to your escape vehicle" },
			{ "town", "town_explosive", "Going Volatile", "Steal ingredients to make your own explosives. They are protected by an alarm system. The hot air balloon security arrives shortly after the alarm is triggered." },
			{ "ravine", "ravine_heist", "Hiding Traces", "Gather all objects stuck in time loops. The objects can be shot down but doing that or grabbing them will alert the hot air balloon." },
			{ "town", "town_destruction", "Preemptive Prohibition", "Demolish the saloon to keep unwanted visitors away from Combustown. The buildings looks very flammable." },
			{ "ravine", "ravine_tornado", "Tumbling around", "Bring the clockwork parts to the escape vehicle. Stay away from tornadoes" },
			{ "town", "town_grease", "Greasing the gears", "Steal grease barrels and jars to lubricate the clockwork" },
			{ "ravine", "ravine_motivational", "Motivational reminder", "Destroy the gunpowder supplies to demonstrate the true value of proper insurance." },
			{ "town", "town_cars", "Four Stolen Hooves", "Bring the wheel, horse and gear to the escape vehicle. Use horses and ropes to drag the heavy targets. If possible, steal some food for the horse and some extra wire." },
			{ "ravine", "ravine_bridge", "Choo-choosing path", "Destroy the bridge and get to the escape vehicle before the train arrives." },
			// Others
			{ "hub_lockelle", "start", "Löckelle Teardown Services", "Family owned demolition company and your home base. Through the computer terminal you can read messages, accept missions and upgrade you tools." },
			{ "hub_wildwest", "main", "Main menu", "Time Campers DLC - Main menu" },
			{ "hub_wildwest", "final", "Credits", "Time Campers DLC - Credits" },
			{ "ravine", "timetravel", "Timetravel", "Crash animation at the ravine" },
			// Hub
			{ "hub_lockelle", "hub_lockelle", "The hub", "The normal hub in löckelle." },
		};
		int dlc_missions_count = sizeof(dlc_missions) / sizeof(dlc_missions[0]);

		for (int i = 0; i < dlc_missions_count; i++) {
			LevelInfo info = { dlc_missions[i][0], dlc_missions[i][1], dlc_missions[i][2], dlc_missions[i][3] };
			levels.push_back(info);
		}
		for (int i = 0; i < 9; i++) {
			string hub_id = "hub" + to_string(i + 1);
			LevelInfo info = { "hub_wildwest", hub_id, "Western Camp v" + to_string(i + 1), "Check your laptop in the camper for messages." };
			levels.push_back(info);
		}
	} else if (filter == "Folkrace") {
		const char* dlc_missions[][4] = {
			// Sandbox
			{ "m_lobby", "lobby_sandbox", "Granriket", "Free roam sandbox play with unlimited resources and no challenge." },
			{ "m_village", "village_sandbox", "Almondman's Farm", "Free roam sandbox play with unlimited resources and no challenge." },
			{ "m_docks", "docks_sandbox", "Glennburgh Docks", "Free roam sandbox play with unlimited resources and no challenge." },
			{ "m_mall", "mall_sandbox", "Möbel Mall", "Free roam sandbox play with unlimited resources and no challenge." },
			{ "m_mall", "mall_storage_sandbox", "Möbel Mall Derby", "Free roam sandbox play with unlimited resources and no challenge." },
			// Missions
			{ "m_lobby", "bluetide", "A Good Deed", "Destroy the BlueTide vending machines with the shredder at Björn's landfill." },
			{ "m_lobby", "bluetide_1", "A Good Deed", "Destroy the BlueTide vending machines with the shredder at Björn's landfill." },
			{ "m_lobby", "bluetide_2", "A Good Deed", "Destroy the BlueTide vending machines with the shredder at Björn's landfill." },
			{ "m_lobby", "paperboy", "Honest work", "Take the newspapers and get ready for the delivery. When you're ready, get in your car. Don't forget the timer." },
			{ "m_lobby", "paperboy_1", "Honest work", "Take the newspapers and get ready for the delivery. When you're ready, get in your car. Don't forget the timer." },
			{ "m_lobby", "paperboy_2", "Honest work", "Take the newspapers and get ready for the delivery. When you're ready, get in your car. Don't forget the timer." },
			{ "m_lobby", "camera", "Photo hunt", "The countdown begins as soon as you take the first picture. Take the picture and return the camera back to the charging station before the timer runs out." },
			{ "m_lobby", "camera_1", "Photo hunt", "The countdown begins as soon as you take the first picture. Take the picture and return the camera back to the charging station before the timer runs out." },
			{ "m_lobby", "camera_2", "Photo hunt", "The countdown begins as soon as you take the first picture. Take the picture and return the camera back to the charging station before the timer runs out." },
			{ "m_lobby", "bjorn_revenge", "Björn's Vengeance", "Steal 4 valuable items belonging to Ingvar Hemmelsen. The hot dog grill needs to be delivered to the garage. Don't forget about the security systems!" },
			{ "m_lobby", "bjorn_revenge_1", "Björn's Vengeance", "Steal 4 valuable items belonging to Ingvar Hemmelsen. The hot dog grill needs to be delivered to the garage. Don't forget about the security systems!" },
			{ "m_lobby", "bjorn_revenge_2", "Björn's Vengeance", "Steal 4 valuable items belonging to Ingvar Hemmelsen. The hot dog grill needs to be delivered to the garage. Don't forget about the security systems!" },
			{ "m_lobby", "demolish", "Good Ol' Wreckin'", "Destroy the houses belonging to Ingvar. Beware of security robots." },
			{ "m_lobby", "demolish_1", "Good Ol' Wreckin'", "Destroy the houses belonging to Ingvar. Beware of security robots." },
			{ "m_lobby", "demolish_2", "Good Ol' Wreckin'", "Destroy the houses belonging to Ingvar. Beware of security robots." },
			// Races
			{ "m_village", "village_derby", "Almondman's Farm Derby", "" },
			{ "m_village", "village_derby_night", "Almondman's Farm Derby Night", "" },
			{ "m_village", "village_derby_winter", "Almondman's Farm Derby Winter", "" },
			{ "m_village", "village_race", "Almondman's Farm", "" },
			{ "m_village", "village_race_night", "Almondman's Farm Night", "" },
			{ "m_village", "village_race_winter", "Almondman's Farm Winter", "" },
			{ "m_village", "village_reverse_race", "Almondman's Farm Track 2", "" },
			{ "m_village", "village_reverse_race_night", "Almondman's Farm Track 2 Night", "" },
			{ "m_village", "village_reverse_race_winter", "Almondman's Farm Track 2 Winter", "" },
			{ "m_docks", "docks_derby_day_1", "Glennburgh Docks Derby", "" },
			{ "m_docks", "docks_derby_night_1", "Glennburgh Docks Derby Night", "" },
			{ "m_docks", "docks_race_day_1", "Glennburgh Docks", "" },
			{ "m_docks", "docks_race_day_2", "Glennburgh Docks Track 2", "" },
			{ "m_docks", "docks_race_day_winter_1", "Glennburgh Docks Winter", "" },
			{ "m_docks", "docks_race_day_winter_2", "Glennburgh Docks Track 2 Winter", "" },
			{ "m_docks", "docks_race_night_1", "Glennburgh Docks Night", "" },
			{ "m_docks", "docks_race_night_2", "Glennburgh Docks Track 2 Night", "" },
			{ "m_mall", "mall_derby", "Möbel Mall Derby", "" },
			{ "m_mall", "mall_race_day", "Möbel Mall", "" },
			{ "m_mall", "mall_race_night", "Möbel Mall Night", "" },
			{ "m_mall", "mall_race_rev_day", "Möbel Mall Track 2", "" },
			{ "m_mall", "mall_race_rev_night", "Möbel Mall Track 2 Night", "" },
			{ "m_village", "village_woo_champ_1", "Woo's Folkrace Grand Prix 1", "" },
			{ "m_mall", "woo_champ_2", "Woo's Folkrace Grand Prix 2", "" },
			{ "m_mall", "woo_champ_3", "Woo's Folkrace Grand Prix 3", "" },
		};
		int dlc_missions_count = sizeof(dlc_missions) / sizeof(dlc_missions[0]);
		for (int i = 0; i < dlc_missions_count; i++) {
			LevelInfo info = { dlc_missions[i][0], dlc_missions[i][1], dlc_missions[i][2], dlc_missions[i][3] };
			levels.push_back(info);
		}
		for (int i = 0; i < 14; i++) {
			string hub_ids[14] = { "lobby_0", "lobby_1", "lobby_2", "lobby_2_1", "lobby_2_1a", "lobby_2_1b", "lobby_2_2", "lobby_2_2a", "lobby_2_2b", "lobby_2_2c", "lobby_3", "lobby_3_0", "lobby_3a", "lobby_4" };
			LevelInfo info = { "m_lobby", hub_ids[i], "Granriket v" + to_string(i + 1), "Small and cozy Granriket. One of many similar locations in the municipality of Löckelle." };
			levels.push_back(info);
		}
	} else if (filter == "The Greenwash Gambit") {
		const char* dlc_missions[][4] = {
			{ "ravine", "distress", "Distress signal", "Investigate the distress signal and bring back Tier-1." },
			{ "ravine", "boats", "All aboard", "Gather the supplies left by the previous crew." },
			{ "evilbase", "evilbasephoto", "The Photoshoot", "Photograph the objects at The recycling plant to gather intel about EFO. Corps. operation." },
			{ "ravine", "ravinephoto", "Data mining", "Steal computers to gather information about the superfluid helium potential of the anti-gravity crystals in The Arroyo Chasm and destroy E.F.O. Corp's back-ups." },
			{ "ravine", "powercrystals", "Crystal collection", "Bring at least three anti-gravity crystals to the escape vehicle to construct the Quantum Vortex Bomb. The facilities are guarded by armed autonomous drones at night, which will pursue anyone identified as an intruder." },
			{ "evilbase", "spacebaseintro", "Emergency overrides", "Steal emergency override protocols for the cryogenic coolers, pressure containers and quantum vortex rotators. The manuals are behind protective glass that will trigger the alarm if broken." },
			{ "evilbase", "spacebaseheist", "Keeping cool", "Get hold of the cryogenic coolers, pressure containers and the rotational energy storage for Tier-1 and Mainframe." },
			{ "ravine", "ravineheist", "Gadget grabber", "Acquire a iodine-based radiation absorber, magnetic field stabilizers, and the auto-aiming scope." },
			{ "evilbase", "spacebasedata", "Locate in space", "Hack the terminals to retrieve the current positional data for the S.C.C.L." },
			{ "spacestation", "spacestationintro", "Send into orbit", "Disable the internal communication at the S.C.C.L by sending the receivers into space." },
			{ "spacestation", "spacestationdestroy", "Connection lost", "Destroy the S.C.C.L to break the stabilizing connection to the black hole generator." },
			{ "spacestation", "spacestationruins", "Fluid Recovery", "Get hold of superfluid helium needed for the Quantum Vortex Bomb. Watch out for asteroids." },
			{ "ravine", "ravinerevisit", "Getaway under way", "Prepare the escape route through The Arroyo Chasm by destroying the checkpoints with explosives and clearing the way. If possible, also destroy some of the infrastructure." },
			{ "evilbase", "blackhole", "Black hole no more", "Destroy the black hole generator. Make any preparations you can before damaging the tower. Drones will be alerted once the tower has been damaged." },
			{ "evilbase", "shooter", "Full speed ahead - E.F.O. Recycling Plant", "With Tier-1 behind the wheel and you as the tail gunner, you must escape E.F.O. Corp." },
			{ "ravine", "shooter_ravine", "Full speed ahead - The Arroyo Chasm", "With Tier-1 behind the wheel and you as the tail gunner, you must escape E.F.O. Corp." },
			{ "hub", "shooter_hub", "Full speed ahead - The Base", "With Tier-1 behind the wheel and you as the tail gunner, you must escape E.F.O. Corp." },

			{ "hub", "sandbox_hub", "The Base", "The hideout for Tier-1 and their crew, from here they launch the efforts of taking down E.F.O. Corps black hole. Kept tidy by Vac-1 and under supervision by Mainframe, but what happened to the original crew members?" },
			{ "ravine", "sandbox_ravine", "The Arroyo Chasm", "The Arroyo Chasm is a menacing rift cutting through the surface of the planet. It is one of the few places on Vigrid-0421t where power crystals are less rare than elsewhere." },
			{ "spacestation", "sandbox_spacestation", "S.C.C.L", "Subspace Collapse Containment Laser, the S.C.C.L is an orbiting space-station which collects cosmic energy and beams it down to the base on the planet to stabilize the black hole generator." },
			{ "evilbase", "sandbox_evilbase", "E.F.O. Recycling Plant", "Eco-Friendly Operations Corp. is a galactic waste disposal company. In an effort to maximize profits they have constructed a huge base on the planet Vigrid-0421t where they are building an artificial black hole, in which they can dump all their clients' trash." },

			{ "hub", "intro", "Silent in space", "A Löckonaut went missing during a mission to test the new Quilez exo-thruster. After four months in space, the signal from their ship suddenly went silent." },
			{ "hub", "leave", "The Base - Takeoff", "The hideout for Tier-1 and their crew, from here they launch the efforts of taking down E.F.O. Corps black hole. Kept tidy by Vac-1 and under supervision by Mainframe, but what happened to the original crew members?" },
			{ "hub", "return", "The Base - Landing", "The hideout for Tier-1 and their crew, from here they launch the efforts of taking down E.F.O. Corps black hole. Kept tidy by Vac-1 and under supervision by Mainframe, but what happened to the original crew members?" },
			{ "hub_lockelle", "ending", "Missing Löckonaut Mysteriously Returns in Unknown Spacecraft", "This morning, the Löckonaut who vanished six months ago returned to Löckelle - in a completely different spacecraft. 'This is a triumph of the human spirit over adversity,' said George Shackelton, head of the Löckelle Space Agency. Authorities are puzzled by the unfamiliar craft and investigating the Löckonaut's mysterious journey." },

			{ "main", "main", "Main menu", "The Greenwash Gambit DLC - Main menu" },
			{ "credits", "shooter_credits", "Credits", "The Greenwash Gambit DLC - Credits" },
			{ "hub_lockelle", "hub", "Löckelle sweet Löckelle", "A lake side cabin in Löckelle, far from any black hole." },
		};
		int dlc_missions_count = sizeof(dlc_missions) / sizeof(dlc_missions[0]);
		for (int i = 0; i < dlc_missions_count; i++) {
			LevelInfo info = { dlc_missions[i][0], dlc_missions[i][1], dlc_missions[i][2], dlc_missions[i][3] };
			levels.push_back(info);
		}
		for (int i = 0; i < 13; i++) {
			string hub_id = "hub" + to_string(i + 1);
			LevelInfo info = { "hub", hub_id, "The Base v" + to_string(i + 1), "The hideout for Tier-1 and their crew, from here they launch the efforts of taking down E.F.O. Corps black hole. Kept tidy by Vac-1 and under supervision by Mainframe, but what happened to the original crew members?" };
			levels.push_back(info);
		}
		for (int i = 0; i < 4; i++) {
			string hub_id = "hub_space" + to_string(i + 1);
			LevelInfo info = { "hub_space", hub_id, "The Shuttle v" + to_string(i + 1), "Lurking in space at a safe distance from the S.C.C.L." };
			levels.push_back(info);
		}
	} else if (filter == "Relic Hunters") {
		const char* dlc_missions[][4] = {
			{ "map_id", "filename", "title", "description" },
		};
		int dlc_missions_count = sizeof(dlc_missions) / sizeof(dlc_missions[0]);
		for (int i = 0; i < dlc_missions_count; i++) {
			LevelInfo info = { dlc_missions[i][0], dlc_missions[i][1], dlc_missions[i][2], dlc_missions[i][3] };
			levels.push_back(info);
		}
	} else {
		const char* others[][4] = {
			{ "about", "about", "Credits", "" },
			{ "menu", "menu", "Character selection", "" },
			{ "lee", "ending10", "Lee Chemicals Part 1 Ending", "" },
			{ "hub", "ending20", "Hub Part 2 Ending", "" },
			{ "mansion", "ending21", "Villa Gordon Part 2 Ending", "" },
			{ "marina", "ending22", "Marina Part 2 Ending", "" },
			{ "preview", "quicksave", "Quicksave", "Last Saved Level" },
			{ "preview", "quicksavecampaign", "Quicksave Campaign", "Last Saved Campaign Level" },
			{ "lee", "test", "Performance Test", "" },
		};
		int others_count = sizeof(others) / sizeof(others[0]);
		for (int i = 0; i < others_count; i++) {
			LevelInfo info = { others[i][0], others[i][1], others[i][2], others[i][3] };
			levels.push_back(info);
		}
	}
	return levels;
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
