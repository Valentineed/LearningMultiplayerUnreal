// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Net/UnrealNetwork.h"
#include "Tiros/Character/TirosCharacter.h"
#include "Tiros/HUD/Announcement.h"
#include "Tiros/HUD/CharacterOverlay.h"
#include "Tiros/HUD/TirosHUD.h"


void ATirosPlayerController::BeginPlay()
{
	Super::BeginPlay();
	TirosHUD = Cast<ATirosHUD>(GetHUD());
	if(TirosHUD)
	{
		TirosHUD->AddAnnouncement();
	}
}

void ATirosPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATirosPlayerController, MatchState);
}

void ATirosPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		RPC_ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ATirosPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		RPC_ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ATirosPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUDTime();

	CheckTimeSync(DeltaSeconds);
	PoolInit();
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
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
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDeaths = Deaths;
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

void ATirosPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	TirosHUD = TirosHUD == nullptr ? Cast<ATirosHUD>(GetHUD()) : TirosHUD;
	if(TirosHUD && TirosHUD->CharacterOverlay && TirosHUD->CharacterOverlay->MatchCountdownText)
	{
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		const int32 Seconds = CountdownTime - Minutes * 60.f;
		
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes, Seconds);
		TirosHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ATirosPlayerController::SetHUDTime()
{
	const uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if(CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}		
	CountdownInt = SecondsLeft;
}

void ATirosPlayerController::PoolInit()
{
	if(CharacterOverlay == nullptr)
	{
		if(TirosHUD && TirosHUD->CharacterOverlay)
		{
			CharacterOverlay = TirosHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				SetHUDHeath(HUDHealth,HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDeaths(HUDDeaths);
			}
		}
	}
}

void ATirosPlayerController::RPC_ClientReportServerTime_Implementation(float TimeOfClientRequest,
                                                                       float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	const float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ATirosPlayerController::RPC_ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	RPC_ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}


float ATirosPlayerController::GetServerTime()
{
	if(HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ATirosPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ATirosPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void ATirosPlayerController::HandleMatchHasStarted()
{
	TirosHUD = TirosHUD == nullptr ? Cast<ATirosHUD>(GetHUD()) : TirosHUD;
	if(TirosHUD)
	{
		TirosHUD->AddCharacterOverlay();
		if(TirosHUD->Announcement)
		{
			TirosHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
