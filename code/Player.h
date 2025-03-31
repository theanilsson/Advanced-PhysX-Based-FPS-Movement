#pragma once
#include <DreamEngine/graphics/Camera.h>
#include <DreamEngine/graphics/ModelInstance.h>
#include "ShaderTool.h"
#include "VFXManager.h"
#include "GameObject.h"
#include "Observer.h"
#include "PlayerStats.h"
#include "Gun.h"

class Player : public GameObject, public Observer
{
public:
	// Init stage functions
	Player() = default;
	Player(const Player& anOtherPlayer) = delete;
	Player& operator=(const Player& anOtherPlayer) = delete;
	Player(Player&& anOtherPlayer) = default;
	Player& operator=(Player&& anOtherPlayer) = default;
	~Player();
	void Init();
	void InitParticleSystem();
	std::vector<ParticleSystem3D*> GetGunParticleSystems();

	// Update stage functions
	void Update(const float aDeltaTime) override;
	void Receive(const Message& aMessage) override;
	const PlayerStats& GetPlayerStats() const;
	Gun& GetGun();
	float GetIframesTimerValue();
	float GetHealingTimerValue();

	// Render stage functions
	void Render(DE::GraphicsEngine& aGraphicsEngine) override;
	DE::ModelInstance GetGrapplingHookCylinder();

private:
	// Init stage functions
	void SetUpComponents();
	void SetUpSubscriptions();
	void SetUpModel();
	void InitializeMemberVariables();

	// Update stage functions
	void HandleInitialFrameState(const float aDeltaTime, physx::PxRigidDynamic* aPhysicsBodyPtr);
	void UpdateCamera();
	void CalculateRocketKnockback(std::pair<physx::PxRigidActor*, int>* aRocketBodyAndDamagePair);
	void CheckForInteractables();
	void CheckIfInGrapplingRange(physx::PxRigidDynamic* aPhysicsBodyPtr);
	bool CheckIfGrounded();
	void ActivateGrapplingHook();
	void DeactivateGrapplingHook();
	void UpdateGrapplingHookVisualization();
	void ApplyGrapplingHookForces(physx::PxRigidDynamic* aPhysicsBodyPtr);
	void TriggerGrapplingHookJump(physx::PxRigidDynamic* aPhysicsBodyPtr);
	void ApplyCustomGroundedFriction(physx::PxRigidDynamic* aPhysicsBodyPtr, float aDeltaTime);
	void ApplyGroundedMovement(physx::PxRigidDynamic* aPhysicsBodyPtr);
	void ApplyGroundSlideBoost(physx::PxRigidDynamic* aPhysicsBodyPtr);
	void ApplyMidAirMovement(physx::PxRigidDynamic* aPhysicsBodyPtr);
	void TriggerJump(physx::PxRigidDynamic* aPhysicsBodyPtr, bool aIsDoubleJump);
	void TriggerRocketKnockback(physx::PxRigidDynamic* aPhysicsBodyPtr);
	void UpdateCameraRotation(float aDeltaTime);
	void UpdateTimers(float aDeltaTime);
	void Respawn(physx::PxRigidDynamic* aPhysicsBodyPtr);
	void ResetConditionalFlags();

	std::shared_ptr<DE::Camera> myCamera;
	PlayerStats myStats;
	Gun myGun;
	DE::ModelInstance myGrapplingHookCylinder;
};