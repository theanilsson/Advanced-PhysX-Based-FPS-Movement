#include "Player.h"
#include "HealthComponent.h"
#include "RigidBodyComponent.h"
#include "Gun.h"
#include "Message.h"
#include "MainSingleton.h"
#include <DreamEngine/engine.h>
#include <DreamEngine/graphics/ModelFactory.h>
#include <DreamEngine/graphics/GraphicsEngine.h>
#include <DreamEngine/utilities/UtilityFunctions.h>

Player::~Player()
{
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerLookAround, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerMove, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerSprint, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerJump, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerGroundSlide, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerUseGrapplingHook, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerShoot, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerShootReleased, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerAmmoSwap, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerSelectBeam, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerSelectRocket, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerSelectGoo, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerReload, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerInteract, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerTakeDamage, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::PlayerTriggerHeal, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::RocketExplode, this);
	MainSingleton::GetInstance()->GetPostMaster().Unsubscribe(eMessageType::WaveNumberChanged, this);
}

void Player::Init()
{
	SetUpComponents();
	SetUpSubscriptions();
	SetUpModel();
	InitializeMemberVariables();
	myGun.Init(myTransform);

	myStats.healingTimer.SetResetValue(3.0f);
	myStats.iFramesTimer.SetResetValue(3.0f);
}

void Player::InitParticleSystem()
{
	myGun.InitParticleSystem();
}

std::vector<ParticleSystem3D*> Player::GetGunParticleSystems()
{
	return myGun.GetParticleSystems();
}

void Player::SetUpComponents()
{
	AddComponent<HealthComponent>();
	HealthComponent* healthCmp = GetComponent<HealthComponent>();
	healthCmp->SetMaxHealth(100);
	healthCmp->SetCurrentHealth(100);

	AddComponent<RigidBodyComponent>();
	physx::PxShape* shape = DE::Engine::GetPhysXPhysics()->createShape(physx::PxCapsuleGeometry(30.0f, 35.0f), *MainSingleton::GetInstance()->GetPhysXMaterials()[0]);
	MainSingleton::GetInstance()->GetCollisionFiltering().setupFiltering(shape, CollisionFiltering::Player, CollisionFiltering::Environment | CollisionFiltering::Enemy | CollisionFiltering::DefensePoint);
	physx::PxVec3 initialPosition = physx::PxVec3(myTransform.GetPosition().x, myTransform.GetPosition().y + myStats.physicsBodyVerticalOffset, myTransform.GetPosition().z);
	physx::PxRigidDynamic* body = DE::Engine::GetPhysXPhysics()->createRigidDynamic(physx::PxTransform(initialPosition, physx::PxQuat(physx::PxPi / 2.0f, physx::PxVec3(0.0f, 0.0f, 1.0f))));
	myStats.previousFramePosition = initialPosition;
	body->setName("Player");
	body->setRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y | physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z);
	body->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
	body->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
	body->setActorFlag(physx::PxActorFlag::eVISUALIZATION, true);
	body->setActorFlag(physx::PxActorFlag::eSEND_SLEEP_NOTIFIES, true);
	body->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
	shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	body->attachShape(*shape);
	physx::PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	MainSingleton::GetInstance()->GetPhysXScene()->addActor(*body);
	GetComponent<RigidBodyComponent>()->SetBody(body);
}

void Player::SetUpSubscriptions()
{
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerLookAround, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerMove, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerSprint, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerJump, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerGroundSlide, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerUseGrapplingHook, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerShoot, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerShootReleased, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerAmmoSwap, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerSelectBeam, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerSelectRocket, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerSelectGoo, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerReload, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerInteract, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerTakeDamage, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::PlayerTriggerHeal, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::RocketExplode, this);
	MainSingleton::GetInstance()->GetPostMaster().Subscribe(eMessageType::WaveNumberChanged, this);
}

void Player::SetUpModel()
{
	myGrapplingHookCylinder = DE::ModelFactory::GetInstance().GetModelInstance(L"3D/grapplingHookCylinder.fbx");
}

void Player::InitializeMemberVariables()
{
	myStats.respawnTimer = CU::CountdownTimer(5.0f);
	myStats.timeSprinting = CU::CountupTimer(0.25f);
	myStats.timeGroundSliding = CU::CountupTimer(0.15f);
	myStats.grapplingCooldown = CU::CountdownTimer(5.0f);
}

