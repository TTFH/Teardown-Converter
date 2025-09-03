#ifndef LEVELS_H
#define LEVELS_H

#include <string>
#include <vector>

using namespace std;

struct LevelInfo {
	string level_id;
	string filename;
	string title;
	string description;
};

vector<LevelInfo> LoadLevels(string filter);
void SaveInfoTxt(string map_folder, string level_name, string level_desc);

#endif
