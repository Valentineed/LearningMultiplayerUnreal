// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosAnimInstance.h"

#include "TirosCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Tiros/Weapon/Weapon.h"

void UTirosAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	TirosCharacter = Cast<ATirosCharacter>(TryGetPawnOwner());
}

void UTirosAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(TirosCharacter == nullptr)
	{
		TirosCharacter = Cast<ATirosCharacter>(TryGetPawnOwner());
		return;
	}
	FVector Velocity = TirosCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = TirosCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = TirosCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bWeaponEquipped = TirosCharacter->IsWeaponEquipped();
	EquippedWeapon = TirosCharacter->GetEquippedWeapon();
	bIsCrouched = TirosCharacter->bIsCrouched;
	bAiming = TirosCharacter->IsAiming();
	TurningInPlace = TirosCharacter->GetTurningInPlace();
	bRotateRootBone = TirosCharacter->ShouldRotateRootBone();
	bEliminated = TirosCharacter->IsEliminated();

	// Offset yaw for Strafing
	const FRotator AimRotation = TirosCharacter->GetBaseAimRotation();
	const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(TirosCharacter->GetVelocity());
	const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation= TirosCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f,90.f);

	AO_Yaw = TirosCharacter->GetAO_Yaw();
	AO_Pitch = TirosCharacter->GetAO_Pitch();

	if(bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && TirosCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		TirosCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(),
			FRotator::ZeroRotator,OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if(TirosCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			const FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), RTS_World);
			const FRotator LookAtRotation =  UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(),
			                                                                        RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - TirosCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 30.f);
		}
	}
}
