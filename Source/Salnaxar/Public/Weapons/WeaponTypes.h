#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	AssaultRifle = 0 UMETA(DisplayName = "Assault Rifle"), 
	MAX          = 1 UMETA(DisplayName "DefaultMAX") // So to retrieve the total item in the Enum
};