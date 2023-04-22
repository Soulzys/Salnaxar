#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SalnaxarGameMode.generated.h"

class ACharacter                ;
class APlayerCharacter          ;
class ASalnaxarPlayerController ;

namespace MatchState
{
	extern SALNAXAR_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer
}

UCLASS()
class SALNAXAR_API ASalnaxarGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	//
	// *** Constructor
	//
	//
	ASalnaxarGameMode();

	//
	// *** Override
	//
	//
	virtual void Tick(float DeltaTime) override;

	//
	// *** Normal functions
	virtual void ELIMINATE_Player
	(
		APlayerCharacter*          p_EliminatedCharacter , 
		ASalnaxarPlayerController* p_VictimController    , 
		ASalnaxarPlayerController* p_AttackerController
	);

	virtual void REQUEST_Respawn(ACharacter* p_EliminatedCharacter, AController* p_EliminatedController);

protected:

	//
	// *** Override
	//
	//
	virtual void BeginPlay      () override ;
	virtual void OnMatchStateSet() override ;

private:

	UPROPERTY(EditDefaultsOnly) float m_WarmUpTime   ;
	UPROPERTY(EditDefaultsOnly) float m_CooldownTime ;
	UPROPERTY(EditDefaultsOnly) float m_MatchTime    ;

	float m_LevelStartingTime ;
	float m_CountdownTime     ;


public:

	FORCEINLINE float GET_WarmUpTime        () const { return m_WarmUpTime        ; }
	FORCEINLINE float GET_MatchTime         () const { return m_MatchTime         ; }
	FORCEINLINE float GET_CooldownTime      () const { return m_CooldownTime      ; }
	FORCEINLINE float GET_LevelStartingTime () const { return m_LevelStartingTime ; }
	FORCEINLINE float GET_CountdownTime     () const { return m_CountdownTime     ; }
};
