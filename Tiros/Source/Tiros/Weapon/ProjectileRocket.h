// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class TIROS_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	virtual void Destroyed() override;
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;
	void DestroyTimerFinished();
	
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
	
	UPROPERTY(EditDefaultsOnly)
	float MinimumDamage = 10.f;
	
	UPROPERTY(EditDefaultsOnly)
	float DamageInnerRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly)
	float DamageOuterRadius = 500.f;

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent*  RocketMesh;
};
