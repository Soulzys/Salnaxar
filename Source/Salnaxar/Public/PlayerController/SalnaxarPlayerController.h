#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SalnaxarPlayerController.generated.h"

class ASalnaxarHUD      ;
class UCharacterOverlay ;
class ASalnaxarGameMode ;

UCLASS()
class SALNAXAR_API ASalnaxarPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	// 
	// *** Constructor
	//
	//
	ASalnaxarPlayerController();
	
	// 
	// *** Override
	//
	//
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override ;
	virtual void ReceivedPlayer            ()                                                  override ; // Sync with server clock as soon as possible
	void         OnPossess                 (APawn* p_InPawn)                                   override ;

	//
	// *** Normal functions
	//
	//
	void SET_HUDHealth                     (float p_Health, float p_MaxHealth) ;
	void SET_HUDScore                      (float p_Score)                     ;
	void SET_HUDDeathCount                 (int32 p_DeathCount)                ;
	void SET_HUDInteractiveMessageOverlay  (FString p_MessageText)             ;
	void RESET_HUDInteractiveMessageOverlay()                                  ;
	void SET_HUDWeaponAmmo                 (int32 p_WeaponAmmo)                ;
	void SET_HUDCarriedWeaponAmmoAmount    (int32 p_Amount)                    ;
	void SET_HUDMatchCountdown             (float p_CountdownTime)             ;
	void SET_HUDAnnouncementCountdown      (float p_CountdownTime)             ;
	void SET_MatchState                    (FName p_State)                     ;
	void HANDLE_MatchHasStarted            ()                                  ;
	void HANDLE_CooldownState              ()                                  ;

	//
	// *** Getters
	// 
	// 
	// Time since game started
	virtual float GET_ServerTime(); // Synced with server world clock

protected:

	// 
	// *** Override
	//
	//
	virtual void BeginPlay()                override ;
	virtual void Tick     (float DeltaTime) override ;

private:

	void SET_HUDTime();

	//
	// *** RPCs
	//
	//

	/* Sync time between clientand server */	

	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable) void SERVER_RequestServerTime(float p_TimeOfClientRequest);
	UFUNCTION(Server, Reliable) void SERVER_CheckMatchState();
	// Reports the current server time to the client in response to SERVER_RequestServerTime
	UFUNCTION(Client, Reliable) void CLIENT_ReportServerTime(float p_TimeOfClientRequest, float p_TimeOfServerReceivedClientRequest);
	UFUNCTION(Client, Reliable)
	void CLIENT_JoinMidGame
	(
		float p_WarmUpTime        ,
		float p_MatchTime         ,
		float p_CooldownTime      , 
		float p_LevelStartingTime , 
		FName p_MatchState
	);
	UFUNCTION()	void ONREP_m_MatchState();

	//
	// *** Normal functions
	//
	//
	void CHECK_TimeSync(float p_DeltaTime) ;
	void POLL_Init     ()                  ;

private:

	ASalnaxarHUD*      m_SalnaxarHUD      = nullptr ;
	ASalnaxarGameMode* m_SalnaxarGameMode = nullptr ;

	float  m_LevelStartingTime ;
	float  m_WarmUpTime        ;
	float  m_MatchTime         ;
	float  m_CooldownTime      ;
	uint32 m_CountdownInt      ;
	float  m_ClientServerDelta ;

	UPROPERTY(EditAnywhere, Category = "Time", meta = (DisplayName = "Time Synchronization Frequency"))
	float m_TimeSyncFrequency; // Client time is synced with Server time every 5 seconds.

	float m_TimeSyncRunningTime;

	UPROPERTY(ReplicatedUsing = ONREP_m_MatchState)
	FName m_MatchState;

	UCharacterOverlay* m_CharacterOverlay = nullptr ;
	bool m_bInitializeCharacterOverlay              ;

	float   m_HUDHealth             ;
	float   m_HUDMaxHealth          ;
	float   m_HUDScore              ;
	int32   m_HUDDeathCount         ;
	FString m_HUDInteractiveMessage ;
};
