#include "Salnaxar/Public/Weapons/ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AProjectileBullet::ON_Hit
(
	UPrimitiveComponent* p_HitComp       ,
	AActor*              p_OtherActor    ,
	UPrimitiveComponent* p_OtherComp     ,
	FVector              p_NormalImpulse ,
	const FHitResult&    p_Hit
)
{
	ACharacter* _OwnerCharacter = Cast<ACharacter>(GetOwner());

	if (_OwnerCharacter)
	{
		AController* _OwnerController = _OwnerCharacter->Controller;

		if (_OwnerController)
		{
			UGameplayStatics::ApplyDamage(p_OtherActor, m_Damage, _OwnerController, this, UDamageType::StaticClass());
		}
	}	

	// We call Super last as it is charged with destroying the Projectile, hence no code will run after it
	Super::ON_Hit(p_HitComp, p_OtherActor, p_OtherComp, p_NormalImpulse, p_Hit);
}