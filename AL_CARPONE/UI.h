#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "Model.h"
#include "State.h"
#include "Player.h"
#include "GraphicsSystem.h"

// Make sure to create this after graphics is initialized!!!
class UI {

public:
	// POPUPS
	Model* press_f_to_rob;
	Model* press_f_to_exit;
	Model* press_f_to_enter_corner_store;
	Model* Item;

	// UI ELEMENTS
	Model* minimap;
	Model* player_marker;

	UI();
	
	void update(State* state, Player* player, GraphicsSystem* graphics);

private:

	glm::mat4 player_movement = glm::mat4(1.f);

	void drawPopups(State* state, GraphicsSystem* graphics);
	glm::mat4 updateMarkerPos(glm::vec3 original_pos);
	glm::vec3 calculateOnMapPos(glm::vec3 pos);
};
