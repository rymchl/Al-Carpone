#include "Bank.h"
#include "PxPhysicsAPI.h"
#include "physx_globals.h"
#include "snippetvehiclecommon/SnippetVehicleFilterShader.h"
#include "PxCustomEventCallback.h"


using namespace physx;
using namespace snippetvehicle;


//TODO cleanup
// TODO move some to physics system
void Bank::createActors() {
	//BANK (BUILDING)
	PxVec3 b_pos(getPos().x, getPos().y, getPos().z);

	float bank_half_width, bank_half_depth;
	if ((getDir() == ORIENT::N) || (getDir() == ORIENT::S)) {		//if bank is facing east or west, we should swap the width and depth dimensions of the bank "hitbox"
		bank_half_width = getWidth() / 2.f;
		bank_half_depth = getDepth() / 2.f;
	}
	else {
		bank_half_width = getDepth() / 2.f;
		bank_half_depth = getWidth() / 2.f;
	}
	PxVec3 dimensions(bank_half_width, getHeight() / 2.f, bank_half_depth);			//PxVec3 represents the half-extents (half-w, half-h, half-d).

	// Setting up a bank(rigidStatic) as a simple box for now
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(dimensions), *gMaterial);
	PxTransform bankPos(b_pos);																	//position of the bank

	PxRigidStatic* body = gPhysics->createRigidStatic(bankPos);									//Static as the buildung will not move
	PxFilterData bankFilter(COLLISION_FLAG_OBSTACLE, COLLISION_FLAG_OBSTACLE_AGAINST, 0, 0);	//(I think) these should set this actor as an obstacle to the vehicle

	shape->setSimulationFilterData(bankFilter);
	body->attachShape(*shape);
	gScene->addActor(*body);
	if (physx_actors.size() == 0) physx_actors.push_back({ body, 0 });
	else physx_actors.push_back({ body, physx_actors.back().actorId + 1 });
	shape->release();

	bankPtr = body;

	//ROBBING TRIGGER
	//Setting up the capsule that will act as a trigger. This is set up in "front" of the bank (will need bank position and orientation).
	PxVec3 t_pos = b_pos;
	t_pos.y = 0.f;															//set height to 0 so the car can actually touch it
	float trigger_half_depth = 7.f;
	float trigger_half_width = 0.f;
	switch (getDir()) {
		case ORIENT::N:		//N
			t_pos.z += (bank_half_depth + trigger_half_depth);		//trigger is further back in the z direction (higher z)
			trigger_half_width = getWidth() / 2.f;
			break;
		case ORIENT::E:		//E
			t_pos.x -= (bank_half_depth + trigger_half_depth);		//trigger is further "right" (x dir)
			trigger_half_width = trigger_half_depth;				//swap width and depth
			trigger_half_depth = getWidth() / 2.f;
			break;
		case ORIENT::S:		//S
			t_pos.z -= (bank_half_depth + trigger_half_depth);		//trigger is further forward in the z direction (smaller z)
			trigger_half_width = getWidth() / 2.f;
			break;
		case ORIENT::W:		//W
			t_pos.x += (bank_half_depth + trigger_half_depth);		//trigger is further "left" (x dir)
			trigger_half_width = trigger_half_depth;				//swap width and depth
			trigger_half_depth = getWidth() / 2.f;
			break;
	}
	//PxShape* triggerShape = gPhysics->createShape(PxCapsuleGeometry(PxReal(5), PxReal(2.5)), *gMaterial);	//radius and half-height of capsule as parameters
	PxShape* triggerShape = gPhysics->createShape(PxBoxGeometry(PxVec3(trigger_half_width, 2.f, trigger_half_depth)), *gMaterial);
	trigger = BoxTrigger(true, trigger_half_width* 2, 4.f, trigger_half_depth*2, t_pos);
}