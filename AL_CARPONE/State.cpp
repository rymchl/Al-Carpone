#include"State.h"


//Updates the state's time-sensitive variables.
void State::updateTime() {
	currTime = glfwGetTime();
	timeStep = currTime - prevTime;
	prevTime = currTime;
	timeSinceLastFpsUpdate += timeStep;
}

//Toggles the camera mode between bound and unbound.
void State::toggleCameraMode() {
	if (cameraMode == CAMERA_MODE_UNBOUND_FREELOOK) {
		cameraMode = CAMERA_MODE_BOUND;
	}
	else {
		cameraMode = CAMERA_MODE_UNBOUND_FREELOOK;
	}
}

void State::resetVehicles() {
	for (PoliceCar* p : activePoliceVehicles) {
		p->reset();
	}
	playerPtr->reset();
}

void State::alertPolice() {
	for (PoliceCar* p : activePoliceVehicles) {
		if (!p->isStunned) {
			p->startChase();
		}
	}
}

bool State::policeAlerted() {
	for (PoliceCar* p : activePoliceVehicles) {
		if (p->chaseTime > 0) return true;
	}
	return false;
}

float State::getAlertLevel() {
	float chaseSum = 0;
	float chaseMax = 0;
	for (PoliceCar* p : activePoliceVehicles) {
		chaseSum += p->chaseTime;
		chaseMax += p->maxChaseTime;
	}
	if (chaseMax <= 0) return 0;

	float ratio = chaseSum / chaseMax;

	return ratio;
}


void State::despawnItems()
{
	for (int i = 0; i < active_items.size(); i++)
	{
		if (active_items[i].getRemainingTime() <= 0.f)
		{
			active_items.erase(active_items.begin() + i);	//erase from simple_renderables
			i = i - 1;
		}
	}
}



void State::checkAchievements(Player& player) {
	//newAchievement = false;
	glm::vec3 pos = player.getPos();
	
	//over the police roof
	if (!isRoofOfPoliceStation)
	{
		if ((176 > pos.x) && (pos.x > 117) && (-190 > pos.z) && (pos.z > -205) && (pos.y > 18)) {
			isRoofOfPoliceStation = true;
			newAchievement = true;
			cout << "Player made it over the police station roof!" << endl;
		}
	}

	//unlock all upgrade
	if (player.numUpgradesPurchased == 24 && !unlockedAllUpgrades) {
		unlockedAllUpgrades = true;
		newAchievement = true;
		cout << "Unlock all upgrade materials!" << endl;
	}

	//Made it out of the map
	if (pos.y < -21) {			//allways check for this, reset the player to start pos if so.
		if (!isJumpOutOfMap)
		{
			isJumpOutOfMap = true;
			newAchievement = true;
			cout << "Player made it outside of the map!" << endl;
		}
		player.reset();
	}


	//Finish Game
	if (!isFinishGame) {
		isFinishGame = gameWon;		//kinda redundant. We dont really need this bool
		if (gameWon) {
			newAchievement = true;
			cout << "Player has won the game!" << endl;
		}
			
	}


	//Dupe police
	if (!isDupeThePolice) {
		
	
		//cout << "dupe_level" << dupe_level << "dupe_timer" << dupe_timer << endl;

		
		bool currentlyChased = false;
		

		////cout << " currentlyChased is : " << currentlyChased << endl;
		
		

		 for (PoliceCar* p : activePoliceVehicles) {	// if at least one police car is still chasing the player, make the "overall" state chase
		 	if ((p->ai_state != AISTATE::PATROL)) {
		 		currentlyChased = true;
		 	}
		 }

		 // If the player was previously being chased by at least one police car but is now being chased by none. All police cars must be patrolling (not idle)
		 if (!currentlyChased && previousStateChase && !GAMESTATE::GAMESTATE_JAILED) {
		 	isDupeThePolice = true;
			newAchievement = true;
		 	cout << "Player has duped the police!" << endl;
		 }

		 if (currentlyChased && dupe_level > 0.9 && dupe_timer == 1)
		 {
			 previousStateChase = true;
		 }
		 else if(!previousStateChase)
		 {
			 previousStateChase = false;
		 }
		 else if (currentlyChased)
		 {
			 previousStateChase = true;
		 }
	}

	if (!isMillionaire) 
	{
		if(player.getCash() > 1000000)
		{
			isMillionaire = true;
			newAchievement = true;
			cout << "you are a millionaire" << endl;
		}
	}
}
