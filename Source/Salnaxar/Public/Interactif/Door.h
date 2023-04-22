#pragma once

#include "CoreMinimal.h"
#include "Salnaxar/Public/Interactif/Interactive.h"
#include "Door.generated.h"

class UStaticMeshComponent   ;
class USkeletalMeshComponent ;
class UStaticMeshSocket      ;
class USkeletalMeshSocket    ;
class UBoxComponent          ;
class UAnimMontage           ;
class UAnimSequence          ;
class APlayerCharacter       ;

UENUM()
enum class EServerDoorState : uint8
{
	ServerDoorOpened = 0, 
	ServerDoorClosed = 1
};

// Serves no real purpose other than self-comforting logic after a strenuous day
UENUM()
enum class EClientDoorState : uint8
{
	ClientDoorOpened = 0, 
	ClientDoorClosed = 1
};

UCLASS()
class SALNAXAR_API ADoor : public AInteractive
{
	GENERATED_BODY()
	
public:	

	//
	// *** Constructor
	//
	//
	ADoor();

	//
	// *** Override
	//
	//
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override ;
	virtual void ACTIVATE_CustomDepth      (bool p_Activate)                                   override ;

	// 
	// *** Normal functions
	//
	//
	void ACTIVATION_Server(EServerDoorState p_ServerDoorState);

protected:

	// 
	// *** Override
	//
	//
	virtual void BeginPlay() override;

	//
	// Callback functions
	//
	//
	UFUNCTION()
	virtual void ON_BoxOverlap_BEGIN
	(
		UPrimitiveComponent* p_OverlappedComp , 
		AActor*              p_OtherActor     , 
		UPrimitiveComponent* p_OtherComp      , 
		int32                p_OtherBodyIndex , 
		bool                 p_bFromSweep     , 
		const FHitResult&    p_SweepResult
	);

	UFUNCTION()
	virtual void ON_BoxOverlap_END
	(
		UPrimitiveComponent* p_OverlappedComp , 
		AActor*              p_OtherActor     , 
		UPrimitiveComponent* p_OtherComp      , 
		int32                p_OtherBodyIndex 
	);

	UFUNCTION() void ON_InteractionKey_PRESSED(APlayerCharacter* p_PlayerCharacter);

private:

	//
	// *** Normal functions
	//
	//
	void INIT_DoorState(EServerDoorState p_DoorState);

public: // Public for testing purposes only

	UPROPERTY(EditAnywhere, Category = "Door", meta = (DisplayName = "Door Frame"))
	UStaticMeshComponent* m_DoorFrame   = nullptr;

	UPROPERTY(EditAnywhere, Category = "Door", meta = (DisplayName = "Door Frame Wall"))
	UStaticMeshComponent* m_DoorFrameWall = nullptr;

	UPROPERTY(EditAnywhere, Category = "Door", meta = (DisplayName = "Door Wall"))
	UStaticMeshComponent* m_DoorWall = nullptr;

	UPROPERTY(EditAnywhere, Category = "Door", meta = (DisplayName = "Door Panel"))
	USkeletalMeshComponent* m_DoorPanel = nullptr;

	UPROPERTY(EditAnywhere, Category = "Door", meta = (DisplayName = "Door Trigger"))
	UBoxComponent* m_DoorTrigger = nullptr;

	UPROPERTY(EditAnywhere, Category = "Door Animation", meta = (DisplayName = "Door Opening Animation"))
	UAnimSequence* m_DoorOpeningAnimation = nullptr;

	UPROPERTY(EditAnywhere, Category = "Door Animation", meta = (DisplayName = "Door Closing Animation"))
	UAnimSequence* m_DoorClosingAnimation = nullptr;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_m_ServerDoorState, Category = "Door State", meta = (DisplayName = "Door State"))
	EServerDoorState m_ServerDoorState = EServerDoorState::ServerDoorClosed;

	EClientDoorState m_ClientDoorState = EClientDoorState::ClientDoorClosed;

	UFUNCTION()
	void OnRep_m_ServerDoorState();
};
