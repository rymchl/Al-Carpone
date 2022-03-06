 //Initializes the physx system and all global variables and calls other includes.
#include"init.h"

namespace sv = snippetvehicle;

extern void renderAll(Camera*, GraphicsSystem*, MainMenu*, Player*, UI*, State*, PoliceCar*);
extern void despawnEnemy(Vehicle*);

int main()
{
	// TODO clean up
	State state;
	Player player;
	Bank bank;

	cout << "Initializing Graphics..." << endl;

	GraphicsSystem graphics; //Must be called first ALWAYS

	// Initialize Windows
	cout << "	Debug Panel..." << endl;
	DebugPanel debugPanel(graphics.window);
	cout << "	Main Menu..." << endl;
	MainMenu mainMenu;
	cout << "	Pause Menu..." << endl;
	PauseMenu pauseMenu;
	cout << "	UI..." << endl;
	UI ui;
	 
	cout << "Initalizing Physics..." << endl;

	//Set up physx with vehicle snippet:
	//Make sure this is called after the shader program is generated
	PhysicsSystem physics(state, player, bank);

	cout << "Initalizing Audio..." << endl;

	AudioSystem audio;

	//TODO Cleanup
	//Setup main player vehicle
	player = Player(0);
	//Add it to the list of active vehicles
	state.activeVehicles.push_back(&player);

	bank.createActors();


	// Initialize Models
	player.createModel(); //TODO: If player is moved here as well, we can create model in constructors instead.
	bank.createModel();
	PoliceCar police_car1;
	PoliceCar police_car2;
	PoliceCar police_car3;
	PoliceCar police_car4;
	

	graphics.enableDepthBuffer();


	// Active camera can be one of [bound/free] at a given time
	BoundCamera boundCamera(player, state);	// Locked in a sphere around the car
	FreeCamera freeCamera(player);		// Move and look freely anywhere (for debugging)
	Camera* activeCamera = &boundCamera;

	//////////////////////////////////////////////////////////////////////////
	// MAIN LOOP
	/////////////////////////////////////////////////////////////////////////
	
	while (!glfwWindowShouldClose(graphics.window) && !state.terminateProgram)
	{
		// Update the time and fps counter.
		state.updateTime();
		if (state.timeSinceLastFpsUpdate >= 1.0f/30.0f) {
			graphics.updateTitle(state, player);
			state.prevTime = state.currTime;
			state.timeSinceLastFpsUpdate = 0;
		}

		
		glfwPollEvents();			// Take care of all GLFW events
		graphics.clearBuffer();
		audio.updateAudio(&player, &state); // Update all audio elements

		if (state.gameWon) {
			mainMenu.drawWinScreen(graphics);
		}

		///////////////////////////////////////////////////////////////
		//MAIN MENU
		///////////////////////////////////////////////////////////////
		else if (state.gamestate == GAMESTATE_MAIN_MENU) {
			//Draw the menu
			mainMenu.drawMenu(graphics, state);
			
			// Despawn any additional active vehicles (enemies)
			while (state.activeVehicles.size() > 1) {
				despawnEnemy(state.activeVehicles.back());
				state.activeVehicles.pop_back();
			}
		
			
			// If exiting the main menu
			if (state.gamestate == GAMESTATE_INGAME) {

				graphics.clearBuffer();
				mainMenu.drawLoadingScreen(graphics);
				graphics.swapBuffers();

				// Setup level
				mainMenu.changeLevel(state.selectedLevel);
				
				if (state.selectedLevel == LEVELS::LEVEL_MAIN) {
					
					PxTransform T(PxVec3(250.f, 2.0f, -90.f));
					T.q = PxQuat(0.000010, 0.999808, 0.000475, -0.019572);
					player.setResetPoint(T);

					police_car1 = PoliceCar(1);
					police_car1.moveStartPoint(PxVec3(559.949, 31.3, -360.091));
					police_car1.createModel();
					state.activeVehicles.push_back(&police_car1);

					police_car2 = PoliceCar(2);
					police_car2.moveStartPoint(PxVec3(419.948730, 21.455765, -60.534622));
					police_car2.createModel();
					state.activeVehicles.push_back(&police_car2);
					
					police_car3 = PoliceCar(3);
					police_car3.moveStartPoint(PxVec3(100.000031, 0.299998, -220.079498));
					police_car3.createModel();
					state.activeVehicles.push_back(&police_car3);
					
					police_car4 = PoliceCar(4);
					police_car4.moveStartPoint(PxVec3(-99.999969, 0.299998, -220.079498));
					police_car4.createModel();
					state.activeVehicles.push_back(&police_car4);
				}
				else {
					player.setResetPoint(PxTransform(PxVec3(0,0,0)));
				}

				//Remove the old level pointer and add the new
				PxFilterData groundPlaneSimFilterData(sv::COLLISION_FLAG_GROUND, sv::COLLISION_FLAG_GROUND_AGAINST, 0, 0);
				gGroundPlane = physics.createDrivablePlane(groundPlaneSimFilterData, gMaterial, gPhysics, gCooking, state.selectedLevel);
				
				gScene->removeActor(*activeLevelActorPtr);
				
				gScene->addActor(*gGroundPlane);

				PxU32 size = gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC) * sizeof(PxActor*);
				PxActor** actors = (PxActor**)malloc(size);
				gScene->getActors(PxActorTypeFlag::eRIGID_STATIC, actors, size, 0);
				activeLevelActorPtr = actors[gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC) - 1];

				if (state.selectedLevel == LEVEL_TUNING) {
					//spawn police car on tuning level
					police_car1 = PoliceCar(1);
					police_car1.createModel();
					state.activeVehicles.push_back(&police_car1);
				}
				
				//Reset active vehicles
				for (Vehicle* v : state.activeVehicles) {
					v->reset();
				}
			}
			
		}
		///////////////////////////////////////////////////////////////
		//PAUSE MENU
		///////////////////////////////////////////////////////////////
		else if (state.gamestate == GAMESTATE_PAUSE_MENU) {
			pauseMenu.handleInputs(graphics.window, state);
			pauseMenu.drawPauseMenu(graphics, state);

			renderAll(activeCamera, &graphics, &mainMenu, &player, &ui, &state, &police_car1);
		}
		///////////////////////////////////////////////////////////////
		//INGAME
		///////////////////////////////////////////////////////////////
		else {	
			//Simulate physics through the timestep
			physics.step(graphics.window);

			//Update the players physics variables (velocity, acceleration, jerk)
			//Dont need to check other vehicles (yet?)
			player.updatePhysicsVariables(state.timeStep);

			//Check for special inputs (currently only camera mode change)
			checkSpecialInputs(graphics.window, state, player, &audio);

			if (state.cameraMode == CAMERA_MODE_BOUND) activeCamera = &boundCamera;
			else if (state.cameraMode == CAMERA_MODE_UNBOUND_FREELOOK) activeCamera = &freeCamera;

			// Camera is disabled in DEBUG MODE
			if (!state.debugMode) activeCamera->handleInput(graphics.window, state);
			if (activeCamera == &boundCamera) boundCamera.checkClipping(graphics.window);

			renderAll(activeCamera, &graphics, &mainMenu, &player, &ui,  &state, &police_car1);

			// DEBUG MODE
			if (state.debugMode) { // Camera is deactivated
				debugPanel.draw();
			}
		}
		graphics.swapBuffers();
	}

	debugPanel.cleanUp();
	graphics.cleanup();
	physics.cleanup();

	return EXIT_SUCCESS;
}




