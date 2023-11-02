// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TirosGameState.generated.h"

class ATirosPlayerState;
/**
 * 
 */
UCLASS()
class TIROS_API ATirosGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void UpdateTopScore(ATirosPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<ATirosPlayerState*> TopScoringPlayers;

private:

	float TopScore = 0.f;
	
};
