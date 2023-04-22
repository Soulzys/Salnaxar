#include "Interactif/Door.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Salnaxar/Public/Characters/PlayerCharacter.h"
#include "Salnaxar/Public/PlayerController/SalnaxarPlayerController.h"
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

ADoor::ADoor()
{
	//
	// *** Variables initialization
	//
	// ** Mother class(es) variables
	//
	PrimaryActorTick.bCanEverTick = false ;	
	bReplicates                   = true  ;

	// ** This class variables
	// 
	m_ServerDoorState = EServerDoorState::ServerDoorClosed ;
	m_ClientDoorState = EClientDoorState::ClientDoorClosed ;
	//m_NbPlayersAround = 0;

	//
	// *** Components initialization
	//
	//
	m_DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame")) ;
	SetRootComponent(m_DoorFrame)                                                 ;
	m_DoorFrame->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN)                     ;
	m_DoorFrame->MarkRenderStateDirty()                                           ;

	m_DoorFrameWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameWall")) ;
	m_DoorFrameWall->SetupAttachment(RootComponent)                                       ;

	m_DoorWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorWall")) ;
	m_DoorWall->SetupAttachment(RootComponent)                                  ;

	m_DoorPanel = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DoorPanel")) ;
	m_DoorPanel->SetupAttachment(RootComponent)                                     ;	
	//m_DoorPanel->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	//m_DoorPanel->MarkRenderStateDirty();

	// See documentation file "Door" for explanations on the line below 
	m_DoorPanel->SetRelativeLocation(RootComponent->GetRelativeLocation() + FVector(-23.75f, 0.0f, 0.0f)) ; 
	m_DoorPanel->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics)                                  ;
	m_DoorPanel->SetCollisionProfileName(FName("PhysicsActor"))                                           ;
	m_DoorPanel->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block)                         ;

	m_DoorTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DoorTrigger")) ;
	m_DoorTrigger->SetupAttachment(RootComponent)                              ;

	//

	ACTIVATE_CustomDepth(true);
}

void ADoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADoor, m_ServerDoorState);
}

void ADoor::BeginPlay()
{
	Super::BeginPlay();	

	m_DoorTrigger->OnComponentBeginOverlap.AddDynamic(this, &ADoor::ON_BoxOverlap_BEGIN) ;
	m_DoorTrigger->OnComponentEndOverlap  .AddDynamic(this, &ADoor::ON_BoxOverlap_END  ) ;	

	INIT_DoorState(m_ServerDoorState);
}

void ADoor::ON_BoxOverlap_BEGIN
(
	UPrimitiveComponent* p_OverlappedComp , 
	AActor*              p_OtherActor     , 
	UPrimitiveComponent* p_OtherComp      , 
	int32                p_OtherBodyIndex , 
	bool                 p_bFromSweep     , 
	const FHitResult&    p_SweepResult
)
{
	if (p_OtherActor && p_OtherActor->IsA<APlayerCharacter>())
	{
		APlayerCharacter* _Character = Cast<APlayerCharacter>(p_OtherActor);

		if (_Character && _Character->GET_SalnaxarPlayerController())
		{
			_Character->GET_SalnaxarPlayerController()->SET_HUDInteractiveMessageOverlay(FString::Printf(TEXT("Press E to open the door"))) ;
			_Character->m_OnInteractDelegate.AddDynamic(this, &ADoor::ON_InteractionKey_PRESSED)                                            ;
		}
	}	
}

void ADoor::ON_BoxOverlap_END
(
	UPrimitiveComponent* p_OverlappedComp , 
	AActor*              p_OtherActor     , 
	UPrimitiveComponent* p_OtherComp      , 
	int32                p_OtherBodyIndex 
)
{
	if (p_OtherActor && p_OtherActor->IsA<APlayerCharacter>())
	{
		APlayerCharacter* _Character = Cast<APlayerCharacter>(p_OtherActor);

		if (_Character && _Character->GET_SalnaxarPlayerController())
		{
			_Character->GET_SalnaxarPlayerController()->RESET_HUDInteractiveMessageOverlay() ;
			_Character->m_OnInteractDelegate.RemoveAll(this)                                 ;
		}
	}
}

void ADoor::ON_InteractionKey_PRESSED(APlayerCharacter* p_PlayerCharacter)
{
	if (m_DoorPanel == nullptr || m_DoorOpeningAnimation == nullptr || m_DoorClosingAnimation == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, TEXT("This should never happened"));
		return;
	}

	if (HasAuthority())
	{
		ACTIVATION_Server(m_ServerDoorState);
	}
	else
	{
		p_PlayerCharacter->SERVER_ACTIVATION_Client(this);
	}
}

// Runs on Clients
void ADoor::OnRep_m_ServerDoorState()
{
	if (m_ServerDoorState == EServerDoorState::ServerDoorOpened)
	{
		m_DoorPanel->PlayAnimation(m_DoorOpeningAnimation, false) ;
		m_ClientDoorState = EClientDoorState::ClientDoorOpened    ;
	}
	else if (m_ServerDoorState == EServerDoorState::ServerDoorClosed)
	{
		m_DoorPanel->PlayAnimation(m_DoorClosingAnimation, false) ;
		m_ClientDoorState = EClientDoorState::ClientDoorOpened    ;
	}
}

// Runs on server
void ADoor::ACTIVATION_Server(EServerDoorState p_ServerDoorState)
{
	if (p_ServerDoorState == EServerDoorState::ServerDoorOpened)
	{
		m_DoorPanel->PlayAnimation(m_DoorClosingAnimation, false) ;
		m_ServerDoorState = EServerDoorState::ServerDoorClosed    ;
	}
	else if (p_ServerDoorState == EServerDoorState::ServerDoorClosed)
	{
		m_DoorPanel->PlayAnimation(m_DoorOpeningAnimation, false) ;
		m_ServerDoorState = EServerDoorState::ServerDoorOpened    ;
	}
}

void ADoor::INIT_DoorState(EServerDoorState p_DoorState)
{
	if (m_ServerDoorState == EServerDoorState::ServerDoorOpened)
	{
		m_DoorPanel->PlayAnimation(m_DoorOpeningAnimation, false) ;
		m_ClientDoorState = EClientDoorState::ClientDoorOpened    ;
	}
	else if (m_ServerDoorState == EServerDoorState::ServerDoorClosed)
	{
		m_DoorPanel->PlayAnimation(m_DoorClosingAnimation, false) ;
		m_ClientDoorState = EClientDoorState::ClientDoorOpened    ;
	}
}

void ADoor::ACTIVATE_CustomDepth(bool p_Activate)
{
	Super::ACTIVATE_CustomDepth(p_Activate);

	if (m_DoorFrame)
	{
		m_DoorFrame->SetRenderCustomDepth(p_Activate);
	}
}