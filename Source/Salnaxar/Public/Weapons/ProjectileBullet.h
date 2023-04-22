#pragma once

#include "CoreMinimal.h"
#include "Salnaxar/Public/Weapons/Projectile.h"
#include "ProjectileBullet.generated.h"

UCLASS()
class SALNAXAR_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

protected:

	// Since it is already marked as a UFUNCTION() in the mother class, we do not need to write it in the child class as well
	virtual void ON_Hit
	(
		UPrimitiveComponent* p_HitComp       ,
		AActor*              p_OtherActor    ,
		UPrimitiveComponent* p_OtherComp     ,
		FVector              p_NormalImpulse ,
		const FHitResult&    p_Hit
	) override;
};
