#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestActor.generated.h"

UCLASS()
class SALNAXAR_API ATestActor : public AActor
{
	GENERATED_BODY()
	
public:	

	ATestActor();

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* m_TestingMesh = nullptr;
};
