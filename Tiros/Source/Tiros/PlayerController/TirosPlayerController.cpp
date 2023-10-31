// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Tiros/Character/TirosCharacter.h"
#include "Tiros/HUD/CharacterOverlay.h"
#include "Tiros/HUD/TirosHUD.h"


void ATirosPlayerController::BeginPlay()
{
	Super::BeginPlay();
	TirosHUD = Cast<ATirosHUD>(GetHUD());
}

void ATirosPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ATirosCharacter* TirosCharacter = Cast<ATirosCharacter>(InPawn);
	if(TirosCharacter)
	{
		SetHUDHeath(TirosCharacter->GetHealth(), TirosCharacter->GetMaxHealth());
	}	
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

void ATirosPlayerController::SetHUDScore(float Score)
{
	TirosHUD = TirosHUD == nullptr ? Cast<ATirosHUD>(GetHUD()) : TirosHUD;
	if(TirosHUD && TirosHUD->CharacterOverlay && TirosHUD->CharacterOverlay->ScoreAmount)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		TirosHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ATirosPlayerController::SetHUDDeaths(int32 Deaths)
{
	TirosHUD = TirosHUD == nullptr ? Cast<ATirosHUD>(GetHUD()) : TirosHUD;
	if(TirosHUD && TirosHUD->CharacterOverlay && TirosHUD->CharacterOverlay->DeathsAmount)
	{
		const FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		TirosHUD->CharacterOverlay->DeathsAmount->SetText(FText::FromString(DeathsText));
	}
}

void ATirosPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	TirosHUD = TirosHUD == nullptr ? Cast<ATirosHUD>(GetHUD()) : TirosHUD;
	if(TirosHUD && TirosHUD->CharacterOverlay && TirosHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		TirosHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ATirosPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	TirosHUD = TirosHUD == nullptr ? Cast<ATirosHUD>(GetHUD()) : TirosHUD;
	if(TirosHUD && TirosHUD->CharacterOverlay && TirosHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		TirosHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}
