// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Tiros/HUD/CharacterOverlay.h"
#include "Tiros/HUD/TirosHUD.h"


void ATirosPlayerController::BeginPlay()
{
	Super::BeginPlay();
	TirosHUD = Cast<ATirosHUD>(GetHUD());
}

void ATirosPlayerController::SetHUDHeath(float Health, float MaxHealth)
{
	TirosHUD = TirosHUD == nullptr ? Cast<ATirosHUD>(GetHUD()) : TirosHUD;
	if(TirosHUD && TirosHUD->CharacterOverlay && TirosHUD->CharacterOverlay->HealthBar && TirosHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		TirosHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health),FMath::CeilToInt(MaxHealth));
		TirosHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}