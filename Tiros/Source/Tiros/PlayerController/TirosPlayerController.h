// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TirosPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TIROS_API ATirosPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHeath(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

	virtual float GetServerTime(); // Sync with server wold clock
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	/**
	 * Sync time between client and server
	 */
	
	// Request the current current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void RPC_ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Server, Reliable)
	void RPC_ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between server and client time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	
	void CheckTimeSync(float DeltaSeconds);
private:
	UPROPERTY()
	class ATirosHUD* TirosHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
	
	
};
