// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Tiros/Character/TirosCharacter.h"
#include "Tiros/PlayerController/TirosPlayerController.h"
#include "Tiros/PlayerState/TirosPlayerState.h"

ATirosGameMode::ATirosGameMode()
{
	bDelayedStart = true;
	
}

void ATirosGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ATirosGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
}

void ATirosGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ATirosPlayerController* TirosPlayer = Cast<ATirosPlayerController>(*It);
		if(TirosPlayer)
		{
			TirosPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ATirosGameMode::PlayerEliminated(ATirosCharacter* EliminatedCharacter, ATirosPlayerController* VictimController,
                                      ATirosPlayerController* AttackerController)
{
	ATirosPlayerState* AttackerPlayerState = AttackerController? Cast<ATirosPlayerState>(AttackerController->PlayerState) : nullptr;
	ATirosPlayerState* VictimPlayerState = VictimController? Cast<ATirosPlayerState>(VictimController->PlayerState) : nullptr;

	if(AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if(VictimPlayerState)
	{
		VictimPlayerState->AddToDeaths(1);
	}
	
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Eliminated();
	}
}

void ATirosGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if(EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(),PlayerStarts);
		const int32 Selection = FMath::RandRange(0, PlayerStarts.Num() -1);
		RestartPlayerAtPlayerStart(EliminatedController,PlayerStarts[Selection]);
	}
}
