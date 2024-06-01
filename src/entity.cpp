#include "entity.h"

const char* EntityName[] = {
	"",
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
		case EntityType::Body:
			delete static_cast<Body*>(self);
			break;
		case EntityType::Shape:
			delete static_cast<Shape*>(self);
			break;
		case EntityType::Light:
			delete static_cast<Light*>(self);
			break;
		case EntityType::Location:
			delete static_cast<Location*>(self);
			break;
		case EntityType::Water:
			delete static_cast<Water*>(self);
			break;
		case EntityType::Joint:
			delete static_cast<Joint*>(self);
			break;
		case EntityType::Vehicle:
			delete static_cast<Vehicle*>(self);
			break;
		case EntityType::Wheel:
			delete static_cast<Wheel*>(self);
			break;
		case EntityType::Screen:
			delete static_cast<Screen*>(self);
			break;
		case EntityType::Trigger:
			delete static_cast<Trigger*>(self);
			break;
		case EntityType::Script:
			delete static_cast<Script*>(self);
			break;
	}
}

Joint::~Joint() {
	if (type == JointType::Rope && rope != NULL)
		delete rope;
}

Script::~Script() {
	for (LuaTable::iterator it = variables->begin(); it != variables->end(); it++)
		delete *it;
	delete variables;
}
