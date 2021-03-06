#pragma once


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "State.h"
#include "Player.h"
#include "AudioSystem.h"

// TODO make functions for each input (so gamepad and keyboard are the same)
//Checks for special inputs that would alter the state, and updates state accordingly
void checkSpecialInputs(GraphicsSystem* graphics, State& state, Player& player, AudioSystem* audio)
{
	GLFWwindow* window = graphics->window;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		if (!state.escape_isHeld) {
			state.gamestate = GAMESTATE::GAMESTATE_MAIN_MENU;
		}
		state.escape_isHeld = true;
		return;
	}
	else if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE){
		state.escape_isHeld = false;
	}

	// Debug Mode
	if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) {
		if (!state.f5_isHeld) {
			state.debugMode = !state.debugMode;
		}
		state.f5_isHeld = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_RELEASE) state.f5_isHeld = false;

	// Freemode Camera
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && debugmode != DEBUGMODE::FALSE)
	{
		if (!state.Q_isHeld) {
			state.toggleCameraMode();
		}
		state.Q_isHeld = true;
	}
	else  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) state.Q_isHeld = false;

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && debugmode != DEBUGMODE::FALSE)
	{
		if (!state.R_isHeld) {
			state.resetVehicles();
		}
		state.R_isHeld = true;
	}
	else  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) state.R_isHeld = false;

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
		if (!state.M_isHeld) {
			audio->toggleMusic();
		}
		state.M_isHeld = true;
	}
	else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) state.M_isHeld = false;

	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS){
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {

			if (!state.alt_isHeld || !state.enter_isHeld) {
				graphics->toggleFullscreen();
			}

			state.alt_isHeld = true;
			state.enter_isHeld = true;
		}
		else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) state.enter_isHeld = false;
	}
	else  if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE) state.alt_isHeld = false;


	//controller input
	if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
	{
		//get controller name
		const char* controller_name = glfwGetGamepadName(GLFW_JOYSTICK_1);
		//std::cout << controller_name << std::endl;

		GLFWgamepadstate controller_state;
		if (glfwGetGamepadState(GLFW_JOYSTICK_1, &controller_state))
		{
			if (controller_state.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS)
			{
				//std::cout << "start (PS4 options, XBOX menu, nspro +) " << std::endl;	//ps4 - options
				if (!state.start_isHeld) 
				{
					state.gamestate = GAMESTATE::GAMESTATE_MAIN_MENU;
				}
				state.start_isHeld = true;
				return;
			}
			else if (controller_state.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_RELEASE)
			{
				state.start_isHeld = false;
			}

			if (controller_state.buttons[GLFW_GAMEPAD_BUTTON_TRIANGLE] == GLFW_PRESS && debugmode != DEBUGMODE::FALSE)
			{
				//std::cout << "TRIANGLE (xbox y, ns pro x)" << std::endl;
				if (!state.triangle_isHeld)
				{
					state.resetVehicles();
				}
				state.R_isHeld = true;
			}
			else if (controller_state.buttons[GLFW_GAMEPAD_BUTTON_TRIANGLE] == GLFW_RELEASE) state.triangle_isHeld = false;

			
		}
	}
}





