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
protected:
	virtual void BeginPlay() override;
private:
	class ATirosHUD* TirosHUD;
	
};
