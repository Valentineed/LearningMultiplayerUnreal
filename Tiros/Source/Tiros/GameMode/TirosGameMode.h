// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TirosGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TIROS_API ATirosGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class ATirosCharacter* EliminatedCharacter, class ATirosPlayerController* VictimController,  class ATirosPlayerController* AttackerController);
	virtual void RequestRespawn( ACharacter* EliminatedCharacter, AController* EliminatedController);
};
