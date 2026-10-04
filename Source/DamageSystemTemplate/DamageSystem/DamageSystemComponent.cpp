// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageSystemComponent.h"


// Sets default values for this component's properties
UDamageSystemComponent::UDamageSystemComponent()
{
	// The component is purely event driven (damage/heal calls), so it never needs to tick.
	PrimaryComponentTick.bCanEverTick = false;
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
	
	if ((IsInvincible && !DamageInfo.ShouldDamageInvincible) || (IsBlocking && DamageInfo.CanBeBlocked))
	{
		OnDamageAvoided.Broadcast(DamageInfo);
		return false;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageInfo.DamageAmount, 0.f, MaxHealth);
	OnDamageTaken.Broadcast(DamageInfo);
	if (CurrentHealth <= 0.0f)
	{
		IsDead = true;
		OnDeath.Broadcast();
	}
	return true;
}

void UDamageSystemComponent::HandleIncomingHeal(float HealAmount, AActor* Healer)
{
	if (IsDead) { return; }
	
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
	OnHealReceived.Broadcast(HealAmount, Healer);
	
}

void UDamageSystemComponent::SetStartingHealth(float StartingHealth)
{
	MaxHealth = StartingHealth;
	CurrentHealth = StartingHealth;
}

