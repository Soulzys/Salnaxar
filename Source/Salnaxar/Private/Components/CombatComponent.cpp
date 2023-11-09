#include "Components/CombatComponent.h"
#include "Components/SphereComponent.h"
#include "Salnaxar/Public/Weapons/Weapon.h"
#include "Salnaxar/Public/Characters/PlayerCharacter.h"
#include "Salnaxar/Public/PlayerController/SalnaxarPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	//
	// *** Variable initialization
	//
	// 
	// ** Mother class(es) variables
	//
	PrimaryComponentTick.bCanEverTick = true;
	
	// ** This class variables
	//
	m_BaseWalkSpeed    = 600.0f ;
	m_AimWalkSpeed     = 450.0f ;
	m_ZoomedFOV        = 30.0f  ;
	m_ZoomInterpSpeed  = 20.0f  ;
	m_bCanFire         = true   ;
	m_Starting_AR_Ammo = 30     ;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME          (UCombatComponent, m_EquippedWeapon                   ) ;
	DOREPLIFETIME          (UCombatComponent, m_bAiming                          ) ;
	DOREPLIFETIME          (UCombatComponent, m_CombatState                      ) ;
	DOREPLIFETIME_CONDITION(UCombatComponent, m_CarriedWeaponAmmo, COND_OwnerOnly) ;
}

// BeginPlay section START
//
//

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	m_bCanFire = true;

	if (m_Character)
	{
		m_Character->GetCharacterMovement()->MaxWalkSpeed = m_BaseWalkSpeed;

		if (m_Character->GET_FollowCamera())
		{
			m_DefaultFOV = m_Character->GET_FollowCamera()->FieldOfView ;
			m_CurrentFOV = m_DefaultFOV                                 ;
		}

		if (m_Character->HasAuthority())
		{
			INIT_CarriedAmmo();
		}
	}
}

void UCombatComponent::INIT_CarriedAmmo()
{
	m_CarriedAmmoMap.Emplace(EWeaponType::AssaultRifle, m_Starting_AR_Ammo); // All players will start the game with 30 assault rifle's ammo
}

//
// 
// BeginPlay section END

//
//

// Tick section START
//
//

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (m_Character && m_Character->IsLocallyControlled())
	{
		FHitResult _HitResult                ;
		TRACE_UnderCrossHairs(_HitResult)    ;
		m_HitTarget = _HitResult.ImpactPoint ;

		SET_HUDCrosshairs(DeltaTime) ;
		INTERP_FOV       (DeltaTime) ;
	}	
}

