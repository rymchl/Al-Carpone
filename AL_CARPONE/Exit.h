#pragma once

#include "Building.h"

class Exit : public Building {
	physx::PxVec3 tpos = physx::PxVec3(99, 20, -720); // Position currently hardcoded

	Model press_f_to_exit;

public:

	Exit() { 
		createTrigger();
	}

	void createTrigger(); // Must only be called once!
	void triggerFunction(Player& player, State& state); 
};