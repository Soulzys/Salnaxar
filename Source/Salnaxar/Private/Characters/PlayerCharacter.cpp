#include "Characters/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Salnaxar/Public/Weapons/Weapon.h"
#include "Salnaxar/Public/Weapons/WeaponTypes.h"
#include "Salnaxar/Public/Components/CombatComponent.h"
#include "Salnaxar/Public/Characters/PlayerAnimInstance.h"
#include "Salnaxar/Public/PlayerController/SalnaxarPlayerController.h"
#include "Salnaxar/Public/GameModes/SalnaxarGameMode.h"
#include "Salnaxar/Public/PlayerStates/SalnaxarPlayerState.h"
#include "Salnaxar/Public/Interactif/Door.h"
#include "Salnaxar/Salnaxar.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

APlayerCharacter::APlayerCharacter()
{
	// 
	// *** Variable initialization
	//
	// 
	// ** Mother class(es) variables
	//
	PrimaryActorTick.bCanEverTick                     = true                                                            ;
	bUseControllerRotationYaw                         = false                                                           ;
	NetUpdateFrequency                                = 66.0f                                                           ;
	MinNetUpdateFrequency                             = 33.0f                                                           ;
	GetCharacterMovement()->bOrientRotationToMovement = true                                                            ;
	GetCharacterMovement()->NavAgentProps.bCanCrouch  = true                                                            ;
	GetCharacterMovement()->RotationRate              = FRotator(0.0f, 0.0f, 850.0f)                                    ;
	// The following line reads as "this capsule component will be ignored by the camera channel"
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore) ;
	GetMesh()->SetCollisionObjectType       (ECC_SkeletalMesh)                                                          ;
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore)             ;
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block)          ;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn                   ;

	// ** this class variables
	//
	m_TurningInPlace  = ETurningInPlace::NotTurning ;
	m_CameraThreshold = 200.0f                      ;
	m_TurnThreshold   = 0.5f                        ;
	m_MaxHealth       = 100.0f                      ;
	m_Health          = 100.0f                      ;
	m_bEliminated     = false                       ;
	m_ElimDelay       = 3.0f                        ;

	//
	// *** Component initialization
	//
	//
	m_CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")) ;
	m_CameraBoom->SetupAttachment(GetMesh())                                       ; // Attached to the mesh rather than the root so it works despite capsule size variations during crouching.
	m_CameraBoom->TargetArmLength         = 600.0f                                 ;
	m_CameraBoom->bUsePawnControlRotation = true                                   ;

	m_FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera")) ;
	m_FollowCamera->SetupAttachment(m_CameraBoom, USpringArmComponent::SocketName)  ;
	//m_FollowCamera->SetupAttachment(GetMesh(), FName("HeadSocket"))               ;
	m_FollowCamera->bUsePawnControlRotation   = false                               ;
	//m_FollowCamera->bUsePawnControlRotation = true                                ;

	m_OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget")) ;
	m_OverheadWidget->SetupAttachment(RootComponent)                                    ;

	m_CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp")) ;
	m_CombatComp->SetIsReplicated(true)                                         ;

	m_DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APlayerCharacter, m_OverlappingWeapon, COND_OwnerOnly) ;
	DOREPLIFETIME          (APlayerCharacter, m_Health)                            ;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UPDATE_HUDHealth();

	// Only runs on the server
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &APlayerCharacter::RECEIVE_Damage);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// The > sign works since it is an Enum, hence int, placed in a specific order
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		COMPUTE_AimOffset(DeltaTime);
	}
	else // The rest of the else section is actually within OnRep_ReplicatedMovement (Explanations in video n92)
	{
		m_TimeSinceLastMovementReplication += DeltaTime;

		if (m_TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}

		COMPUTE_AimOffsetPitch();
	}

	HIDE_CameraIfCharacterClose() ;
	POLL_Init                  () ;
}

