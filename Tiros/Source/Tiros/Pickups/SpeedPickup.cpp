// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"

#include "Tiros/Character/TirosCharacter.h"
#include "Tiros/TirosComponents/BuffComponent.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlapComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if(const ATirosCharacter* TirosCharacter = Cast<ATirosCharacter>(OtherActor))
	{
		if(UBuffComponent* Buff = TirosCharacter->GetBuff())
		{
			Buff->BuffSpeed(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
		
	}
	Destroy();
}
