#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Salnaxar/Public/HUD/SalnaxarHUD.h"
#include "Salnaxar/Public/Weapons/WeaponTypes.h"
#include "Salnaxar/Public/SalnaxarTypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 100000.0f

class APlayerCharacter          ;
class AWeapon                   ;
class ASalnaxarPlayerController ;
class ASalnaxarHUD              ;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SALNAXAR_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	//
	// ***  Constructor
	//
	//
	UCombatComponent();

	//
	// *** Friend class 
	// This is done in order to allow APlayerCharacter to access this class private member as public, meaning there is no need for Getter functions
	//
	friend APlayerCharacter;


	//
	// *** Override
	//
	//
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override ;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override                      ;

	// 
	// *** Normal functions
	//
	//
	UFUNCTION(BlueprintCallable) void RELOAD_Finished() ;
	void EQUIP_Weapon(AWeapon* p_WeaponToEquip)         ;
	void RELOAD_Weapon()                                ;

protected:

	// 
	// *** Override
	//
	//
	virtual void BeginPlay() override;

	// 
	// *** RPCs
	//
	//
	UFUNCTION()                       void OnRep_m_EquippedWeapon   ()                                            ;
	UFUNCTION()                       void OnRep_m_CarriedWeaponAmmo()                                            ;
	UFUNCTION()                       void OnRep_m_CombatState      ()                                            ;
	UFUNCTION(Server      , Reliable) void SERVER_SET_Aiming        (bool p_bIsAiming)                            ;
	UFUNCTION(Server      , Reliable) void SERVER_Fire              (const FVector_NetQuantize& p_TraceHitTarget) ; // Called from a client or a server. Runs only on the server
	UFUNCTION(Server      , Reliable) void SERVER_ReloadWeapon      ()                                            ;
	UFUNCTION(NetMulticast, Reliable) void MULTICAST_Fire           (const FVector_NetQuantize& p_TraceHitTarget) ; // Runs on all clients

	//
	// Normal functions
	//
	//
	void PRESSED_FireButton   (bool p_Pressed)               ; // Called locally whether on client or server
	void SET_Aiming           (bool p_bIsAiming)             ;
	void TRACE_UnderCrossHairs(FHitResult& p_TraceHitResult) ;
	void SET_HUDCrosshairs    (float p_DeltaTime)            ;
	void INTERP_FOV           (float p_DeltaTime)            ;
	void TIMER_StartFireTimer ()                             ;
	void TIMER_FireFinished   ()                             ;
	void FIRE_Weapon          ()                             ;
	void HANDLE_Reload        ()                             ; // Runs on both Server and Clients

private:

	//
	// Normal functions
	//
	//
	bool  CAN_Fire                () ;
	void  INIT_CarriedAmmo        () ;
	int32 CALCULATE_AmountToReload() ;
	void  UPDATE_AmmoValues       () ;

private:

	APlayerCharacter*          m_Character        = nullptr;
	ASalnaxarPlayerController* m_PlayerController = nullptr;
	ASalnaxarHUD*              m_HUD              = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_m_EquippedWeapon)
	AWeapon* m_EquippedWeapon = nullptr;

	UPROPERTY(Replicated)
	bool m_bAiming;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Base Walk Speed"))
	float m_BaseWalkSpeed;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Aim Walk Speed"))
	float m_AimWalkSpeed;

	bool m_bFireButtonPressed;

	//
	// HUD and crosshairs
	//
	float m_CrosshairsVelocityFactor ;
	float m_CrosshairsInAirFactor    ;
	float m_CrosshairsAimFactor      ;
	float m_CrosshairsShootingFactor ;

	FVector     m_HitTarget  ;
	FHUDPackage m_HUDPackage ;

	//
	// Aiming and FOV (Field Of View)
	//
	//
	float m_DefaultFOV ; // FOV when not aiming. Set to the camera's base FOV in BeginPlay
	float m_CurrentFOV ;

	UPROPERTY(EditAnywhere, category = "Combat", meta = (DisplayName = "Zoomed FOV"))
	float m_ZoomedFOV;

	// This is used when we're "unzooming" and is currently common for every weapon. We could also add a specific unzoom interp speed for every weapon
	UPROPERTY(EditAnywhere, category = "Combat", meta = (DisplayName = "Zoom Interp Speed"))
	float m_ZoomInterpSpeed; 

	//
	// Automatic Fire
	//
	//
	FTimerHandle m_FireTimer ;
	bool         m_bCanFire  ;

	//
	//
	
	// Carried ammo for the currently-equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_m_CarriedWeaponAmmo)
	int32 m_CarriedWeaponAmmo;

	UPROPERTY(EditAnywhere, Category = "Ammo", meta = (DisplayName = "Starting Assault Rifle Ammo"))
	int32 m_Starting_AR_Ammo;

	TMap<EWeaponType, int32> m_CarriedAmmoMap;

	UPROPERTY(ReplicatedUsing = OnRep_m_CombatState)
	ECombatState m_CombatState;
};
