#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "ProjectileWeapon.generated.h"

class AProjectile;

UCLASS()
class SALNAXAR_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void FIRE(const FVector& p_HitTarget) override;

private:

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Projectile Class"))
	TSubclassOf<AProjectile> m_ProjectileClass = nullptr;
	
};
