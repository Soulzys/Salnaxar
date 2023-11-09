#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SalnaxarHUD.generated.h"

class UTexture2D                 ;
class UUserWidger                ;
class UCharacterOverlay          ;
class UInteractiveMessageOverlay ;
class UAnnouncement              ;

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:

	UTexture2D* s_CrosshairsCenter = nullptr;
	UTexture2D* s_CrosshairsRight  = nullptr;
	UTexture2D* s_CrosshairsLeft   = nullptr;
	UTexture2D* s_CrosshairsTop    = nullptr;
	UTexture2D* s_CrosshairsBottom = nullptr;

	float        s_CrosshairsSpread ;
	FLinearColor s_CrosshairsColor  ;
};

UCLASS()
class SALNAXAR_API ASalnaxarHUD : public AHUD
{
	GENERATED_BODY()

public:

	//
	// *** Constructor
	//
	//
	ASalnaxarHUD();

	// 
	// *** Override
	//
	//
	virtual void DrawHUD() override;

	//
	// *** Normal functions
	//
	//
	void ADD_CharacterOverlay         () ;
	void ADD_InteractiveMessageOverlay() ;
	void ADD_WarmupAnnouncement       () ;

	//
	// *** Setters
	//
	//
	FORCEINLINE void SET_HUDPackage(const FHUDPackage& p_Package) { m_HUDPackage = p_Package; }

protected:

	virtual void BeginPlay() override;	

private:

	void DRAW_Crosshairs(UTexture2D* p_Texture, const FVector2D& p_ViewportCenter, const FVector2D& p_Spread, const FLinearColor& p_CrosshairsColor);

public:

	// CharacterOverlay Widget
	//
	UPROPERTY(EditAnywhere, Category = "Player Stats", meta = (DisplayName = "CharacterOverlay Class"))
	TSubclassOf<UUserWidget> m_CharacterOverlayClass = nullptr;

	UCharacterOverlay* m_CharacterOverlay = nullptr;

	// InteractiveMessageOverlay Widget
	//
	UPROPERTY(EditAnywhere, Category = "Interactive Messages", meta = (DisplayName = "InteractiveMessageOverlay Class"))
	TSubclassOf<UUserWidget> m_InteractiveMessageOverlayClass = nullptr;

	UInteractiveMessageOverlay* m_InteractiveMessageOverlay = nullptr;

	// Warmup Announcement Widget
	//
	UPROPERTY(EditAnywhere, Category = "Interactive Messages", meta = (DisplayName = "WarmupAnnouncementWidget Class"))
	TSubclassOf<UUserWidget> m_WarmupAnnouncementWidgetClass = nullptr;

	UAnnouncement* m_WarmupAnnouncementWidget = nullptr;

private:

	FHUDPackage m_HUDPackage;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Crosshair Spread Max"))
	float m_CrosshairSpreadMax;

};
