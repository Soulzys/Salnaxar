#include "Interactif/Interactive.h"

AInteractive::AInteractive()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AInteractive::BeginPlay()
{
	Super::BeginPlay();	
}

void AInteractive::ACTIVATE_CustomDepth(bool p_Activate)
{
	
}