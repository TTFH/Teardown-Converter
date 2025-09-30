#include <math.h>
#include <string.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <stdexcept>

#include "scene.h"
#include "misc_utils.h"
#include "vox_writer.h"
#include "zlib_utils.h"

static const char* MaterialPrefix = "$TD_";

static const char* td_notes[32] = {
	"snow/hole",
	"reserved",
	"unphysical",
	"unphysical",
	"reserved",
	"reserved",
	"reserved",
	"reserved",
	"ice",
	"hardmasonry",
	"hardmetal",
	"plastic",
	"plastic",
	"heavymetal",
	"heavymetal",
	"metal",
	"metal",
	"plaster",
	"plaster",
	"masonry",
	"masonry",
	"masonry",
	"masonry",
	"wood",
	"wood",
	"rock",
	"rock",
	"dirt",
	"dirt",
	"foliage",
	"foliage",
	"glass"
};

bool MV_Shape::operator==(const MV_Shape& other) const {
	if (voxels.sizex != other.voxels.sizex || voxels.sizey != other.voxels.sizey || voxels.sizez != other.voxels.sizez)
		return false;
	for (int x = 0; x < voxels.sizex; x++)
		for (int y = 0; y < voxels.sizey; y++)
			for (int z = 0; z < voxels.sizez; z++)
				if (voxels.Get(x, y, z) != other.voxels.Get(x, y, z))
					return false;
	return true;
}

MV_FILE::MV_FILE(string filename, bool write_imap) {
	this->filename = filename;
	this->write_imap = write_imap;
	vox_file = nullptr;

	for (int i = 0; i < 256; i++) {
		palette[i] = { 75, 75, 75, 255};
		material[i] = { Material::None, DIFFUSE, {} };
		is_index_used[i] = false;
		palette_map[i] = i;
	}
	palette[0] = { 0, 0, 0, 255 };
	for (int i = 0; i < ROWS; i++)
		notes[i] = td_notes[i];
}

void MV_FILE::WriteInt(int val) {
	fwrite(&val, sizeof(int), 1, vox_file);
}

void MV_FILE::WriteDICT(DICT dict) {
	WriteInt(dict.size());
	for (DICT::iterator it = dict.begin(); it != dict.end(); it++) {
		WriteInt(it->first.length());
		fwrite(it->first.c_str(), it->first.length(), sizeof(char), vox_file);
		WriteInt(it->second.length());
		fwrite(it->second.c_str(), it->second.length(), sizeof(char), vox_file);
	}
}

void MV_FILE::WriteChunkHeader(int id, int content_size, int children_size) {
	WriteInt(id);
	WriteInt(content_size);
	WriteInt(children_size);
}

void MV_FILE::WriteFileHeader() {
	WriteInt(VOX);
	WriteInt(VERSION);
	WriteChunkHeader(MAIN, 0, 0);
	children_size_ptr = ftell(vox_file) - sizeof(int);
}

void MV_FILE::WriteSIZE(const MV_Shape& shape) {
	WriteChunkHeader(SIZE, 12, 0);
	WriteInt(shape.voxels.sizex);
	WriteInt(shape.voxels.sizey);
	WriteInt(shape.voxels.sizez);
}

void MV_FILE::WriteXYZI(const MV_Shape& shape) {
	int voxel_count = shape.voxels.GetNonZeroCount();
	WriteChunkHeader(XYZI, 4 * (1 + voxel_count), 0);
	WriteInt(voxel_count);

	for (int x = 0; x < shape.voxels.sizex; x++) {
		for (int y = 0; y < shape.voxels.sizey; y++) {
			for (int z = 0; z < shape.voxels.sizez; z++) {
				uint8_t index = shape.voxels.Get(x, y, z);
				if (index != 0) {
					MV_Voxel voxel = { (uint8_t)x, (uint8_t)y, (uint8_t)z, index };
					fwrite(&voxel, sizeof(MV_Voxel), 1, vox_file);
				}
			}
		}
	}
}

void MV_FILE::WriteTDCZ(const MV_Shape& shape) {
	const uint8_t* voxel_array = shape.voxels.ToArray();
	vector<uint8_t> compressed_data;
	if (ZlibBlockCompress(voxel_array, shape.voxels.GetVolume(), 9, compressed_data)) {
		WriteChunkHeader(TDCZ, 3 * sizeof(int) + compressed_data.size(), 0);
		WriteInt(shape.voxels.sizex);
		WriteInt(shape.voxels.sizey);
		WriteInt(shape.voxels.sizez);
		fwrite(compressed_data.data(), sizeof(uint8_t), compressed_data.size(), vox_file);
	} else
		printf("[Warning] Failed to compress shape %s\n", shape.name.c_str());
	delete[] voxel_array;
}

void MV_FILE::Write_nSHP(int i) {
	WriteChunkHeader(nSHP, 20, 0);
	WriteInt(2 * i + 1); // node_id
	WriteInt(0); // node_attr
	WriteInt(1); // num_models
	WriteInt(i - 1); // ref_model_id
	WriteInt(0); // Empty DICT (nodeAttribs)
}