void renderAll(Camera* activeCamera, GraphicsSystem* graphics, MainMenu* mainMenu, Player* player, UI* ui, State* state, PoliceCar* police_car) {

	glm::mat4 projection = glm::perspective(glm::radians(activeCamera->zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);
	glm::mat4 view = glm::mat4(glm::mat3(activeCamera->GetViewMatrix())); // remove translation from the view matrix
	graphics->skybox->Draw(projection, view);
	view = activeCamera->GetViewMatrix();

	ui->update(state, player, graphics);

	graphics->shader3D->use();
	// send them to shader
	graphics->shader3D->setMat4("projection", projection);
	graphics->shader3D->setMat4("view", view);

	graphics->shader3D->setInt("numLights", mainMenu->light_positions->size());

	for (int i = 0; i < mainMenu->light_positions->size(); i++) {
		std::string path = "light_positions[" + std::to_string(i) + "]";
		graphics->shader3D->setVec3(path.c_str(), (*mainMenu->light_positions)[i]);
	}

	// render the loaded model

	glm::mat4 model = glm::mat4(1.0f);
	graphics->shader3D->setMat4("model", model);
	graphics->shader3D->setVec3("camPos", glm::vec3(activeCamera->pos.x, activeCamera->pos.y, activeCamera->pos.z));
	graphics->shader3D->setInt("shaderMode", SHADER_MODE_DIFFUSE);
	mainMenu->active_level->Draw(*graphics->shader3D);

	// Render dynamic physx shapes


	{
		const int MAX_NUM_ACTOR_SHAPES = 128;
		PxShape* shapes[MAX_NUM_ACTOR_SHAPES];

		// Loop over each actor in the scene
		for (PxU32 i = 0; i < static_cast<PxU32>(physx_actors.size()); i++)
		{
			// Fetch the number of shapes that make up the actor
			const PxU32 nbShapes = physx_actors[i].actorPtr->getNbShapes();
			PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
			physx_actors[i].actorPtr->getShapes(shapes, nbShapes);

			for (PxU32 j = 0; j < nbShapes; j++)
			{
				// Get the geometry of the shape
				const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *physx_actors[i].actorPtr));
				const PxGeometryHolder h = shapes[j]->getGeometry();

				// Generate a mat4 out of the shape position so can send it to the vertex shader
				model = glm::make_mat4(&shapePose.column0.x);

				// check what geometry type the shape is
				if (h.any().getType() == PxGeometryType::eBOX)
				{
					glActiveTexture(GL_TEXTURE0);
					//texture_crate.Bind();
					// This texture is also sent to the fragment shader 
					//texture_crate.texUnit(shaderProgram, "tex0", 0);
					// Export camera matrix to the vertex shader
					//camera.exportMatrix(shaderProgram, "camMatrix");
					// Export the shape's model matrix to the vertex shader
					glUniformMatrix4fv(glGetUniformLocation(graphics->shader3D->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
					// render a box (hardcoded with same dimensions as physx one) using the model and cam matrix
					if(!(police_car->headlights ==  nullptr)) police_car->headlights->draw();
				}
				else if (h.any().getType() == PxGeometryType::eSPHERE)
				{

				}
				else if (h.any().getType() == PxGeometryType::eCONVEXMESH) {

					graphics->shader3D->setInt("shaderMode", SHADER_MODE_DIFFUSE);
					CarModel4W* activeCar;
					activeCar = player->car;
					if (i != 0) activeCar = police_car->car;

					if (j == 0) {
						activeCar->Draw(FRWHEEL, *graphics->shader3D, model);;
					}
					else if (j == 1) {
						activeCar->Draw(FLWHEEL, *graphics->shader3D, model);
					}
					else if (j == 2) {
						activeCar->Draw(BRWHEEL, *graphics->shader3D, model);
					}
					else if (j == 3) {
						activeCar->Draw(BLWHEEL, *graphics->shader3D, model);
					}
					else if (j == 4) {
						activeCar->Draw(CHASSIS, *graphics->shader3D, model);
					}
				}
			}
		}
	}
}


void despawnEnemy(Vehicle* enemy) {

	gScene->removeActor(*enemy->actorPtr);
	for (int i = 0; i < physx_actors.size(); i++) {
		if (physx_actors[i].actorPtr == enemy->actorPtr) {
			printf("ERASING\n");
			physx_actors.erase(physx_actors.begin() + i);
			return;
		}
	}
	delete(enemy);
}