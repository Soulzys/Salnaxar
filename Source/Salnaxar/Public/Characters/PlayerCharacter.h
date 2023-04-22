#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Salnaxar/Public/SalnaxarTypes/TurningInPlace.h"
#include "Salnaxar/Public/SalnaxarTypes/CombatState.h"
#include "Salnaxar/Public/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "PlayerCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractSignature, APlayerCharacter*, Owner);

class USpringArmComponent       ;
class UCameraComponent          ;
class UWidgetComponent          ;
class UCombatComponent          ;
class AWeapon                   ;
class UAnimMontage              ;
class ASalnaxarPlayerController ;
class ASalnaxarPlayerState      ;
class AController               ;
class ADoor                     ;

UCLASS()
class SALNAXAR_API APlayerCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:

	//
	// *** Constructors
	//
	//
	APlayerCharacter();

	//
	// *** Override
	//
	//
	virtual void Tick                      (float DeltaTime)                                   override ;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override ;
	virtual void OnRep_ReplicatedMovement  ()                                                  override ;

	//
	// *** RPCs
	//
	//
	UFUNCTION(Server      , Reliable) void SERVER_ACTIVATION_Client(ADoor* p_Door) ;
	UFUNCTION(NetMulticast, Reliable) void MULTICAST_ELIMINATE     ()              ; // For when player gets eliminated. Called on server and all clients

	//
	// *** Normal functions
	//
	//
	bool IS_WeaponEquipped       ()               ;
	bool IS_Aiming               ()               ;
	void PLAY_FireMontage        (bool p_bAiming) ;
	void PLAY_ElimMontage        ()               ;
	void PLAY_ReloadWeaponMontage()               ;
	void ELIMINATE               ()               ;

	//
	// *** Getters
	//
	//
	// ** Inline Getters
	//
	FORCEINLINE float                      GET_AO_Yaw                  () const { return m_AO_Yaw                  ; }
	FORCEINLINE float                      GET_AO_Pitch                () const { return m_AO_Pitch                ; }
	FORCEINLINE ETurningInPlace            GET_TurningInPlace          () const { return m_TurningInPlace          ; }
	FORCEINLINE UCameraComponent*          GET_FollowCamera            () const { return m_FollowCamera            ; }
	FORCEINLINE bool                       GET_RotateRootBone          () const { return m_bRotateRootBone         ; }
	FORCEINLINE bool                       GET_Eliminated              () const { return m_bEliminated             ; }
	FORCEINLINE float                      GET_Health                  () const { return m_Health                  ; }
	FORCEINLINE float                      GET_MaxHealth               () const { return m_MaxHealth               ; }
	FORCEINLINE ASalnaxarPlayerController* GET_SalnaxarPlayerController() const { return m_SalnaxarPlayerController; }

	// ** Normal Getters
	//
	AWeapon*     GET_EquippedWeapon()       ; // Why isn't it const ? Check if making it const changes anything
	FVector      GET_HitTarget     () const ;
	ECombatState GET_CombatState   () const ;

	//
	// *** Setters
	//
	//
	// ** Normal Setters
	//
	void SET_OverlappingWeapon(AWeapon* p_Weapon);
	
protected:

	//
	// *** Main Override
	//
	//
	virtual void BeginPlay                ()                                            override ;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override ;
	virtual void PostInitializeComponents ()                                            override ;

	//
	// *** Callback functions 
	//
	//
	// ** Override callbacks
	//
	virtual void Jump() override;

	// ** Normal callbacks
	//
	// * Movement mapping
	//
	void ACTION_MoveForwardKey_PRESSED(float p_Value) ;
	void ACTION_MoveRightKey_PRESSED  (float p_Value) ;
	void ACTION_TurnKey_PRESSED       (float p_Value) ;
	void ACTION_LookUpKey_PRESSED     (float p_Value) ;

	// * Action mapping
	//
	void ACTION_EquipKey_PRESSED    () ;
	void ACTION_CrouchKey_PRESSED   () ;
	void ACTION_AimKey_PRESSED      () ;
	void ACTION_AimKey_RELEASED     () ;
	void ACTION_FireKey_PRESSED     () ;
	void ACTION_FireKey_RELEASED    () ;
	void ACTION_InteractKey_PRESSED () ;
	void ACTION_ProfileKey_PRESSED  () ;
	void ACTION_InventoryKey_PRESSED() ;
	void ACTION_DropItem_PRESSED    () ;
	void ACTION_DisplayMap_PRESSED  () ;
	void ACTION_DisplayMap_RELEASED () ;
	void ACTION_Reload_PRESSED      () ;
	void ACTION_ChatKey_PRESSED     () ;

	// * Binded to delegate callbacks
	UFUNCTION() // Callbacks always need to be UFUNCTION()
	void RECEIVE_Damage
	(
		AActor*            p_DamageActor          , 
		float              p_Damage               ,  
		const UDamageType* p_DamageType           , 
		AController*       p_InstigatorController , 
		AActor*            p_DamageCauser
	); // Callback function for OnTakeAnyDamage delegate

	//
	// *** Normal functions
	//
	//
	void  UPDATE_HUDHealth      ()                  ;
	void  TURN_SimulatedProxies ()                  ;
	void  COMPUTE_AimOffset     (float p_DeltaTime) ;
	void  COMPUTE_AimOffsetPitch()                  ;
	void  COMPUTE_TurningInPlace(float p_DeltaTime) ;
	float COMPUTE_Speed         ()                  ;
	void  PLAY_HitReactMontage  ()                  ;
	void  POLL_Init             ()                  ; // Poll for any relevant classes and initialize our HUD

