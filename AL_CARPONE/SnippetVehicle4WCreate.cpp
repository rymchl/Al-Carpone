//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2021 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "snippetvehiclecommon/SnippetVehicleCreate.h"
#include "snippetvehiclecommon/SnippetVehicleTireFriction.h"
#include "snippetvehiclecommon/SnippetVehicleSceneQuery.h"


//Here is a bunch of vehicle settings I found to modify, there are probably more we can find to toy with aswell.
#define VEHICLE_ENGINE_PEAK_TORQUE 500.0f //init 500
#define VEHICLE_ENGINE_MAX_OMEGA 600.0f //init 600
#define VEHICLE_GEAR_SWITCH_TIME 0.5f //init 0.5
#define VEHICLE_CLUTCH_STRENGTH 10.0f //init 10
#define VEHICLE_HANDBREAK_MAX_TORQUE 4000.0f //init 4000
#define VEHICLE_SUSPENSION_MAX_COMPRESSION 0.3f //init 0.3
#define VEHICLE_SUSPENSION_MAX_DROOP 0.1f //init 0.1
#define VEHICLE_SUSPENSION_SPRING_STRENGTH 35000.0f //init 35000
#define VEHICLE_SUSPENSION_DAMPER_RATE 4500.0f //init 4500
#define VEHICLE_MAX_STEER_COEFFICIENT 0.3333f //init 0.3333

namespace snippetvehicle
{
	
using namespace physx;

namespace fourwheel
{

void computeWheelCenterActorOffsets4W(const PxF32 wheelFrontZ, const PxF32 wheelRearZ, const PxVec3& chassisDims, const PxF32 wheelWidth, const PxF32 wheelRadius, const PxU32 numWheels, PxVec3* wheelCentreOffsets, unsigned int type)
{
	//chassisDims.z is the distance from the rear of the chassis to the front of the chassis.
	//The front has z = 0.5*chassisDims.z and the rear has z = -0.5*chassisDims.z.
	//Compute a position for the front wheel and the rear wheel along the z-axis.
	//Compute the separation between each wheel along the z-axis.
	const PxF32 numLeftWheels = numWheels/2.0f;
	const PxF32 deltaZ = (wheelFrontZ - wheelRearZ)/(numLeftWheels-1.0f);
	//Set the outside of the left and right wheels to be flush with the chassis.
	//Set the top of the wheel to be just touching the underside of the chassis.
	//Begin by setting the rear-left/rear-right/front-left,front-right wheels.

	float blenderOutwardOffset = 0; 
	float blenderVerticalOffset = 0;
	float blenderForwardWheelOffset = 0;
	float blenderBackwardWheelOffset = 0;

	//al_carpone
	if (type == 0) {
		blenderOutwardOffset = 0.9;// 1.016185;
		blenderVerticalOffset = -0.363796;
		blenderForwardWheelOffset = 1.503269;
		blenderBackwardWheelOffset = -1.496731;
	}
	//copcar
	else {
		blenderOutwardOffset = 0.809849;
		blenderVerticalOffset = -0.563796;
		blenderForwardWheelOffset = 1.38798;
		blenderBackwardWheelOffset = -1.6241;
	}

	PxVec3 blenderRearLeft(blenderOutwardOffset, blenderVerticalOffset, blenderBackwardWheelOffset);
	PxVec3 blenderRearRight(-blenderOutwardOffset, blenderVerticalOffset, blenderBackwardWheelOffset);
	PxVec3 blenderFrontLeft(blenderOutwardOffset, blenderVerticalOffset, blenderForwardWheelOffset);
	PxVec3 blenderFrontRight(-blenderOutwardOffset, blenderVerticalOffset, blenderForwardWheelOffset);

	PxVec3 rearLeft = blenderRearLeft;
	PxVec3 rearRight = blenderRearRight;
	PxVec3 frontLeft = blenderFrontLeft;
	PxVec3 frontRight = blenderFrontRight;

	rearLeft.x = - rearLeft.x;
	rearRight.x = -rearRight.x;
	frontLeft.x = -frontLeft.x;
	frontRight.x = -frontRight.x;

	wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_LEFT] = rearLeft;// PxVec3(-0.85543, 0.329486, -0.932926);	// blender coordinates: (x, y, z) -> physx coordinates: (-x, z, -y)
	wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eREAR_RIGHT] = rearRight;// PxVec3(0.85543, 0.329486, -0.932926);
	wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_LEFT] = frontLeft;// PxVec3(-0.85543, 0.329486, 1.58178);
	wheelCentreOffsets[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] = frontRight;// PxVec3(0.85543, 0.329486, 1.58178);
	//Set the remaining wheels.
	for(PxU32 i = 2, wheelCount = 4; i < numWheels-2; i+=2, wheelCount+=2)
	{
		wheelCentreOffsets[wheelCount + 0] = PxVec3((-chassisDims.x + wheelWidth)*0.5f, -(chassisDims.y/2 + wheelRadius), wheelRearZ + i*deltaZ*0.5f);
		wheelCentreOffsets[wheelCount + 1] = PxVec3((+chassisDims.x - wheelWidth)*0.5f, -(chassisDims.y/2 + wheelRadius), wheelRearZ + i*deltaZ*0.5f);
	}
}

