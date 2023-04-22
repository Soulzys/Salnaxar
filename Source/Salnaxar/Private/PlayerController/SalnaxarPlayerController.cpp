#include "PlayerController/SalnaxarPlayerController.h"
#include "Salnaxar/Public/HUD/SalnaxarHUD.h"
#include "Salnaxar/Public/HUD/CharacterOverlay.h"
#include "Salnaxar/Public/HUD/InteractiveMessageOverlay.h"
#include "Salnaxar/Public/HUD/Announcement.h"
#include "Salnaxar/Public/Characters/PlayerCharacter.h"
#include "Salnaxar/Public/GameModes/SalnaxarGameMode.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

ASalnaxarPlayerController::ASalnaxarPlayerController()
{
	// 
	// *** Variables initialization
	//
	//
	// ** This class variables
	//
	m_LevelStartingTime           = 0.0f  ;
	m_WarmUpTime                  = 0.0f  ;
	m_MatchTime                   = 0.0f  ;
	m_CooldownTime                = 0.0f  ;
	m_CountdownInt                = 0     ;
	m_ClientServerDelta           = 0.0f  ;
	m_TimeSyncFrequency           = 5.0f  ;
	m_TimeSyncRunningTime         = 0.0f  ;
	m_bInitializeCharacterOverlay = false ;
}

void ASalnaxarPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASalnaxarPlayerController, m_MatchState);
}

// ReceivePlayer section START
//
//

void ASalnaxarPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		SERVER_RequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ASalnaxarPlayerController::SERVER_RequestServerTime_Implementation(float p_TimeOfClientRequest)
{
	float _ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	CLIENT_ReportServerTime(p_TimeOfClientRequest, _ServerTimeOfReceipt);
}

void ASalnaxarPlayerController::CLIENT_ReportServerTime_Implementation(float p_TimeOfClientRequest, float p_TimeOfServerReceivedClientRequest)
{
	float _RoundTripTime = GetWorld()->GetTimeSeconds() - p_TimeOfClientRequest;
	float _CurrentServerTime = p_TimeOfServerReceivedClientRequest + (0.5f * _RoundTripTime);

	m_ClientServerDelta = _CurrentServerTime - GetWorld()->GetTimeSeconds();
}

//
//
// ReceivePlayer section END

//
// 

// OnPosses section START 
//
//

void ASalnaxarPlayerController::OnPossess(APawn* p_InPawn)
{
	Super::OnPossess(p_InPawn);

	APlayerCharacter* _PlayerCharacter = Cast<APlayerCharacter>(p_InPawn);

	if (_PlayerCharacter)
	{
		SET_HUDHealth(_PlayerCharacter->GET_Health(), _PlayerCharacter->GET_MaxHealth());
	}
}

void ASalnaxarPlayerController::SET_HUDHealth(float p_Health, float p_MaxHealth)
{
	if (m_SalnaxarHUD == nullptr)
	{
		m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());
	}

	bool _bHUDValid = m_SalnaxarHUD                    &&
		m_SalnaxarHUD->m_CharacterOverlay              &&
		m_SalnaxarHUD->m_CharacterOverlay->m_HealthBar &&
		m_SalnaxarHUD->m_CharacterOverlay->m_HealthText ;

	if (_bHUDValid)
	{
		const float _HealthPercent = p_Health / p_MaxHealth                                                             ;
		m_SalnaxarHUD->m_CharacterOverlay->m_HealthBar->SetPercent(_HealthPercent)                                      ;
		FString _HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(p_Health), FMath::CeilToInt(p_MaxHealth)) ;
		m_SalnaxarHUD->m_CharacterOverlay->m_HealthText->SetText(FText::FromString(_HealthText))                        ;
	}
	else
	{
		m_bInitializeCharacterOverlay = true        ;
		m_HUDHealth                   = p_Health    ;
		m_HUDMaxHealth                = p_MaxHealth ;
	}
}

//
//
// OnPosses section END

// BeginPlay section START
//
//

void ASalnaxarPlayerController::BeginPlay()
{
	Super::BeginPlay();

	m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());

	UE_LOG(LogTemp, Warning, TEXT("Un"));

	SERVER_CheckMatchState();
}

void ASalnaxarPlayerController::SERVER_CheckMatchState_Implementation()
{
	ASalnaxarGameMode* _SalnaxarGameMode = Cast<ASalnaxarGameMode>(UGameplayStatics::GetGameMode(this));

	if (_SalnaxarGameMode)
	{
		m_WarmUpTime        = _SalnaxarGameMode->GET_WarmUpTime()        ;
		m_MatchTime         = _SalnaxarGameMode->GET_MatchTime()         ;
		m_CooldownTime      = _SalnaxarGameMode->GET_CooldownTime()      ;
		m_LevelStartingTime = _SalnaxarGameMode->GET_LevelStartingTime() ;
		m_MatchState        = _SalnaxarGameMode->GetMatchState()         ;

		CLIENT_JoinMidGame(m_WarmUpTime, m_MatchTime, m_CooldownTime, m_LevelStartingTime, m_MatchState);
	}
}

