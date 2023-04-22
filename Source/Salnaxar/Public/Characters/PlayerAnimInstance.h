#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Salnaxar/Public/SalnaxarTypes/TurningInPlace.h"
#include "PlayerAnimInstance.generated.h"

class APlayerCharacter;
class AWeapon;

UCLASS()
class SALNAXAR_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation()              override;
	virtual void NativeUpdateAnimation(float p_DeltaTime) override;

private:

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Player Character"), meta = (AllowPrivateAccess = "true"))
	APlayerCharacter* m_PlayerCharacter = nullptr;

	// Speed
	//
	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (DisplayName = "Speed"), meta = (AllowPrivateAccess = "true"))
	float m_Speed;

	// States & Conditions
	//
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "IsInAir"), meta = (AllowPrivateAccess = "true"))
	bool m_bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "IsAccelerating"), meta = (AllowPrivateAccess = "true"))
	bool m_bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "WeaponEquipped"), meta = (AllowPrivateAccess = "true"))
	bool m_bWeaponEquipped;

	AWeapon* m_EquippedWeapon = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "IsCrouched"), meta = (AllowPrivateAccess = "true"))
	bool m_bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Aiming"), meta = (AllowPrivateAccess = "true"))
	bool m_bAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Turning In Place"), meta = (AllowPrivateAccess = "true"))
	ETurningInPlace m_TurningInPlace;

	UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "bRotateRootBone"), meta = (AllowPrivateAccess = "true"))
	bool m_bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "bEliminated"), meta = (AllowPrivateAccess = "true"))
	bool m_bEliminated;

	// Aim Rotation
	//
	FRotator m_DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "YawOffset"), meta = (AllowPrivateAccess = "true"))
	float m_YawOffset;

	// Lean & Rotation
	//
	FRotator m_CharacterRotationLastFrame ;
	FRotator m_CharacterRotation          ;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Lean"), meta = (AllowPrivateAccess = "true"))
	float m_Lean;

	// Aim Offset
	//
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "AO Yaw"), meta = (AllowPrivateAccess = "true"))
	float m_AO_Yaw;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "AO Pitch"), meta = (AllowPrivateAccess = "true"))
	float m_AO_Pitch;

	// Transform Hands (Left & Right)
	//
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (DisplayName = "Left Hand Transform"), meta = (AllowPrivateAccess = "true"))
	FTransform m_LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "IsLocallyControlled"), meta = (AllowPrivateAccess = "true"))
	bool m_bIsLocallyControlled;

	UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "Right Hand Rotation"), meta = (AllowPrivateAccess = "true"))
	FRotator m_RightHandRotation;


	// See .cpp
	UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "bUseFABRIK"), meta = (AllowPrivateAccess = "true"))
	bool m_bUseFABRIK;

	UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "bUseAimOffset"), meta = (AllowPrivateAccess = "true"))
	bool m_bUseAimOffsets;

	UPROPERTY(BlueprintReadOnly, meta = (DisplayName = "bTransformRightHand"), meta = (AllowPrivateAccess = "true"))
	bool m_bTransformRightHand;
};