void setupWheelsSimulationData
(const PxF32 wheelMass, const PxF32 wheelMOI, const PxF32 wheelRadius, const PxF32 wheelWidth, 
 const PxU32 numWheels, const PxVec3* wheelCenterActorOffsets,
 const PxVec3& chassisCMOffset, const PxF32 chassisMass,
 PxVehicleWheelsSimData* wheelsSimData)
{
	//Set up the wheels.
	PxVehicleWheelData wheels[PX_MAX_NB_WHEELS];
	{
		//Set up the wheel data structures with mass, moi, radius, width.
		for(PxU32 i = 0; i < numWheels; i++)
		{
			wheels[i].mMass = wheelMass;
			wheels[i].mMOI = wheelMOI;
			wheels[i].mRadius = wheelRadius;
			wheels[i].mWidth = wheelWidth;
		}

		//Enable the handbrake for the rear wheels only.
		wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque=VEHICLE_HANDBREAK_MAX_TORQUE;
		wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque=VEHICLE_HANDBREAK_MAX_TORQUE;
		//Enable steering for the front wheels only.
		wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer=PxPi*VEHICLE_MAX_STEER_COEFFICIENT;
		wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer=PxPi*VEHICLE_MAX_STEER_COEFFICIENT;
	}

	//Set up the tires.
	PxVehicleTireData tires[PX_MAX_NB_WHEELS];
	{
		//Set up the tires.
		for(PxU32 i = 0; i < numWheels; i++)
		{
			tires[i].mType = TIRE_TYPE_NORMAL;
		}
	}

	//Set up the suspensions
	PxVehicleSuspensionData suspensions[PX_MAX_NB_WHEELS];
	{
		//Compute the mass supported by each suspension spring.
		PxF32 suspSprungMasses[PX_MAX_NB_WHEELS];
		PxVehicleComputeSprungMasses
			(numWheels, wheelCenterActorOffsets, 
			 chassisCMOffset, chassisMass, 1, suspSprungMasses);

		//Set the suspension data.
		for(PxU32 i = 0; i < numWheels; i++)
		{
			suspensions[i].mMaxCompression = VEHICLE_SUSPENSION_MAX_COMPRESSION;
			suspensions[i].mMaxDroop = VEHICLE_SUSPENSION_MAX_DROOP;
			suspensions[i].mSpringStrength = VEHICLE_SUSPENSION_SPRING_STRENGTH;
			suspensions[i].mSpringDamperRate = VEHICLE_SUSPENSION_DAMPER_RATE;
			suspensions[i].mSprungMass = suspSprungMasses[i];
		}

		//Set the camber angles.
		const PxF32 camberAngleAtRest=0.0;
		const PxF32 camberAngleAtMaxDroop=0.01f;
		const PxF32 camberAngleAtMaxCompression=-0.01f;
		for(PxU32 i = 0; i < numWheels; i+=2)
		{
			suspensions[i + 0].mCamberAtRest =  camberAngleAtRest;
			suspensions[i + 1].mCamberAtRest =  -camberAngleAtRest;
			suspensions[i + 0].mCamberAtMaxDroop = camberAngleAtMaxDroop;
			suspensions[i + 1].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
			suspensions[i + 0].mCamberAtMaxCompression = camberAngleAtMaxCompression;
			suspensions[i + 1].mCamberAtMaxCompression = -camberAngleAtMaxCompression;
		}
	}

	//Set up the wheel geometry.
	PxVec3 suspTravelDirections[PX_MAX_NB_WHEELS];
	PxVec3 wheelCentreCMOffsets[PX_MAX_NB_WHEELS];
	PxVec3 suspForceAppCMOffsets[PX_MAX_NB_WHEELS];
	PxVec3 tireForceAppCMOffsets[PX_MAX_NB_WHEELS];
	{
		//Set the geometry data.
		for(PxU32 i = 0; i < numWheels; i++)
		{
			//Vertical suspension travel.
			suspTravelDirections[i] = PxVec3(0,-1,0);

			//Wheel center offset is offset from rigid body center of mass.
			wheelCentreCMOffsets[i] = 
				wheelCenterActorOffsets[i] - chassisCMOffset;

			//Suspension force application point 0.3 metres below 
			//rigid body center of mass.
			suspForceAppCMOffsets[i] =
				PxVec3(wheelCentreCMOffsets[i].x,-0.3f,wheelCentreCMOffsets[i].z);

			//Tire force application point 0.3 metres below 
			//rigid body center of mass.
			tireForceAppCMOffsets[i] =
				PxVec3(wheelCentreCMOffsets[i].x,-0.3f,wheelCentreCMOffsets[i].z);
		}
	}

	//Set up the filter data of the raycast that will be issued by each suspension.
	PxFilterData qryFilterData;
	setupNonDrivableSurface(qryFilterData);

	//Set the wheel, tire and suspension data.
	//Set the geometry data.
	//Set the query filter data
	for(PxU32 i = 0; i < numWheels; i++)
	{
		wheelsSimData->setWheelData(i, wheels[i]);
		wheelsSimData->setTireData(i, tires[i]);
		wheelsSimData->setSuspensionData(i, suspensions[i]);
		wheelsSimData->setSuspTravelDirection(i, suspTravelDirections[i]);
		wheelsSimData->setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
		wheelsSimData->setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
		wheelsSimData->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
		wheelsSimData->setSceneQueryFilterData(i, qryFilterData);
		wheelsSimData->setWheelShapeMapping(i, PxI32(i)); 
	}

	//Add a front and rear anti-roll bar
	PxVehicleAntiRollBarData barFront;
	barFront.mWheel0 = PxVehicleDrive4WWheelOrder::eFRONT_LEFT;
	barFront.mWheel1 = PxVehicleDrive4WWheelOrder::eFRONT_RIGHT;
	barFront.mStiffness = 10000.0f;
	wheelsSimData->addAntiRollBarData(barFront);
	PxVehicleAntiRollBarData barRear;
	barRear.mWheel0 = PxVehicleDrive4WWheelOrder::eREAR_LEFT;
	barRear.mWheel1 = PxVehicleDrive4WWheelOrder::eREAR_RIGHT;
	barRear.mStiffness = 10000.0f;
	wheelsSimData->addAntiRollBarData(barRear);
}

} //namespace fourwheel

