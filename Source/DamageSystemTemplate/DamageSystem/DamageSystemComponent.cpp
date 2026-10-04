// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageSystemComponent.h"
#include "GameFramework/Actor.h"


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
	
	if (IsInvincible && !DamageInfo.ShouldDamageInvincible)
	{
		OnDamageAvoided.Broadcast(DamageInfo);
		return false;
	}
	
	// A parry inside the parry window negates the hit entirely and lets the owner react (counter-attack, etc.).
	if (IsParrying && DamageInfo.CanBeParried && IsInBlockArc(DamageInfo))
	{
		OnDamageParried.Broadcast(DamageInfo);
		return false;
	}
	
	if (IsBlocking && DamageInfo.CanBeBlocked && IsInBlockArc(DamageInfo))
	{
		OnDamageAvoided.Broadcast(DamageInfo);
		return false;
	}
	
	const float FinalDamage = CalculateFinalDamage(DamageInfo);
	if (DamageInfo.DamageAmount > 0.0f && FinalDamage <= 0.0f)
	{
		// Fully absorbed by resistance/armor.
		OnDamageAvoided.Broadcast(DamageInfo);
		return false;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.f, MaxHealth);
	
	// Listeners receive the damage that was actually applied.
	FDamageInfo AppliedDamage = DamageInfo;
	AppliedDamage.DamageAmount = FinalDamage;
	OnDamageTaken.Broadcast(AppliedDamage);
	if (CurrentHealth <= 0.0f)
	{
		IsDead = true;
		OnDeath.Broadcast();
	}
	return true;
}

bool UDamageSystemComponent::IsInBlockArc(const FDamageInfo& DamageInfo) const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !DamageInfo.DamageCauser) return true;
	
	FVector ToCauser = DamageInfo.DamageCauser->GetActorLocation() - OwnerActor->GetActorLocation();
	ToCauser.Z = 0.0f;
	FVector Forward = OwnerActor->GetActorForwardVector();
	Forward.Z = 0.0f;
	if (!ToCauser.Normalize() || !Forward.Normalize()) return true;
	
	const float CosLimit = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(BlockHalfAngle, 0.0f, 180.0f)));
	return FVector::DotProduct(Forward, ToCauser) >= CosLimit;
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
	OnHealReceived.Broadcast(HealAmount, Healer);
	
}

void UDamageSystemComponent::SetStartingHealth(float StartingHealth)
{
	MaxHealth = StartingHealth;
	CurrentHealth = StartingHealth;
}

