#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* p_InLevel, UWorld* p_InWorld)
{
	RemoveFromParent();

	Super::OnLevelRemovedFromWorld(p_InLevel, p_InWorld);
}

void UOverheadWidget::SHOW_PlayerNetRole(APawn* p_InPawn)
{
	ENetRole _LocalRole = p_InPawn->GetLocalRole() ;
	FString  _Role                                 ;

	switch (_LocalRole)
	{
		case ENetRole::ROLE_Authority    :
			_Role = FString("Authority") ;
			break                        ;

		case ENetRole::ROLE_AutonomousProxy     :
			_Role = FString("Autonomous Proxy") ;
			break                               ;

		case ENetRole::ROLE_SimulatedProxy     :
			_Role = FString("Simulated Proxy") ;
			break                              ;

		case ENetRole::ROLE_None    :
			_Role = FString("None") ;
			break                   ;
	}

	FString _LocalRoleString = FString::Printf(TEXT("Local Role : %s"), *_Role);

	SET_DisplayText(_LocalRoleString);
}

void UOverheadWidget::SET_DisplayText(const FString& p_TextToDisplay)
{
	if (m_DisplayText)
	{
		m_DisplayText->SetText(FText::FromString(p_TextToDisplay));
	}
}