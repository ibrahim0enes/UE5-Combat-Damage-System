// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageSystemTypes.h"
#include "DamageSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageTaken, const FDamageInfo& , DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageAvoided, const FDamageInfo& , DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageParried, const FDamageInfo& , DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealReceived, float, HealAmount, AActor*, Healer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
struct FDamageInfo;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DAMAGESYSTEMTEMPLATE_API UDamageSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UDamageSystemComponent();
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	float MaxHealth = 100.0f;

	// Flat damage reduction, applied to Physical damage after resistance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense", meta = (ClampMin = "0.0"))
	float Armor = 0.0f;

	// Resistance per damage type. 0 = no resistance, 0.5 = takes half damage, 1 = immune, negative = weakness.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense")
	TMap<EDamageType, float> Resistances;

	// Blocking only works against attackers inside this cone in front of the owner (degrees from forward).
	// 90 = the whole front half, 180 = every direction.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float BlockHalfAngle = 90.0f;
	
private:
	
	// Synced with MaxHealth in BeginPlay, so changing MaxHealth in Blueprint/editor still starts at full health
	UPROPERTY(Replicated)
	float CurrentHealth = 0.0f;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool IsDead = false;
	
	UPROPERTY()
	bool IsBlocking =false;
	
	UPROPERTY()
	bool IsInvincible = false;
	
	UPROPERTY()
	bool IsParrying = false;
	
	// True if the attacker is inside the owner's block cone (or if the direction cannot be determined).
	bool IsInBlockArc(const FDamageInfo& DamageInfo) const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Clients learn about death through replication of IsDead (reliable, also works for late joiners).
	UFUNCTION()
	void OnRep_IsDead();
	
	// Cosmetic events are broadcast on remote clients through these (the server broadcasts locally).
	UFUNCTION(Server, Reliable)
	void ServerSetIsBlocking(bool NewBlocking);
	
	UFUNCTION(Server, Reliable)
	void ServerSetIsParrying(bool NewParrying);
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDamageTaken(const FDamageInfo& DamageInfo);
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDamageAvoided(const FDamageInfo& DamageInfo);
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDamageParried(const FDamageInfo& DamageInfo);
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHealReceived(float HealAmount, AActor* Healer);

	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Damage")
	bool HandleIncomingDamage(const FDamageInfo& DamageInfo);
	
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void HandleIncomingHeal(float HealAmount, AActor* Healer);

	// Damage that would actually be applied after resistance and armor: max(0, Amount * (1 - Resistance) - Armor).
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Damage")
	float CalculateFinalDamage(const FDamageInfo& DamageInfo) const;
	
	// GETTER FUNCTIONS //
	UFUNCTION(BlueprintCallable, BlueprintPure , Category = "Health")
	float GetCurrentHealth() { return CurrentHealth; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure , Category = "Health")
	float GetMaxHealth() { return MaxHealth; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure , Category = "Health")
	bool GetIsDead() { return IsDead; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure , Category = "States")
	bool GetIsInvincible() { return IsInvincible; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure , Category = "States")
	bool GetIsBlocking() { return IsBlocking; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure , Category = "States")
	bool GetIsParrying() { return IsParrying; }
	
	// SETTER FUNCTIONS //
	UFUNCTION(BlueprintCallable, Category = "States")
	void SetIsInvincible(bool NewInvincible) {IsInvincible = NewInvincible; }
	
	// Blocking/parrying are set locally for responsiveness and forwarded to the server, which decides damage.
	UFUNCTION(BlueprintCallable, Category = "States")
	void SetIsBlocking(bool NewBlocking);
	
	// Open the parry window (e.g. from an AnimNotifyState); attacks with CanBeParried are then negated.
	UFUNCTION(BlueprintCallable, Category = "States")
	void SetIsParrying(bool NewParrying);
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetStartingHealth(float StartingHealth);
	
	
	// DELEGATES // 
	UPROPERTY(BlueprintAssignable, Category = "Damage Delegates")
	FOnDamageTaken OnDamageTaken;
	
	UPROPERTY(BlueprintAssignable, Category = "Damage Delegates")
	FOnDamageAvoided OnDamageAvoided;
	
	UPROPERTY(BlueprintAssignable, Category = "Damage Delegates")
	FOnDamageParried OnDamageParried;
	
	UPROPERTY(BlueprintAssignable, Category = "Damage Delegates")
	FOnDeath OnDeath;
	
	UPROPERTY(BlueprintAssignable, Category = "Damage Delegates")
	FOnHealReceived OnHealReceived;
	
	
};
