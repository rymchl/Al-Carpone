#pragma once
#include<vector>
#include<glm/glm.hpp>
#include "Upgrade.h"
#include "Building.h"
#include "GraphicsSystem.h"

enum GarageTypes {
	ENGINE_GARAGE = 0,
	HANDLING_GARAGE,
	ROBBING_GARAGE
};

class Garage : public Building {
public:
	unsigned int type;
	std::vector<Upgrade> upgradeList;
	physx::PxVec3 position;	// Position currently hardcoded

	float depth = 21.6f;	 //y axis
	float height = 4.f; //z axis
	float width = 21.6;	 //x axis

	Garage(unsigned int upgradeType, PxVec3 pos, PxVec3 dims) { 
		type = upgradeType;
		position = pos;
		createTrigger(dims); 
	}

	void createTrigger(PxVec3 dimensions); // Must only be called once!
	void drawGarageMenu(GraphicsSystem& graphics, State& state, Player& player);
	void triggerFunction(Player& player, State& state); // ENTER GARAGE
};