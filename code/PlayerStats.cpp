#include "PlayerStats.h"
#include "GameObject.h"
#include "HealthComponent.h"
#include <DreamEngine\utilities\UtilityFunctions.h>
#include "MainSingleton.h"

void PlayerStats::SetCurrentHealth(GameObject* aPlayerPtr, const int aHealthAmount)
{
	aPlayerPtr->GetComponent<HealthComponent>()->SetCurrentHealth(aHealthAmount);
}

const int PlayerStats::GetCurrentHealth(GameObject* aPlayerPtr) const
{
	return aPlayerPtr->GetComponent<HealthComponent>()->GetCurrentHealth();
}

const int PlayerStats::GetMaxHealth(GameObject* aPlayerPtr) const
{
	return aPlayerPtr->GetComponent<HealthComponent>()->GetMaxHealth();
}

void PlayerStats::TakeDamage(GameObject* aPlayerPtr, const int& aDamageAmount)
{
	if (IsAlive(aPlayerPtr))
	{
		SetCurrentHealth(aPlayerPtr, UtilityFunctions::Max(GetCurrentHealth(aPlayerPtr) - aDamageAmount, 0));

		MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &std::pair<int, int>(GetCurrentHealth(aPlayerPtr), GetMaxHealth(aPlayerPtr)), eMessageType::PlayerHealthChange });
		if (!IsAlive(aPlayerPtr))
		{
			respawnTimer.Reset();
			MainSingleton::GetInstance()->GetAudioManager().StopAudio(eAudioEvent::PlayerDeath);
			MainSingleton::GetInstance()->GetAudioManager().PlayAudio(eAudioEvent::PlayerDeath, aPlayerPtr->GetTransform()->GetPosition());
			MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ nullptr, eMessageType::PlayerDied });
		}
	}
}

void PlayerStats::Heal(GameObject* aPlayerPtr, const int& aHealingAmount)
{
	SetCurrentHealth(aPlayerPtr, UtilityFunctions::Min(GetCurrentHealth(aPlayerPtr) + aHealingAmount, GetMaxHealth(aPlayerPtr)));
	MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &std::pair<int, int>(GetCurrentHealth(aPlayerPtr), GetMaxHealth(aPlayerPtr)), eMessageType::PlayerHealthChange });
}

void PlayerStats::HealToMax(GameObject* aPlayerPtr)
{
	SetCurrentHealth(aPlayerPtr, GetMaxHealth(aPlayerPtr));
	MainSingleton::GetInstance()->GetPostMaster().TriggerMessage({ &std::pair<int, int>(GetCurrentHealth(aPlayerPtr), GetMaxHealth(aPlayerPtr)), eMessageType::PlayerHealthChange });
}

const bool PlayerStats::IsAlive(GameObject* aPlayerPtr) const
{
	return aPlayerPtr->GetComponent<HealthComponent>()->IsAlive();
}