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
	
	const float FinalDamage = CalculateFinalDamage(DamageInfo);
	if (DamageInfo.DamageAmount > 0.0f && FinalDamage <= 0.0f)
	{
		// Fully absorbed by resistance/armor.
		OndDamageAvoided.Broadcast(DamageInfo);
		return false;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.f, MaxHealth);
	
	// Listeners receive the damage that was actually applied.
	FDamageInfo AppliedDamage = DamageInfo;
	AppliedDamage.DamageAmount = FinalDamage;
	OndDamageTaken.Broadcast(AppliedDamage);
	if (CurrentHealth <= 0.0f)
	{
		IsDead = true;
		OndDeath.Broadcast();
	}
	return true;
}

float UDamageSystemComponent::CalculateFinalDamage(const FDamageInfo& DamageInfo) const
{
	float Resistance = 0.0f;
	if (const float* Found = Resistances.Find(DamageInfo.DamageType))
	{
		Resistance = FMath::Min(*Found, 1.0f);
	}
	
	float Damage = DamageInfo.DamageAmount * (1.0f - Resistance);
	if (DamageInfo.DamageType == EDamageType::Physical)
	{
		Damage -= Armor;
	}
	return FMath::Max(Damage, 0.0f);
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

