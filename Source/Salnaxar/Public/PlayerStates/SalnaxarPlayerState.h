#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SalnaxarPlayerState.generated.h"

class APlayerCharacter;
class ASalnaxarPlayerController;

UCLASS()
class SALNAXAR_API ASalnaxarPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// 
	// Replication notifies
	//
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_m_DeathCount();

	void ADD_ToScore(float p_ScoreAmount); // Should only be called on the server
	void ADD_ToDeathCount(int32 p_DeathCountAmount);

private:

	void CHECKINIT_CharacterAndControllerInitialization();

private:

	APlayerCharacter* m_Character = nullptr;
	ASalnaxarPlayerController* m_Controller = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_m_DeathCount)
	int32 m_DeathCount;
};
