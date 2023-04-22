#include "Weapons/ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Salnaxar/Public/Weapons/Projectile.h"

void AProjectileWeapon::FIRE(const FVector& p_HitTarget)
{
	Super::FIRE(p_HitTarget);

	// Make sure we only spawn projectiles on the server
	// It works because the weapon itself is replicated, meaning it will have authority only on the server instead of on all the client machines
	if (HasAuthority() == false)
	{
		return;
	}

	APawn* _InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* _MuzzleFlashSocket = GET_WeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (_MuzzleFlashSocket)
	{
		FTransform _SocketTransform = _MuzzleFlashSocket->GetSocketTransform(GET_WeaponMesh());
		FVector _ToTarget = p_HitTarget - _SocketTransform.GetLocation(); // From muzzle flash socket to hit location from TRACE_UnderCrosshairs
		FRotator _TargetRotation = _ToTarget.Rotation();

		if (m_ProjectileClass && _InstigatorPawn)
		{
			FActorSpawnParameters _SpawnParams;
			_SpawnParams.Owner = GetOwner(); // Get the owner of the Weapon
			_SpawnParams.Instigator = _InstigatorPawn;

			UWorld* _World = GetWorld();

			if (_World)
			{
				_World->SpawnActor<AProjectile>(m_ProjectileClass, _SocketTransform.GetLocation(), _TargetRotation, _SpawnParams);
			}
		}
	}
}