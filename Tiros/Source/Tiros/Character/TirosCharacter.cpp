// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Tiros/Tiros.h"
#include "Tiros/GameMode/TirosGameMode.h"
#include "Tiros/PlayerController/TirosPlayerController.h"
#include "Tiros/PlayerState/TirosPlayerState.h"
#include "Tiros/TirosComponents/CombatComponent.h"
#include "Tiros/TirosComponents/BuffComponent.h"
#include "Tiros/Weapon/Weapon.h"


ATirosCharacter::ATirosCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
	
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
	
	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f,0.f,850.f);
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void ATirosCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATirosCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ATirosCharacter, Health);
	DOREPLIFETIME(ATirosCharacter, Shield);
	DOREPLIFETIME(ATirosCharacter, bDisableGameplay);
}

void ATirosCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character = this;
	}
	if(Buff)
	{
		Buff->Character = this;
		if(const UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
		{
			Buff->SetInitialSpeed(MovementComponent->MaxWalkSpeed,MovementComponent->MaxWalkSpeedCrouched);
			Buff->SetInitialJumpVelocity(MovementComponent->JumpZVelocity);
		}
	}
}

void ATirosCharacter::PlayFireMontage(bool bAiming)
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ATirosCharacter::PlayReloadMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ATirosCharacter::PlayEliminatedMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && EliminatedMontage)
	{
		AnimInstance->Montage_Play(EliminatedMontage);
	}
}

void ATirosCharacter::PlayHitReactMontage()
{
	if(Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		const FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ATirosCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	if(bEliminated)
	{
		return;
	}

	float DamageToHealth = Damage;
	if(Shield > 0.f)
	{
		if(Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	
	UpdateHUDShield();
	UpdateHUDHealth();
	
	PlayHitReactMontage();
	if(Health != 0.f)
	{
		return;
	}
	if(ATirosGameMode* TirosGameMode = GetWorld()->GetAuthGameMode<ATirosGameMode>())
	{
		TirosPlayerController = TirosPlayerController == nullptr? Cast<ATirosPlayerController>(Controller) : TirosPlayerController;
		ATirosPlayerController* AttackerController = Cast<ATirosPlayerController>(InstigatorController);
		TirosGameMode->PlayerEliminated(this, TirosPlayerController, AttackerController);
	}
}

void ATirosCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ATirosCharacter::Eliminated()
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	RPC_MulticastEliminated();
	GetWorldTimerManager().SetTimer(EliminatedTimer, this, &ThisClass::EliminatedTimerFinished, EliminatedDelay);
}

void ATirosCharacter::RPC_MulticastEliminated_Implementation()
{
	if(TirosPlayerController)
	{
		TirosPlayerController->SetHUDWeaponAmmo(0);
	}
	bEliminated = true;
	PlayEliminatedMontage();

	//Start dissolve effect
	if(DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance,this, FName(""));
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	bDisableGameplay = true;
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if(IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
}

void ATirosCharacter::EliminatedTimerFinished()
{
	if(ATirosGameMode* TirosGameMode = GetWorld()->GetAuthGameMode<ATirosGameMode>())
	{
		TirosGameMode->RequestRespawn(this, Controller);
	}
}

void ATirosCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateHUDHealth();
	UpdateHUDShield();
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ATirosCharacter::ReceiveDamage);
	}
}

void ATirosCharacter::Destroyed()
{
	Super::Destroyed();
	ATirosGameMode* TirosGameMode = Cast<ATirosGameMode>(UGameplayStatics::GetGameMode(this));
	const bool bMatchNotInProgress = TirosGameMode && TirosGameMode->GetMatchState() != MatchState::InProgress;
	if(Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

void ATirosCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ATirosCharacter::RotateInPlace(float DeltaTime)
{
	if(bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		
		return;
	}
	if(GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);		
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if(TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}


void ATirosCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ThisClass::Jump);
	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ATirosCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ATirosCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ATirosCharacter::CrouchButtonRelease);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ATirosCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ATirosCharacter::AimButtonRelease);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATirosCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ATirosCharacter::FireButtonRelease);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ATirosCharacter::ReloadButtonPressed);
}

