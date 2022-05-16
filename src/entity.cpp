#include "entity.h"

const char* EntityKindName[] = {
	"", // EntityKind enum starts at 1
	"Body",
	"Shape",
	"Light",
	"Location",
	"Water",
	"Joint",
	"Vehicle",
	"Wheel",
	"Screen",
	"Trigger",
	"Script"
};

Entity::~Entity() {
	for (unsigned int i = 0; i < children.getSize(); i++)
		delete children[i];
	switch (kind_byte) {
		case KindBody:
			delete static_cast<Body*>(kind);
			break;
		case KindShape:
			delete static_cast<Shape*>(kind);
			break;
		case KindLight:
			delete static_cast<Light*>(kind);
			break;
		case KindLocation:
			delete static_cast<Location*>(kind);
			break;
		case KindWater:
			delete static_cast<Water*>(kind);
			break;
		case KindJoint:
			delete static_cast<Joint*>(kind);
			break;
		case KindVehicle:
			delete static_cast<Vehicle*>(kind);
			break;
		case KindWheel:
			delete static_cast<Wheel*>(kind);
			break;
		case KindScreen:
			delete static_cast<Screen*>(kind);
			break;
		case KindTrigger:
			delete static_cast<Trigger*>(kind);
			break;
		case KindScript:
			delete static_cast<Script*>(kind);
			break;
	}
}
