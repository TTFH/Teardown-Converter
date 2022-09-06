#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include <SDL.h>

#include "src/parser.h"

using namespace std;
float progress = 0;

// TODO: use alpha
Rgba operator*(const Rgba& color, float scale) {
	return { color.r * scale, color.g * scale, color.b * scale, 1 };
}

Rgba operator+(const Rgba& color1, const Rgba& color2) {
	return { color1.r + color2.r, color1.g + color2.g, color1.b + color2.b, 1 };
}

Rgba operator-(const Rgba& color1, const Rgba& color2) {
	return { color1.r - color2.r, color1.g - color2.g, color1.b - color2.b, 1 };
}

class Spraycan : public TDBIN {
private:
	FILE* out_file = NULL;
public:
	Spraycan(const char* bin_path) : TDBIN(bin_path) { }
	void hijackTintTable();
	void save(const char* filename);

	void WriteByte(uint8_t byte) {
		fwrite(&byte, sizeof(uint8_t), 1, out_file);
	}
	void WriteInt(uint32_t integer) {
		fwrite(&integer, sizeof(uint32_t), 1, out_file);
	}
	void WriteFloat(float flt) {
		fwrite(&flt, sizeof(float), 1, out_file);
	}
	void WriteString(string str) {
		fwrite(str.c_str(), sizeof(char), str.length() + 1, out_file);
	}
	void WriteTransform(Transform transform) {
		fwrite(&transform, sizeof(Transform), 1, out_file);
	}
	void WriteRgba(Rgba rgba) {
		fwrite(&rgba, sizeof(Rgba), 1, out_file);
	}
	void WriteFire(Fire fire) {
		fwrite(&fire, sizeof(Fire), 1, out_file);
	}
	void WritePalette(Palette p) {
		//fwrite(&p, sizeof(Palette), 1, out_file); // TODO: byte packing
		for (int i = 0; i < 256; i++) {
			WriteByte(p.materials[i].kind);
			WriteRgba(p.materials[i].rgba);
			WriteFloat(p.materials[i].reflectivity);
			WriteFloat(p.materials[i].shinyness);
			WriteFloat(p.materials[i].metalness);
			WriteFloat(p.materials[i].emissive);
			WriteByte(p.materials[i].replacable);
		}
		fwrite(&p.tint_table, sizeof(uint8_t), 2 * 4 * 256, out_file);
		WriteByte(p.z_u8);
	}
};

