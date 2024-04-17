// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Tiros/Character/TirosCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr)
	{
		return;
	}
	AController* InstigatorController = OwnerPawn->GetController();
	if(const USkeletalMeshSocket* MuzzleFlashSocket = 	GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		//Maps hit character to number of times hits
		TMap<ATirosCharacter*, uint32> HitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start,HitTarget,FireHit);
			
			ATirosCharacter* TirosCharacter = Cast<ATirosCharacter>(FireHit.GetActor());
			if(TirosCharacter)
			{
				if(HitMap.Contains(TirosCharacter))
				{
					HitMap[TirosCharacter]++;
				}
				else
				{
					HitMap.Emplace(TirosCharacter,1);
				}
				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}
				if(HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, .5f, FMath::FRandRange(-0.5f, 0.5f));
				}
			}
			
		}
		if(HasAuthority() && InstigatorController)
		{
			for(auto HitPair : HitMap)
			{
				if(HitPair.Key)
				{				
					UGameplayStatics::ApplyDamage(HitPair.Key,// Character that was hit
						Damage * HitPair.Value ,// Multiply Damage by number of times hit
						InstigatorController, this, UDamageType::StaticClass());
				}
			}
		}
		
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = 	GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if(MuzzleFlashSocket == nullptr)
	{
		return;
	}
	
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f,SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - TraceStart;
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		HitTargets.Add(ToEndLoc);
	}
}