void ATirosCharacter::MoveForward(float Value)
{
	if(bDisableGameplay)
	{
		return;
	}
	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ATirosCharacter::MoveRight(float Value)
{
	if(bDisableGameplay)
	{
		return;
	}
	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f,Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ATirosCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ATirosCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ATirosCharacter::EquipButtonPressed()
{
	if(bDisableGameplay)
	{
		return;
	}
	if(Combat)
	{
		if(HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			RPC_ServerEquipButtonPressed();
		}
	}
}

void ATirosCharacter::CrouchButtonPressed()
{
	if(bDisableGameplay)
	{
		return;
	}
	Crouch();
}

void ATirosCharacter::CrouchButtonRelease()
{
	if(bDisableGameplay)
	{
		return;
	}
	UnCrouch();
}

void ATirosCharacter::ReloadButtonPressed()
{
	if(bDisableGameplay)
	{
		return;
	}
	if(Combat)
	{
		Combat->Reload();
	}
}

void ATirosCharacter::AimButtonPressed()
{
	if(bDisableGameplay)
	{
		return;
	}
	if(Combat)
	{
		Combat->SetAiming(true);
	}
}

void ATirosCharacter::AimButtonRelease()
{
	if(bDisableGameplay)
	{
		return;
	}
	if(Combat)
	{
		Combat->SetAiming(false);
	}
}

void ATirosCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if(AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270,360] to [-90,0] because Unreal compress behind the scene to optimize the bandwidth
		const FVector2D InRange(270.f,360.f);
		const FVector2D OutRange(-90.f,0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ATirosCharacter::AimOffset(float DeltaTime)
{
	if(Combat && Combat->EquippedWeapon == nullptr)
	{
		return;
	}
	const float Speed = CalculateSpeed();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if(Speed == 0.f && !bIsInAir)// standing still, not jumping
	{
		bRotateRootBone = true;
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator( CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if(TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if(Speed > 0.f || bIsInAir)// running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ATirosCharacter::SimProxiesTurn()
{
	if(Combat && Combat->EquippedWeapon == nullptr)
	{
		return;
	}
	
	bRotateRootBone = false;
	const float Speed = CalculateSpeed();
	if(Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ATirosCharacter::Jump()
{
	if(bDisableGameplay)
	{
		return;
	}
	Super::Jump();
}

void ATirosCharacter::FireButtonPressed()
{
	if(bDisableGameplay)
	{
		return;
	}
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ATirosCharacter::FireButtonRelease()
{
	if(bDisableGameplay)
	{
		return;
	}
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ATirosCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f)
	{		
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if(TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if(FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ATirosCharacter::RPC_ServerEquipButtonPressed_Implementation()
{
	if(Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ATirosCharacter::HideCameraIfCharacterClose()
{
	if(!IsLocallyControlled())
	{
		return;
	}
	
	//TODO maybe improve to not do that all the time, maybe add a see true shader
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

float ATirosCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ATirosCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if(Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ATirosCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if(Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ATirosCharacter::UpdateHUDHealth()
{
	TirosPlayerController = TirosPlayerController == nullptr ? Cast<ATirosPlayerController>(Controller) : TirosPlayerController;
	if(TirosPlayerController)
	{
		TirosPlayerController->SetHUDHeath(Health, MaxHealth);
	}
}

void ATirosCharacter::UpdateHUDShield()
{
	TirosPlayerController = TirosPlayerController == nullptr ? Cast<ATirosPlayerController>(Controller) : TirosPlayerController;
	if(TirosPlayerController)
	{
		TirosPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ATirosCharacter::PollInit()
{
	if(TirosPlayerState == nullptr)
	{
		TirosPlayerState = GetPlayerState<ATirosPlayerState>();
		if(TirosPlayerState)
		{
			TirosPlayerState->AddToScore(0.f);
			TirosPlayerState->AddToDeaths(0);
		}
	}
}

void ATirosCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ATirosCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	if(DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ATirosCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);			
	}
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);			
		}
	}
}

bool ATirosCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ATirosCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* ATirosCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr)
	{
		return nullptr;
	}
	return Combat->EquippedWeapon;
}

FVector ATirosCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState ATirosCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

void ATirosCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}




