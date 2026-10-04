// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageableCharacterBase.h"
#include "DamageSystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/PlayerController.h"


// Sets default values
ADamageableCharacterBase::ADamageableCharacterBase()
{
	// Nothing here needs per-frame updates; damage handling is event driven.
	PrimaryActorTick.bCanEverTick = false;
	
	DamageSystemComponent = CreateDefaultSubobject<UDamageSystemComponent>(TEXT("DamageSystemComponent"));
	
	
}

// Called when the game starts or when spawned
void ADamageableCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (DamageSystemComponent)
	{
		DamageSystemComponent->OnDamageTaken.AddDynamic(this, &ADamageableCharacterBase::RespondToDamageTaken);
		DamageSystemComponent->OnDamageAvoided.AddDynamic(this, &ADamageableCharacterBase::RespondToDamageAvoided);
		DamageSystemComponent->OnDamageParried.AddDynamic(this, &ADamageableCharacterBase::RespondToDamageParried);
		DamageSystemComponent->OnHealReceived.AddDynamic(this, &ADamageableCharacterBase::RespondToHealReceived);
		DamageSystemComponent->OnDeath.AddDynamic(this, &ADamageableCharacterBase::RespondToDeath);
	}
}

void ADamageableCharacterBase::RespondToDamageTaken_Implementation(const FDamageInfo& DamageInfo)
{
	switch (DamageInfo.DamageResponse)
	{
	case EDamageResponse::HitReaction:
		PlayResponseMontage(HitReactionMontage, DamageInfo.ShouldForceInterrupt);
		break;
	case EDamageResponse::Knockback:
		ApplyKnockback(DamageInfo);
		break;
	case EDamageResponse::Stagger:
		// A stagger always breaks whatever the character is doing.
		PlayResponseMontage(StaggerMontage, true);
		break;
	case EDamageResponse::Stun:
		ApplyStun(DamageInfo.StunDuration);
		break;
	default:
		break;
	}
}

void ADamageableCharacterBase::PlayResponseMontage(UAnimMontage* Montage, bool bForceInterrupt)
{
	if (!Montage || !GetMesh()) return;
	
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		// Without ShouldForceInterrupt, an action that is already playing (e.g. an attack) is not interrupted.
		if (!bForceInterrupt && AnimInstance->IsAnyMontagePlaying()) return;
		
		AnimInstance->Montage_Play(Montage);
	}
}

void ADamageableCharacterBase::ApplyKnockback(const FDamageInfo& DamageInfo)
{
	if (DamageInfo.KnockbackStrength <= 0.0f) return;
	
	// Push away from the attacker; without a causer, fall back to pushing backwards.
	FVector Direction = -GetActorForwardVector();
	if (DamageInfo.DamageCauser)
	{
		Direction = GetActorLocation() - DamageInfo.DamageCauser->GetActorLocation();
	}
	Direction.Z = 0.0f;
	if (!Direction.Normalize()) return;
	
	const FVector LaunchVelocity = Direction * DamageInfo.KnockbackStrength + FVector(0.0f, 0.0f, DamageInfo.KnockbackStrength * 0.25f);
	LaunchCharacter(LaunchVelocity, true, true);
}

void ADamageableCharacterBase::ApplyStun(float Duration)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent || Duration <= 0.0f) return;
	
	MovementComponent->DisableMovement();
	GetWorldTimerManager().SetTimer(StunTimerHandle, FTimerDelegate::CreateUObject(this, &ADamageableCharacterBase::EndStun), Duration, false);
}

void ADamageableCharacterBase::EndStun()
{
	// A character that died while stunned must stay immobile.
	if (DamageSystemComponent && DamageSystemComponent->GetIsDead()) return;
	
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}
}

void ADamageableCharacterBase::RespondToDamageAvoided_Implementation(const FDamageInfo& DamageInfo)
{
}

void ADamageableCharacterBase::RespondToDamageParried_Implementation(const FDamageInfo& DamageInfo)
{
}

void ADamageableCharacterBase::RespondToHealReceived_Implementation(float HealAmount, AActor* Healer)
{
}

void ADamageableCharacterBase::RespondToDeath_Implementation()
{
	// Stop all control: no pending stun recovery, no movement, no player input, no AI behaviour.
	GetWorldTimerManager().ClearTimer(StunTimerHandle);
	
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
	
	if (AController* OwningController = GetController())
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(OwningController))
		{
			DisableInput(PlayerController);
		}
		else if (AAIController* AIController = Cast<AAIController>(OwningController))
		{
			AIController->StopMovement();
			if (AIController->GetBrainComponent())
			{
				AIController->GetBrainComponent()->StopLogic(TEXT("Dead"));
			}
		}
	}
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(true);
}

// Called to bind functionality to input
void ADamageableCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ADamageableCharacterBase::GetMaxHealth_Implementation()
{
	if (!DamageSystemComponent) return 0.0f;
	
	return DamageSystemComponent->GetMaxHealth();
}

bool ADamageableCharacterBase::GetIsDead_Implementation()
{
	if (!DamageSystemComponent) return false;
	
	return DamageSystemComponent->GetIsDead();
}

float ADamageableCharacterBase::GetCurrentHealth_Implementation()
{
	if (!DamageSystemComponent) return 0.0f;
	
	return DamageSystemComponent->GetCurrentHealth();
}

void ADamageableCharacterBase::Heal_Implementation(float HealAmount, AActor* Healer)
{
	if (DamageSystemComponent)
	{
		DamageSystemComponent->HandleIncomingHeal(HealAmount, Healer);
	}
	
}

bool ADamageableCharacterBase::ReceiveDamage_Implementation(const FDamageInfo& DamageInfo)
{
	if (!DamageSystemComponent) return false;
	
	return DamageSystemComponent->HandleIncomingDamage(DamageInfo);
}
