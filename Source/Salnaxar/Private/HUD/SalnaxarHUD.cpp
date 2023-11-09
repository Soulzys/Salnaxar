#include "HUD/SalnaxarHUD.h"
#include "GameFramework/PlayerController.h"
#include "Salnaxar/Public/HUD/CharacterOverlay.h"
#include "Salnaxar/Public/HUD/InteractiveMessageOverlay.h"
#include "Salnaxar/Public/HUD/Announcement.h"

ASalnaxarHUD::ASalnaxarHUD()
{
	// 
	// *** Variable initialization
	//
	// ** This class variables
	//
	m_CrosshairSpreadMax = 16.0f;

}

void ASalnaxarHUD::BeginPlay()
{
	Super::BeginPlay();

	//ADD_CharacterOverlay();
	//ADD_InteractiveMessageOverlay();
}

// DrawHUD section START
//
//

void ASalnaxarHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D _ViewportSize;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(_ViewportSize);

		const FVector2D _ViewportCenter(_ViewportSize.X / 2.0f, _ViewportSize.Y / 2.0f);

		float _SpreadScale = m_CrosshairSpreadMax * m_HUDPackage.s_CrosshairsSpread;

		// NOTE ! Ways for potential improvement : probably put all this garbage into a function managed by an ENUM, it's quite messy atm

		FVector2D _Spread = FVector2D(0.0f, 0.0f)                                                                   ; // Center
		DRAW_Crosshairs(m_HUDPackage.s_CrosshairsCenter , _ViewportCenter, _Spread, m_HUDPackage.s_CrosshairsColor) ;

		_Spread.X = _SpreadScale                                                                                    ;
		_Spread.Y = 0.0f                                                                                            ;
		DRAW_Crosshairs(m_HUDPackage.s_CrosshairsRight  , _ViewportCenter, _Spread, m_HUDPackage.s_CrosshairsColor) ;

		_Spread.X = -_SpreadScale                                                                                   ;
		_Spread.Y =  0.0f                                                                                           ;
		DRAW_Crosshairs(m_HUDPackage.s_CrosshairsLeft   , _ViewportCenter, _Spread, m_HUDPackage.s_CrosshairsColor) ;

		_Spread.X =  0.0f                                                                                           ; 
		_Spread.Y = -_SpreadScale                                                                                   ; // Upwards is negative
		DRAW_Crosshairs(m_HUDPackage.s_CrosshairsTop    , _ViewportCenter, _Spread, m_HUDPackage.s_CrosshairsColor) ;

		_Spread.X = 0.0f                                                                                            ;
		_Spread.Y = _SpreadScale                                                                                    ;
		DRAW_Crosshairs(m_HUDPackage.s_CrosshairsBottom , _ViewportCenter, _Spread, m_HUDPackage.s_CrosshairsColor) ;
	}
}

void ASalnaxarHUD::DRAW_Crosshairs(UTexture2D* p_Texture, const FVector2D& p_ViewportCenter, const FVector2D& p_Spread, const FLinearColor& p_CrosshairsColor)
{
	if (p_Texture == nullptr)
	{
		return;
	}

	const float     _TextureWidth  = p_Texture->GetSizeX() ;
	const float     _TextureHeight = p_Texture->GetSizeY() ;
	const FVector2D _TextureDrawPoint
	(
		p_ViewportCenter.X - (_TextureWidth  / 2.0f) + p_Spread.X,
		p_ViewportCenter.Y - (_TextureHeight / 2.0f) + p_Spread.Y
	);

	DrawTexture(p_Texture, _TextureDrawPoint.X, _TextureDrawPoint.Y, _TextureWidth, _TextureHeight, 0.0f, 0.0f, 1.0f, 1.0f, p_CrosshairsColor);
}

//
//
// DrawHUD section END

//
//

// Adding widgets to viewport section START
//
//

void ASalnaxarHUD::ADD_CharacterOverlay()
{
	APlayerController* _PlayerController = GetOwningPlayerController();

	if (_PlayerController && m_CharacterOverlayClass)
	{
		m_CharacterOverlay = CreateWidget<UCharacterOverlay>(_PlayerController, m_CharacterOverlayClass) ;
		m_CharacterOverlay->AddToViewport()                                                              ;
	}
}

void ASalnaxarHUD::ADD_InteractiveMessageOverlay()
{
	APlayerController* _PlayerController = GetOwningPlayerController();

	if (_PlayerController && m_InteractiveMessageOverlayClass)
	{
		m_InteractiveMessageOverlay = CreateWidget<UInteractiveMessageOverlay>(_PlayerController, m_InteractiveMessageOverlayClass) ;
		m_InteractiveMessageOverlay->AddToViewport()                                                                                ;
		m_InteractiveMessageOverlay->SetVisibility(ESlateVisibility::Hidden)                                                        ;
	}
}

void ASalnaxarHUD::ADD_WarmupAnnouncement()
{
	APlayerController* _PlayerController = GetOwningPlayerController();

	if (_PlayerController && m_WarmupAnnouncementWidgetClass)
	{
		m_WarmupAnnouncementWidget = CreateWidget<UAnnouncement>(_PlayerController, m_WarmupAnnouncementWidgetClass) ;
		m_WarmupAnnouncementWidget->AddToViewport()                                                                  ;
		m_WarmupAnnouncementWidget->SetVisibility(ESlateVisibility::Hidden)                                          ;
	}
}

//
//
// Adding widgets to viewport section END