// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosPlayerState.h"

#include "Tiros/Character/TirosCharacter.h"
#include "Tiros/PlayerController/TirosPlayerController.h"
#include "Net/UnrealNetwork.h"

void ATirosPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATirosPlayerState, Deaths);
}

void ATirosPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<ATirosCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ATirosPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ATirosPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	Character = Character == nullptr ? Cast<ATirosCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ATirosPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
	
}

void ATirosPlayerState::AddToDeaths(int32 DeathsAmount)
{
	Deaths += DeathsAmount;
	Character = Character == nullptr ? Cast<ATirosCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ATirosPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDDeaths(Deaths);
		}
	}
}


void ATirosPlayerState::OnRep_Deaths()
{
	Character = Character == nullptr ? Cast<ATirosCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = Controller == nullptr ? Cast<ATirosPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDDeaths(Deaths);
		}
	}
}

