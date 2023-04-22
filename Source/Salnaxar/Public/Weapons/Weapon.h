#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Salnaxar/Public/Weapons/WeaponTypes.h"
#include "Weapon.generated.h"

class USkeletalMeshComponent    ;
class USphereComponent          ;
class UWidgetComponent          ;
class UAnimationAsset           ;
class ACasing                   ;
class UTexture2D                ;
class APlayerCharacter          ;
class ASalnaxarPlayerController ;
class USoundCue                 ;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	InitialState  = 0 UMETA(DisplayName = "Initial State" ) ,
	EquippedState = 1 UMETA(DisplayName = "Equipped State") ,
	DroppedState  = 2 UMETA(DisplayName = "Dropped State" ) ,
	MAX           = 3 UMETA(DisplayName = "Default MAX"   )
};

UCLASS()
class SALNAXAR_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	// 
	// *** Constructor
	//
	//
	AWeapon();

	//
	// *** Override
	//
	//
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override ;
	virtual void OnRep_Owner               ()                                                  override ;

	// 
	// *** Virtual
	//
	//
	virtual void FIRE(const FVector& p_HitTarget);

	//
	// *** Normal functions
	//
	//
	void SHOW_PickupWidget(bool p_bShowWidget) ;
	void DROP_Weapon      ()                   ;
	void SET_HUDAmmo      ()                   ;
	bool IS_MagEmpty      ()                   ; // If the mag runs out of bullets
	void ADD_Ammo         (int32 p_AmmoToAdd)  ;

	//
	// *** Getters
	//
	//
	FORCEINLINE USphereComponent*       GET_AreaSphere       () const { return m_SC_AreaSphere      ; }
	FORCEINLINE USkeletalMeshComponent* GET_WeaponMesh       () const { return m_SKC_WeaponMesh     ; }
	FORCEINLINE UTexture2D*             GET_CrosshairsCenter () const { return m_CrosshairsCenter   ; }
	FORCEINLINE UTexture2D*             GET_CrosshairsRight  () const { return m_CrosshairsRight    ; }
	FORCEINLINE UTexture2D*             GET_CrosshairsLeft   () const { return m_CrosshairsLeft     ; }
	FORCEINLINE UTexture2D*             GET_CrosshairsTop    () const { return m_CrosshairsTop      ; }
	FORCEINLINE UTexture2D*             GET_CrosshairsBottom () const { return m_CrosshairsBottom   ; }
	FORCEINLINE USoundCue*              GET_EquipSound       () const { return m_EquipSound         ; }
	FORCEINLINE float                   GET_ZoomedFOV        () const { return m_ZoomedFOV          ; }
	FORCEINLINE float                   GET_ZoomInterpSpeed  () const { return m_ZoomInterpSpeed    ; }
	FORCEINLINE float                   GET_FireDelay        () const { return m_FireDelay          ; }
	FORCEINLINE bool                    GET_IsWeaponAutomatic() const { return m_bIsWeaponAutomatic ; }
	FORCEINLINE int32                   GET_Ammo             () const { return m_Ammo               ; }
	FORCEINLINE int32                   GET_MagCapacity      () const { return m_MagCapacity        ; }
	FORCEINLINE EWeaponType             GET_WeaponType       () const { return m_WeaponType         ; }

	//
	// *** Setters
	//
	//
	void SET_WeaponState(EWeaponState p_State);

protected:

	// 
	// *** Override
	//
	//
	virtual void BeginPlay()                override ;
	virtual void Tick     (float DeltaTime) override ;


	//
	// *** Callback
	//
	// 
	// These two events are only called on the server !
	UFUNCTION()
	virtual void OnSphereOverlap
	(
		UPrimitiveComponent* OverlappedComponent , 
		AActor*              OtherActor          , 
		UPrimitiveComponent* OtherComp           , 
		int32                OtherBodyIndex      , 
		bool                 bFromSweep          , 
		const FHitResult&    SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap
	(
		UPrimitiveComponent* OverlappedComponent , 
		AActor*              OtherActor          , 
		UPrimitiveComponent* OtherComp           , 
		int32                OtherBodyIndex
	);

private:

	//
	// *** RPCs
	//
	//
	UFUNCTION() void OnRep_m_WeaponState() ;
	UFUNCTION()	void OnRep_m_Ammo       () ;

	//
	// *** Normal function
	//
	//
	UFUNCTION()	void SPEND_Round();

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Weapon Mesh"))
	USkeletalMeshComponent* m_SKC_WeaponMesh = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Area Sphere"))
	USphereComponent* m_SC_AreaSphere = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Pickup"))
	UWidgetComponent* m_WC_Pickup = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_m_WeaponState, VisibleAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Weapon State"))
	EWeaponState m_WeaponState;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties", meta = (DisplayName = "Fire Animation"))
	UAnimationAsset* m_FireAnimation = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Casing Class"))
	TSubclassOf<ACasing> m_CasingClass = nullptr;

	//
	// Textures for the weapon crosshairs
	//
	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshairs Center"))
	UTexture2D* m_CrosshairsCenter = nullptr;

	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshairs Right"))
	UTexture2D* m_CrosshairsRight = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshairs Left"))
	UTexture2D* m_CrosshairsLeft = nullptr;	

	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshairs Top"))
	UTexture2D* m_CrosshairsTop = nullptr;

	UPROPERTY(EditAnywhere, Category = "Crosshairs", meta = (DisplayName = "Crosshairs Bottom"))
	UTexture2D* m_CrosshairsBottom = nullptr;

	//
	// Zoomed FOV (Field Of View) while aiming
	//

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Zoomed FOV"))
	float m_ZoomedFOV;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Zoom Interp Speed"))
	float m_ZoomInterpSpeed;

	//
	// Automatic Fire
	//

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "Fire Delay"))
	float m_FireDelay;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "bIsWeaponAutomatic"))
	bool m_bIsWeaponAutomatic = true;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_m_Ammo, meta = (DisplayName = "Ammo"))
	int32 m_Ammo;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Mag Capacity"))
	int32 m_MagCapacity;

	APlayerCharacter* m_OwnerPlayerCharacter = nullptr;
	ASalnaxarPlayerController* m_OwnerPlayerController = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Weapon Type"))
	EWeaponType m_WeaponType;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Equip sound"))
	USoundCue* m_EquipSound = nullptr;

};