void Player::Update(const float aDeltaTime)
{
	physx::PxRigidDynamic* body = static_cast<physx::PxRigidDynamic*>(GetComponent<RigidBodyComponent>()->GetBody());
	HandleInitialFrameState(aDeltaTime, body);

	if (aDeltaTime > 0.0f && MainSingleton::GetInstance()->GetWasActiveWindowLastFrame())
	{
		UpdateCamera();

		CheckIfInGrapplingRange(body);

		if (myStats.respawnTimer.IsDone())
		{
			const bool isGrounded = CheckIfGrounded();

			if (myStats.shouldUseGrapplingHook && !myStats.isGrappleActive && myStats.grapplingCooldown.IsDone() && myStats.isInGrapplingRange)
			{
				ActivateGrapplingHook();
			}
			else if (myStats.shouldUseGrapplingHook && myStats.isGrappleActive)
			{
				DeactivateGrapplingHook();
			}

			if (myStats.isGrappleActive)
			{
				if (myStats.isSprinting)
				{
					myStats.isSprinting = false;
				}
				myStats.timeGroundSliding.SetCurrentValue(myStats.timeGroundSliding.GetCurrentValue() - aDeltaTime);
				if (myStats.timeGroundSliding.GetCurrentValue() < 0.0f)
				{
					myStats.timeGroundSliding.SetCurrentValue(0.0f);
				}
				myStats.grapplingDuration.Update(aDeltaTime);
				UpdateGrapplingHookVisualization();
				ApplyGrapplingHookForces(body);
				if (isGrounded)
				{
					myStats.hasDoubleJumpedSinceGrounded = false;
					myStats.airTime.Reset();
					if (myStats.shouldJump)
					{
						TriggerGrapplingHookJump(body);
					}
				}
				else if (!isGrounded)
				{
					myStats.airTime.Update(aDeltaTime);
				}
			}
			else
			{
				// Friction & related flags
				if (isGrounded)
				{
					myStats.airTime.Reset();
					myStats.hasDoubleJumpedSinceGrounded = false;
					ApplyCustomGroundedFriction(body, aDeltaTime);
				}
				else
				{
					myStats.airTime.Update(aDeltaTime);
				}

				// WASD or Leftstick movement
				if (isGrounded && myStats.shouldMove)
				{
					if (myStats.shouldGroundSlide && !myStats.gaveSlideInputLastFrame)
					{
						ApplyGroundSlideBoost(body);
					}
					ApplyGroundedMovement(body);
				}
				else if (isGrounded)
				{
					if (myStats.isSprinting)
					{
						myStats.isSprinting = false;
					}
				}
				else if (myStats.shouldMove)
				{
					ApplyMidAirMovement(body);
				}
				else if (myStats.isSprinting)
				{
					myStats.isSprinting = false;
				}

				// Calculate sprint time for FOV lerp
				if (myStats.isSprinting)
				{
					myStats.timeSprinting.Update(aDeltaTime);
					if (myStats.timeSprinting.ReachedThreshold())
					{
						myStats.timeSprinting.SetCurrentToThreshold();
					}
				}
				else
				{
					myStats.timeSprinting.SetCurrentValue(myStats.timeSprinting.GetCurrentValue() - aDeltaTime);
					if (myStats.timeSprinting.GetCurrentValue() < 0.0f)
					{
						myStats.timeSprinting.SetCurrentValue(0.0f);
					}
				}

				// Calculate ground sliding time for camera vertical offset lerp
				if (myStats.shouldGroundSlide && isGrounded)
				{
					if (body->getLinearVelocity().y < -100.0f) myStats.timeGroundSliding.SetCurrentToThreshold();
					myStats.timeGroundSliding.Update(aDeltaTime);
					if (myStats.timeGroundSliding.ReachedThreshold())
					{
						myStats.timeGroundSliding.SetCurrentToThreshold();
					}
				}
				else
				{
					myStats.timeGroundSliding.SetCurrentValue(myStats.timeGroundSliding.GetCurrentValue() - aDeltaTime);
					if (myStats.timeGroundSliding.GetCurrentValue() < 0.0f)
					{
						myStats.timeGroundSliding.SetCurrentValue(0.0f);
					}
				}

				// Jumping
				if (isGrounded && myStats.shouldJump || myStats.shouldJump && myStats.airTime.GetCurrentValue() <= myStats.coyoteTime)
				{
					TriggerJump(body, false);
				}
				else if (!isGrounded && myStats.shouldJump && !myStats.hasDoubleJumpedSinceGrounded)
				{
					TriggerJump(body, true);
				}
				if (myStats.shouldApplyRocketKnockback)
				{
					TriggerRocketKnockback(body);
				}
			}

			if (myStats.shouldRotate)
			{
				UpdateCameraRotation(aDeltaTime);
			}

			myGun.Update(*this, aDeltaTime);
			if (myStats.shouldShoot)
			{
				myGun.Shoot(myCamera->GetTransform().GetMatrix().GetForward(), myTransform);
			}
			if (myStats.shouldSwapAmmo && !myStats.shootIsHeld)
			{
				myGun.SwapAmmoType(myStats.swapAmmoValue);
			}
		}
		else // Dead
		{
			body->setLinearVelocity(physx::PxVec3(0.0f));
			myGun.IdleUpdate();
			myStats.respawnTimer.Update(aDeltaTime);
			if (myStats.respawnTimer.IsDone())
			{
				Respawn(body);
			}
		}
	}

	ResetConditionalFlags(); 
	UpdateTimers(aDeltaTime);
}

