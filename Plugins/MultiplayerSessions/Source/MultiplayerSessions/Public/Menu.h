#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

class UButton;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void SETUP_Menu
	(
		int32 p_NumberOfPublicConnections = 4,
		FString p_TypeOfMatch = "FreeForAll", 
		FString p_LobbyPath = "/Game/ThirdPerson/Maps/Lobby"
	);

protected:

	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* p_InLevel, UWorld* p_InWorld) override;

	//
	// Callback functions for the custom delegates on the MultiplayerSessionsSubsystem
	//
	UFUNCTION()
	void ON_CreateSession(bool p_bWasSuccessful);

	// These two callback functions are not marked as UFUNCTION() as they are not dynamic (meaning they cannot be used in Blueprint)
	void ON_FindSessions(const TArray<FOnlineSessionSearchResult>& p_SessionResults, bool p_bWasSuccessful);
	void ON_JoinSession(EOnJoinSessionCompleteResult::Type p_Result);

	UFUNCTION()
	void ON_DestroySession(bool p_bWasSuccessful);

	UFUNCTION()
	void ON_StartSession(bool p_bWasSuccessful);

private: // Callback functions for the UI elements

	UFUNCTION()
	void ON_HostButton_Clicked();

	UFUNCTION()
	void ON_JoinButton_Clicked();

private:

	void TEARDOWN_Menu();

private: // UI variables

	UPROPERTY(meta = (BindWidget))
	UButton* m_Host_Btn;

	UPROPERTY(meta = (BindWidget))
	UButton* m_Join_Btn;

private:
	
	// The subsystem designed to handle all online session functionalities.
	class UMultiplayerSessionsSubsystem* m_MultiplayerSessionsSubsystem;

	int32 m_NumPublicConnections = 4;
	FString m_MatchType = "FreeForAll";
	FString m_PathToLobby = "";
};