void APlayerCharacter::COMPUTE_AimOffset(float p_DeltaTime)
{
	if (m_CombatComp && m_CombatComp->m_EquippedWeapon == nullptr)
	{
		return;
	}

	float _Speed    = COMPUTE_Speed()                     ;
	bool  _bIsInAir = GetCharacterMovement()->IsFalling() ;

	if (_Speed == 0.0f && !_bIsInAir) // Standing still and not jumping
	{
		m_bRotateRootBone            = true                                                                                   ;
		FRotator _CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f)                                         ;
		FRotator _DeltaAimRotation   = UKismetMathLibrary::NormalizedDeltaRotator(_CurrentAimRotation, m_StartingAimRotation) ;
		m_AO_Yaw                     = _DeltaAimRotation.Yaw                                                                  ;

		if (m_TurningInPlace == ETurningInPlace::NotTurning)
		{
			m_Interp_AO_Yaw = m_AO_Yaw;
		}

		bUseControllerRotationYaw = true;

		COMPUTE_TurningInPlace(p_DeltaTime);
	}

	if (_Speed > 0.0f || _bIsInAir) // Running or jumping
	{
		m_bRotateRootBone         = false                                          ;
		m_StartingAimRotation     = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f) ;
		m_AO_Yaw                  = 0.0f                                           ;
		bUseControllerRotationYaw = true                                           ;
		m_TurningInPlace          = ETurningInPlace::NotTurning                    ;
	}

	COMPUTE_AimOffsetPitch();
}

void APlayerCharacter::COMPUTE_TurningInPlace(float p_DeltaTime)
{
	if (m_AO_Yaw > 90.0f)
	{
		m_TurningInPlace = ETurningInPlace::Right;
	}
	else if (m_AO_Yaw < -90.0f)
	{
		m_TurningInPlace = ETurningInPlace::Left;
	}

	if (m_TurningInPlace != ETurningInPlace::NotTurning)
	{
		m_Interp_AO_Yaw = FMath::FInterpTo(m_Interp_AO_Yaw, 0.0f, p_DeltaTime, 4.0f) ;
		m_AO_Yaw        = m_Interp_AO_Yaw                                            ;

		if (FMath::Abs(m_AO_Yaw) < 15.0f)
		{
			m_TurningInPlace      = ETurningInPlace::NotTurning                    ;
			m_StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f) ;
		}
	}
}

void APlayerCharacter::COMPUTE_AimOffsetPitch()
{
	m_AO_Pitch = GetBaseAimRotation().Pitch;

	if (m_AO_Pitch > 90.0f && IsLocallyControlled() == false)
	{
		// Map pitch from [270, 360) to [-90, 0)
		FVector2D _InRange ( 270.0f , 360.0f)                                           ;
		FVector2D _OutRange(-90.0f  , 0.0f  )                                           ;
		m_AO_Pitch = FMath::GetMappedRangeValueClamped(_InRange, _OutRange, m_AO_Pitch) ;
	}
}

void APlayerCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	TURN_SimulatedProxies();

	m_TimeSinceLastMovementReplication = 0.0f;
}

void APlayerCharacter::TURN_SimulatedProxies()
{
	if (m_CombatComp == nullptr || m_CombatComp->m_EquippedWeapon == nullptr)
	{
		return;
	}

	m_bRotateRootBone = false;

	float _Speed = COMPUTE_Speed();

	if (_Speed > 0.0f)
	{
		m_TurningInPlace = ETurningInPlace::NotTurning;
		return;
	}

	m_ProxyRotationLastFrame = m_ProxyRotation;
	m_ProxyRotation = GetActorRotation();
	m_ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(m_ProxyRotation, m_ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(m_ProxyYaw) > m_TurnThreshold)
	{
		if (m_ProxyYaw > m_TurnThreshold)
		{
			m_TurningInPlace = ETurningInPlace::Right;
		}
		else if (m_ProxyYaw < -m_TurnThreshold)
		{
			m_TurningInPlace = ETurningInPlace::Left;
		}
		else
		{
			m_TurningInPlace = ETurningInPlace::NotTurning;
		}

		return;
	}

	m_TurningInPlace = ETurningInPlace::NotTurning;
}

