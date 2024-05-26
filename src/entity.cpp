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
	switch (type) {
		case KindBody:
			delete static_cast<Body*>(self);
			break;
		case KindShape:
			delete static_cast<Shape*>(self);
			break;
		case KindLight:
			delete static_cast<Light*>(self);
			break;
		case KindLocation:
			delete static_cast<Location*>(self);
			break;
		case KindWater:
			delete static_cast<Water*>(self);
			break;
		case KindJoint:
			delete static_cast<Joint*>(self);
			break;
		case KindVehicle:
			delete static_cast<Vehicle*>(self);
			break;
		case KindWheel:
			delete static_cast<Wheel*>(self);
			break;
		case KindScreen:
			delete static_cast<Screen*>(self);
			break;
		case KindTrigger:
			delete static_cast<Trigger*>(self);
			break;
		case KindScript:
			delete static_cast<Script*>(self);
			break;
	}
}

Joint::~Joint() {
	if (type == _Rope && rope != NULL)
		delete rope;
}

Script::~Script() {
	for (LuaTable::iterator it = variables->begin(); it != variables->end(); it++)
		delete *it;
	delete variables;
}
