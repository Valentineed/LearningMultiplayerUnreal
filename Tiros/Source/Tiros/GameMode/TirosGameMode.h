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
	ATirosGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(class ATirosCharacter* EliminatedCharacter, class ATirosPlayerController* VictimController,  class ATirosPlayerController* AttackerController);
	virtual void RequestRespawn( ACharacter* EliminatedCharacter, AController* EliminatedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;
	float LevelStartingTime = 0.f;
protected:
	virtual void BeginPlay() override;
private:
	float CountdownTime = 0.f;
};
