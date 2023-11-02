// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosPlayerController.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Tiros/Character/TirosCharacter.h"
#include "Tiros/GameMode/TirosGameMode.h"
#include "Tiros/HUD/Announcement.h"
#include "Tiros/HUD/CharacterOverlay.h"
#include "Tiros/HUD/TirosHUD.h"
#include "Tiros/TirosComponents/CombatComponent.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

void ATirosPlayerController::BeginPlay()
{
	Super::BeginPlay();
	TirosHUD = Cast<ATirosHUD>(GetHUD());
	RPC_ServerCheckMatchState();
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

void ATirosPlayerController::RPC_ServerCheckMatchState_Implementation()
{
	if(const ATirosGameMode* GameMode = Cast<ATirosGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		RPC_ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
		if(TirosHUD && MatchState == MatchState::WaitingToStart)
		{
			TirosHUD->AddAnnouncement();
		}
	}
}

void ATirosPlayerController::RPC_ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if(TirosHUD && MatchState == MatchState::WaitingToStart)
	{
		TirosHUD->AddAnnouncement();
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
		if(CooldownTime < 0.f)
		{
			TirosHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		const int32 Seconds = CountdownTime - Minutes * 60.f;
		
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes, Seconds);
		TirosHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ATirosPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	TirosHUD = TirosHUD == nullptr ? Cast<ATirosHUD>(GetHUD()) : TirosHUD;
	if(TirosHUD && TirosHUD->Announcement && TirosHUD->Announcement->WarmupTime)
	{
		if(CooldownTime < 0.f)
		{
			TirosHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		const int32 Seconds = CountdownTime - Minutes * 60.f;
		
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"),Minutes, Seconds);
		TirosHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ATirosPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if(MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if(MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if(HasAuthority())
	{
		TirosGameMode = TirosGameMode == nullptr ? Cast<ATirosGameMode>(UGameplayStatics::GetGameMode(this)) : TirosGameMode;
		if(TirosGameMode)
		{
			SecondsLeft = FMath::CeilToInt(TirosGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	
	if(CountdownInt != SecondsLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
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
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ATirosPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
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

void ATirosPlayerController::HandleCooldown()
{
	TirosHUD = TirosHUD == nullptr ? Cast<ATirosHUD>(GetHUD()) : TirosHUD;
	if(TirosHUD)
	{
		TirosHUD->CharacterOverlay->RemoveFromParent();
		if(TirosHUD->Announcement && TirosHUD->Announcement->AnnouncementText && TirosHUD->Announcement->InfoText)
		{
			TirosHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			const FString AnnouncementText("New Match Starts In:");
			TirosHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			TirosHUD->Announcement->InfoText->SetText(FText());
		}
	}
	ATirosCharacter* TirosCharacter = Cast<ATirosCharacter>(GetPawn());
	if(TirosCharacter && TirosCharacter->GetCombat())
	{
		TirosCharacter->bDisableGameplay = true;
		TirosCharacter->GetCombat()->FireButtonPressed(false);
	}
}