void Player::Receive(const Message& aMessage)
{
	switch (aMessage.messageType)
	{
		case eMessageType::PlayerLookAround:
		{
			if (MainSingleton::GetInstance()->GetWasActiveWindowLastFrame() && GetForegroundWindow() == *DE::Engine::GetInstance()->GetHWND())
			{
				myStats.shouldRotate = true;
				myStats.rotationDelta = *static_cast<DE::Vector2i*>(aMessage.messageData);
			}
			break;
		}
		case eMessageType::PlayerMove:
		{
			myStats.shouldMove = true;
			DE::Vector2i moveDir = *static_cast<DE::Vector2i*>(aMessage.messageData);
			myStats.moveInputDirection = DE::Vector2f(static_cast<float>(moveDir.x) / 100.0f, static_cast<float>(moveDir.y) / 100.0f);
			break;
		}
		case eMessageType::PlayerSprint:
		{
			myStats.shouldSprint = true;
			break;
		}
		case eMessageType::PlayerJump:
		{
			myStats.shouldJump = true;
			break;
		}
		case eMessageType::PlayerGroundSlide:
		{
			myStats.shouldGroundSlide = true;
			break;
		}
		case eMessageType::PlayerUseGrapplingHook:
		{
			myStats.shouldUseGrapplingHook = true;
			break;
		}
		case eMessageType::PlayerShoot:
		{
			static_cast<DE::Vector2i*>(aMessage.messageData)->x == 0 ? myStats.shootIsHeld = false : myStats.shootIsHeld = true;
			myStats.shouldShoot = true;
			break;
		}
		case eMessageType::PlayerShootReleased:
		{
			myStats.shootIsReleased = true;
			break;
		}
		case eMessageType::PlayerAmmoSwap:
		{
			myStats.swapAmmoValue = static_cast<DE::Vector2i*>(aMessage.messageData)->x;
			myStats.shouldSwapAmmo = true;
			break;
		}
		case eMessageType::PlayerSelectBeam:
		{
			myGun.SetCurrentAmmoType(eAmmoType::Beam);
			break;
		}
		case eMessageType::PlayerSelectRocket:
		{
			myGun.SetCurrentAmmoType(eAmmoType::Rocket);
			break;
		}
		case eMessageType::PlayerSelectGoo:
		{
			myGun.SetCurrentAmmoType(eAmmoType::Goo);
			break;
		}
		case eMessageType::PlayerReload:
		{
			myGun.ManualReload();
			break;
		}
		case eMessageType::PlayerInteract:
		{
			CheckForInteractables();
			break;
		}
		case eMessageType::PlayerTakeDamage:
		{
			myStats.iFramesTimer.Reset();
			myStats.TakeDamage(this, *static_cast<int*>(aMessage.messageData));

			break;
		}		
		case eMessageType::RocketExplode:
		{
			CalculateRocketKnockback(static_cast<std::pair<physx::PxRigidActor*, int>*>(aMessage.messageData));
			break;
		}
		case eMessageType::PlayerTriggerHeal:
		{
			myStats.HealToMax(this);
			myStats.healingTimer.Reset();
			break;
		}
		case eMessageType::WaveNumberChanged:
		{
			if (*static_cast<int*>(aMessage.messageData) > 1)
			{
				myStats.currentMoneyCurrency += myStats.moneyCurrencyReward;
				MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &myStats.currentMoneyCurrency, eMessageType::PlayerCurrencyChange });
				MainSingleton::GetInstance()->GetAudioManager().StopAudio(eAudioEvent::NextWaveIncoming);
				MainSingleton::GetInstance()->GetAudioManager().PlayAudio(eAudioEvent::NextWaveIncoming, myTransform.GetPosition());
			}
			break;
		}
	}
}

void Player::Respawn(physx::PxRigidDynamic* aPhysicsBodyPtr)
{
	physx::PxRigidDynamic* body = static_cast<physx::PxRigidDynamic*>(GetComponent<RigidBodyComponent>()->GetBody());
	body->setGlobalPose(physx::PxTransform(myStats.spawnPoint, body->getGlobalPose().q));
	body->setLinearVelocity(physx::PxVec3(0.0f));
	myTransform.SetPosition(DE::Vector3f(myStats.spawnPoint.x, myStats.spawnPoint.y, myStats.spawnPoint.z));

	myStats.HealToMax(this);
	myStats.healingTimer.Reset();
	myStats.isGrappleActive = false;
	myStats.grappledObject = nullptr;

	MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ nullptr, eMessageType::PlayerRespawned });

	myGun.Reset();
}

const PlayerStats& Player::GetPlayerStats() const
{
	return myStats;
}

Gun& Player::GetGun()
{
	return myGun;
}

float Player::GetIframesTimerValue()
{
	return myStats.iFramesTimer.GetCurrentValue();
}

float Player::GetHealingTimerValue()
{
	return myStats.healingTimer.GetCurrentValue();
}

void Player::HandleInitialFrameState(const float aDeltaTime, physx::PxRigidDynamic* aPhysicsBodyPtr)
{
	if (aDeltaTime == 0.0f || !MainSingleton::GetInstance()->GetWasActiveWindowLastFrame())
	{
		aPhysicsBodyPtr->setGlobalPose(physx::PxTransform(myStats.previousFramePosition, aPhysicsBodyPtr->getGlobalPose().q));
		aPhysicsBodyPtr->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
		myGun.IdleUpdate();
	}
	else
	{
		aPhysicsBodyPtr->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
	}

	// Out of bounds safety check
	if (aPhysicsBodyPtr->getGlobalPose().p.y < myStats.spawnPoint.y - 5000.0f) aPhysicsBodyPtr->setGlobalPose(physx::PxTransform(myStats.spawnPoint, aPhysicsBodyPtr->getGlobalPose().q));
	
	myTransform.SetPosition(DE::Vector3f(aPhysicsBodyPtr->getGlobalPose().p.x, aPhysicsBodyPtr->getGlobalPose().p.y - myStats.physicsBodyVerticalOffset, aPhysicsBodyPtr->getGlobalPose().p.z));
	myStats.previousFramePosition = aPhysicsBodyPtr->getGlobalPose().p;
}

