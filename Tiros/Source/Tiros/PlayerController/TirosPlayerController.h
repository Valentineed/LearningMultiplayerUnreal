// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TirosPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TIROS_API ATirosPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHeath(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	virtual void OnPossess(APawn* InPawn) override;
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	class ATirosHUD* TirosHUD;
	
};