void Spraycan::save(const char* filename) {
	out_file = fopen(filename, "wb");
	for (int i = 0; i < 5; i++)
		WriteByte(scene.magic[i]);
	for (int i = 0; i < 3; i++)
		WriteByte(scene.version[i]);
	WriteString(scene.level);
	WriteInt(scene.driven_vehicle);
	for (int i = 0; i < 3; i++)
		WriteFloat(scene.shadowVolume[i]);
	WriteTransform(scene.spawnpoint);

	for (int i = 0; i < 4; i++)
		WriteInt(scene.z_u32_4[i]);
	WriteFloat(scene.postpro.brightness);
	WriteRgba(scene.postpro.colorbalance);
	WriteFloat(scene.postpro.saturation);
	WriteFloat(scene.postpro.gamma);
	WriteFloat(scene.postpro.bloom);

	// PLAYER
	WriteTransform(scene.player.transform);
	WriteFloat(scene.player.yaw);
	WriteFloat(scene.player.pitch);
	for (int i = 0; i < 3; i++)
		WriteFloat(scene.player.velocity[i]);
	WriteFloat(scene.player.health);
	for (int i = 0; i < 4; i++)
		WriteFloat(scene.player.z_f32_4[i]);

	// SKYBOX
	Skybox* skybox = &scene.environment.skybox;
	WriteString(skybox->texture);
	WriteRgba(skybox->tint);
	WriteFloat(skybox->brightness);
	WriteFloat(skybox->rot);
	for (int i = 0; i < 3; i++)
		WriteFloat(skybox->sun.tint_brightness[i]);
	WriteRgba(skybox->sun.colorTint);
	for (int i = 0; i < 3; i++)
		WriteFloat(skybox->sun.dir[i]);
	WriteFloat(skybox->sun.brightness);
	WriteFloat(skybox->sun.spread);
	WriteFloat(skybox->sun.length);
	WriteFloat(skybox->sun.fogScale);
	WriteFloat(skybox->sun.glare);
	WriteByte(skybox->z_u8);
	WriteRgba(skybox->constant);
	WriteFloat(skybox->ambient);
	WriteFloat(skybox->ambientexponent);

	for (int i = 0; i < 2; i++)
		WriteFloat(scene.environment.exposure[i]);
	WriteFloat(scene.environment.brightness);

	// FOG
	Fog* fog = &scene.environment.fog;
	WriteRgba(fog->color);
	WriteFloat(fog->start);
	WriteFloat(fog->distance);
	WriteFloat(fog->amount);
	WriteFloat(fog->exponent);

	// WATER
	EnvWater* water = &scene.environment.water;
	WriteFloat(water->wetness);
	WriteFloat(water->puddleamount);
	WriteFloat(water->puddlesize);
	WriteFloat(water->rain);

	WriteByte(scene.environment.nightlight);
	WriteString(scene.environment.ambience.path);
	WriteFloat(scene.environment.ambience.volume);
	WriteFloat(scene.environment.slippery);
	WriteFloat(scene.environment.fogscale);

	// SNOW
	Snow* snow = &scene.environment.snow;
	for (int i = 0; i < 3; i++)
		WriteFloat(snow->dir[i]);
	WriteFloat(snow->spread);
	WriteFloat(snow->amount);
	WriteFloat(snow->speed);
	WriteByte(snow->onground);

	for (int i = 0; i < 3; i++)
		WriteFloat(scene.environment.wind[i]);
	WriteFloat(scene.environment.waterhurt);

	// BOUNDARY
	unsigned int boundary_count = scene.boundary.vertices.getSize();
	WriteInt(boundary_count);
	for (unsigned int i = 0; i < boundary_count; i++) {
		WriteFloat(scene.boundary.vertices[i].pos[0]);
		WriteFloat(scene.boundary.vertices[i].pos[1]);
	}
	WriteFloat(scene.boundary.padleft);
	WriteFloat(scene.boundary.padtop);
	WriteFloat(scene.boundary.padright);
	WriteFloat(scene.boundary.padbottom);

	// FIRE
	unsigned int fire_count = scene.fires.getSize();
	WriteInt(fire_count);
	for (unsigned int i = 0; i < fire_count; i++)
		WriteFire(scene.fires[i]); // COOL!

	// PALETTE
	unsigned int palette_count = scene.palettes.getSize();
	WriteInt(palette_count);
	for (unsigned int i = 0; i < palette_count; i++)
		WritePalette(scene.palettes[i]);

	// TODO: save the rest of the file

	fclose(out_file);
}

// strength in range [0-4], 0: no paint, 4: fully painted
Rgba paint(Rgba spray_color, Rgba voxel_color, uint8_t strength) {
	Rgba diff = (spray_color - voxel_color) * 0.25;
	return voxel_color + diff * strength;
}

void Spraycan::hijackTintTable() {
	Rgba tint = { 0.9, 0.7, 0.1, 1.0 };
	unsigned int palette_count = scene.palettes.getSize();
	for (unsigned int j = 0; j < palette_count; j++) {
		Palette& palette = scene.palettes[j];

		// Generate "random" color
		int k = j % 8;
		float r = (k & 4) >> 2 ? 0.9 : 0.1;
		float g = (k & 2) >> 1 ? 0.9 : 0.1;
		float b = k & 1 ? 0.9 : 0.1;
		tint = { r, g, b, 1.0 };

		for (int i = 0; i < 255; i++) { // Last index not used (?)
			for (int strength = 0; strength < 4; strength++) {
				int tint_index = 4 * 256 + 256 * strength + i + 1; // Index offset by 1 (?)
				assert(tint_index < 2 * 4 * 256);
				int index = palette.tint_table[tint_index];
				Material& original_color = palette.materials[i];
				Material& tinted_color = palette.materials[index];
				if (!tinted_color.hacked) { // Change color only once
					tinted_color.rgba = paint(tint, original_color.rgba, strength + 1);
					tinted_color.hacked = true; // Mark this entry as modified
				}
			}
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("Usage: %s <bin_path>\n", argv[0]);
		return 1;
	}

	Spraycan parser(argv[1]);
	parser.parse();
	parser.hijackTintTable();
	parser.save("quicksave.tdbin");
	printf("Done!\n");

	return 0;
}
