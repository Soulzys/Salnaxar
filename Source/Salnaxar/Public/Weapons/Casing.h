#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

class USoundCue     ;
struct FTimerHandle ;

UCLASS()
class SALNAXAR_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	

	//
	// *** Constructor
	//
	//
	ACasing();

protected:

	//
	// *** Override
	//
	//
	virtual void BeginPlay() override;

	//
	// *** Callback
	//
	//
	UFUNCTION()
	virtual void ON_Hit
	(
		UPrimitiveComponent* p_HitComp       ,
		AActor*              p_OtherActor    , 
		UPrimitiveComponent* p_OtherComp     , 
		FVector              p_NormalImpulse , 
		const FHitResult&    p_Hit
	);

	//
	// *** Normal functions
	//
	//
	UFUNCTION() void TIMER_DestroyShell();

private:

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Casing Mesh"))
	UStaticMeshComponent* m_CasingMesh = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Shell Ejection Impulse"))
	float m_ShellEjectionImpulse;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Shell Sound"))
	USoundCue* m_ShellSound = nullptr;

	FTimerHandle m_TimerHandle;

	bool m_bHasHitFloor;
};
