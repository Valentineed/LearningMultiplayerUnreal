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
	ShieldRampUp(DeltaTime);
}

void UBuffComponent::Heal(const float HealAmount, const float HealingTime)
{
	bHealing = true;
	AmountToHeal += HealAmount;
	HealingRate = AmountToHeal / HealingTime;
}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishAmount += ShieldAmount;
	ShieldReplenishRate = ShieldReplenishAmount / ReplenishTime;
}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
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

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if(Character == nullptr || Character->IsEliminated())
	{
		return;
	}
	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this,&UBuffComponent::ResetJump,BuffTime);
	
	SetCharacterJumpVelocity(BuffJumpVelocity);
	RPC_MulticastJumpBuff(BuffJumpVelocity);
}

void UBuffComponent::ResetJump()
{
	SetCharacterJumpVelocity(InitialJumpVelocity);
	RPC_MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::RPC_MulticastJumpBuff_Implementation(float JumpVelocity)
{
	SetCharacterJumpVelocity(JumpVelocity);
}

void UBuffComponent::SetCharacterJumpVelocity(float Velocity)
{
	if(Character == nullptr || Character->IsEliminated())
	{
		return;
	}
	
	if(UCharacterMovementComponent* MovementComponent =  Character->GetCharacterMovement())
	{
		MovementComponent->JumpZVelocity = Velocity;
	}
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

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if(!bReplenishingShield || Character == nullptr || Character->IsEliminated())
	{
		return;
	}
	
	const float ReplenishFrame = ShieldReplenishRate * DeltaTime;
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishFrame, 0.f, Character->GetMaxShield()));
	Character->UpdateHUDShield();
	ShieldReplenishAmount -= ReplenishFrame;

	if(ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}	
}

