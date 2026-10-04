// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageableCharacterBase.h"
#include "DamageSystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "AIController.h"
#include "BrainComponent.h"


// Sets default values
ADamageableCharacterBase::ADamageableCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
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
		DamageSystemComponent->OnHealReceived.AddDynamic(this, &ADamageableCharacterBase::RespondToHealReceived);
		DamageSystemComponent->OnDeath.AddDynamic(this, &ADamageableCharacterBase::RespondToDeath);
	}
}

void ADamageableCharacterBase::RespondToDamageTaken_Implementation(const FDamageInfo& DamageInfo)
{
}

void ADamageableCharacterBase::RespondToDamageAvoided_Implementation(const FDamageInfo& DamageInfo)
{
}

void ADamageableCharacterBase::RespondToHealReceived_Implementation(float HealAmount, AActor* Healer)
{
}

void ADamageableCharacterBase::RespondToDeath_Implementation()
{
	// Stop all control: no movement, no player input, no AI behaviour.
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

// Called every frame
void ADamageableCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

bool ADamageableCharacterBase::TakeDamage_Implementation(const FDamageInfo& DamageInfo)
{
	if (!DamageSystemComponent) return false;
	
	return DamageSystemComponent->HandleIncomingDamage(DamageInfo);
}