void ASalnaxarPlayerController::CLIENT_JoinMidGame_Implementation
(
	float p_WarmUpTime, 
	float p_MatchTime, 
	float p_CooldownTime, 
	float p_LevelStartingTime, 
	FName p_MatchState
)
{
	m_WarmUpTime        = p_WarmUpTime        ;
	m_MatchTime         = p_MatchTime         ;
	m_CooldownTime      = p_CooldownTime      ;
	m_LevelStartingTime = p_LevelStartingTime ;
	m_MatchState        = p_MatchState        ;

	SET_MatchState(m_MatchState);

	if (m_SalnaxarHUD && m_MatchState == MatchState::WaitingToStart)
	{
		m_SalnaxarHUD->ADD_WarmupAnnouncement();

		if (m_SalnaxarHUD->m_WarmupAnnouncementWidget)
		{
			m_SalnaxarHUD->m_WarmupAnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

//
//
// BeginPlay section END

//
//

// Tick section START
//
//

void ASalnaxarPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SET_HUDTime();
	CHECK_TimeSync(DeltaTime);
	POLL_Init();
}

// SET_HUDTime section START
//

void ASalnaxarPlayerController::SET_HUDTime()
{
	float _TimeLeft = 0.0f;

	if (m_MatchState == MatchState::WaitingToStart)
	{
		_TimeLeft = m_WarmUpTime - GET_ServerTime() + m_LevelStartingTime;
	}
	else if (m_MatchState == MatchState::InProgress)
	{
		_TimeLeft = m_WarmUpTime + m_MatchTime - GET_ServerTime() + m_LevelStartingTime;
	}
	else if (m_MatchState == MatchState::Cooldown)
	{
		_TimeLeft = m_CooldownTime + m_WarmUpTime + m_MatchTime - GET_ServerTime() + m_LevelStartingTime;
	}

	uint32 _SecondsLeft = FMath::CeilToInt(_TimeLeft);

	/*if (HasAuthority())
	{
		m_SalnaxarGameMode = m_SalnaxarGameMode == nullptr ? Cast<ASalnaxarGameMode>(UGameplayStatics::GetGameMode(this)) : m_SalnaxarGameMode;

		if (m_SalnaxarGameMode)
		{
			_SecondsLeft = FMath::CeilToInt(m_SalnaxarGameMode->GET_CountdownTime() + m_LevelStartingTime);
		}
	}*/

	// So one second has passed
	if (m_CountdownInt != _SecondsLeft)
	{
		if (m_MatchState == MatchState::WaitingToStart || m_MatchState == MatchState::Cooldown)
		{
			SET_HUDAnnouncementCountdown(_TimeLeft);
		}

		if (m_MatchState == MatchState::InProgress)
		{
			SET_HUDMatchCountdown(_TimeLeft);
		}
	}

	m_CountdownInt = _SecondsLeft;
}

void ASalnaxarPlayerController::SET_HUDAnnouncementCountdown(float p_CountdownTime)
{
	if (m_SalnaxarHUD == nullptr)
	{
		m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());
	}

	bool _bHUDValid = m_SalnaxarHUD                            &&
		m_SalnaxarHUD->m_WarmupAnnouncementWidget              &&
		m_SalnaxarHUD->m_WarmupAnnouncementWidget->m_WarmupTime ;

	if (_bHUDValid)
	{
		if (p_CountdownTime < 0.0f)
		{
			m_SalnaxarHUD->m_WarmupAnnouncementWidget->m_WarmupTime->SetText(FText());
			return;
		}

		int32 _Minutes = FMath::FloorToInt(p_CountdownTime / 60.0f) ;
		int32 _Seconds = p_CountdownTime - _Minutes * 60            ;

		FString _CountdownText = FString::Printf(TEXT("%02d:%02d"), _Minutes, _Seconds)                     ;
		m_SalnaxarHUD->m_WarmupAnnouncementWidget->m_WarmupTime->SetText(FText::FromString(_CountdownText)) ;
	}
}

void ASalnaxarPlayerController::SET_HUDMatchCountdown(float p_CountdownTime)
{
	if (m_SalnaxarHUD == nullptr)
	{
		m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());
	}

	bool _bHUDValid = m_SalnaxarHUD                        &&
		m_SalnaxarHUD->m_CharacterOverlay                  &&
		m_SalnaxarHUD->m_CharacterOverlay->m_MatchCountdown ;

	if (_bHUDValid)
	{
		if (p_CountdownTime < 0.0f)
		{
			m_SalnaxarHUD->m_CharacterOverlay->m_MatchCountdown->SetText(FText());
			return;
		}

		int32 _Minutes = FMath::FloorToInt(p_CountdownTime / 60.0f) ;
		int32 _Seconds = p_CountdownTime - _Minutes * 60            ;

		// %02d --> 2 digits intergers where if there is in fact only one number inputted, then a 0 is set as the 'left' number
		FString _CountdownText = FString::Printf(TEXT("%02d:%02d"), _Minutes, _Seconds)                 ;
		m_SalnaxarHUD->m_CharacterOverlay->m_MatchCountdown->SetText(FText::FromString(_CountdownText)) ;
	}
}

