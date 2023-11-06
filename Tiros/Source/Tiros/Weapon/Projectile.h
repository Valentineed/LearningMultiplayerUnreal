// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class TIROS_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	
	UPROPERTY(EditDefaultsOnly)
	float MinimumDamage = 10.f;
	
	UPROPERTY(EditDefaultsOnly)
	float DamageInnerRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly)
	float DamageOuterRadius = 500.f;
	
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;
	
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;
	
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;
	
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent*  ProjectileMesh;
private:	
	FTimerHandle DestroyTimer;
	
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
	
	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;
	UPROPERTY()
	UParticleSystemComponent* TracerComponent;
};