void UCombatComponent::TRACE_UnderCrossHairs(FHitResult& p_TraceHitResult)
{
	FVector2D _ViewportSize;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(_ViewportSize);
	}

	FVector2D _CrosshairLocation(_ViewportSize.X / 2.0f, _ViewportSize.Y / 2.0f) ;
	FVector   _CrosshairWorldPosition                                            ;
	FVector   _CrosshairWorldDirection                                           ;

	bool _bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld
	(
		UGameplayStatics::GetPlayerController(this, 0) ,
		_CrosshairLocation                             ,
		_CrosshairWorldPosition                        ,
		_CrosshairWorldDirection
	);

	if (_bScreenToWorld)
	{
		FVector _Start = _CrosshairWorldPosition;

		if (m_Character)
		{
			float _DistanceToCharacter = (m_Character->GetActorLocation() - _Start).Size()           ;
			_Start                     += _CrosshairWorldDirection * (_DistanceToCharacter + 100.0f) ;
		}

		FVector _End = _Start + _CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(p_TraceHitResult, _Start, _End, ECollisionChannel::ECC_Visibility);

		// Checking if the hit actor implements our interface
		if (p_TraceHitResult.GetActor() && p_TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			m_HUDPackage.s_CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			m_HUDPackage.s_CrosshairsColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SET_HUDCrosshairs(float p_DeltaTime)
{
	if (m_Character == nullptr || m_Character->Controller == nullptr)
	{
		return;
	}

	// If m_PlayerController is NULL, then Cast (rather expensive operation). If it is not NULL, then set it to itself (very cheap operation)
	m_PlayerController = m_PlayerController == nullptr ? Cast<ASalnaxarPlayerController>(m_Character->Controller) : m_PlayerController;

	if (m_PlayerController)
	{
		m_HUD = m_HUD == nullptr ? Cast<ASalnaxarHUD>(m_PlayerController->GetHUD()) : m_HUD;

		if (m_HUD)
		{
			if (m_EquippedWeapon)
			{
				m_HUDPackage.s_CrosshairsCenter = m_EquippedWeapon->GET_CrosshairsCenter() ;
				m_HUDPackage.s_CrosshairsRight  = m_EquippedWeapon->GET_CrosshairsRight () ;
				m_HUDPackage.s_CrosshairsLeft   = m_EquippedWeapon->GET_CrosshairsLeft  () ;
				m_HUDPackage.s_CrosshairsTop    = m_EquippedWeapon->GET_CrosshairsTop   () ;
				m_HUDPackage.s_CrosshairsBottom = m_EquippedWeapon->GET_CrosshairsBottom() ;
			}
			else
			{
			m_HUDPackage.s_CrosshairsCenter = nullptr ;
			m_HUDPackage.s_CrosshairsRight  = nullptr ;
			m_HUDPackage.s_CrosshairsLeft   = nullptr ;
			m_HUDPackage.s_CrosshairsTop    = nullptr ;
			m_HUDPackage.s_CrosshairsBottom = nullptr ;
			}

			// Calculate crosshair spread

			// Map the walk speed from 0 -> 600 to 0 -> 1
			// Also expressed as [0, 600] -> [0, 1]
			FVector2D _WalkSpeedRange         (0.0f, m_Character->GetCharacterMovement()->MaxWalkSpeed) ;
			FVector2D _VelocityMultiplierRange(0.0f, 1.0f)                                              ;
			FVector   _Velocity = m_Character->GetVelocity()                                            ;
			_Velocity.Z = 0.0f                                                                          ;

			m_CrosshairsVelocityFactor = FMath::GetMappedRangeValueClamped(_WalkSpeedRange, _VelocityMultiplierRange, _Velocity.Size());

			if (m_Character->GetCharacterMovement()->IsFalling())
			{
				m_CrosshairsInAirFactor = FMath::FInterpTo(m_CrosshairsInAirFactor, 2.25f, p_DeltaTime, 2.25f);
			}
			else
			{
				m_CrosshairsInAirFactor = FMath::FInterpTo(m_CrosshairsInAirFactor, 0.0f, p_DeltaTime, 30.0f);
			}

			if (m_bAiming)
			{
				m_CrosshairsAimFactor = FMath::FInterpTo(m_CrosshairsAimFactor, 0.58f, p_DeltaTime, 30.0f);
			}
			else
			{
				m_CrosshairsAimFactor = FMath::FInterpTo(m_CrosshairsAimFactor, 0.0f, p_DeltaTime, 30.0f);
			}

			m_CrosshairsShootingFactor = FMath::FInterpTo(m_CrosshairsShootingFactor, 0.0f, p_DeltaTime, 40.0f);

			m_HUDPackage.s_CrosshairsSpread =
				0.5f                        +
				m_CrosshairsVelocityFactor  +
				m_CrosshairsInAirFactor     -
				m_CrosshairsAimFactor       +
				m_CrosshairsShootingFactor  ;

			m_HUD->SET_HUDPackage(m_HUDPackage);
		}
	}
}

void UCombatComponent::INTERP_FOV(float p_DeltaTime)
{
	if (m_EquippedWeapon == nullptr)
	{
		return;
	}

	if (m_bAiming)
	{
		m_CurrentFOV = FMath::FInterpTo(m_CurrentFOV, m_EquippedWeapon->GET_ZoomedFOV(), p_DeltaTime, m_EquippedWeapon->GET_ZoomInterpSpeed());
	}
	else
	{
		m_CurrentFOV = FMath::FInterpTo(m_CurrentFOV, m_DefaultFOV, p_DeltaTime, m_ZoomInterpSpeed);
	}

	if (m_Character && m_Character->GET_FollowCamera())
	{
		m_Character->GET_FollowCamera()->SetFieldOfView(m_CurrentFOV);
	}
}

//
//
// Tick section END

//
// 


// Equipping weapon section START
//
//

void UCombatComponent::EQUIP_Weapon(AWeapon* p_WeaponToEquip)
{
	if (m_Character == nullptr || p_WeaponToEquip == nullptr)
	{
		return;
	}

	if (m_EquippedWeapon)
	{
		m_EquippedWeapon->DROP_Weapon();
	}

	m_EquippedWeapon = p_WeaponToEquip                                                                              ;
	m_EquippedWeapon->SET_WeaponState(EWeaponState::EquippedState)                                                  ;
	const USkeletalMeshSocket* _RightHandSocket = m_Character->GetMesh()->GetSocketByName(FName("RightHandSocket")) ;

	if (_RightHandSocket)
	{
		_RightHandSocket->AttachActor(m_EquippedWeapon, m_Character->GetMesh()) ;
		m_EquippedWeapon->SetOwner(m_Character)                                 ;
		m_EquippedWeapon->SET_HUDAmmo()                                         ;

		if (m_CarriedAmmoMap.Contains(m_EquippedWeapon->GET_WeaponType()))
		{
			m_CarriedWeaponAmmo = m_CarriedAmmoMap[m_EquippedWeapon->GET_WeaponType()];
		}

		if (m_PlayerController == nullptr)
		{
			m_PlayerController = Cast<ASalnaxarPlayerController>(m_Character->Controller);
		}

		if (m_PlayerController)
		{
			m_PlayerController->SET_HUDCarriedWeaponAmmoAmount(m_CarriedWeaponAmmo);
		}

		if (m_EquippedWeapon->GET_EquipSound())
		{
			UGameplayStatics::PlaySoundAtLocation(this, m_EquippedWeapon->GET_EquipSound(), m_Character->GetActorLocation());
		}

		if (m_EquippedWeapon->IS_MagEmpty())
		{
			RELOAD_Weapon();
		}

		m_Character->GetCharacterMovement()->bOrientRotationToMovement = false ;
		m_Character->bUseControllerRotationYaw                         = true  ;
	}
}

void UCombatComponent::OnRep_m_EquippedWeapon()
{
	if (m_EquippedWeapon && m_Character)
	{
		m_EquippedWeapon->SET_WeaponState(EWeaponState::EquippedState);

		// The const and the if line were added after seeing video 107 at 14:43min. Didn't notice such changes beforehand. Weird.
		const USkeletalMeshSocket* _RightHandSocket = m_Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (_RightHandSocket)
		{
			_RightHandSocket->AttachActor(m_EquippedWeapon, m_Character->GetMesh());
		}

		m_Character->GetCharacterMovement()->bOrientRotationToMovement = false ;
		m_Character->bUseControllerRotationYaw                         = true  ;

		if (m_EquippedWeapon->GET_EquipSound())
		{
			UGameplayStatics::PlaySoundAtLocation(this, m_EquippedWeapon->GET_EquipSound(), m_Character->GetActorLocation());
		}
	}
}

//
// 
// Equipping weapon section END

//
// 

// Aiming section START
//
//

void UCombatComponent::SET_Aiming(bool p_bIsAiming)
{
	m_bAiming = p_bIsAiming;

	// We don't need to check whether we are on the server or the client side since both cases will end up calling the function on the server
	// For more info, check the "52.Aiming" video of the course from 13min30sec
	SERVER_SET_Aiming(m_bAiming);

	if (m_Character)
	{
		m_Character->GetCharacterMovement()->MaxWalkSpeed = m_bAiming ? m_AimWalkSpeed : m_BaseWalkSpeed;
	}
}

void UCombatComponent::SERVER_SET_Aiming_Implementation(bool p_bIsAiming)
{
	m_bAiming = p_bIsAiming;

	if (m_Character)
	{
		m_Character->GetCharacterMovement()->MaxWalkSpeed = m_bAiming ? m_AimWalkSpeed : m_BaseWalkSpeed;
	}
}

//
//
// Aiming section END

//
//

// Fire section START
//
//

void UCombatComponent::PRESSED_FireButton(bool p_Pressed)
{
	m_bFireButtonPressed = p_Pressed;

	if (m_bFireButtonPressed)
	{
		FIRE_Weapon();
	}
}

void UCombatComponent::FIRE_Weapon()
{
	UE_LOG(LogTemp, Warning, TEXT("m_bCanFire is FALSE"));

	if (CAN_Fire())
	{
		UE_LOG(LogTemp, Warning, TEXT("3 || FIRE_Weapon in CombatComponent OK"));

		m_bCanFire = false;

		SERVER_Fire(m_HitTarget);

		if (m_EquippedWeapon)
		{
			m_CrosshairsShootingFactor = 0.75f;
		}

		TIMER_StartFireTimer();
	}
}

bool UCombatComponent::CAN_Fire()
{
	if (m_EquippedWeapon == nullptr)
	{
		return false;
	}

	if (m_EquippedWeapon->IS_MagEmpty() == true || m_bCanFire == false || m_CombatState == ECombatState::Reloading)
	{
		return false;
	}

	return true;
}

void UCombatComponent::TIMER_StartFireTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("Fire Timer is starting !"));

	if (m_EquippedWeapon == nullptr || m_Character == nullptr)
	{
		return;
	}

	m_Character->GetWorldTimerManager().SetTimer(m_FireTimer, this, &UCombatComponent::TIMER_FireFinished, m_EquippedWeapon->GET_FireDelay());
}

