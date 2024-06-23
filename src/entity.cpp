#include "entity.h"

const char* EntityName[] = {
	"Unknown",
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
	"Script",
	"Animator"
};

static void DeleteEntity(uint8_t type, void* entity) {
	switch (type) {
		case Entity::Body:
			delete static_cast<Body*>(entity);
			break;
		case Entity::Shape:
			delete static_cast<Shape*>(entity);
			break;
		case Entity::Light:
			delete static_cast<Light*>(entity);
			break;
		case Entity::Location:
			delete static_cast<Location*>(entity);
			break;
		case Entity::Water:
			delete static_cast<Water*>(entity);
			break;
		case Entity::Joint:
			delete static_cast<Joint*>(entity);
			break;
		case Entity::Vehicle:
			delete static_cast<Vehicle*>(entity);
			break;
		case Entity::Wheel:
			delete static_cast<Wheel*>(entity);
			break;
		case Entity::Screen:
			delete static_cast<Screen*>(entity);
			break;
		case Entity::Trigger:
			delete static_cast<Trigger*>(entity);
			break;
		case Entity::Script:
			delete static_cast<Script*>(entity);
			break;
		case Entity::Animator:
			delete static_cast<Animator*>(entity);
			break;
	}
}

Entity::~Entity() {
	for (unsigned int i = 0; i < children.getSize(); i++)
		delete children[i];
	DeleteEntity(type, self);
}

Joint::~Joint() {
	delete rope;
}

Script::~Script() {
	for (LuaTable::iterator it = variables->begin(); it != variables->end(); it++)
		delete *it;
	delete variables;
}
