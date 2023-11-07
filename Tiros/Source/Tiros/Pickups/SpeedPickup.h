// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class TIROS_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
public:
	ASpeedPickup();
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:

	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;
	UPROPERTY(EditAnywhere)
	float CrouchSpeedBuff = 850.f;
	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 15.f;
};
