#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Unoccupied = 0, 
	Reloading  = 1, 
	MAX        = 2 UMETA(DisplayName = "DefaultMAX")
};