void Player::UpdateCamera()
{
	if (myCamera == nullptr)
	{
		myCamera = MainSingleton::GetInstance()->GetActiveCamera();
	}
	if (myStats.nearPlane == 10.0f)
	{
		myCamera->GetProjectionPlanes(myStats.nearPlane, myStats.farPlane);
		myStats.spawnPoint = physx::PxVec3(myTransform.GetPosition().x, myTransform.GetPosition().y + myStats.physicsBodyVerticalOffset, myTransform.GetPosition().z);
	}

	const float groundSlideLerpPercent = CommonUtilities::Clamp(myStats.timeGroundSliding.GetCurrentValue() / myStats.timeGroundSliding.GetThresholdValue(), 0.0f, 1.0f);
	const float cameraVerticalOffsetResult = CommonUtilities::Lerp(myStats.cameraVerticalOffset, myStats.groundSlideCameraVerticalOffset, groundSlideLerpPercent);
	myCamera->SetPosition(myTransform.GetPosition() + DE::Vector3f(0.0f, cameraVerticalOffsetResult, 0.0f));

	const float fovSprintLerpPercent = CommonUtilities::Clamp(myStats.timeSprinting.GetCurrentValue() / myStats.timeSprinting.GetThresholdValue(), 0.0f, 1.0f);
	const float fovResult = CommonUtilities::Lerp(MainSingleton::GetInstance()->GetFOV(), MainSingleton::GetInstance()->GetFOV() + 15.0f, fovSprintLerpPercent);
	DE::Vector2f resolution = DE::Vector2f(static_cast<float>(MainSingleton::GetInstance()->GetResolution().x), static_cast<float>(MainSingleton::GetInstance()->GetResolution().y));
	myCamera->SetPerspectiveProjection(fovResult, resolution, myStats.nearPlane, myStats.farPlane);
}

void Player::CalculateRocketKnockback(std::pair<physx::PxRigidActor*, int>* aRocketBodyAndDamagePair)
{
	const DE::Vector3f convertedExplosionPosition(aRocketBodyAndDamagePair->first->getGlobalPose().p.x, aRocketBodyAndDamagePair->first->getGlobalPose().p.y, aRocketBodyAndDamagePair->first->getGlobalPose().p.z);
	const DE::Vector3f explosionVector = (myTransform.GetPosition() - convertedExplosionPosition);
	const float distanceSquared = explosionVector.LengthSqr();

	if (distanceSquared <= myGun.GetRocketExplosionSize())
	{
		const float distanceFactor = 1.0f - (distanceSquared / myGun.GetRocketExplosionSize());
		myStats.rocketKnockbackForce = explosionVector.GetNormalized() * myStats.rocketKnockbackStrength * distanceFactor;
		myStats.shouldApplyRocketKnockback = true;

		myStats.TakeDamage(this, aRocketBodyAndDamagePair->second * distanceFactor);
		myStats.iFramesTimer.Reset();
	}
}

void Player::CheckForInteractables()
{
	const DE::Vector3f forward = myCamera->GetTransform().GetMatrix().GetForward();
	const physx::PxVec3 direction(forward.x, forward.y, forward.z);
	physx::PxQueryFilterData queryFilterData;
	queryFilterData.data.word0 = MainSingleton::GetInstance()->GetCollisionFiltering().Station;
	physx::PxRaycastBuffer hitBuffer;

	if (MainSingleton::GetInstance()->GetPhysXScene()->raycast(GetComponent<RigidBodyComponent>()->GetBody()->getGlobalPose().p, direction, myStats.maximumInteractRange, hitBuffer, physx::PxHitFlag::eDEFAULT, queryFilterData))
	{
		if (hitBuffer.block.actor->getName() == nullptr) return;

		const bool canUpgradeBeam = myGun.GetAmmoUpgradeLevel(eAmmoType::Beam) == 1 && myStats.currentMoneyCurrency >= myStats.firstAmmoUpgradeCost
			|| myGun.GetAmmoUpgradeLevel(eAmmoType::Beam) == 2 && myStats.currentMoneyCurrency >= myStats.secondAmmoUpgradeCost;

		const bool canUpgradeRocket = myGun.GetAmmoUpgradeLevel(eAmmoType::Rocket) == 1 && myStats.currentMoneyCurrency >= myStats.firstAmmoUpgradeCost
			|| myGun.GetAmmoUpgradeLevel(eAmmoType::Rocket) == 2 && myStats.currentMoneyCurrency >= myStats.secondAmmoUpgradeCost;

		const bool canUpgradeGoo = myGun.GetAmmoUpgradeLevel(eAmmoType::Goo) == 1 && myStats.currentMoneyCurrency >= myStats.firstAmmoUpgradeCost
			|| myGun.GetAmmoUpgradeLevel(eAmmoType::Goo) == 2 && myStats.currentMoneyCurrency >= myStats.secondAmmoUpgradeCost;

		if (canUpgradeBeam && strcmp(hitBuffer.block.actor->getName(), "UpgradeBeam") == 0)
		{
			const int upgradeCost = myGun.GetAmmoUpgradeLevel(eAmmoType::Beam) == 1 ? myStats.firstAmmoUpgradeCost : myStats.secondAmmoUpgradeCost;
			myGun.UpgradeBeamAmmo();
			myStats.currentMoneyCurrency -= upgradeCost;
			MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &myStats.currentMoneyCurrency, eMessageType::PlayerCurrencyChange });
		}
		else if (canUpgradeRocket && strcmp(hitBuffer.block.actor->getName(), "UpgradeRocket") == 0)
		{
			const int upgradeCost = myGun.GetAmmoUpgradeLevel(eAmmoType::Rocket) == 1 ? myStats.firstAmmoUpgradeCost : myStats.secondAmmoUpgradeCost;
			myGun.UpgradeRocketAmmo();
			myStats.currentMoneyCurrency -= upgradeCost;
			MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &myStats.currentMoneyCurrency, eMessageType::PlayerCurrencyChange });
		}
		else if (canUpgradeGoo && strcmp(hitBuffer.block.actor->getName(), "UpgradeGoo") == 0)
		{
			const int upgradeCost = myGun.GetAmmoUpgradeLevel(eAmmoType::Goo) == 1 ? myStats.firstAmmoUpgradeCost : myStats.secondAmmoUpgradeCost;
			myGun.UpgradeGooAmmo();
			myStats.currentMoneyCurrency -= upgradeCost;
			MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &myStats.currentMoneyCurrency, eMessageType::PlayerCurrencyChange });
		}
		else if (strcmp(hitBuffer.block.actor->getName(), "HealingStation") == 0)
		{
			myStats.HealToMax(this);
			myStats.healingTimer.Reset();
		}
	}
}