PxVehicleDrive4W* createVehicle4W(const VehicleDesc& vehicle4WDesc, PxPhysics* physics, PxCooking* cooking, unsigned int type)
{
	const PxVec3 chassisDims = vehicle4WDesc.chassisDims;
	const PxF32 wheelWidth = vehicle4WDesc.wheelWidth;
	const PxF32 wheelRadius = vehicle4WDesc.wheelRadius;
	const PxU32 numWheels = vehicle4WDesc.numWheels;

	const PxFilterData& chassisSimFilterData = vehicle4WDesc.chassisSimFilterData;
	const PxFilterData& wheelSimFilterData = vehicle4WDesc.wheelSimFilterData;

	//Construct a physx actor with shapes for the chassis and wheels.
	//Set the rigid body mass, moment of inertia, and center of mass offset.
	PxRigidDynamic* veh4WActor = NULL;
	{
		//Construct a convex mesh for a cylindrical wheel.
		PxConvexMesh* wheelMesh = createWheelMesh(wheelWidth, wheelRadius, *physics, *cooking, type);
		//Assume all wheels are identical for simplicity.
		PxConvexMesh* wheelConvexMeshes[PX_MAX_NB_WHEELS];
		PxMaterial* wheelMaterials[PX_MAX_NB_WHEELS];

		//Set the meshes and materials for the driven wheels.
		for(PxU32 i = PxVehicleDrive4WWheelOrder::eFRONT_LEFT; i <= PxVehicleDrive4WWheelOrder::eREAR_RIGHT; i++)
		{
			wheelConvexMeshes[i] = wheelMesh;
			wheelMaterials[i] = vehicle4WDesc.wheelMaterial;
		}
		//Set the meshes and materials for the non-driven wheels
		for(PxU32 i = PxVehicleDrive4WWheelOrder::eREAR_RIGHT + 1; i < numWheels; i++)
		{
			wheelConvexMeshes[i] = wheelMesh;
			wheelMaterials[i] = vehicle4WDesc.wheelMaterial;
		}

		//Chassis just has a single convex shape for simplicity.
		PxConvexMesh* chassisConvexMesh = createChassisMesh(chassisDims, *physics, *cooking, type);
		PxConvexMesh* chassisConvexMeshes[1] = {chassisConvexMesh};
		PxMaterial* chassisMaterials[1] = {vehicle4WDesc.chassisMaterial};

		//Rigid body data.
		PxVehicleChassisData rigidBodyData;
		rigidBodyData.mMOI = vehicle4WDesc.chassisMOI;
		rigidBodyData.mMass = vehicle4WDesc.chassisMass;
		rigidBodyData.mCMOffset = vehicle4WDesc.chassisCMOffset;

		veh4WActor = createVehicleActor
			(rigidBodyData,
			wheelMaterials, wheelConvexMeshes, numWheels, wheelSimFilterData,
			chassisMaterials, chassisConvexMeshes, 1, chassisSimFilterData,
			*physics);
	}

	//Set up the sim data for the wheels.
	PxVehicleWheelsSimData* wheelsSimData = PxVehicleWheelsSimData::allocate(numWheels);
	{
		//Compute the wheel center offsets from the origin.
		PxVec3 wheelCenterActorOffsets[PX_MAX_NB_WHEELS];
		const PxF32 frontZ = chassisDims.z*0.3f;
		const PxF32 rearZ = -chassisDims.z*0.3f;
		fourwheel::computeWheelCenterActorOffsets4W(frontZ, rearZ, chassisDims, wheelWidth, wheelRadius, numWheels, wheelCenterActorOffsets, type);

		//Set up the simulation data for all wheels.
		fourwheel::setupWheelsSimulationData
			(vehicle4WDesc.wheelMass, vehicle4WDesc.wheelMOI, wheelRadius, wheelWidth, 
			 numWheels, wheelCenterActorOffsets,
			 vehicle4WDesc.chassisCMOffset, vehicle4WDesc.chassisMass,
			 wheelsSimData);
	}

	//Set up the sim data for the vehicle drive model.
	PxVehicleDriveSimData4W driveSimData;
	{
		//Diff
		PxVehicleDifferential4WData diff;
		diff.mType=PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
		driveSimData.setDiffData(diff);

		//Engine
		PxVehicleEngineData engine;
		engine.mPeakTorque= VEHICLE_ENGINE_PEAK_TORQUE;
		engine.mMaxOmega=VEHICLE_ENGINE_MAX_OMEGA;//approx 6000 rpm
		driveSimData.setEngineData(engine);

		//Gears
		PxVehicleGearsData gears;
		gears.mSwitchTime=VEHICLE_GEAR_SWITCH_TIME;
		driveSimData.setGearsData(gears);

		//Clutch
		PxVehicleClutchData clutch;
		clutch.mStrength= VEHICLE_CLUTCH_STRENGTH;
		driveSimData.setClutchData(clutch);

		//Ackermann steer accuracy
		PxVehicleAckermannGeometryData ackermann;
		ackermann.mAccuracy=1.0f;
		ackermann.mAxleSeparation=
			wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).z-
			wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).z;
		ackermann.mFrontWidth=
			wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).x-
			wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).x;
		ackermann.mRearWidth=
			wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).x -
			wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).x;
		driveSimData.setAckermannGeometryData(ackermann);
	}

	//Create a vehicle from the wheels and drive sim data.
	PxVehicleDrive4W* vehDrive4W = PxVehicleDrive4W::allocate(numWheels);
	vehDrive4W->setup(physics, veh4WActor, *wheelsSimData, driveSimData, numWheels - 4);

	//Configure the userdata
	configureUserData(vehDrive4W, vehicle4WDesc.actorUserData, vehicle4WDesc.shapeUserDatas);

	//Free the sim data because we don't need that any more.
	wheelsSimData->free();

	return vehDrive4W;
}

} //namespace snippetvehicle


