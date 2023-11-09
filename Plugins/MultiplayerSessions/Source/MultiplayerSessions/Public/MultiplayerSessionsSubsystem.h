#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

//
// Declaring our own customs delegates for the Menu class to bind callback functions to.
//
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete , bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams       (FMultiplayerOnFindSessionsComplete  , const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam        (FMultiplayerOnJoinSessionComplete   , EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete  , bool, bWasSuccessful);

UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UMultiplayerSessionsSubsystem();

	//
	// To handle session functionality. The Menu class will call these.
	//
	void CREATE_Session (int32 p_NumPublicConnections, FString p_MatchType); // POSS_IMPROVEMENT : const FString&
	void FIND_Sessions  (int32 p_MaxSearchResults);
	void JOIN_Session   (const FOnlineSessionSearchResult& p_SessionResult);
	void DESTROY_Session();
	void START_Session  ();

public:

	//
	// Our own custom delegate for the Menu class to bind callback functions to.
	//
	FMultiplayerOnCreateSessionComplete  m_DELEGATE_MultiplayerOnCreateSessionComplete  ;
	FMultiplayerOnFindSessionsComplete   m_DELEGATE_MultiplayerOnFindSessionsComplete   ;
	FMultiplayerOnJoinSessionComplete    m_DELEGATE_MultiplayerOnJoinSessionComplete    ;
	FMultiplayerOnDestroySessionComplete m_DELEGATE_MultiplayerOnDestroySessionComplete ;
	FMultiplayerOnStartSessionComplete   m_DELEGATE_MultiplayerOnStartSessionComplete   ;


protected:

	//
	// Internal callback functions for the delegates we'll add to the Online Session Interface delegate list.
	// These don't need to be called outside this class.
	//
	void ON_CreateSessionComplete (FName p_SessionName, bool p_bWasSuccessful);
	void ON_FindSessionsComplete  (bool p_bWasSuccessful);
	void ON_JoinSessionComplete   (FName p_SessionName, EOnJoinSessionCompleteResult::Type p_Result); 
	void ON_DestroySessionComplete(FName p_SessionName, bool p_bWasSuccessful);
	void ON_StartSessionComplete  (FName p_SessionName, bool p_bWasSuccessful);

private:

	IOnlineSessionPtr                  m_SessionInterface    ; // Smart pointer to OnlineSessionInterface
	TSharedPtr<FOnlineSessionSettings> m_LastSessionSettings ;
	TSharedPtr<FOnlineSessionSearch  > m_LastSessionSearch   ;

	//
	// To add to the Online Session Interface delegate list.
	// We'll bind our MultiplayerSessionSubsystem internal callback functions to these.
	//
	FOnCreateSessionCompleteDelegate  m_DELEGATE_CreateSessionComplete  ;
	FOnFindSessionsCompleteDelegate   m_DELEGATE_FindSessionsComplete   ;
	FOnJoinSessionCompleteDelegate    m_DELEGATE_JoinSessionComplete    ;
	FOnDestroySessionCompleteDelegate m_DELEGATE_DestroySessionComplete ;
	FOnStartSessionCompleteDelegate   m_DELEGATE_StartSessionComplete   ;

	FDelegateHandle m_DELHANDLE_CreateSessionComplete  ;
	FDelegateHandle m_DELHANDLE_FindSessionsComplete   ;
	FDelegateHandle m_DELHANDLE_JoinSessionComplete    ;
	FDelegateHandle m_DELHANDLE_DestroySessionComplete ;
	FDelegateHandle m_DELHANDLE_StartSessionComplete   ;

	bool m_bCreateSessionOnDestroy = false;
	int32 m_LastNumPublicConnections;
	FString m_LastMatchType;
};
