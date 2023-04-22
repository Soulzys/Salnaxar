#include "Characters/PlayerAnimInstance.h"
#include "../../Public/Characters/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Salnaxar/Public/Weapons/Weapon.h"
#include "Salnaxar/Public/SalnaxarTypes/CombatState.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	m_PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float p_DeltaTime)
{
	Super::NativeUpdateAnimation(p_DeltaTime);

	if (m_PlayerCharacter == nullptr)
	{
		m_PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	if (m_PlayerCharacter == nullptr)
	{
		return;
	}

	// Speed
	//
	FVector _Velocity = m_PlayerCharacter->GetVelocity() ;
	_Velocity.Z       = 0.0f                             ;
	m_Speed           = _Velocity.Size()                 ;

	// States & Conditions
	//
	m_bIsInAir        = m_PlayerCharacter->GetCharacterMovement()->IsFalling()                                           ;
	m_bIsAccelerating = m_PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f ? true : false ;
	m_bWeaponEquipped = m_PlayerCharacter->IS_WeaponEquipped   ()                                                        ;
	m_EquippedWeapon  = m_PlayerCharacter->GET_EquippedWeapon  ()                                                        ;
	m_bIsCrouched     = m_PlayerCharacter->bIsCrouched                                                                   ;
	m_bAiming         = m_PlayerCharacter->IS_Aiming           ()                                                        ;
	m_TurningInPlace  = m_PlayerCharacter->GET_TurningInPlace  ()                                                        ;
	m_bRotateRootBone = m_PlayerCharacter->GET_RotateRootBone  ()                                                        ; // Should the root bone be rotated ?
	m_bEliminated     = m_PlayerCharacter->GET_Eliminated      ()                                                        ;

	// Aim Rotation
	//
	FRotator _AimRotation      = m_PlayerCharacter->GetBaseAimRotation()                                     ;
	FRotator _MovementRotation = UKismetMathLibrary::MakeRotFromX(m_PlayerCharacter->GetVelocity())          ;
	FRotator _DeltaRot         = UKismetMathLibrary::NormalizedDeltaRotator(_MovementRotation, _AimRotation) ;
	m_DeltaRotation            = FMath::RInterpTo(m_DeltaRotation, _DeltaRot, p_DeltaTime, 6.0f)             ;
	m_YawOffset                = m_DeltaRotation.Yaw                                                         ;

	// Lean & Rotation
	//
	m_CharacterRotationLastFrame = m_CharacterRotation                                                                           ;
	m_CharacterRotation          = m_PlayerCharacter->GetActorRotation()                                                         ;
	const       FRotator _Delta  = UKismetMathLibrary::NormalizedDeltaRotator(m_CharacterRotation, m_CharacterRotationLastFrame) ;
	const float _Target          = _Delta.Yaw / p_DeltaTime                                                                      ;
	const float _Interp          = FMath::FInterpTo(m_Lean, _Target, p_DeltaTime, 6.0f)                                          ;
	m_Lean                       = FMath::Clamp(_Interp, -90.0f, 90.0f)                                                          ;

	// Aim Offset
	//
	m_AO_Yaw   = m_PlayerCharacter->GET_AO_Yaw()   ;
	m_AO_Pitch = m_PlayerCharacter->GET_AO_Pitch() ;

	// Transform Hands (Left & Right)
	if (m_bWeaponEquipped && m_EquippedWeapon && m_EquippedWeapon->GET_WeaponMesh() && m_PlayerCharacter->GetMesh())
	{
		// Possible way to improve : get the socket name into a global variable (enum or whatever)
		m_LeftHandTransform = m_EquippedWeapon->GET_WeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		
		FVector  _OutPosition                                                                     ;
		FRotator _OutRotation                                                                     ;
		m_PlayerCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r")                        , 
			m_LeftHandTransform.GetLocation(), FRotator::ZeroRotator, _OutPosition, _OutRotation) ;
		m_LeftHandTransform.SetLocation(_OutPosition)                                             ;
		m_LeftHandTransform.SetRotation(FQuat(_OutRotation))                                      ;

		if (m_PlayerCharacter->IsLocallyControlled())
		{
			m_bIsLocallyControlled         = true                                                                                                        ;
			FTransform _RightHandTransform = m_EquippedWeapon->GET_WeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World) ;
			
			FRotator _LookAtRotation = UKismetMathLibrary::FindLookAtRotation(_RightHandTransform.GetLocation()               ,
				_RightHandTransform.GetLocation() + (_RightHandTransform.GetLocation() - m_PlayerCharacter->GET_HitTarget())) ;
			m_RightHandRotation = FMath::RInterpTo(m_RightHandRotation, _LookAtRotation, p_DeltaTime, 30.0f)                  ;
		}

		// Debug 
		// Draw an orange line from the muzzle to the crosshair (ideal line) and a red line from the muzzle in the forward direction (actual line)
		FTransform _MuzzleTipTransform = m_EquippedWeapon->GET_WeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		FVector _MuzzleX(FRotationMatrix(_MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X)); // Normalized Vector
		DrawDebugLine(GetWorld(), _MuzzleTipTransform.GetLocation(), m_PlayerCharacter->GET_HitTarget(), FColor::Orange);
		DrawDebugLine(GetWorld(), _MuzzleTipTransform.GetLocation(), _MuzzleTipTransform.GetLocation() + _MuzzleX * 1000.0f, FColor::Red);
	}
	
	// Other conditions ||| Could this be move up top along the other conditions ? Need to try when back home
	m_bUseFABRIK          = m_PlayerCharacter->GET_CombatState() != ECombatState::Reloading;
	m_bUseAimOffsets      = m_PlayerCharacter->GET_CombatState() != ECombatState::Reloading;
	m_bTransformRightHand = m_PlayerCharacter->GET_CombatState() != ECombatState::Reloading;
}