void Player::CheckIfInGrapplingRange(physx::PxRigidDynamic* aPhysicsBodyPtr)
{
	const physx::PxVec3 origin = aPhysicsBodyPtr->getGlobalPose().p + physx::PxVec3(0.0f, myStats.physicsBodyVerticalOffset, 0.0f);
	const DE::Vector3f forward = myCamera->GetTransform().GetMatrix().GetForward();
	const physx::PxVec3 direction(forward.x, forward.y, forward.z);
	physx::PxQueryFilterData queryFilterData;
	queryFilterData.data.word0 = MainSingleton::GetInstance()->GetCollisionFiltering().Environment
		| MainSingleton::GetInstance()->GetCollisionFiltering().Enemy
		| MainSingleton::GetInstance()->GetCollisionFiltering().Companion
		| MainSingleton::GetInstance()->GetCollisionFiltering().Rocket
		| MainSingleton::GetInstance()->GetCollisionFiltering().DefensePoint;

	myStats.isInGrapplingRange = MainSingleton::GetInstance()->GetPhysXScene()->raycast(origin, direction, myStats.maxGrapplingDistance, myStats.grapplingHitBuffer, physx::PxHitFlag::ePOSITION, queryFilterData);
	MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &myStats.isInGrapplingRange, eMessageType::SetCrosshairState });
}

bool Player::CheckIfGrounded()
{
	const physx::PxVec3 direction(0.0f, -1.0f, 0.0f);
	physx::PxQueryFilterData queryFilterData;
	queryFilterData.data.word0 = MainSingleton::GetInstance()->GetCollisionFiltering().Environment;
	physx::PxRaycastBuffer hitBuffer;

	if (MainSingleton::GetInstance()->GetPhysXScene()->raycast(GetComponent<RigidBodyComponent>()->GetBody()->getGlobalPose().p, direction, myStats.groundedTraceLength, hitBuffer, physx::PxHitFlag::eNORMAL, queryFilterData))
	{
		myStats.groundedDotProduct = hitBuffer.block.normal.dot(direction);
 		if (myStats.groundedDotProduct < -0.8f)
		{
			myStats.groundedNormal = hitBuffer.block.normal;
			return true;
		}
	}
	myStats.groundedDotProduct = 0.0f;
	return false;
}

void Player::ActivateGrapplingHook()
{
	myGun.PlayGrappleActivationAnimation();
	if (strcmp(myStats.grapplingHitBuffer.block.actor->getName(), "Enemy") == 0 || strcmp(myStats.grapplingHitBuffer.block.actor->getName(), "Companion") == 0 || strcmp(myStats.grapplingHitBuffer.block.actor->getName(), "Rocket") == 0)
	{
		myStats.grappledObject = myStats.grapplingHitBuffer.block.actor;
		myStats.grapplingPoint = myStats.grapplingHitBuffer.block.actor->getGlobalPose().p;
	}
	else
	{
		myStats.grappledObject = nullptr;
		myStats.grapplingPoint = myStats.grapplingHitBuffer.block.position;
	}
	myStats.isGrappleActive = true;
	myStats.grapplingCooldown.Reset();
	bool deactivationMessageData = myStats.grapplingCooldown.IsDone();
	MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &deactivationMessageData, eMessageType::PlayerGrappleCooldownStateChanged });
	MainSingleton::GetInstance()->GetAudioManager().StopAudio(eAudioEvent::PlayerActivateGrapple);
	MainSingleton::GetInstance()->GetAudioManager().PlayAudio(eAudioEvent::PlayerActivateGrapple, myTransform.GetPosition());
}

void Player::DeactivateGrapplingHook()
{
	myStats.isGrappleActive = false;
	myStats.grapplingDuration.Reset();
	myStats.grappledObject = nullptr;
	myStats.jumpedDuringGrapple = false;
}

void Player::UpdateGrapplingHookVisualization()
{
	const DE::Matrix4x4f cameraMatrix = myCamera->GetTransform().GetMatrix();
	const DE::Vector3f grappleHandPosition = cameraMatrix.GetPosition() + cameraMatrix.GetUp() * -1.0f * 10.0f + cameraMatrix.GetForward() * 10.0f * -1.0f + cameraMatrix.GetRight() * -1.0f * 15.0f;
	DE::Vector3f grapplingVector = DE::Vector3f(myStats.grapplingPoint.x, myStats.grapplingPoint.y, myStats.grapplingPoint.z) - grappleHandPosition;
	myGrapplingHookCylinder.SetLocation(grappleHandPosition + grapplingVector / 2.0f);
	grapplingVector = grapplingVector.GetNormalized() * (grapplingVector.Length() * myStats.grapplingVisualizationLength);
	
	const DE::Vector3f up = grapplingVector.GetNormalized();
	DE::Vector3f forward = DE::Vector3f(0.0f, 0.0f, 1.0f);
	forward = (forward - forward.Dot(up) * up).GetNormalized();
	const DE::Vector3f right = up.Cross(forward);
	const DE::Quatf calculatedQuaternion = DE::Quatf::CreateFromOrthonormalBasisVectors(right, up, forward);
	const DE::Vector3f calculatedRotation = calculatedQuaternion.GetEulerAnglesDegrees();

	myGrapplingHookCylinder.SetRotation(calculatedRotation);
	myGrapplingHookCylinder.SetScale(DE::Vector3f(0.01f, grapplingVector.Length() / 2.0f, 0.01f));
}

