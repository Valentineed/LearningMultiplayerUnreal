// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Tiros/HUD/TirosHUD.h"
#include "Tiros/Weapon/WeaponTypes.h"
#include "Tiros/TirosTypes/CombatState.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TIROS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ATirosCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapons();
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
	void FireButtonPressed(bool bPressed);

	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();
	
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void RPC_ServerLaunchGrenade(const FVector_NetQuantize& Target);

	
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void RPC_ServerAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	
	UFUNCTION()
	void OnRep_SecondaryWeapon();
	
	void Fire();
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgunWeapon();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(Server,Reliable)
	void RPC_ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void RPC_MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server,Reliable)
	void RPC_ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
	void RPC_MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);
	
	UFUNCTION(Server,Reliable)
	void RPC_ServerReload();

	void HandleReload();
	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void RPC_ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;
	
	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
private:

	UPROPERTY()
	ATirosCharacter* Character;
	UPROPERTY()
	class ATirosPlayerController* Controller;
	UPROPERTY()
	class ATirosHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	// Hud and Crosshairs
	float CrosshairVelocityFactor;
	float CrosshairInAirFactorFactor;
	float CrosshairAimingFactor;
	float CrosshairShootingFactor;
	
	FVector HitTarget;
	
	FHUDPackage HUDPackage;

	/**
	* Aiming and FOV
	*/
	
	//FOV when not aiming; set to the camera base FOV in BeginPlay
	float DefaultFOV;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;
	
	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/**
	* Automatic fire
	*/
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	// Carried ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingAssaultRifleAmmo = 30;
	
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;
	
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 45;
	
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 45;
	
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 15;
	
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 15;
	
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 15;
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	void UpdateHUDGrenade();
public:
	FORCEINLINE int32 GetGrenades() const {return Grenades;}
	bool ShouldSwapWeapons();
};
