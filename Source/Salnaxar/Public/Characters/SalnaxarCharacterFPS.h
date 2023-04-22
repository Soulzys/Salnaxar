#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SalnaxarCharacterFPS.generated.h"

class UCameraComponent;

UCLASS()
class SALNAXAR_API ASalnaxarCharacterFPS : public ACharacter
{
	GENERATED_BODY()

public:

	ASalnaxarCharacterFPS();

public:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:

	virtual void BeginPlay() override;

private:

	void INIT_Inputs();

	void MOVE_Forward(const float p_Value);
	void MOVE_Right  (const float p_Value);
	void Turn        (const float p_Value);
	void LookUp      (const float p_Value);

private:

	UPROPERTY(EditAnywhere, Category = "Camera", meta = (DisplayName = "Camera Component"))
	UCameraComponent* m_CameraComp = nullptr;

};
