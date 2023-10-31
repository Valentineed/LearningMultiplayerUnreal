// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TirosPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TIROS_API ATirosPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Deaths();
	
	void AddToScore(float ScoreAmount);
	void AddToDeaths(int32 DeathsAmount);
private:
	UPROPERTY()
	class ATirosCharacter* Character;
	UPROPERTY()
	class ATirosPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;

};
