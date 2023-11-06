// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Tiros/Character/TirosCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr)
	{
		return;
	}
	AController* InstigatorController = OwnerPawn->GetController();
	if(const USkeletalMeshSocket* MuzzleFlashSocket = 	GetWeaponMesh()->GetSocketByName("MuzzleFlash"))
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		
		TMap<ATirosCharacter*, uint32> HitMap;
		for(uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start,HitTarget,FireHit);
			ATirosCharacter* TirosCharacter = Cast<ATirosCharacter>(FireHit.GetActor());
			if(TirosCharacter && HasAuthority() && InstigatorController)
			{
				if(HitMap.Contains(TirosCharacter))
				{
					HitMap[TirosCharacter]++;
				}
				else
				{
					HitMap.Emplace(TirosCharacter,1);
				}
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
		for(auto HitPair : HitMap)
		{
			if(InstigatorController && HitPair.Key && HasAuthority())
			{				
				UGameplayStatics::ApplyDamage(HitPair.Key,Damage * HitPair.Value , InstigatorController, this, UDamageType::StaticClass());
			}
		}
	}
}
