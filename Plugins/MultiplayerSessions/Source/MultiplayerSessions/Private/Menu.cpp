#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"


void UMenu::SETUP_Menu(int32 p_NumberOfPublicConnections, FString p_TypeOfMatch, FString p_LobbyPath)
{
	m_PathToLobby = FString::Printf(TEXT("%s?listen"), *p_LobbyPath);
	m_NumPublicConnections = p_NumberOfPublicConnections;
	m_MatchType = p_TypeOfMatch;

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* _World = GetWorld();
	
	if (_World)
	{
		APlayerController* _PC = _World->GetFirstPlayerController();

		if (_PC)
		{
			FInputModeUIOnly _InputModeData;
			_InputModeData.SetWidgetToFocus(TakeWidget());
			_InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			
			_PC->SetInputMode(_InputModeData);
			_PC->SetShowMouseCursor(true);
		}
	}

	UGameInstance* _GameInstance = GetGameInstance();

	if (_GameInstance)
	{
		m_MultiplayerSessionsSubsystem = _GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	// Binding the MultiplayerSessionSubsystem delegate to our Menu callback functions
	if (m_MultiplayerSessionsSubsystem)
	{
		// We use AddUObject in order to bind non-dynamic delegate.
		m_MultiplayerSessionsSubsystem->m_DELEGATE_MultiplayerOnCreateSessionComplete .AddDynamic(this, &UMenu::ON_CreateSession );
		m_MultiplayerSessionsSubsystem->m_DELEGATE_MultiplayerOnFindSessionsComplete  .AddUObject(this, &UMenu::ON_FindSessions  );
		m_MultiplayerSessionsSubsystem->m_DELEGATE_MultiplayerOnJoinSessionComplete   .AddUObject(this, &UMenu::ON_JoinSession   );
		m_MultiplayerSessionsSubsystem->m_DELEGATE_MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMenu::ON_DestroySession);
		m_MultiplayerSessionsSubsystem->m_DELEGATE_MultiplayerOnStartSessionComplete  .AddDynamic(this, &UMenu::ON_StartSession  );
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	// Security check
	if (m_Host_Btn == nullptr || m_Join_Btn == nullptr)
	{
		return false;
	}

	m_Host_Btn->OnClicked.AddDynamic(this, &UMenu::ON_HostButton_Clicked);
	m_Join_Btn->OnClicked.AddDynamic(this, &UMenu::ON_JoinButton_Clicked);

	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* p_InLevel, UWorld* p_InWorld)
{
	TEARDOWN_Menu();
	Super::OnLevelRemovedFromWorld(p_InLevel, p_InWorld);
}

//
// Customs callback functions which are bound to the UMultiplayerSessionsSubsystem delegate.
//
void UMenu::ON_CreateSession(bool p_bWasSuccessful)
{
	if (p_bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Yellow,
				FString(TEXT("Session created successfully"))
			);
		}

		UWorld* _World = GetWorld();

		if (_World)
		{
			// The '?listen' is there so if a session was just created, it will be set as a listening server so we can wait for other players to join.
			_World->ServerTravel(m_PathToLobby);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Failed to create session"))
			);
		}

		m_Host_Btn->SetIsEnabled(true);
	}
}

void UMenu::ON_FindSessions(const TArray<FOnlineSessionSearchResult>& p_SessionResults, bool p_bWasSuccessful)
{
	// Safety check
	if (m_MultiplayerSessionsSubsystem == nullptr)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Error : ON_FindSessions -->> m_MultiplayerSessionsSubsystem is NULL !"))
			);
		}

		return;
	}

	for (auto bo_Result : p_SessionResults)
	{
		FString _SettingsValue;
		bo_Result.Session.SessionSettings.Get(FName("MatchType"), _SettingsValue);

		if (_SettingsValue == m_MatchType)
		{
			m_MultiplayerSessionsSubsystem->JOIN_Session(bo_Result);
			return; // There is no need to keep looping
		}
	}

	if (!p_bWasSuccessful || p_SessionResults.Num() == 0)
	{
		m_Join_Btn->SetIsEnabled(true);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage
		(
			-1,
			15.0f,
			FColor::Orange,
			FString(TEXT("Problem : ON_FindSessions -->> We haven't find any search result matching our MatchType !"))
		);
	}
}

void UMenu::ON_JoinSession(EOnJoinSessionCompleteResult::Type p_Result)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage
		(
			-1,
			15.0f,
			FColor::Magenta,
			FString(TEXT("Check : ON_JoinSessions -->> Entered function !"))
		);
	}

	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();

	if (_Subsystem)
	{
		IOnlineSessionPtr _SessionInterface = _Subsystem->GetSessionInterface();

		if (_SessionInterface.IsValid())
		{
			FString _Address;
			_SessionInterface->GetResolvedConnectString(NAME_GameSession, _Address);

			APlayerController* _PlayerController = GetGameInstance()->GetFirstLocalPlayerController();

			if (_PlayerController)
			{
				_PlayerController->ClientTravel(_Address, ETravelType::TRAVEL_Absolute);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage
				(
					-1,
					15.0f,
					FColor::Red,
					FString(TEXT("Error : ON_JoinSessions -->> _PlayerController is NULL !"))
				);
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Error : ON_JoinSessions -->> _SessionInterface is Unvalid !"))
			);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage
		(
			-1,
			15.0f,
			FColor::Red,
			FString(TEXT("Error : ON_JoinSessions -->> _Subsystem is NULL !"))
		);
	}

	if (p_Result != EOnJoinSessionCompleteResult::Success)
	{
		m_Join_Btn->SetIsEnabled(true);
	}

}

void UMenu::ON_DestroySession(bool p_bWasSuccessful)
{

}

void UMenu::ON_StartSession(bool p_bWasSuccessful)
{

}

void UMenu::ON_HostButton_Clicked()
{
	m_Host_Btn->SetIsEnabled(false);

	if (m_MultiplayerSessionsSubsystem)
	{
		m_MultiplayerSessionsSubsystem->CREATE_Session(m_NumPublicConnections, m_MatchType);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage
		(
			-1,
			15.0f,
			FColor::Red,
			FString(TEXT("m_MultiplayerSessionsSubsystem is NULL !"))
		);
	}
}

void UMenu::ON_JoinButton_Clicked()
{
	m_Join_Btn->SetIsEnabled(false);

	if (m_MultiplayerSessionsSubsystem)
	{
		m_MultiplayerSessionsSubsystem->FIND_Sessions(100000);
	}
}

void UMenu::TEARDOWN_Menu()
{
	RemoveFromParent();

	UWorld* _World = GetWorld();

	if (_World)
	{
		APlayerController* _PlayerController = _World->GetFirstPlayerController();

		if (_PlayerController)
		{
			FInputModeGameOnly _InputModeData;
			_PlayerController->SetInputMode(_InputModeData);
			_PlayerController->SetShowMouseCursor(false);
		}
	}
}
