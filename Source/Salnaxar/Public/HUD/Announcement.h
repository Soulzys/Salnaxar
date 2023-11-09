#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;

UCLASS()
class SALNAXAR_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget)) UTextBlock* m_WarmupTime       = nullptr ;
	UPROPERTY(meta = (BindWidget)) UTextBlock* m_AnnouncementText = nullptr ;
	UPROPERTY(meta = (BindWidget)) UTextBlock* m_InfoText         = nullptr ;
};
