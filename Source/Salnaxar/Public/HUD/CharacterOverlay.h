#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UProgressBar ;
class UTextBlock   ;

UCLASS()
class SALNAXAR_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget)) UProgressBar* m_HealthBar               = nullptr ;
	UPROPERTY(meta = (BindWidget)) UTextBlock*   m_HealthText              = nullptr ;
	UPROPERTY(meta = (BindWidget)) UTextBlock*   m_Score                   = nullptr ;
	UPROPERTY(meta = (BindWidget)) UTextBlock*   m_DeathCount              = nullptr ;
	UPROPERTY(meta = (BindWidget)) UTextBlock*   m_WeaponAmmoAmount        = nullptr ;
	UPROPERTY(meta = (BindWidget)) UTextBlock*   m_CarriedWeaponAmmoAmount = nullptr ;
	UPROPERTY(meta = (BindWidget)) UTextBlock*   m_MatchCountdown          = nullptr ;
};
