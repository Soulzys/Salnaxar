#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	Left       = 0 UMETA(DisplayName = "Turning Left" ), 
	Right      = 1 UMETA(DisplayName = "Turning Right"), 
	NotTurning = 2 UMETA(DisplayName = "Not Turning"  ), 
	MAX        = 3 UMETA(DisplayName = "DefaultMAX"   )
};
