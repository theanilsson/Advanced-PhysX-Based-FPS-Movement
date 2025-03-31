#pragma once
#include <DreamEngine\math\Vector3.h>
#include <DreamEngine\utilities\CountTimer.h>
#include <PhysX\PxPhysicsAPI.h>

class GameObject;

class PlayerStats 
{
public:
	// Health management
	void SetCurrentHealth(GameObject* aPlayerPtr, const int aHealthAmount);
	const int GetCurrentHealth(GameObject* aPlayerPtr) const;
	const int GetMaxHealth(GameObject* aPlayerPtr) const;
	void TakeDamage(GameObject* aPlayerPtr, const int& aDamageAmount);
	void Heal(GameObject* aPlayerPtr, const int& aHealingAmount);
	void HealToMax(GameObject* aPlayerPtr);
	const bool IsAlive(GameObject* aPlayerPtr) const;
	CommonUtilities::CountdownTimer iFramesTimer;
	CommonUtilities::CountdownTimer healingTimer;

	// Camera
	float nearPlane = 10.0f;
	float farPlane = 10000.0f;
	const float physicsBodyVerticalOffset = 65.0f;
	const float cameraVerticalOffset = 130.0f;
	const float groundSlideCameraVerticalOffset = 85.0f;

	// Interaction
	const float maximumInteractRange = 250.0f;

	// Ammo type upgrading
	const int ammoMaxUpgradeLevel = 3;
	const int firstAmmoUpgradeCost = 750;
	const int secondAmmoUpgradeCost = 1000;
	const int moneyCurrencyReward = 1500;
	int currentMoneyCurrency = 0;

	// Grappling hook movement
	const float grapplingPointCloseSnapDistance = 10.0f;
	const float maxGrapplingDistance = 2500.0f;
	const float directionalGrapplingSpeed = 2150.0f;
	const float grapplingSpeedControlMax = 2050.0f;
	const float grapplingSpringStiffness = 10.0f;
	const float grapplingSpringDampeningFactor = -3.5f;
	const float grapplingMoveDampeningFactor = -1.05f;
	const float grapplingVisualizationLength = 0.011f;
	const float swingForceScalar = 5.0f;
	const float maxAirVelocity = 900.0f;
	bool jumpedDuringGrapple = false;
	bool isGrappleActive = false;
	bool isInGrapplingRange = false;
	CU::CountdownTimer grapplingCooldown;
	CU::CountupTimer grapplingDuration;
	physx::PxVec3 grapplingPoint;
	physx::PxRigidActor* grappledObject;
	physx::PxRaycastBuffer grapplingHitBuffer;

	// Mid air movement
	const float directionalAirSpeed = 1000.0f;
	const float airControlSpeedMax = 900.0f;
	const float airControlDampeningFactor = -0.15f;
	CU::CountupTimer airTime;

	// Grounded movement
	CU::CountdownTimer respawnTimer;
	physx::PxVec3 spawnPoint;
	const float directionalSlidingSpeed = 250.0f;
	const float slidingSpeedControlMax = 150.0f;
	const float directionalWalkingSpeed = 1050.0f;
	const float walkingSpeedControlMax = 650.0f;
	const float directionalSprintSpeed = 1300.0f;
	const float sprintSpeedControlMax = 850.0f;
	const float jumpForce = 850.0f;
	physx::PxVec3 groundedNormal;
	float groundedDotProduct;
	const float groundedTraceLength = 70.0f;
	const float grapplingGroundedTraceLength = 175.0f;
	const float groundedFriction = 0.00001f;
	const float groundedFullStopDistance = 25.0f;
	const physx::PxVec3 groundedGravityVector = physx::PxVec3(0.0f, -9.81f * 172.3f, 0.0f);
	const float groundSlidingFriction = 4.0f;
	const physx::PxVec3 groundSlidingGravityVector = physx::PxVec3(0.0f, -9.81f * 100.0f, 0.0f);
	const float groundSlideBoostFactor = 1.4f;
	physx::PxVec3 previousFramePosition;
	const float rocketKnockbackStrength = 1250.0f;
	const float rocketJumpMinUpDirScalar = 0.5f;
	bool shouldApplyRocketKnockback = false;

	// Rotation
	bool shouldRotate = false;
	DE::Vector2i rotationDelta;
	const float rotationSpeed = 12.0f;
	const float maxPitch = 90.0f;
	const float minPitch = -90.0f;

	// Movement input
	DE::Vector2f moveInputDirection;
	bool shouldMove = false;

	// Sprinting
	bool shouldSprint = false;
	bool isSprinting = false;
	CU::CountupTimer timeSprinting;

	// Ground sliding
	CU::CountupTimer timeGroundSliding;
	bool shouldGroundSlide = false;
	bool gaveSlideInputLastFrame = false;

	// Jumping
	bool shouldJump = false;
	bool hasDoubleJumpedSinceGrounded = false;
	const float coyoteTime = 0.25;
	DE::Vector3f rocketKnockbackForce;

	// Grappling hook input
	bool shouldUseGrapplingHook = false;

	// Shooting input
	bool shouldShoot = false;
	bool shootIsHeld = false;
	bool shootIsReleased = false;

	// Ammo swap input
	bool shouldSwapAmmo = false;
	int swapAmmoValue = 0;
};