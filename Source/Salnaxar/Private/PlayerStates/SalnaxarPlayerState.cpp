#include "PlayerStates/SalnaxarPlayerState.h"
#include "Salnaxar/Public/Characters/PlayerCharacter.h"
#include "Salnaxar/Public/PlayerController/SalnaxarPlayerController.h"
#include "Net/UnrealNetwork.h"

void ASalnaxarPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASalnaxarPlayerState, m_DeathCount);
}

void ASalnaxarPlayerState::ADD_ToScore(float p_ScoreAmount)
{
	SetScore(GetScore() + p_ScoreAmount);

	CHECKINIT_CharacterAndControllerInitialization();

	if (m_Character && m_Controller)
	{
		m_Controller->SET_HUDScore(GetScore());
	}
}

void ASalnaxarPlayerState::ADD_ToDeathCount(int32 p_DeathCountAmount)
{
	m_DeathCount += p_DeathCountAmount;

	CHECKINIT_CharacterAndControllerInitialization();

	if (m_Character && m_Controller)
	{
		m_Controller->SET_HUDDeathCount(m_DeathCount);
	}
}

void ASalnaxarPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	CHECKINIT_CharacterAndControllerInitialization();

	if (m_Character && m_Controller)
	{
		m_Controller->SET_HUDScore(GetScore());
	}
}

void ASalnaxarPlayerState::OnRep_m_DeathCount()
{
	CHECKINIT_CharacterAndControllerInitialization();

	if (m_Character && m_Controller)
	{
		m_Controller->SET_HUDDeathCount(m_DeathCount);
	}
}

void ASalnaxarPlayerState::CHECKINIT_CharacterAndControllerInitialization()
{
	if (m_Character == nullptr)
	{
		m_Character = Cast<APlayerCharacter>(GetPawn());

		if (m_Character)
		{
			if (m_Controller == nullptr)
			{
				m_Controller = Cast<ASalnaxarPlayerController>(m_Character->Controller);
			}
		}
	}
}