// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageSystemComponent.h"


// Sets default values for this component's properties
UDamageSystemComponent::UDamageSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDamageSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// MaxHealth may have been changed in Blueprint/editor; always start at full health.
	CurrentHealth = MaxHealth;
}

bool UDamageSystemComponent::HandleIncomingDamage(const FDamageInfo& DamageInfo)
{
	if (IsDead) { return false; }
	
	if ((IsInvincible && !DamageInfo.ShouldDamageInvincible) || (IsBlocking && DamageInfo.CanBeBlock))
	{
		OndDamageAvoided.Broadcast(DamageInfo);
		return false;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageInfo.DamageAmount, 0.f, MaxHealth);
	OndDamageTaken.Broadcast(DamageInfo);
	if (CurrentHealth <= 0.0f)
	{
		IsDead = true;
		OndDeath.Broadcast();
	}
	return true;
}

void UDamageSystemComponent::HandleIncomingHeal(float HealAmount, AActor* Healer)
{
	if (IsDead) { return; }
	
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
	OndHealReceived.Broadcast(HealAmount, Healer);
	
}

void UDamageSystemComponent::SetStartingHealth(float StartingHealth)
{
	MaxHealth = StartingHealth;
	CurrentHealth = StartingHealth;
}

