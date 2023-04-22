#include "Testing/TestActor.h"

ATestActor::ATestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	m_TestingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TestingMesh"));
	SetRootComponent(m_TestingMesh);

}

void ATestActor::BeginPlay()
{
	Super::BeginPlay();
	
}