void MV_FILE::Write_nTRN(int i, string pos) {
	WriteChunkHeader(nTRN, 38 + pos.length() + 13 + models[i - 1].name.length(), 0);
	WriteInt(2 * i); // node_id

	DICT node_attr;
	node_attr["_name"] = models[i - 1].name;
	WriteDICT(node_attr);

	WriteInt(2 * i + 1); // child_node_id
	WriteInt(-1); // reserved_id
	WriteInt(0); // layer_id
	WriteInt(1); // num_frames

	DICT frame_attr;
	frame_attr["_t"] = pos;
	WriteDICT(frame_attr);

	Write_nSHP(i);
}

void MV_FILE::Write_nGRP() {
	int num_models = models.size();
	WriteChunkHeader(nGRP, 4 * (3 + num_models), 0);
	WriteInt(1); // node_id
	WriteInt(0); // node_attr
	WriteInt(num_models); // node_childrens
	for (int i = num_models; i > 0; i--)
		WriteInt(2 * i); // child_node

	for (int i = 1; i <= num_models; i++) {
		string pos = to_string(models[i - 1].pos_x) + " " + to_string(models[i - 1].pos_y) + " " + to_string(models[i - 1].pos_z);
		Write_nTRN(i, pos);
	}
}

void MV_FILE::WriteRGBA() {
	WriteChunkHeader(RGBA, 1024, 0);
	fwrite(&palette[1], sizeof(MV_Color), 255, vox_file);
	fwrite(&palette[0], sizeof(MV_Color), 1, vox_file);
}

void MV_FILE::WriteIMAP() {
	if (!write_imap) return;
	try {
		FIX_PALETTE_MAPPING();
	} catch (logic_error& e) {
		printf("[ERROR] Could not fix palette mapping for file %s: %s\n", filename.c_str(), e.what());
	}
	int i = 0;
	bool is_mapped = false;
	while (i < 256 && !is_mapped) {
		if (palette_map[i] != i)
			is_mapped = true;
		i++;
	}
	if (!is_mapped) return;

	WriteChunkHeader(IMAP, 256, 0);
	fwrite(&palette_map[1], sizeof(uint8_t), 255, vox_file);
	fwrite(&palette_map[0], sizeof(uint8_t), 1, vox_file);
}

void MV_FILE::WriteMATL(uint8_t index, const MV_Material& mat) {
	DICT material_attr;
	if (mat.type == METAL) {
		material_attr["_type"] = "_metal";
		material_attr["_rough"] = FloatToString(mat.properties.metal.roughness);
		material_attr["_sp"] = FloatToString(mat.properties.metal.specular);
		material_attr["_metal"] = FloatToString(mat.properties.metal.metallic);
	} else if (mat.type == GLASS) {
		material_attr["_type"] = "_glass";
		material_attr["_rough"] = FloatToString(mat.properties.glass.roughness);
		material_attr["_alpha"] = "0.5";
	} else if (mat.type == EMIT) {
		material_attr["_type"] = "_emit";
		material_attr["_emit"] = FloatToString(mat.properties.emit.emission);
		material_attr["_flux"] = to_string(mat.properties.emit.power);
	} else return;
	int matl_size = 8 + 8 * material_attr.size();
	for (DICT::iterator it = material_attr.begin(); it != material_attr.end(); it++)
		matl_size += it->first.length() + it->second.length();

	WriteChunkHeader(MATL, matl_size, 0);
	WriteInt(index);
	WriteDICT(material_attr);
}

void MV_FILE::WriteNOTE() {
	int note_chunk_size = 4 + 4 * ROWS;
	for (int i = 0; i < ROWS; i++)
		note_chunk_size += notes[i].length();

	WriteChunkHeader(NOTE, note_chunk_size, 0);
	WriteInt(ROWS);
	for (int i = 0; i < ROWS; i++) {
		WriteInt(notes[i].length());
		fwrite(notes[i].c_str(), sizeof(char), notes[i].length(), vox_file);
	}
}

void MV_FILE::SaveModel(bool compress) {
	vox_file = fopen(filename.c_str(), "wb+");
	if (vox_file == nullptr) {
		printf("[ERROR] Could not open %s for writing\n", filename.c_str());
		return;
	}

	WriteFileHeader();
	for (unsigned int i = 0; i < models.size(); i++) {
		WriteSIZE(models[i]);
		if (compress)
			WriteTDCZ(models[i]);
		else
			WriteXYZI(models[i]);
	}

	WriteChunkHeader(nTRN, 28, 0);
	WriteInt(0); // node_id
	WriteInt(0); // Empty DICT (nodeAttribs)
	WriteInt(1); // child_node_id
	WriteInt(-1); // reserved_id
	WriteInt(-1); // layer_id
	WriteInt(1); // num_frames
	WriteInt(0); // Empty DICT (frames)
	Write_nGRP();

	WriteRGBA();
	WriteIMAP();
	for (int i = 0; i < 256; i++)
		if (is_index_used[i])
			WriteMATL(i, material[i]);
	WriteNOTE();

	// Update childrenSize in the MAIN chunk
	long int size = ftell(vox_file);
	fseek(vox_file, children_size_ptr, SEEK_SET);
	WriteInt(size - children_size_ptr - sizeof(int)); // Minus 4 bytes of childrenSize
	fclose(vox_file);
}

