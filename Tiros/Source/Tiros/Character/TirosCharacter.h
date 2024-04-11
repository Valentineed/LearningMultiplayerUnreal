// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "Tiros/Interfaces/InteractWithCrosshairInterface.h"
#include "Tiros/TirosTypes/TurningPlace.h"
#include "Tiros/TirosTypes/CombatState.h"
#include "TirosCharacter.generated.h"

class FOnTimelineFloat;

UCLASS()
class TIROS_API ATirosCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()
	

public:
	ATirosCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayEliminatedMontage();
	void PlayThrowGrenadeMontage();
	virtual void OnRep_ReplicatedMovement() override;

	void Eliminated();
	UFUNCTION(NetMulticast, Reliable)
	void RPC_MulticastEliminated();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	void UpdateHUDHealth();
	void UpdateHUDShield();
protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void CrouchButtonRelease();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonRelease();
	void CalculateAO_Pitch();
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonRelease();
	void PlayHitReactMontage();
	void GrenadeButtonPressed();
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	// Pool for any revelant class and initialize your HUD
	void PollInit();
	void RotateInPlace(float DeltaTime);
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;
	
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UBuffComponent* Buff;

	UFUNCTION(Server, Reliable)
	void RPC_ServerEquipButtonPressed();

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;
	
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* EliminatedMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ThrowGrenadeMontage;


	void HideCameraIfCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	/**
	 *  Player Health
	 */
	 
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/**
	 * Player shield
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	UPROPERTY()
	class ATirosPlayerController* TirosPlayerController;

	bool bEliminated = false;

	FTimerHandle EliminatedTimer;
	
	UPROPERTY(EditDefaultsOnly)
	float EliminatedDelay = 3.f;
	
	void EliminatedTimerFinished();

	/**
	 * Dissolve effect
	 */
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Eliminated)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the BP, used with dynamic instance
	UPROPERTY(EditAnywhere, Category = Eliminated)
	UMaterialInstance* DissolveMaterialInstance;
	
	/**
	 * Elim Bot
	 */
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;
	
	UPROPERTY()
	class ATirosPlayerState* TirosPlayerState;

	/**
	 * Grenade
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;
	
public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch;}
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const {return TurningInPlace;}
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
	FORCEINLINE bool ShouldRotateRootBone() const {return bRotateRootBone;} 
	FORCEINLINE bool IsEliminated() const {return bEliminated;} 
	FORCEINLINE float GetHealth() const {return Health;}
	FORCEINLINE void SetHealth(const float Amount) { Health = Amount;}
	FORCEINLINE float GetMaxHealth() const {return MaxHealth;}
	FORCEINLINE float GetShield() const {return Shield;}
	FORCEINLINE void SetShield(const float Amount) { Shield = Amount;}
	FORCEINLINE float GetMaxShield() const {return MaxShield;}
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const {return Combat;}
	FORCEINLINE bool GetDisableGameplay() const {return bDisableGameplay;}
	FORCEINLINE UBuffComponent* GetBuff() const {return Buff;}
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const {return AttachedGrenade;}
};
