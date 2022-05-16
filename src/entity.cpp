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
			delete (Body*)kind;
			break;
		case KindShape:
			delete (Shape*)kind;
			break;
		case KindLight:
			delete (Light*)kind;
			break;
		case KindLocation:
			delete (Location*)kind;
			break;
		case KindWater:
			delete (Water*)kind;
			break;
		case KindJoint:
			delete (Joint*)kind;
			break;
		case KindVehicle:
			delete (Vehicle*)kind;
			break;
		case KindWheel:
			delete (Wheel*)kind;
			break;
		case KindScreen:
			delete (Screen*)kind;
			break;
		case KindTrigger:
			delete (Trigger*)kind;
			break;
		case KindScript:
			delete (Script*)kind;
			break;
	}
}
