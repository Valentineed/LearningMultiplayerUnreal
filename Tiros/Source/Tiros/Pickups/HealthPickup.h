// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class TIROS_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	AHealthPickup();
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	UPROPERTY(EditAnywhere)
	float HealingTime = 3.5f;
};
