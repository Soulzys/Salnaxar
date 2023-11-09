#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :  // NEED-TO-TRY : put these within the constructor {}
	m_DELEGATE_CreateSessionComplete (FOnCreateSessionCompleteDelegate ::CreateUObject(this, &UMultiplayerSessionsSubsystem::ON_CreateSessionComplete  )), 
	m_DELEGATE_FindSessionsComplete  (FOnFindSessionsCompleteDelegate  ::CreateUObject(this, &UMultiplayerSessionsSubsystem::ON_FindSessionsComplete   )), 
	m_DELEGATE_JoinSessionComplete   (FOnJoinSessionCompleteDelegate   ::CreateUObject(this, &UMultiplayerSessionsSubsystem::ON_JoinSessionComplete    )), 
	m_DELEGATE_DestroySessionComplete(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::ON_DestroySessionComplete )), 
	m_DELEGATE_StartSessionComplete  (FOnStartSessionCompleteDelegate  ::CreateUObject(this, &UMultiplayerSessionsSubsystem::ON_StartSessionComplete   ))
{
	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();

	if (_Subsystem)
	{
		m_SessionInterface = _Subsystem->GetSessionInterface();
	}
}

//
// Handling session functionalities. Called outside the class by the Menu class.
//

void UMultiplayerSessionsSubsystem::CREATE_Session(int32 p_NumPublicConnections, FString p_MatchType)
{
	if (!m_SessionInterface.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Error : CREATE_Session -->> m_SessionInterface is NULL !"))
			);

			return;
		}
	}

	// Deals with a potential already existing session.
	auto _ExistingSession = m_SessionInterface->GetNamedSession(NAME_GameSession);

	if (_ExistingSession != nullptr)
	{
		m_bCreateSessionOnDestroy = true;
		m_LastNumPublicConnections = p_NumPublicConnections;
		m_LastMatchType = p_MatchType;

		DESTROY_Session();
	}

	// Stores the delegate in a FDelegateHandle so we can later remove it from the delegate list.
	m_DELHANDLE_CreateSessionComplete = m_SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(m_DELEGATE_CreateSessionComplete);

	m_LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	m_LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	m_LastSessionSettings->NumPublicConnections = p_NumPublicConnections;
	m_LastSessionSettings->bAllowJoinInProgress = true;
	m_LastSessionSettings->bAllowJoinViaPresence = true;
	m_LastSessionSettings->bShouldAdvertise = true;
	m_LastSessionSettings->bUsesPresence = true;
	m_LastSessionSettings->bUseLobbiesIfAvailable = true;
	m_LastSessionSettings->Set(FName("MatchType"), p_MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	m_LastSessionSettings->BuildUniqueId = 1;

	const ULocalPlayer* _LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	// If creating a session fails, we then remove the delegate created above so as to clean things up.
	if (!m_SessionInterface->CreateSession(*_LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *m_LastSessionSettings))
	{
		GEngine->AddOnScreenDebugMessage
		(
			-1,
			15.0f,
			FColor::Red,
			FString(TEXT("Error : CREATE_Session -->> Failed to create a session !"))
		);

		m_SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(m_DELHANDLE_CreateSessionComplete);

		// Broadcast our own custom delegate which is bind by our Menu class.
		m_DELEGATE_MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FIND_Sessions(int32 p_MaxSearchResults)
{
	// Security check
	if (!m_SessionInterface.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Error : FIND_Session -->> m_SessionInterface is NULL !"))
			);

			return;
		}
	}

	m_DELHANDLE_FindSessionsComplete = m_SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(m_DELEGATE_FindSessionsComplete);

	m_LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	m_LastSessionSearch->MaxSearchResults = p_MaxSearchResults;
	m_LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	m_LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* _LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	// If we haven't find any sessions
	if (!m_SessionInterface->FindSessions(*_LocalPlayer->GetPreferredUniqueNetId(), m_LastSessionSearch.ToSharedRef()))
	{
		GEngine->AddOnScreenDebugMessage
		(
			-1,
			15.0f,
			FColor::Red,
			FString(TEXT("Error : FIND_Session -->> We haven't find any sessions !"))
		);

		m_SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(m_DELHANDLE_FindSessionsComplete);

		// In order to pass an empty TArray, create the TArray followed by '()'. See below for an example.
		m_DELEGATE_MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JOIN_Session(const FOnlineSessionSearchResult& p_SessionResult)
{
	if (!m_SessionInterface.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Error : JOIN_Session -->> m_SessionInterface is NULL !"))
			);
		}

		m_DELEGATE_MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);

		return;
	}

	m_DELHANDLE_JoinSessionComplete = m_SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(m_DELEGATE_JoinSessionComplete);

	const ULocalPlayer* _LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!m_SessionInterface->JoinSession(*_LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, p_SessionResult))
	{
		GEngine->AddOnScreenDebugMessage
		(
			-1,
			15.0f,
			FColor::Red,
			FString(TEXT("Error : JOIN_Session -->> We haven't been able to join the session !"))
		);

		m_SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(m_DELHANDLE_JoinSessionComplete);

		m_DELEGATE_MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DESTROY_Session()
{
	if (!m_SessionInterface.IsValid())
	{
		m_DELEGATE_MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	m_DELHANDLE_DestroySessionComplete = m_SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(m_DELEGATE_DestroySessionComplete);

	if (!m_SessionInterface->DestroySession(NAME_GameSession))
	{
		m_SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(m_DELHANDLE_DestroySessionComplete);
		m_DELEGATE_MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::START_Session()
{
	if (!m_SessionInterface.IsValid())
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Red,
				FString(TEXT("Error : START_Session -->> m_SessionInterface is NULL !"))
			);

			return;
		}
	}
}

//
// Internal callback functions.
//

void UMultiplayerSessionsSubsystem::ON_CreateSessionComplete(FName p_SessionName, bool p_bWasSuccessful)
{
	if (m_SessionInterface)
	{
		m_SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(m_DELHANDLE_CreateSessionComplete);
	}

	m_DELEGATE_MultiplayerOnCreateSessionComplete.Broadcast(p_bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::ON_FindSessionsComplete(bool p_bWasSuccessful)
{
	if (m_SessionInterface)
	{
		m_SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(m_DELHANDLE_FindSessionsComplete);
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
				FString(TEXT("Error : ON_FindSessionsComplete -->> m_SessionInterface is NULL !"))
			);
		}
	}

	// If the search is successful but the TArray is empty
	if (m_LastSessionSearch->SearchResults.Num() <= 0)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage
			(
				-1,
				15.0f,
				FColor::Magenta,
				FString(TEXT("Check : ON_FindSessionsComplete -->> The search result TArray is empty !"))
			);
		}

		m_DELEGATE_MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	m_DELEGATE_MultiplayerOnFindSessionsComplete.Broadcast(m_LastSessionSearch->SearchResults, p_bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::ON_JoinSessionComplete(FName p_SessionName, EOnJoinSessionCompleteResult::Type p_Result)
{
	if (m_SessionInterface)
	{
		m_SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(m_DELHANDLE_JoinSessionComplete);
	}

	m_DELEGATE_MultiplayerOnJoinSessionComplete.Broadcast(p_Result);
}

void UMultiplayerSessionsSubsystem::ON_DestroySessionComplete(FName p_SessionName, bool p_bWasSuccessful)
{
	if (m_SessionInterface)
	{
		m_SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(m_DELHANDLE_DestroySessionComplete);
	}

	if (p_bWasSuccessful && m_bCreateSessionOnDestroy)
	{
		m_bCreateSessionOnDestroy = false;
		CREATE_Session(m_LastNumPublicConnections, m_LastMatchType);
	}

	m_DELEGATE_MultiplayerOnDestroySessionComplete.Broadcast(p_bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::ON_StartSessionComplete(FName p_SessionName, bool p_bWasSuccessful)
{

}