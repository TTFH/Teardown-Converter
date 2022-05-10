#include <stdint.h>

#include "scene.h"

bool operator==(uint8_t lhs, const MaterialKind& rhs) {
	return lhs == static_cast<uint8_t>(rhs);
}

const char* MaterialKindName[] = {
	"none",
	"glass",
	"wood",
	"masonry",
	"plaster",
	"metal",
	"heavymetal",
	"rock",
	"dirt",
	"foliage",
	"plastic",
	"hardmetal",
	"hardmasonry",
	"ice",
	"unphysical",
	"invalid",
};
