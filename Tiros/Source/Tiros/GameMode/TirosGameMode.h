// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TirosGameMode.generated.h"

namespace MatchState
{
	extern TIROS_API const FName Cooldown; // Match duration has been reached. Display winner
}

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
	
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	
	float LevelStartingTime = 0.f;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	float CountdownTime = 0.f;
	
public:
	FORCEINLINE float GetCountdownTime() const {return CooldownTime;}
};