void UCombatComponent::TIMER_FireFinished()
{
	if (m_EquippedWeapon == nullptr)
	{
		return;
	}

	m_bCanFire = true;

	if (m_bFireButtonPressed && m_EquippedWeapon->GET_IsWeaponAutomatic())
	{
		FIRE_Weapon();
	}

	if (m_EquippedWeapon->IS_MagEmpty())
	{
		RELOAD_Weapon();
	}
}

void UCombatComponent::SERVER_Fire_Implementation(const FVector_NetQuantize& p_TraceHitTarget)
{
	MULTICAST_Fire(p_TraceHitTarget);
}

void UCombatComponent::MULTICAST_Fire_Implementation(const FVector_NetQuantize& p_TraceHitTarget)
{
	if (m_EquippedWeapon == nullptr)
	{
		return;
	}

	if (m_Character && m_CombatState == ECombatState::Unoccupied)
	{
		m_Character->PLAY_FireMontage(m_bAiming) ;
		m_EquippedWeapon->FIRE(p_TraceHitTarget) ;
	}
}

//
//
// Fire section END

//
//

// Reload section START
//
//

void UCombatComponent::RELOAD_Weapon()
{
	if (m_CarriedWeaponAmmo > 0 && m_CombatState != ECombatState::Reloading)
	{
		SERVER_ReloadWeapon();
	}
}