void MV_FILE::AddShape(const MV_Shape& shape) {
	models.push_back(shape);
}

bool MV_FILE::GetShapeName(const MV_Shape& shape, string& name) const {
	bool found = false;
	for (vector<MV_Shape>::const_iterator it = models.begin(); it != models.end() && !found; it++)
		if (*it == shape) {
			found = true;
			name = it->name;
		}
	return found;
}

void MV_FILE::SetEntry(uint8_t index, const MV_Color& color, MV_Material mat) {
	if (index == 0 || is_index_used[index]) return;
	palette[index] = color;
	material[index] = mat;
	is_index_used[index] = true;
}

string MV_FILE::GetIndexNote(int index) {
	if (index == 0) index = 256;
	int row = (ROWS - 1) - (index - 1) / 8;
	if (row == 0) return "none";
	return notes[row];
}

void MV_FILE::FIX_PALETTE_MAPPING() {
	bool fixed[256];
	bool occupied[256];
	uint8_t reverse_map[256];
	for (int i = 0; i < 256; i++) {
		fixed[i] = false;
		occupied[i] = false;
		reverse_map[i] = i;
	}

	// Mark materials already in the correct row
	for (int i = 0; i < 256; i++) {
		if (is_index_used[i] && GetIndexNote(i) == MaterialName[material[i].td_type]) {
			fixed[i] = true;
			occupied[i] = true;
		}
	}
	fixed[0] = true; // empty
	fixed[254] = true; // snow
	fixed[255] = true; // hole
	occupied[0] = true;
	occupied[254] = true;
	occupied[255] = true;

	auto swap_mapping = [this, &reverse_map](int i, int j) {
		int index_i = reverse_map[i];
		int index_j = j; //reverse_map[j];

		uint8_t palette_map_i = palette_map[index_i];
		uint8_t palette_map_j = palette_map[index_j];
		palette_map[index_i] = palette_map_j;
		palette_map[index_j] = palette_map_i;

		reverse_map[palette_map_i] = index_j;
		reverse_map[palette_map_j] = index_i;
	};

	// Move materials to an empty space in a correct row
	for (int i = 0; i < 256; i++) {
		if (is_index_used[i] && !fixed[i]) {
			string mat_name = MaterialName[material[i].td_type];
			for (int j = 0; j < 256; j++) {
				if (!occupied[j] && GetIndexNote(j) == mat_name) {
					fixed[i] = true;
					occupied[j] = true;
					swap_mapping(i, j);
					break;
				}
			}
		}
	}

	// Move materials to a renamed empty row
	for (int i = 0; i < 256; i++) {
		if (is_index_used[i] && !fixed[i]) {
			string mat_name = MaterialName[material[i].td_type];
			// Check if row is empty
			for (int j = 0; j < ROWS; j++) {
				bool row_empty = true;
				for (int k = 0; k < 8; k++) {
					int idx = 8 * j + k + 1;
					if (idx < 256 && occupied[idx]) {
						row_empty = false;
						break;
					}
				}
				if (row_empty) {
					int note_idx = ROWS - 1 - j;
					int starting_index = 8 * j + 1;
					fixed[i] = true;
					occupied[starting_index] = true;
					swap_mapping(i, starting_index);
					notes[note_idx] = MaterialPrefix + mat_name;

					// Move materials of the same type to this new row
					for (int l = 0; l < 256; l++) {
						if (is_index_used[l] && !fixed[l] && material[l].td_type == material[i].td_type) {
							for (int m = starting_index; m < starting_index + 8; m++) {
								if (m < 256 && !occupied[m]) {
									fixed[l] = true;
									occupied[m] = true;
									swap_mapping(l, m);
									// Pigeon paradise!
									// (because of the nesting, get it?)
									break;
								}
							}
						}
					}
					break;
				}
			}
		}
	}

	// Check valid permutation
	bool mapped[256];
	for (int i = 0; i < 256; i++)
		mapped[i] = false;
	for (int i = 0; i < 256; i++)
		mapped[palette_map[i]] = true;
	for (int i = 0; i < 256; i++)
		if (!mapped[i])
			throw logic_error("Index " + to_string(i) + " not mapped");

	// Check correct mapping
	for (int i = 1; i < 254; i++) { // Last two indices are correct
		if (is_index_used[i]) {
			int j = reverse_map[i];
			string note = GetIndexNote(j);
			if (note.find(MaterialPrefix) == 0)
				note = note.substr(strlen(MaterialPrefix));
			string mat_name = MaterialName[material[i].td_type];
			if (note != mat_name)
				throw logic_error("Index " + to_string(i) + " mapped to " + to_string(j)
				+ " with incorrect row " + note + " for material " + mat_name);
		}
	}
}