void Player::ApplyGrapplingHookForces(physx::PxRigidDynamic* aPhysicsBodyPtr)
{
	const physx::PxVec3 currentVelocity = aPhysicsBodyPtr->getLinearVelocity();
	if (myStats.grappledObject != nullptr) myStats.grapplingPoint = myStats.grappledObject->getGlobalPose().p;
	const physx::PxVec3 grapplingVector = myStats.grapplingPoint - aPhysicsBodyPtr->getGlobalPose().p;
	const physx::PxVec3 pullDirection = grapplingVector.getNormalized() * 0.75f + currentVelocity.getNormalized() * 0.25f;
	const float elapsedTimeScalar = UtilityFunctions::Clamp(myStats.grapplingDuration.GetCurrentValue() / myStats.grapplingDuration.GetThresholdValue(), 0.0f, 1.0f);
	const float pullStrength = UtilityFunctions::Lerp(0.0f, myStats.maxAirVelocity, elapsedTimeScalar);


	// Spring dampening effect
	const physx::PxVec3 pullForce = myStats.grapplingSpringStiffness * pullDirection * pullStrength;
	const physx::PxVec3 springDampeningForce = myStats.grapplingSpringDampeningFactor * currentVelocity;
	physx::PxVec3 totalSpringForce = pullForce + springDampeningForce;
	totalSpringForce.y = UtilityFunctions::Max(totalSpringForce.y, 0.0f);
	aPhysicsBodyPtr->addForce(totalSpringForce, physx::PxForceMode::eACCELERATION);


	// Swing effect
	const float velocityAlongPull = currentVelocity.dot(pullDirection);
	const physx::PxVec3 tangentialVelocity = currentVelocity - pullDirection * velocityAlongPull;
	physx::PxVec3 totalSwingForce = pullDirection * velocityAlongPull + tangentialVelocity * myStats.swingForceScalar;
	totalSwingForce.y = UtilityFunctions::Max(totalSwingForce.y, 0.0f);
	aPhysicsBodyPtr->addForce(totalSwingForce, physx::PxForceMode::eFORCE);


	// Allow some directional control to fight against the pull force
	if (myStats.shouldMove)
	{
		const float moveSpeed = UtilityFunctions::Min(myStats.grapplingSpeedControlMax, myStats.directionalGrapplingSpeed / (UtilityFunctions::Abs(myStats.moveInputDirection.x) + UtilityFunctions::Abs(myStats.moveInputDirection.y)));
		const DE::Matrix4x4f matrix = myTransform.GetMatrix();
		const DE::Vector3f forward = matrix.GetForward() * myStats.moveInputDirection.y;
		const DE::Vector3f right = matrix.GetRight() * myStats.moveInputDirection.x;
		const physx::PxVec3 desiredVelocity = physx::PxVec3(forward.x, forward.y, forward.z) * moveSpeed * 0.5f + physx::PxVec3(right.x, right.y, right.z) * moveSpeed;
		const physx::PxVec3 moveDampeningForce = myStats.grapplingMoveDampeningFactor * currentVelocity;
		const physx::PxVec3 totalMoveForce = desiredVelocity + moveDampeningForce;
		aPhysicsBodyPtr->addForce(totalMoveForce, physx::PxForceMode::eACCELERATION);
	}


	// Calculate whether the grapple line should snap
	if (grapplingVector.magnitudeSquared() < myStats.grapplingPointCloseSnapDistance * myStats.grapplingPointCloseSnapDistance
		|| aPhysicsBodyPtr->getLinearVelocity().getNormalized().dot(pullDirection) < 0.5f && myStats.grapplingDuration.GetCurrentValue() > 0.75f)
	{
		myStats.isGrappleActive = false;
		myStats.grappledObject = nullptr;
		myStats.grapplingDuration.Reset();
		myStats.jumpedDuringGrapple = false;
	}
}

void Player::TriggerGrapplingHookJump(physx::PxRigidDynamic* aPhysicsBodyPtr)
{
	aPhysicsBodyPtr->setLinearVelocity(physx::PxVec3(aPhysicsBodyPtr->getLinearVelocity().x, myStats.jumpForce * 1.6f, aPhysicsBodyPtr->getLinearVelocity().z));
	myStats.jumpedDuringGrapple = true;
}

