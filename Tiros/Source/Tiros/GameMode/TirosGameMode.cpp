// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Tiros/Character/TirosCharacter.h"

void ATirosGameMode::PlayerEliminated(ATirosCharacter* EliminatedCharacter, ATirosPlayerController* VictimController,
                                      ATirosPlayerController* AttackerController)
{
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
