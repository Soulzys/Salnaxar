#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractiveMessageOverlay.generated.h"

class UTextBlock;

UCLASS()
class SALNAXAR_API UInteractiveMessageOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget)) UTextBlock* m_MessageText = nullptr;
};
