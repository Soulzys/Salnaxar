#include "Weapons/Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Salnaxar/Public/Characters/PlayerCharacter.h"
#include "Salnaxar/Salnaxar.h"

AProjectile::AProjectile()
{
	//
	// *** Variables initialization
	//
	//
	// ** Mother class(es) variables
	//
	PrimaryActorTick.bCanEverTick = true ;
	bReplicates                   = true ;

	// ** This class variables
	//
	m_Damage = 20.0f;

	//
	// *** Components initialization
	m_CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"))                                     ;
	SetRootComponent(m_CollisionBox)                                                                                 ;
	m_CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic)                                      ;
	m_CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)                                          ;
	m_CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore)                                ;
	m_CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block)  ;
	m_CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block) ;
	m_CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block)                   ; // So to block projectiles 

	m_ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectMovementComponent")) ;
	m_ProjectileMovementComponent->bRotationFollowsVelocity = true                                                         ;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (m_Tracer)
	{
		m_TracerComponent = UGameplayStatics::SpawnEmitterAttached
		(
			m_Tracer                           , 
			m_CollisionBox                     , 
			FName()                            , 
			GetActorLocation()                 , 
			GetActorRotation()                 , 
			EAttachLocation::KeepWorldPosition
		);
	}
	
	// Only runs on server
	if (HasAuthority())
	{
		m_CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::ON_Hit);
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (m_ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), m_ImpactParticles, GetActorTransform());
	}

	if (m_ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, m_ImpactSound, GetActorLocation());
	}
}

void AProjectile::ON_Hit
(
	UPrimitiveComponent* p_HitComp       ,
	AActor*              p_OtherActor    ,
	UPrimitiveComponent* p_OtherComp     ,
	FVector              p_NormalImpulse ,
	const FHitResult&    p_Hit
)
{
	// Since we replicate the APlayerCharacter m_Health variable when taking damage, we don't need to use RPC [...]
	// as variable replication is more efficient than RPCs (video 100 around 15 mins)

	Destroy();
}