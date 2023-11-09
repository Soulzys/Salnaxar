#include "Weapons/Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ACasing::ACasing()
{
	// 
	// *** Variables initialization
	//
	// ** Mother class(es) variables
	//
	PrimaryActorTick.bCanEverTick = false;

	// ** This class variables
	//
	m_ShellEjectionImpulse = 8.0f;
	m_bHasHitFloor = false;

	//
	// *** Components initialization
	//
	//
	m_CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"))                            ;
	SetRootComponent(m_CasingMesh)                                                                             ;
	m_CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore) ;
	m_CasingMesh->SetSimulatePhysics(true)                                                                     ;
	m_CasingMesh->SetEnableGravity(true)                                                                       ;
	m_CasingMesh->SetNotifyRigidBodyCollision(true)                                                            ;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();

	m_CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::ON_Hit)            ;
	m_CasingMesh->AddImpulse(GetActorForwardVector() * m_ShellEjectionImpulse) ;	
}

void ACasing::ON_Hit
(
	UPrimitiveComponent* p_HitComp       ,
	AActor*              p_OtherActor    ,
	UPrimitiveComponent* p_OtherComp     ,
	FVector              p_NormalImpulse ,
	const FHitResult&    p_Hit
)
{
	if (m_bHasHitFloor == false)
	{
		if (m_ShellSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, m_ShellSound, GetActorLocation());
		}

		GetWorldTimerManager().SetTimer(m_TimerHandle, this, &ACasing::TIMER_DestroyShell, 2.0f, false);

		m_bHasHitFloor = true;
	}	
}

void ACasing::TIMER_DestroyShell()
{
	GetWorldTimerManager().ClearTimer(m_TimerHandle);

	Destroy();
}
