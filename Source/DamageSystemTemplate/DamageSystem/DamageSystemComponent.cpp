// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageSystemComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UDamageSystemComponent::UDamageSystemComponent()
{
	// The component is purely event driven (damage/heal calls), so it never needs to tick.
	PrimaryComponentTick.bCanEverTick = false;
	
	// Health and death state live on the server and are replicated to clients.
	SetIsReplicatedByDefault(true);
}

void UDamageSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UDamageSystemComponent, MaxHealth);
	DOREPLIFETIME(UDamageSystemComponent, CurrentHealth);
	DOREPLIFETIME(UDamageSystemComponent, IsDead);
}

void UDamageSystemComponent::OnRep_IsDead()
{
	if (IsDead)
	{
		OnDeath.Broadcast();
	}
}

void UDamageSystemComponent::MulticastDamageTaken_Implementation(const FDamageInfo& DamageInfo)
{
	// The server already broadcast this locally.
	if (GetOwner() && !GetOwner()->HasAuthority()) { OnDamageTaken.Broadcast(DamageInfo); }
}

void UDamageSystemComponent::MulticastDamageAvoided_Implementation(const FDamageInfo& DamageInfo)
{
	if (GetOwner() && !GetOwner()->HasAuthority()) { OnDamageAvoided.Broadcast(DamageInfo); }
}

void UDamageSystemComponent::MulticastDamageParried_Implementation(const FDamageInfo& DamageInfo)
{
	if (GetOwner() && !GetOwner()->HasAuthority()) { OnDamageParried.Broadcast(DamageInfo); }
}

void UDamageSystemComponent::MulticastHealReceived_Implementation(float HealAmount, AActor* Healer)
{
	if (GetOwner() && !GetOwner()->HasAuthority()) { OnHealReceived.Broadcast(HealAmount, Healer); }
}


// Called when the game starts
void UDamageSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// MaxHealth may have been changed in Blueprint/editor; always start at full health.
	// Only the server sets it, clients receive the value through replication.
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentHealth = MaxHealth;
	}
}

bool UDamageSystemComponent::HandleIncomingDamage(const FDamageInfo& DamageInfo)
{
	// Damage is server authoritative.
	if (!GetOwner() || !GetOwner()->HasAuthority()) { return false; }
	if (IsDead) { return false; }
	
	if (IsInvincible && !DamageInfo.ShouldDamageInvincible)
	{
		OnDamageAvoided.Broadcast(DamageInfo);
		MulticastDamageAvoided(DamageInfo);
		return false;
	}
	
	// A parry inside the parry window negates the hit entirely and lets the owner react (counter-attack, etc.).
	if (IsParrying && DamageInfo.CanBeParried && IsInBlockArc(DamageInfo))
	{
		OnDamageParried.Broadcast(DamageInfo);
		MulticastDamageParried(DamageInfo);
		return false;
	}
	
	if (IsBlocking && DamageInfo.CanBeBlocked && IsInBlockArc(DamageInfo))
	{
		OnDamageAvoided.Broadcast(DamageInfo);
		MulticastDamageAvoided(DamageInfo);
		return false;
	}
	
	const float FinalDamage = CalculateFinalDamage(DamageInfo);
	if (DamageInfo.DamageAmount > 0.0f && FinalDamage <= 0.0f)
	{
		// Fully absorbed by resistance/armor.
		OnDamageAvoided.Broadcast(DamageInfo);
		MulticastDamageAvoided(DamageInfo);
		return false;
	}
	
	CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.f, MaxHealth);
	
	// Listeners receive the damage that was actually applied.
	FDamageInfo AppliedDamage = DamageInfo;
	AppliedDamage.DamageAmount = FinalDamage;
	OnDamageTaken.Broadcast(AppliedDamage);
	MulticastDamageTaken(AppliedDamage);
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
	if (!GetOwner() || !GetOwner()->HasAuthority()) { return; }
	if (IsDead) { return; }
	
	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.f, MaxHealth);
	OnHealReceived.Broadcast(HealAmount, Healer);
	MulticastHealReceived(HealAmount, Healer);
}

void UDamageSystemComponent::SetIsBlocking(bool NewBlocking)
{
	IsBlocking = NewBlocking;
	if (GetOwner() && !GetOwner()->HasAuthority()) { ServerSetIsBlocking(NewBlocking); }
}

void UDamageSystemComponent::SetIsParrying(bool NewParrying)
{
	IsParrying = NewParrying;
	if (GetOwner() && !GetOwner()->HasAuthority()) { ServerSetIsParrying(NewParrying); }
}

void UDamageSystemComponent::ServerSetIsBlocking_Implementation(bool NewBlocking)
{
	IsBlocking = NewBlocking;
}

void UDamageSystemComponent::ServerSetIsParrying_Implementation(bool NewParrying)
{
	IsParrying = NewParrying;
}

void UDamageSystemComponent::SetStartingHealth(float StartingHealth)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) { return; }
	
	MaxHealth = StartingHealth;
	CurrentHealth = StartingHealth;
}