private:

	// 
	// *** RPCs
	//
	//

	/***
	    Replications only works one way : from the server to the client(s) !
	    In other ways, this function is only called on the client(s), never on the server !
	***/

	UFUNCTION()                 void ONREP_m_OverlappingWeapon     (AWeapon* p_LastWeapon) ;
	UFUNCTION()                 void ONREP_m_Health                ()                      ;
	UFUNCTION(Server, Reliable) void SERVER_ACTION_EquipKey_PRESSED()                      ;

	//
	// *** Normal functions
	//
	//
	UFUNCTION()	void UPDATE_DissolveMaterial(float p_DissolveValue) ; // Callback function for Timeline dissolve
	void HIDE_CameraIfCharacterClose        ()                      ;
	void ElimTimer_FINISHED                 ()                      ;
	void START_Dissolve                     ()                      ;
	void INIT_Inputs                        ()                      ; 

public: // Testing

	FInteractSignature m_OnInteractDelegate ;
	ADoor*             m_Door = nullptr     ;

private:

	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (DisplayName = "Camera Boom"))
	USpringArmComponent* m_CameraBoom = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Camera", meta = (DisplayName = "Camera"))
	UCameraComponent* m_FollowCamera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), meta = (DisplayName = "Overhead Widget"))
	UWidgetComponent* m_OverheadWidget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (DisplayName = "Combat Component"), meta = (AllowPrivateAccess = "true"))
	UCombatComponent* m_CombatComp = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_m_OverlappingWeapon)
	AWeapon* m_OverlappingWeapon = nullptr;

	float           m_AO_Yaw              ;
	float           m_Interp_AO_Yaw       ;
	float           m_AO_Pitch            ;
	FRotator        m_StartingAimRotation ;
	ETurningInPlace m_TurningInPlace      ;

	//
	// Montages
	//

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "Fire Weapon Montage"))
	UAnimMontage* m_FireWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "Hit React Montage"))
	UAnimMontage* m_HitReactMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "Elim Montage"))
	UAnimMontage* m_ElimMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (DisplayName = "Reload Montage"))
	UAnimMontage* m_ReloadWeaponMontage = nullptr;

	UPROPERTY(EditAnywhere, meta = (DisplayName = "Camera Threshold"))
	float m_CameraThreshold;

	bool     m_bRotateRootBone                  ;
	float    m_TurnThreshold                    ;
	FRotator m_ProxyRotationLastFrame           ;
	FRotator m_ProxyRotation                    ;
	float    m_ProxyYaw                         ;
	float    m_TimeSinceLastMovementReplication ;

	//
	// Player Health
	//
	UPROPERTY(EditAnywhere, Category = "Player Stats", meta = (DisplayName = "Max Health"))
	float m_MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_m_Health, VisibleAnywhere, Category = "Player Stats", meta = (DisplayName = "Health"))
	float m_Health;

	ASalnaxarPlayerController* m_SalnaxarPlayerController = nullptr;

	bool m_bEliminated;

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Elim Delay"))
	float m_ElimDelay;

	FTimerHandle m_ElimTimer;

	//
	// Dissolve effect
	//
	UPROPERTY(VisibleAnywhere, meta = (DisplayName = "Dissolve Timeline"))
	UTimelineComponent* m_DissolveTimeline = nullptr;

	FOnTimelineFloat m_DissolveTrack;
	
	UPROPERTY(EditAnywhere, meta = (DisplayName = "Dissolve Curve"))
	UCurveFloat* m_DissolveCurve = nullptr;

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = "Elim", meta = (DisplayName = "Dynamic Dissolve Material Instance"))
	UMaterialInstanceDynamic* m_DynamicDissolveMaterialInstance = nullptr;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = "Elim", meta = (DisplayName = "Dissolve Material Instance"))
	UMaterialInstance* m_DissolveMaterialInstance = nullptr;

	ASalnaxarPlayerState* m_SalnaxarPlayerState = nullptr;

};