void Player::ApplyCustomGroundedFriction(physx::PxRigidDynamic* aPhysicsBodyPtr, float aDeltaTime)
{
	const physx::PxVec3 currentVelocity = aPhysicsBodyPtr->getLinearVelocity();
	physx::PxVec3 horizontalVelocity(currentVelocity.x, 0.0f, currentVelocity.z);
	if (myStats.shouldGroundSlide)
	{
		const physx::PxVec3 projectedGravity = myStats.groundSlidingGravityVector - myStats.groundedNormal * (myStats.groundSlidingGravityVector.dot(myStats.groundedNormal));
		const physx::PxVec3 additionalSlideForce = projectedGravity * aPhysicsBodyPtr->getMass();
		aPhysicsBodyPtr->addForce(additionalSlideForce, physx::PxForceMode::eFORCE);

		float frictionFactor = myStats.groundSlidingFriction * (UtilityFunctions::Abs(myStats.groundedDotProduct) * 0.1f);
		horizontalVelocity *= pow(frictionFactor, aDeltaTime);
		aPhysicsBodyPtr->setLinearVelocity(physx::PxVec3(horizontalVelocity.x, currentVelocity.y, horizontalVelocity.z));
	}
	else
	{
		const physx::PxVec3 projectedGravity = myStats.groundedGravityVector - myStats.groundedNormal * (myStats.groundedGravityVector.dot(myStats.groundedNormal));
		const physx::PxVec3 antiSlideForce = projectedGravity * aPhysicsBodyPtr->getMass() * -1.0f;
		aPhysicsBodyPtr->addForce(antiSlideForce, physx::PxForceMode::eFORCE);

		float frictionFactor = myStats.groundedFriction * (UtilityFunctions::Abs(myStats.groundedDotProduct) * 0.1f);
		horizontalVelocity *= pow(frictionFactor, aDeltaTime);
		if (horizontalVelocity.magnitudeSquared() < myStats.groundedFullStopDistance * myStats.groundedFullStopDistance)
		{
			horizontalVelocity = physx::PxVec3(0.0f);
		}
		aPhysicsBodyPtr->setLinearVelocity(physx::PxVec3(horizontalVelocity.x, currentVelocity.y, horizontalVelocity.z));
	}
}

void Player::ApplyGroundedMovement(physx::PxRigidDynamic* aPhysicsBodyPtr)
{
	if (myStats.shouldGroundSlide)
	{
		if (myStats.shouldSprint) myStats.isSprinting = false;
		if (myStats.moveInputDirection.y <= 0.0f) myStats.isSprinting = false;

		const DE::Matrix4x4f matrix = myTransform.GetMatrix();
		const DE::Vector3f forward = matrix.GetForward() * myStats.moveInputDirection.y;
		const DE::Vector3f right = matrix.GetRight() * myStats.moveInputDirection.x;
		const DE::Vector3f combinedControlDirection = forward + right;
		physx::PxVec3 horizontalVelocity(aPhysicsBodyPtr->getLinearVelocity().x, 0.0f, aPhysicsBodyPtr->getLinearVelocity().z);
		const float minimumSlidingSpeed = UtilityFunctions::Min(myStats.slidingSpeedControlMax, myStats.directionalSlidingSpeed / (UtilityFunctions::Abs(myStats.moveInputDirection.x) + UtilityFunctions::Abs(myStats.moveInputDirection.y)));
		const float horizontalVelocityLength = horizontalVelocity.magnitude();
		horizontalVelocity += physx::PxVec3(combinedControlDirection.x, 0.0f, combinedControlDirection.z) * 5.0f;
		horizontalVelocity.normalize();
		horizontalVelocity *= UtilityFunctions::Max(horizontalVelocityLength, minimumSlidingSpeed);

		aPhysicsBodyPtr->setLinearVelocity(physx::PxVec3(horizontalVelocity.x, aPhysicsBodyPtr->getLinearVelocity().y, horizontalVelocity.z));
	}
	else 
	{
		if (myStats.shouldSprint && !myStats.isSprinting) myStats.isSprinting = true;
		else if (myStats.shouldSprint) myStats.isSprinting = false;
		if (myStats.moveInputDirection.y <= 0.0f) myStats.isSprinting = false;

		const DE::Matrix4x4f matrix = myTransform.GetMatrix();
		const DE::Vector3f forward = matrix.GetForward() * myStats.moveInputDirection.y;
		const DE::Vector3f right = matrix.GetRight() * myStats.moveInputDirection.x;
		const float directionalSpeed = myStats.isSprinting ? myStats.directionalSprintSpeed : myStats.directionalWalkingSpeed;
		const float directionalSpeedMax = myStats.isSprinting ? myStats.sprintSpeedControlMax : myStats.walkingSpeedControlMax;
		const float moveSpeed = UtilityFunctions::Min(directionalSpeedMax, directionalSpeed / (UtilityFunctions::Abs(myStats.moveInputDirection.x) + UtilityFunctions::Abs(myStats.moveInputDirection.y)));
		const DE::Vector3f speed = forward * moveSpeed + right * moveSpeed;

		aPhysicsBodyPtr->setLinearVelocity(physx::PxVec3(speed.x, aPhysicsBodyPtr->getLinearVelocity().y, speed.z));
	}
}

void Player::ApplyGroundSlideBoost(physx::PxRigidDynamic* aPhysicsBodyPtr)
{
	const physx::PxVec3 currentVelocity = aPhysicsBodyPtr->getLinearVelocity();
	physx::PxVec3 horizontalVelocity(currentVelocity.x, 0.0f, currentVelocity.z);
	horizontalVelocity *= myStats.groundSlideBoostFactor;
	aPhysicsBodyPtr->setLinearVelocity(physx::PxVec3(horizontalVelocity.x, currentVelocity.y, horizontalVelocity.z));
}