void APlayerCharacter::HIDE_CameraIfCharacterClose()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if ((m_FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < m_CameraThreshold)
	{
		GetMesh()->SetVisibility(false);

		if (m_CombatComp && m_CombatComp->m_EquippedWeapon && m_CombatComp->m_EquippedWeapon->GET_WeaponMesh())
		{
			m_CombatComp->m_EquippedWeapon->GET_WeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);

		if (m_CombatComp && m_CombatComp->m_EquippedWeapon && m_CombatComp->m_EquippedWeapon->GET_WeaponMesh())
		{
			m_CombatComp->m_EquippedWeapon->GET_WeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void APlayerCharacter::POLL_Init()
{
	if (m_SalnaxarPlayerState == nullptr)
	{
		m_SalnaxarPlayerState = GetPlayerState<ASalnaxarPlayerState>();

		if (m_SalnaxarPlayerState)
		{
			m_SalnaxarPlayerState->ADD_ToScore(0.0f);
			m_SalnaxarPlayerState->ADD_ToDeathCount(0);
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	INIT_Inputs();

	PlayerInputComponent->BindAction("Jump"      , IE_Pressed , this, &APlayerCharacter::Jump                       );
	PlayerInputComponent->BindAction("Equip"     , IE_Pressed , this, &APlayerCharacter::ACTION_EquipKey_PRESSED    );
	PlayerInputComponent->BindAction("Crouch"    , IE_Pressed , this, &APlayerCharacter::ACTION_CrouchKey_PRESSED   );
	PlayerInputComponent->BindAction("Aim"       , IE_Pressed , this, &APlayerCharacter::ACTION_AimKey_PRESSED      );
	PlayerInputComponent->BindAction("Aim"       , IE_Released, this, &APlayerCharacter::ACTION_AimKey_RELEASED     );
	PlayerInputComponent->BindAction("Fire"      , IE_Pressed , this, &APlayerCharacter::ACTION_FireKey_PRESSED     );
	PlayerInputComponent->BindAction("Fire"      , IE_Released, this, &APlayerCharacter::ACTION_FireKey_RELEASED    );
	PlayerInputComponent->BindAction("Interact"  , IE_Pressed , this, &APlayerCharacter::ACTION_InteractKey_PRESSED );
	PlayerInputComponent->BindAction("Profile"   , IE_Pressed , this, &APlayerCharacter::ACTION_ProfileKey_PRESSED  );
	PlayerInputComponent->BindAction("Inventory" , IE_Pressed , this, &APlayerCharacter::ACTION_InventoryKey_PRESSED);
	PlayerInputComponent->BindAction("DropItem"  , IE_Pressed , this, &APlayerCharacter::ACTION_DropItem_PRESSED    ); 
	PlayerInputComponent->BindAction("DisplayMap", IE_Pressed , this, &APlayerCharacter::ACTION_DisplayMap_PRESSED  );
	PlayerInputComponent->BindAction("DisplayMap", IE_Released, this, &APlayerCharacter::ACTION_DisplayMap_RELEASED );
	PlayerInputComponent->BindAction("Reload"    , IE_Pressed , this, &APlayerCharacter::ACTION_Reload_PRESSED      );
	PlayerInputComponent->BindAction("Chat"      , IE_Pressed , this, &APlayerCharacter::ACTION_ChatKey_PRESSED     );

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::ACTION_MoveForwardKey_PRESSED);
	PlayerInputComponent->BindAxis("MoveRight"  , this, &APlayerCharacter::ACTION_MoveRightKey_PRESSED  );
	PlayerInputComponent->BindAxis("Turn"       , this, &APlayerCharacter::ACTION_TurnKey_PRESSED       );
	PlayerInputComponent->BindAxis("LookUp"     , this, &APlayerCharacter::ACTION_LookUpKey_PRESSED     );
}

void APlayerCharacter::INIT_Inputs()
{
	static bool _bAreBindingsAdded = false;

	if (_bAreBindingsAdded == false)
	{
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Jump"      , EKeys::SpaceBar        ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Equip"     , EKeys::E               ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Crouch"    , EKeys::LeftControl     ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Aim"       , EKeys::RightMouseButton));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Fire"      , EKeys::LeftMouseButton ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Interact"  , EKeys::E               ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Profile"   , EKeys::C               ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Inventory" , EKeys::B               ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("DropItem"  , EKeys::G               ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("DisplayMap", EKeys::M               ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Reload"    , EKeys::R               ));
		UPlayerInput::AddEngineDefinedActionMapping(FInputActionKeyMapping("Chat"      , EKeys::Enter           ));

		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::W     ,  1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::S     , -1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveRight"  , EKeys::A     , -1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveRight"  , EKeys::D     ,  1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("Turn"       , EKeys::MouseX,  1.0f));
		UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("LookUp"     , EKeys::MouseY, -1.0f));

		_bAreBindingsAdded = true;
	}
}

void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (m_CombatComp)
	{
		m_CombatComp->m_Character = this;
	}
}

void APlayerCharacter::ONREP_m_OverlappingWeapon(AWeapon* p_LastWeapon)
{
	if (m_OverlappingWeapon)
	{
		m_OverlappingWeapon->SHOW_PickupWidget(true);
	}

	if (p_LastWeapon)
	{
		p_LastWeapon->SHOW_PickupWidget(false);
	}
}

void APlayerCharacter::ONREP_m_Health()
{
	UPDATE_HUDHealth();
	PLAY_HitReactMontage(); // Runs on all clients
}

bool APlayerCharacter::IS_WeaponEquipped()
{
	if (m_CombatComp && m_CombatComp->m_EquippedWeapon)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool APlayerCharacter::IS_Aiming()
{
	if (m_CombatComp && m_CombatComp->m_bAiming)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void APlayerCharacter::PLAY_FireMontage(bool p_bAiming)
{
	if (m_CombatComp == nullptr || m_CombatComp->m_EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* _AnimInstance = GetMesh()->GetAnimInstance();

	if (_AnimInstance && m_FireWeaponMontage)
	{
		_AnimInstance->Montage_Play(m_FireWeaponMontage);

		FName _SectionName;
		_SectionName = p_bAiming ? FName("RifleAim") : FName("RifleHip");

		_AnimInstance->Montage_JumpToSection(_SectionName);
	}
}

void APlayerCharacter::PLAY_HitReactMontage()
{
	if (m_CombatComp == nullptr || m_CombatComp->m_EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* _AnimInstance = GetMesh()->GetAnimInstance();

	if (_AnimInstance && m_HitReactMontage)
	{
		_AnimInstance->Montage_Play(m_HitReactMontage);

		FName _SectionName("FromFront");

		_AnimInstance->Montage_JumpToSection(_SectionName);
	}
}

float APlayerCharacter::COMPUTE_Speed()
{
	FVector _Velocity = GetVelocity();
	_Velocity.Z = 0.0f;
	return _Velocity.Size();
}

void APlayerCharacter::RECEIVE_Damage
(
	AActor* p_DamageActor,
	float p_Damage,
	const UDamageType* p_DamageType,
	AController* p_InstigatorController,
	AActor* p_DamageCauser
)
{
	m_Health = FMath::Clamp(m_Health - p_Damage, 0.0f, m_MaxHealth);
	UPDATE_HUDHealth();
	PLAY_HitReactMontage(); // Only run on server

	if (m_Health == 0.0f)
	{
		ASalnaxarGameMode* _GameMode = GetWorld()->GetAuthGameMode<ASalnaxarGameMode>();

		if (_GameMode)
		{
			// Safety check. Makes sure we have a valid PlayerController
			if (m_SalnaxarPlayerController == nullptr)
			{
				m_SalnaxarPlayerController = Cast<ASalnaxarPlayerController>(Controller);
			}

			ASalnaxarPlayerController* _AttackerController = Cast<ASalnaxarPlayerController>(p_InstigatorController);

			_GameMode->ELIMINATE_Player(this, m_SalnaxarPlayerController, _AttackerController);
		}
	}	
}

void APlayerCharacter::UPDATE_HUDHealth()
{
	if (m_SalnaxarPlayerController == nullptr)
	{
		m_SalnaxarPlayerController = Cast<ASalnaxarPlayerController>(Controller);
	}

	if (m_SalnaxarPlayerController)
	{
		m_SalnaxarPlayerController->SET_HUDHealth(m_Health, m_MaxHealth);
	}
}

void APlayerCharacter::ELIMINATE()
{
	if (m_CombatComp && m_CombatComp->m_EquippedWeapon)
	{
		m_CombatComp->m_EquippedWeapon->DROP_Weapon();
	}

	MULTICAST_ELIMINATE();

	GetWorldTimerManager().SetTimer
	(
		m_ElimTimer, 
		this, 
		&APlayerCharacter::ElimTimer_FINISHED, 
		m_ElimDelay
	);
}

void APlayerCharacter::MULTICAST_ELIMINATE_Implementation()
{
	if (m_SalnaxarPlayerController)
	{
		m_SalnaxarPlayerController->SET_HUDWeaponAmmo(0);
	}

	m_bEliminated = true;
	PLAY_ElimMontage();

	// Start dissolve material effect
	if (m_DissolveMaterialInstance)
	{
		m_DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(m_DissolveMaterialInstance, this);

		GetMesh()->SetMaterial(0, m_DynamicDissolveMaterialInstance);
		m_DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		m_DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 150.0f);
	}

	START_Dissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately(); // Impedes rotating the character with the mouse. Basically, locks it in place

	if (m_SalnaxarPlayerController)
	{
		DisableInput(m_SalnaxarPlayerController);
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerCharacter::PLAY_ElimMontage()
{
	UAnimInstance* _AnimInstance = GetMesh()->GetAnimInstance();

	if (_AnimInstance && m_ElimMontage)
	{
		_AnimInstance->Montage_Play(m_ElimMontage);
	}
}

void APlayerCharacter::ElimTimer_FINISHED()
{
	ASalnaxarGameMode* _SalnaxarGameMode = GetWorld()->GetAuthGameMode<ASalnaxarGameMode>();

	if (_SalnaxarGameMode)
	{
		_SalnaxarGameMode->REQUEST_Respawn(this, Controller);
	}
}

void APlayerCharacter::UPDATE_DissolveMaterial(float p_DissolveValue)
{
	if (m_DynamicDissolveMaterialInstance)
	{
		m_DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), p_DissolveValue);
	}
}

void APlayerCharacter::START_Dissolve()
{
	m_DissolveTrack.BindDynamic(this, &APlayerCharacter::UPDATE_DissolveMaterial);

	if (m_DissolveCurve && m_DissolveTimeline)
	{
		m_DissolveTimeline->AddInterpFloat(m_DissolveCurve, m_DissolveTrack);
		m_DissolveTimeline->Play(); 
	}
}

void APlayerCharacter::ACTION_MoveForwardKey_PRESSED(float p_Value)
{
	if (Controller != nullptr && p_Value != 0.0f)
	{
		// We do it this way so that we base our movement onto the Mesh direction rather than the capsule component direction, which is the root. 
		//
		const FRotator _YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector _Direction(FRotationMatrix(_YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(_Direction, p_Value);
	}
}

void APlayerCharacter::ACTION_MoveRightKey_PRESSED(float p_Value)
{
	if (Controller != nullptr && p_Value != 0.0f)
	{
		// We do it this way so that we base our movement onto the Mesh direction rather than the capsule component direction, which is the root. 
		//
		const FRotator _YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector _Direction(FRotationMatrix(_YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(_Direction, p_Value);
	}
}

void APlayerCharacter::ACTION_TurnKey_PRESSED(float p_Value)
{
	AddControllerYawInput(p_Value);
}

void APlayerCharacter::ACTION_LookUpKey_PRESSED(float p_Value)
{
	AddControllerPitchInput(p_Value);
}

void APlayerCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void APlayerCharacter::ACTION_EquipKey_PRESSED()
{
	if (m_CombatComp)
	{
		if (HasAuthority()) // If this is a server.
		{
			m_CombatComp->EQUIP_Weapon(m_OverlappingWeapon);
		}
		else // If this is a client.
		{
			SERVER_ACTION_EquipKey_PRESSED();
		}
	}
}

void APlayerCharacter::SERVER_ACTION_EquipKey_PRESSED_Implementation()
{
	if (m_CombatComp)
	{
		m_CombatComp->EQUIP_Weapon(m_OverlappingWeapon);
	}
}

void APlayerCharacter::ACTION_CrouchKey_PRESSED()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void APlayerCharacter::ACTION_AimKey_PRESSED()
{
	if (m_CombatComp)
	{
		m_CombatComp->SET_Aiming(true);
	}
}

void APlayerCharacter::ACTION_AimKey_RELEASED()
{
	if (m_CombatComp)
	{
		m_CombatComp->SET_Aiming(false);
	}
}

void APlayerCharacter::ACTION_FireKey_PRESSED()
{
	if (m_CombatComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("1 || PRESSED_FireButton in PlayerCharacter OK"));
		m_CombatComp->PRESSED_FireButton(true);
	}
}

void APlayerCharacter::ACTION_FireKey_RELEASED()
{
	if (m_CombatComp)
	{
		m_CombatComp->PRESSED_FireButton(false);
	}
}

void APlayerCharacter::ACTION_InteractKey_PRESSED()
{
	m_OnInteractDelegate.Broadcast(this);
}

void APlayerCharacter::ACTION_ProfileKey_PRESSED()
{

}

void APlayerCharacter::ACTION_InventoryKey_PRESSED()
{

}

void APlayerCharacter::ACTION_DropItem_PRESSED()
{

}

void APlayerCharacter::ACTION_DisplayMap_PRESSED()
{

}

void APlayerCharacter::ACTION_DisplayMap_RELEASED()
{

}

void APlayerCharacter::ACTION_Reload_PRESSED()
{
	if (m_CombatComp)
	{
		m_CombatComp->RELOAD_Weapon();
	}
}

void APlayerCharacter::ACTION_ChatKey_PRESSED()
{

}

void APlayerCharacter::SERVER_ACTIVATION_Client_Implementation(ADoor* p_Door)
{
	m_Door = p_Door;

	if (m_Door)
	{
		m_Door->ACTIVATION_Server(m_Door->m_ServerDoorState);
	}
}

void APlayerCharacter::PLAY_ReloadWeaponMontage()
{
	if (m_CombatComp == nullptr || m_CombatComp->m_EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* _AnimInstance = GetMesh()->GetAnimInstance();

	if (_AnimInstance && m_ReloadWeaponMontage)
	{
		_AnimInstance->Montage_Play(m_ReloadWeaponMontage);
		FName _SectionName;

		switch (m_CombatComp->m_EquippedWeapon->GET_WeaponType())
		{
			case EWeaponType::AssaultRifle:
				_SectionName = FName("Rifle");
				break;
		}

		_AnimInstance->Montage_JumpToSection(_SectionName);
	}
}

//
// GETTERS
//

AWeapon* APlayerCharacter::GET_EquippedWeapon()
{
	if (m_CombatComp == nullptr)
	{
		return nullptr;
	}

	return m_CombatComp->m_EquippedWeapon;
}

FVector APlayerCharacter::GET_HitTarget() const
{
	if (m_CombatComp == nullptr)
	{
		return FVector();
	}

	return m_CombatComp->m_HitTarget;
}

ECombatState APlayerCharacter::GET_CombatState() const
{
	if (m_CombatComp == nullptr)
	{
		return ECombatState::MAX;
	}

	return m_CombatComp->m_CombatState;
}

//
// SETTERS
//

void APlayerCharacter::SET_OverlappingWeapon(AWeapon* p_Weapon)
{
	if (m_OverlappingWeapon)
	{
		m_OverlappingWeapon->SHOW_PickupWidget(false);
	}

	m_OverlappingWeapon = p_Weapon;

	if (IsLocallyControlled())
	{
		if (m_OverlappingWeapon)
		{
			m_OverlappingWeapon->SHOW_PickupWidget(true);
		}
	}
}