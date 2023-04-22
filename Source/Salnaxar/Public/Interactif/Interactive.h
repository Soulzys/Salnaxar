#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Salnaxar/Public/Interactif/InteractiveTypes.h"
#include "Interactive.generated.h"

UCLASS()
class SALNAXAR_API AInteractive : public AActor
{
	GENERATED_BODY()
	
public:	

	//
	// *** Constructor
	//
	//
	AInteractive();

	//
	// *** Virtual functions
	//
	//
	virtual void ACTIVATE_CustomDepth(bool p_Activate);

protected:

	// 
	// *** Override
	//
	//
	virtual void BeginPlay() override;

};
