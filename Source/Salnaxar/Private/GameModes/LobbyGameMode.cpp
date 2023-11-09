#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* p_NewPlayer)
{
	Super::PostLogin(p_NewPlayer);

	int32 _NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if (_NumberOfPlayers == 2)
	{
		UWorld* _World = GetWorld();

		if (_World)
		{
			bUseSeamlessTravel = true                                    ;
			_World->ServerTravel(FString("/Game/Levels/Salnarm?listen")) ;				
		}
	}
}