//
// SET_HUDTime section END

void ASalnaxarPlayerController::CHECK_TimeSync(float p_DeltaTime)
{
	m_TimeSyncRunningTime += p_DeltaTime;

	if (IsLocalController() && m_TimeSyncRunningTime > m_TimeSyncFrequency)
	{
		SERVER_RequestServerTime(GetWorld()->GetTimeSeconds()) ;
		m_TimeSyncRunningTime = 0.0f                           ;
	}
}

// POLL_Init section START
//

void ASalnaxarPlayerController::POLL_Init()
{
	if (m_CharacterOverlay == nullptr)
	{
		if (m_SalnaxarHUD && m_SalnaxarHUD->m_CharacterOverlay)
		{
			m_CharacterOverlay = m_SalnaxarHUD->m_CharacterOverlay;

			if (m_CharacterOverlay)
			{
				SET_HUDHealth    (m_HUDHealth, m_HUDMaxHealth) ;
				SET_HUDScore     (m_HUDScore)                  ;
				SET_HUDDeathCount(m_HUDDeathCount)             ;
			}
		}
	}
}

void ASalnaxarPlayerController::SET_HUDScore(float p_Score)
{
	if (m_SalnaxarHUD == nullptr)
	{
		m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());
	}

	bool _bHUDValid = m_SalnaxarHUD               &&
		m_SalnaxarHUD->m_CharacterOverlay         &&
		m_SalnaxarHUD->m_CharacterOverlay->m_Score ;

	if (_bHUDValid)
	{
		FString _ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(p_Score))       ;
		m_SalnaxarHUD->m_CharacterOverlay->m_Score->SetText(FText::FromString(_ScoreText)) ;
	}
	else
	{
		m_bInitializeCharacterOverlay = true    ;
		m_HUDScore                    = p_Score ;
	}
}

void ASalnaxarPlayerController::SET_HUDDeathCount(int32 p_DeathCount)
{
	if (m_SalnaxarHUD == nullptr)
	{
		m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());
	}

	bool _bHUDValid = m_SalnaxarHUD                    &&
		m_SalnaxarHUD->m_CharacterOverlay              &&
		m_SalnaxarHUD->m_CharacterOverlay->m_DeathCount ;

	if (_bHUDValid)
	{
		FString _DeathCountText = FString::Printf(TEXT("%d"), p_DeathCount)                          ;
		m_SalnaxarHUD->m_CharacterOverlay->m_DeathCount->SetText(FText::FromString(_DeathCountText)) ;
	}
	else
	{
		m_bInitializeCharacterOverlay = true         ;
		m_HUDDeathCount               = p_DeathCount ;
	}
}

//
// POLL_Init section END

//
//
// Tick section END

//
//

// SET_MatchState section START
//
//

void ASalnaxarPlayerController::SET_MatchState(FName p_State)
{
	m_MatchState = p_State;

	if (m_MatchState == MatchState::InProgress)
	{
		HANDLE_MatchHasStarted();
	}
	else if (m_MatchState == MatchState::Cooldown)
	{
		HANDLE_CooldownState();
	}
}

void ASalnaxarPlayerController::ONREP_m_MatchState()
{
	if (m_MatchState == MatchState::InProgress)
	{
		HANDLE_MatchHasStarted();
	}
	else if (m_MatchState == MatchState::Cooldown)
	{
		HANDLE_CooldownState();
	}
}