void Player::ApplyMidAirMovement(physx::PxRigidDynamic* aPhysicsBodyPtr)
{
	if (myStats.moveInputDirection.y <= 0.0f) myStats.isSprinting = false;

	const float moveSpeed = UtilityFunctions::Min(myStats.airControlSpeedMax, myStats.directionalAirSpeed / (UtilityFunctions::Abs(myStats.moveInputDirection.x) + UtilityFunctions::Abs(myStats.moveInputDirection.y)));
	
	const DE::Matrix4x4f matrix = myTransform.GetMatrix();
	const DE::Vector3f forward = matrix.GetForward() * myStats.moveInputDirection.y;
	const DE::Vector3f right = matrix.GetRight() * myStats.moveInputDirection.x;

	const physx::PxVec3 currentVelocity = aPhysicsBodyPtr->getLinearVelocity();
	const physx::PxVec3 desiredVelocity = physx::PxVec3(forward.x, forward.y, forward.z) * moveSpeed + physx::PxVec3(right.x, right.y, right.z) * moveSpeed;
	const float directionalDampeningScalar = CU::Abs(CU::Clamp(currentVelocity.getNormalized().dot(desiredVelocity.getNormalized()), 0.0f, 1.0f) - 1.0f);

	float elapsedTimeScalar = myStats.airTime.GetCurrentValue() / myStats.airTime.GetThresholdValue();
	elapsedTimeScalar = UtilityFunctions::Clamp(elapsedTimeScalar, 0.0f, 1.0f);
	const physx::PxVec3 momentumBasedDampeningForce = myStats.airControlDampeningFactor * currentVelocity;
	const physx::PxVec3 totalMoveForce = desiredVelocity * directionalDampeningScalar * UtilityFunctions::Lerp(1.0f, 2.0f, elapsedTimeScalar) + momentumBasedDampeningForce;
	aPhysicsBodyPtr->addForce(totalMoveForce, physx::PxForceMode::eACCELERATION);
	if (myStats.groundedDotProduct > -0.75f && myStats.groundedDotProduct < 0.0f)
	{
		aPhysicsBodyPtr->addForce(totalMoveForce * 0.5f, physx::PxForceMode::eACCELERATION);
	}
}

void Player::TriggerJump(physx::PxRigidDynamic* aPhysicsBodyPtr, bool aIsDoubleJump)
{
	const physx::PxVec3 currentVelocity = aPhysicsBodyPtr->getLinearVelocity();
	switch (aIsDoubleJump) 
	{
		case true:
		{
			myStats.hasDoubleJumpedSinceGrounded = true;
			aPhysicsBodyPtr->setLinearVelocity(physx::PxVec3(currentVelocity.x, UtilityFunctions::Min(myStats.jumpForce, myStats.jumpForce + currentVelocity.y * 0.5f), currentVelocity.z));
			break;
		}
		case false:
		{
			aPhysicsBodyPtr->setLinearVelocity(physx::PxVec3(currentVelocity.x, myStats.jumpForce, currentVelocity.z));
			break;
		}
	}
}

void Player::TriggerRocketKnockback(physx::PxRigidDynamic* aPhysicsBodyPtr)
{
	aPhysicsBodyPtr->addForce(physx::PxVec3(myStats.rocketKnockbackForce.x, myStats.rocketKnockbackForce.y, myStats.rocketKnockbackForce.z), physx::PxForceMode::eVELOCITY_CHANGE);
}

void Player::UpdateCameraRotation(float aDeltaTime)
{
	DreamEngine::Rotator currentRotation = myCamera->GetTransform().GetRotation();
	currentRotation.x += myStats.rotationDelta.y * myStats.rotationSpeed * aDeltaTime;
	currentRotation.y += myStats.rotationDelta.x * myStats.rotationSpeed * aDeltaTime;
	currentRotation.x = UtilityFunctions::Clamp(currentRotation.x, myStats.minPitch, myStats.maxPitch);
	myCamera->SetRotation(currentRotation);
	myTransform.SetRotation(DE::Vector3f(myTransform.GetRotation().x, currentRotation.y, myTransform.GetRotation().z));
}

void Player::UpdateTimers(float aDeltaTime)
{
	if (!myStats.grapplingCooldown.IsDone())
	{
		myStats.grapplingCooldown.Update(aDeltaTime);
		bool isDoneAfterUpdate = myStats.grapplingCooldown.IsDone();
		if (isDoneAfterUpdate)
		{
			MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &isDoneAfterUpdate, eMessageType::PlayerGrappleCooldownStateChanged });
		}
	}
	if (!myStats.healingTimer.IsDone())
	{
		myStats.healingTimer.Update(aDeltaTime);
	}
	if (!myStats.iFramesTimer.IsDone())
	{
		myStats.iFramesTimer.Update(aDeltaTime);
	}
}

void Player::ResetConditionalFlags()
{
	myStats.shouldRotate = false;
	myStats.shouldMove = false;
	myStats.shouldSprint = false;
	myStats.shouldJump = false;
	myStats.gaveSlideInputLastFrame = myStats.shouldGroundSlide;
	myStats.shouldGroundSlide = false;
	myStats.shouldUseGrapplingHook = false;
	myStats.shouldShoot = false;
	myStats.shootIsHeld = false;
	myStats.shootIsReleased = false;
	myStats.shouldSwapAmmo = false;
	myStats.shouldApplyRocketKnockback = false;
}

void Player::Render(DE::GraphicsEngine& aGraphicsEngine)
{
	if (myStats.respawnTimer.IsDone())
	{
		myGun.Render(aGraphicsEngine);
	}
}

DE::ModelInstance Player::GetGrapplingHookCylinder()
{
	return myGrapplingHookCylinder;
}