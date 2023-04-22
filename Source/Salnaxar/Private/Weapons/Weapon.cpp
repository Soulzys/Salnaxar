#include "Weapons/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Salnaxar/Public/Characters/PlayerCharacter.h"
#include "Salnaxar/Public/PlayerController/SalnaxarPlayerController.h"
#include "Salnaxar/Public/Weapons/Casing.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapon::AWeapon()
{
	//
	// *** Variables initialization
	//
	//
	// ** Mother class(es) variables
	//
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// ** This class variables
	//
	m_ZoomedFOV          = 30.0f ;
	m_ZoomInterpSpeed    = 20.0f ;
	m_FireDelay          = 0.15f ;
	m_bIsWeaponAutomatic = true  ;

	//
	// *** Components initialization
	//
	//
	m_SKC_WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"))                        ;
	m_SKC_WeaponMesh->SetupAttachment(RootComponent)                                                             ;
	m_SKC_WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block)                           ;
	m_SKC_WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore) ;
	m_SKC_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision)                                        ;
	SetRootComponent(m_SKC_WeaponMesh)                                                                           ;

	m_SC_AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"))     ;
	m_SC_AreaSphere->SetupAttachment(RootComponent)                                    ;
	m_SC_AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore) ;
	m_SC_AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision)               ;

	m_WC_Pickup = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup")) ;
	m_WC_Pickup->SetupAttachment(RootComponent)                            ;
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, m_WeaponState) ;
	DOREPLIFETIME(AWeapon, m_Ammo)        ;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	// This ensures this piece of code will only run on the server
	if (HasAuthority())
	{
		m_SC_AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)                                     ;
		m_SC_AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap) ;
		m_SC_AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap)                         ;
		m_SC_AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap)                        ;
	}

	if (m_WC_Pickup)
	{
		m_WC_Pickup->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::OnSphereOverlap
(
	UPrimitiveComponent* OverlappedComponent ,
	AActor*              OtherActor          ,
	UPrimitiveComponent* OtherComp           ,
	int32                OtherBodyIndex      ,
	bool                 bFromSweep          ,
	const FHitResult&    SweepResult
)
{
	APlayerCharacter* _PlayerCharacter = Cast<APlayerCharacter>(OtherActor);

	if (_PlayerCharacter)
	{
		_PlayerCharacter->SET_OverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap
(
	UPrimitiveComponent* OverlappedComponent ,
	AActor*              OtherActor          ,
	UPrimitiveComponent* OtherComp           ,
	int32                OtherBodyIndex
)
{
	APlayerCharacter* _PlayerCharacter = Cast<APlayerCharacter>(OtherActor);

	if (_PlayerCharacter)
	{
		_PlayerCharacter->SET_OverlappingWeapon(nullptr);
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		m_OwnerPlayerCharacter  = nullptr ;
		m_OwnerPlayerController = nullptr ;
	}
	else
	{
		SET_HUDAmmo();
	}
}

void AWeapon::OnRep_m_Ammo()
{
	// No need to substract an Ammo as it has already been replicated

	SET_HUDAmmo();
}

// *** Handles WeaponState changes START
//
//

void AWeapon::SET_WeaponState(EWeaponState p_State)
{
	m_WeaponState = p_State;

	switch (m_WeaponState)
	{
		case EWeaponState::EquippedState                                          :
			SHOW_PickupWidget(false)                                              ;
			m_SC_AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision)  ;
			m_SKC_WeaponMesh->SetSimulatePhysics(false)                           ;
			m_SKC_WeaponMesh->SetEnableGravity(false)                             ;
			m_SKC_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision) ;
			break                                                                 ;

		case EWeaponState::DroppedState:

			if (HasAuthority())
			{
				m_SC_AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}

			m_SKC_WeaponMesh->SetSimulatePhysics(true)                                ;
			m_SKC_WeaponMesh->SetEnableGravity(true)                                  ;
			m_SKC_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics) ;
			break                                                                     ;
	}
}

void AWeapon::OnRep_m_WeaponState()
{
	switch (m_WeaponState)
	{
		case EWeaponState::EquippedState                                          :
			SHOW_PickupWidget(false)                                              ;
			m_SC_AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision)  ;
			m_SKC_WeaponMesh->SetSimulatePhysics(false)                           ;
			m_SKC_WeaponMesh->SetEnableGravity(false)                             ;
			m_SKC_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision) ;
			break                                                                 ;

		case EWeaponState::DroppedState                                               :
			m_SKC_WeaponMesh->SetSimulatePhysics(true)                                ;
			m_SKC_WeaponMesh->SetEnableGravity(true)                                  ;
			m_SKC_WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics) ;
	}
}

//
//
// *** Handles WeaponState changes END

void AWeapon::SHOW_PickupWidget(bool p_bShowWidget)
{
	if (m_WC_Pickup)
	{
		m_WC_Pickup->SetVisibility(p_bShowWidget);
	}
}

// *** Handles firing weapon START
//
// 

void AWeapon::FIRE(const FVector& p_HitTarget)
{
	if (m_FireAnimation)
	{
		m_SKC_WeaponMesh->PlayAnimation(m_FireAnimation, false)  ;
	}

	if (m_CasingClass)
	{
		const USkeletalMeshSocket* _AmmoEjectSocket = m_SKC_WeaponMesh->GetSocketByName(FName("AmmoEject"));

		if (_AmmoEjectSocket)
		{
			FTransform _SocketTransform = _AmmoEjectSocket->GetSocketTransform(m_SKC_WeaponMesh);

			UWorld* _World = GetWorld();

			if (_World)
			{
				_World->SpawnActor<ACasing>(m_CasingClass, _SocketTransform.GetLocation(), _SocketTransform.GetRotation().Rotator());
			}
		}
	}

	SPEND_Round();
}

void AWeapon::SPEND_Round()
{
	m_Ammo = FMath::Clamp(m_Ammo - 1, 0, m_MagCapacity);

	SET_HUDAmmo();
}

void AWeapon::SET_HUDAmmo()
{
	// Safety check
	if (m_OwnerPlayerCharacter == nullptr)
	{
		m_OwnerPlayerCharacter = Cast<APlayerCharacter>(GetOwner());

		if (m_OwnerPlayerCharacter)
		{
			if (m_OwnerPlayerController == nullptr)
			{
				m_OwnerPlayerController = Cast<ASalnaxarPlayerController>(m_OwnerPlayerCharacter->Controller);
			}
		}
	}

	if (m_OwnerPlayerCharacter && m_OwnerPlayerController)
	{
		m_OwnerPlayerController->SET_HUDWeaponAmmo(m_Ammo);
	}
}

//
//
// *** Handles firing weapon END

void AWeapon::DROP_Weapon()
{
	SET_WeaponState(EWeaponState::DroppedState);

	FDetachmentTransformRules _TransformRules(EDetachmentRule::KeepWorld, true) ;
	m_SKC_WeaponMesh->DetachFromComponent(_TransformRules)                      ;
	SetOwner(nullptr)                                                           ;
	m_OwnerPlayerCharacter  = nullptr                                           ;
	m_OwnerPlayerController = nullptr                                           ;
}

bool AWeapon::IS_MagEmpty()
{
	if (m_Ammo <= 0)
	{
		return true;
	}

	return false;
}

void AWeapon::ADD_Ammo(int32 p_AmmoToAdd)
{
	m_Ammo = FMath::Clamp(m_Ammo - p_AmmoToAdd, 0, m_MagCapacity);
	SET_HUDAmmo();
}