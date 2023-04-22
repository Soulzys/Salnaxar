#include "Characters/SalnaxarCharacterFPS.h"
#include "Camera/CameraComponent.h"

ASalnaxarCharacterFPS::ASalnaxarCharacterFPS()
{
	PrimaryActorTick.bCanEverTick = false;

	m_CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	m_CameraComp->bUsePawnControlRotation = true;
	m_CameraComp->SetupAttachment(GetMesh(), FName("HeadSocket"));
}

void ASalnaxarCharacterFPS::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASalnaxarCharacterFPS::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	INIT_Inputs();

	PlayerInputComponent->BindAxis("MoveForward", this, &ASalnaxarCharacterFPS::MOVE_Forward);
	PlayerInputComponent->BindAxis("MoveRight"  , this, &ASalnaxarCharacterFPS::MOVE_Right  );
	PlayerInputComponent->BindAxis("Turn"       , this, &ASalnaxarCharacterFPS::Turn        );
	PlayerInputComponent->BindAxis("LookUp"     , this, &ASalnaxarCharacterFPS::LookUp      );
}

void ASalnaxarCharacterFPS::INIT_Inputs()
{
	static bool _bAreBindingsAdded = false;

	if (_bAreBindingsAdded == false)
	{
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::W     ,  1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::S     , -1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveRight"  , EKeys::A     , -1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveRight"  , EKeys::D     ,  1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("Turn"       , EKeys::MouseX,  1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("LookUp"     , EKeys::MouseY, -1.0f));

		_bAreBindingsAdded = true;
	}
}

void ASalnaxarCharacterFPS::MOVE_Forward(const float p_Value)
{
	if (p_Value != 0.0f)
	{
		// We do it this way so that we base our movement onto the Mesh direction rather than the capsule component direction, which is the root. 
		//
		const FVector _Direction(FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::X));
		AddMovementInput(_Direction, p_Value);
		UE_LOG(LogTemp, Warning, TEXT("I'm here"));
	}

}

void ASalnaxarCharacterFPS::MOVE_Right(const float p_Value)
{
	if (p_Value != 0.0f)
	{
		// We do it this way so that we base our movement onto the Mesh direction rather than the capsule component direction, which is the root. 
		//
		const FVector _Direction(FRotationMatrix(FRotator(0.0f, GetControlRotation().Yaw, 0.0f)).GetUnitAxis(EAxis::Y));
		AddMovementInput(_Direction, p_Value);
	}
}

void ASalnaxarCharacterFPS::Turn(const float p_Value)
{
	AddControllerYawInput(p_Value);
}

void ASalnaxarCharacterFPS::LookUp(const float p_Value)
{
	AddControllerPitchInput(p_Value);
}