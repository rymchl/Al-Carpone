 //Initializes the physx system and all global variables and calls other includes.
#include"init.h"




#define NEAR_CLIPPING_PLANE 0.01f
#define FAR_CLIPPING_PLANE 1000.f

namespace sv = snippetvehicle;

int main()
{
	// TODO clean up
	State state;
	Player player;
	Bank bank;




	cout << "Initializing Graphics..." << endl;

	GraphicsSystem graphics; //Must be called first ALWAYS

	// Initialize Windows
	DebugPanel debugPanel(graphics.window);
	MainMenu mainMenu;
	UI ui;
	 
	cout << "Initalizing Physics..." << endl;

	//Set up physx with vehicle snippet:
	//Make sure this is called after the shader program is generated
	PhysicsSystem physics(state, player, bank);

	//TODO Cleanup
	//Setup main player vehicle
	player = Player(0);
	//Add it to the list of active vehicles
	state.activeVehicles.push_back(&player);

	bank.createActors();


	// Initialize Models
	player.createModel(); //TODO: If player is moved here as well, we can create model in constructors instead.
	bank.createModel();
	PoliceCar police_car;
	police_car.createModel();

	graphics.enableDepthBuffer();


	// Active camera can be one of [bound/free] at a given time
	BoundCamera boundCamera(player, state);	// Locked in a sphere around the car
	FreeCamera freeCamera(player);		// Move and look freely anywhere (for debugging)
	Camera* activeCamera = &boundCamera;


	// Main loop
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

	
		// Check if in the main menu
		if (state.mainMenu) {
			//Draw the menu
			mainMenu.drawMenu(graphics, state);
			
			// If exiting the main menu
			if (!state.mainMenu) {
				// Setup level

				mainMenu.changeLevel(state.selectedLevel);
				
				//Remove the old level pointer and add the new
				PxFilterData groundPlaneSimFilterData(sv::COLLISION_FLAG_GROUND, sv::COLLISION_FLAG_GROUND_AGAINST, 0, 0);
				gGroundPlane = physics.createDrivablePlane(groundPlaneSimFilterData, gMaterial, gPhysics, gCooking, state.selectedLevel);
				
				gScene->removeActor(*activeLevelActorPtr);
				
				gScene->addActor(*gGroundPlane);

				PxU32 size = gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC) * sizeof(PxActor*);
				PxActor** actors = (PxActor**)malloc(size);
				gScene->getActors(PxActorTypeFlag::eRIGID_STATIC, actors, size, 0);
				activeLevelActorPtr = actors[gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC) - 1];

				if (state.selectedLevel == 0) {
					if (police_car.actorPtr == NULL) police_car = PoliceCar(1);
					state.activeVehicles.push_back(&police_car);
				}
				
				//Reset active vehicles
				for (Vehicle* v : state.activeVehicles) {
					v->reset();
				}
			}
			
		}
		else {

			// view/projection transformations

			glm::mat4 projection = glm::perspective(glm::radians(activeCamera->zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, NEAR_CLIPPING_PLANE, FAR_CLIPPING_PLANE);

			// view/projection transformations
			glm::mat4 view = glm::mat4(glm::mat3(activeCamera->GetViewMatrix())); // remove translation from the view matrix
			graphics.skybox->Draw(projection, view);


			view = activeCamera->GetViewMatrix();

			//Tell player if they can rob
			if (player.canRob(state)) {
				graphics.shader2D->use();
				ui.press_f_to_rob->Draw(*graphics.shader2D);
			}
			graphics.shader3D->use();
			//Simulate physics through the timestep
			physics.step(graphics.window);

			//Check for special inputs (currently only camera mode change)
			checkSpecialInputs(graphics.window, state, player);

			if (state.cameraMode == CAMERA_MODE_BOUND) activeCamera = &boundCamera;
			else if (state.cameraMode == CAMERA_MODE_UNBOUND_FREELOOK) activeCamera = &freeCamera;


			// Camera is disabled in DEBUG MODE
			if (!state.debugMode) activeCamera->handleInput(graphics.window, state);
			if (activeCamera == &boundCamera) boundCamera.checkClipping(graphics.window);


			// send them to shader
			graphics.shader3D->setMat4("projection", projection);
			graphics.shader3D->setMat4("view", view);

			graphics.shader3D->setInt("numLights", mainMenu.light_positions->size());

			for (int i = 0; i < mainMenu.light_positions->size(); i++) {
				std::string path = "light_positions[" + std::to_string(i) + "]";
				graphics.shader3D->setVec3(path.c_str(), (*mainMenu.light_positions)[i]);
			}

			// render the loaded model

			glm::mat4 model = glm::mat4(1.0f);
			graphics.shader3D->setMat4("model", model);
			graphics.shader3D->setVec3("camPos", glm::vec3(activeCamera->pos.x, activeCamera->pos.y, activeCamera->pos.z));
			graphics.shader3D->setInt("shaderMode", SHADER_MODE_DIFFUSE);
			mainMenu.active_level->Draw(*graphics.shader3D);

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
							//BANK MODEL NOT INCLUDED FOR NOW, BANK IS PART OF GROUND PLANE	
							//Note that the trigger is also of pxGeometryType::eBox now

						}
						else if (h.any().getType() == PxGeometryType::eSPHERE)
						{

						}
						else if (h.any().getType() == PxGeometryType::eCONVEXMESH) {

							graphics.shader3D->setInt("shaderMode", SHADER_MODE_DIFFUSE);
							CarModel4W* activeCar;
							activeCar = player.car;
							if (i != 0) activeCar = police_car.car;

							if (j == 0) {
								activeCar->Draw(FRWHEEL, *graphics.shader3D, model);;
							}
							else if (j == 1) {
								activeCar->Draw(FLWHEEL, *graphics.shader3D, model);
							}
							else if (j == 2) {
								activeCar->Draw(BRWHEEL, *graphics.shader3D, model);
							}
							else if (j == 3) {
								activeCar->Draw(BLWHEEL, *graphics.shader3D, model);
							}
							else if (j == 4) {
								activeCar->Draw(CHASSIS, *graphics.shader3D, model);
							}

						}
					}
				}
			}

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