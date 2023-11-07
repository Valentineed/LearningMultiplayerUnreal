// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Tiros/Character/TirosCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
}

void UBuffComponent::Heal(const float HealAmount, const float HealingTime)
{
	bHealing = true;
	AmountToHeal += HealAmount;
	HealingRate = AmountToHeal / HealingTime;
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if(Character == nullptr || Character->IsEliminated())
	{
		return;
	}
	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this,&UBuffComponent::ResetSpeeds,BuffTime);
	
	SetCharacterSpeed(BuffBaseSpeed, BuffCrouchSpeed);
	RPC_MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::ResetSpeeds()
{
	SetCharacterSpeed(InitialBaseSpeed, InitialCrouchSpeed);
	RPC_MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::SetCharacterSpeed(float BaseSpeed, float CrouchSpeed)
{
	if(Character == nullptr || Character->IsEliminated())
	{
		return;
	}
	
	if(UCharacterMovementComponent* MovementComponent =  Character->GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = BaseSpeed;
		MovementComponent->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

void UBuffComponent::RPC_MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	SetCharacterSpeed(BaseSpeed, CrouchSpeed);
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if(!bHealing || Character == nullptr || Character->IsEliminated())
	{
		return;
	}
	
	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if(AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}	
}