void ASalnaxarPlayerController::HANDLE_MatchHasStarted()
{
	m_SalnaxarHUD = m_SalnaxarHUD == nullptr ? Cast<ASalnaxarHUD>(GetHUD()) : m_SalnaxarHUD;

	if (m_SalnaxarHUD)
	{
		m_SalnaxarHUD->ADD_CharacterOverlay         () ;
		m_SalnaxarHUD->ADD_InteractiveMessageOverlay() ;

		if (m_SalnaxarHUD->m_WarmupAnnouncementWidget)
		{
			m_SalnaxarHUD->m_WarmupAnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ASalnaxarPlayerController::HANDLE_CooldownState()
{
	m_SalnaxarHUD = m_SalnaxarHUD == nullptr ? Cast<ASalnaxarHUD>(GetHUD()) : m_SalnaxarHUD;

	if (m_SalnaxarHUD)
	{
		m_SalnaxarHUD->m_CharacterOverlay         ->RemoveFromParent() ;
		m_SalnaxarHUD->m_InteractiveMessageOverlay->RemoveFromParent() ;

		bool _bIsHUDValid = m_SalnaxarHUD->m_WarmupAnnouncementWidget     &&
			m_SalnaxarHUD->m_WarmupAnnouncementWidget->m_AnnouncementText &&
			m_SalnaxarHUD->m_WarmupAnnouncementWidget->m_InfoText          ;

		if (_bIsHUDValid)
		{
			m_SalnaxarHUD->m_WarmupAnnouncementWidget->SetVisibility(ESlateVisibility::Visible);

			FString _AnnoncementText("New match starts in : ")                                                          ;
			m_SalnaxarHUD->m_WarmupAnnouncementWidget->m_AnnouncementText->SetText(FText::FromString(_AnnoncementText)) ;
			m_SalnaxarHUD->m_WarmupAnnouncementWidget->m_InfoText        ->SetText(FText())                             ;
		}
	}
}

//
//
// SET_MatchState section END


void ASalnaxarPlayerController::SET_HUDInteractiveMessageOverlay(FString p_MessageText)
{
	if (m_SalnaxarHUD == nullptr)
	{
		m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());
	}

	bool _bHUDValid = m_SalnaxarHUD                              &&
		m_SalnaxarHUD->m_InteractiveMessageOverlay               &&
		m_SalnaxarHUD->m_InteractiveMessageOverlay->m_MessageText ;

	if (_bHUDValid)
	{
		m_SalnaxarHUD->m_InteractiveMessageOverlay->m_MessageText->SetText(FText::FromString(p_MessageText)) ;
		m_SalnaxarHUD->m_InteractiveMessageOverlay->SetVisibility(ESlateVisibility::Visible)                 ;
	}
}

void ASalnaxarPlayerController::RESET_HUDInteractiveMessageOverlay()
{
	if (m_SalnaxarHUD == nullptr)
	{
		m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());
	}

	bool _bHUDValid = m_SalnaxarHUD                              &&
		m_SalnaxarHUD->m_InteractiveMessageOverlay               &&
		m_SalnaxarHUD->m_InteractiveMessageOverlay->m_MessageText ;

	if (_bHUDValid)
	{
		m_SalnaxarHUD->m_InteractiveMessageOverlay->m_MessageText->SetText(FText::FromString(TEXT(""))) ;
		m_SalnaxarHUD->m_InteractiveMessageOverlay->SetVisibility(ESlateVisibility::Hidden)             ;
	}
}

void ASalnaxarPlayerController::SET_HUDWeaponAmmo(int32 p_WeaponAmmo)
{
	if (m_SalnaxarHUD == nullptr)
	{
		m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());
	}

	bool _bHUDValid = m_SalnaxarHUD                          &&
		m_SalnaxarHUD->m_CharacterOverlay                    &&
		m_SalnaxarHUD->m_CharacterOverlay->m_WeaponAmmoAmount ;

	if (_bHUDValid)
	{
		FString _WeaponAmmoText = FString::Printf(TEXT("%d"), p_WeaponAmmo)                                ;
		m_SalnaxarHUD->m_CharacterOverlay->m_WeaponAmmoAmount->SetText(FText::FromString(_WeaponAmmoText)) ;
	}
}

void ASalnaxarPlayerController::SET_HUDCarriedWeaponAmmoAmount(int32 p_Amount)
{
	if (m_SalnaxarHUD == nullptr)
	{
		m_SalnaxarHUD = Cast<ASalnaxarHUD>(GetHUD());
	}

	bool _bHUDValid = m_SalnaxarHUD                                 &&
		m_SalnaxarHUD->m_CharacterOverlay                           &&
		m_SalnaxarHUD->m_CharacterOverlay->m_CarriedWeaponAmmoAmount ;

	if (_bHUDValid)
	{
		FString _AmmoText = FString::Printf(TEXT("%d"), p_Amount)                                           ;
		m_SalnaxarHUD->m_CharacterOverlay->m_CarriedWeaponAmmoAmount->SetText(FText::FromString(_AmmoText)) ;
	}
}

float ASalnaxarPlayerController::GET_ServerTime()
{
	return GetWorld()->GetTimeSeconds() + m_ClientServerDelta;
}
