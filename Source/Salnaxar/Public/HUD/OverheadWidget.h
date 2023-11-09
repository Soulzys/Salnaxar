#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class UTextBlock;

UCLASS()
class SALNAXAR_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable) void SHOW_PlayerNetRole(APawn* p_InPawn) ;
	void SET_DisplayText(const FString& p_TextToDisplay)                  ;

protected:

	virtual void OnLevelRemovedFromWorld(ULevel* p_InLevel, UWorld* p_InWorld) override;

public:

	UPROPERTY(meta = (BindWidget)) UTextBlock* m_DisplayText = nullptr; 
	
};
