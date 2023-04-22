#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UBoxComponent                ;
class UProjectileMovementComponent ;
class UParticleSystem              ;
class UParticleSystemComponent     ;
class USoundCue                    ;

UCLASS()
class SALNAXAR_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	//
	// *** Constructor
	//
	//
	AProjectile();

	//
	// *** Override
	//
	//
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed()           override; // The action of destroying a replicated Actor propagates to all the clients

protected:

	// 
	// *** Override
	//
	//
	virtual void BeginPlay() override;

	// 
	// *** Callback
	//
	//
	UFUNCTION()
	virtual void ON_Hit
	(
		UPrimitiveComponent* p_HitComp       ,
		AActor*              p_OtherActor    , 
		UPrimitiveComponent* p_OtherComp     , 
		FVector              p_NormalImpulse , 
		const FHitResult&    p_Hit
	);

protected:

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Damage"))
	float m_Damage;

private:

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Collision Box"))
	UBoxComponent* m_CollisionBox = nullptr;

	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Projectile Movement Component"))
	UProjectileMovementComponent* m_ProjectileMovementComponent = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Tracer"))
	UParticleSystem* m_Tracer = nullptr;

	UParticleSystemComponent* m_TracerComponent = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Impact Particles"))
	UParticleSystem* m_ImpactParticles = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Impact Sound"))
	USoundCue* m_ImpactSound = nullptr;
};