void UCombatComponent::SERVER_ReloadWeapon_Implementation()
{
	if (m_Character == nullptr || m_EquippedWeapon == nullptr)
	{
		return;
	}

	m_CombatState = ECombatState::Reloading ;
	HANDLE_Reload()                         ;
}

void UCombatComponent::HANDLE_Reload()
{
	m_Character->PLAY_ReloadWeaponMontage();
}

void UCombatComponent::RELOAD_Finished()
{
	if (m_Character == nullptr)
	{
		return;
	}

	if (m_Character->HasAuthority())
	{
		m_CombatState = ECombatState::Unoccupied ;
		UPDATE_AmmoValues()                      ;
	}

	if (m_bFireButtonPressed)
	{
		FIRE_Weapon();
	}
}

//
//
// Reload section END

//
//

 
// Ammo section START
//
//

void UCombatComponent::UPDATE_AmmoValues()
{
	if (m_Character == nullptr || m_EquippedWeapon == nullptr)
	{
		return;
	}

	int32 _ReloadAmount = CALCULATE_AmountToReload();

	if (m_CarriedAmmoMap.Contains(m_EquippedWeapon->GET_WeaponType()))
	{
		m_CarriedAmmoMap[m_EquippedWeapon->GET_WeaponType()] -= _ReloadAmount                                       ;
		m_CarriedWeaponAmmo                                  = m_CarriedAmmoMap[m_EquippedWeapon->GET_WeaponType()] ;
	}

	if (m_PlayerController == nullptr)
	{
		m_PlayerController = Cast<ASalnaxarPlayerController>(m_Character->Controller);
	}

	if (m_PlayerController)
	{
		m_PlayerController->SET_HUDCarriedWeaponAmmoAmount(m_CarriedWeaponAmmo);
	}

	m_EquippedWeapon->ADD_Ammo(-_ReloadAmount);
}

int32 UCombatComponent::CALCULATE_AmountToReload()
{
	if (m_EquippedWeapon == nullptr)
	{
		return 0;
	}

	int32 _RoomInMag = m_EquippedWeapon->GET_MagCapacity() - m_EquippedWeapon->GET_Ammo();

	if (m_CarriedAmmoMap.Contains(m_EquippedWeapon->GET_WeaponType()))
	{
		int32 _CarriedAmmoAmount = m_CarriedAmmoMap[m_EquippedWeapon->GET_WeaponType()] ;
		int32 _Least             = FMath::Min(_RoomInMag, _CarriedAmmoAmount)           ;

		return FMath::Clamp(_RoomInMag, 0, _Least);
	}

	return 0;
}

void UCombatComponent::OnRep_m_CarriedWeaponAmmo()
{
	if (m_PlayerController == nullptr)
	{
		m_PlayerController = Cast<ASalnaxarPlayerController>(m_Character->Controller);
	}

	if (m_PlayerController)
	{
		m_PlayerController->SET_HUDCarriedWeaponAmmoAmount(m_CarriedWeaponAmmo);
	}
}

//
//
// Ammo section END

//
//

// Other section START
// 
//

void UCombatComponent::OnRep_m_CombatState()
{
	switch (m_CombatState)
	{
	case ECombatState::Reloading:
		HANDLE_Reload();
		break;

	case ECombatState::Unoccupied:
		if (m_bFireButtonPressed)
		{
			FIRE_Weapon();
		}
		break;
	}
}

//
//
// Other section END