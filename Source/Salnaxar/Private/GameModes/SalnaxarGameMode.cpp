#include "GameModes/SalnaxarGameMode.h"
#include "Salnaxar/Public/Characters/PlayerCharacter.h"
#include "Salnaxar/Public/PlayerController/SalnaxarPlayerController.h"
#include "Salnaxar/Public/PlayerStates/SalnaxarPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ASalnaxarGameMode::ASalnaxarGameMode()
{
	//
	// *** Variable initialization
	//
	//
	// ** Mother class(es) variables
	//
	bDelayedStart = true;

	// ** This class variables
	//
	m_WarmUpTime        = 5.0f   ;
	m_CooldownTime      = 5.0f   ;
	m_MatchTime         = 120.0f ;
	m_LevelStartingTime = 0.0f   ;
	m_CountdownTime     = 0.0f   ;
}

void ASalnaxarGameMode::BeginPlay()
{
	Super::BeginPlay();

	m_LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ASalnaxarGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		ASalnaxarPlayerController* _SalnaxarPlayerController = Cast<ASalnaxarPlayerController>(*It);

		if (_SalnaxarPlayerController)
		{
			_SalnaxarPlayerController->SET_MatchState(MatchState);
		}
	}
}

void ASalnaxarGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		m_CountdownTime = m_WarmUpTime - GetWorld()->GetTimeSeconds() + m_LevelStartingTime;

		if (m_CountdownTime <= 0.0f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		m_CountdownTime = m_WarmUpTime + m_MatchTime - GetWorld()->GetTimeSeconds() + m_LevelStartingTime;

		if (m_CountdownTime <= 0.0f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
}

void ASalnaxarGameMode::ELIMINATE_Player
(
	APlayerCharacter*          p_EliminatedCharacter ,
	ASalnaxarPlayerController* p_VictimController    ,
	ASalnaxarPlayerController* p_AttackerController
)
{
	ASalnaxarPlayerState* _AttackerPlayerState = p_AttackerController ? Cast<ASalnaxarPlayerState>(p_AttackerController->PlayerState) : nullptr ;
	ASalnaxarPlayerState* _VictimPlayerState   = p_VictimController   ? Cast<ASalnaxarPlayerState>(p_VictimController  ->PlayerState) : nullptr ;

	if (_AttackerPlayerState && _AttackerPlayerState != _VictimPlayerState)
	{
		_AttackerPlayerState->ADD_ToScore(1.0f);
	}

	if (_VictimPlayerState)
	{
		_VictimPlayerState->ADD_ToDeathCount(1);
	}

	if (p_EliminatedCharacter)
	{
		p_EliminatedCharacter->ELIMINATE();
	}
}

void ASalnaxarGameMode::REQUEST_Respawn(ACharacter* p_EliminatedCharacter, AController* p_EliminatedController)
{
	if (p_EliminatedCharacter)
	{
		p_EliminatedCharacter->Reset  () ;
		p_EliminatedCharacter->Destroy() ;
	}

	if (p_EliminatedController)
	{
		TArray<AActor*> _PlayerStarts                                                           ;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), _PlayerStarts) ;
		int32 _Selection = FMath::RandRange(0, _PlayerStarts.Num() - 1)                         ;

		RestartPlayerAtPlayerStart(p_EliminatedController, _PlayerStarts[_Selection]);
	}
}