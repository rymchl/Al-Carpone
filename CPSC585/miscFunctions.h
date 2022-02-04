//Used for debugging
void printMat4(glm::mat4 m) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%.2f ", m[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

//Updates the fps/ms in the window title.
void updateTitle(GLFWwindow* window)
{
	std::string FPS = std::to_string((int)ceil(1. / state.timeStep * state.frameCount));
	std::string RT = std::to_string((state.timeStep / state.frameCount) * 1000);
	std::string title = "Al Carpone / " + FPS + "FPS / " + RT + "ms / PLAYER CASH: $" + std::to_string(player.getCash());
	glfwSetWindowTitle(window, title.c_str());

}
//Checks for special inputs that would alter the state, and updates state accordingly
void checkSpecialInputs(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		state.terminateProgram = true;
		return;
	}
	
	// Handles key inputs
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		if (!state.Q_isHeld) {
			state.toggleCameraMode();
		}
		state.Q_isHeld = true;
	}
	else state.Q_isHeld = false;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (state.cameraMode == CAMERA_MODE_BOUND) state.cameraMode = CAMERA_MODE_BOUND_FREELOOK;
	}
	else if (state.cameraMode == CAMERA_MODE_BOUND_FREELOOK) state.cameraMode = CAMERA_MODE_BOUND;
}