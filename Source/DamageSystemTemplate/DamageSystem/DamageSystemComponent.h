// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageSystemTypes.h"
#include "DamageSystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageTaken, const FDamageInfo& , DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageAvoided, const FDamageInfo& , DamageInfo);
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
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;
	
private:
	
	// Synced with MaxHealth in BeginPlay, so changing MaxHealth in Blueprint/editor still starts at full health
	UPROPERTY()
	float CurrentHealth = 0.0f;
	
	UPROPERTY()
	bool IsDead = false;
	
	UPROPERTY()
	bool IsBlocking =false;
	
	UPROPERTY()
	bool IsInvincible = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Damage")
	bool HandleIncomingDamage(const FDamageInfo& DamageInfo);
	
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void HandleIncomingHeal(float HealAmount, AActor* Healer);
	
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
	
	// SETTER FUNCTIONS //
	UFUNCTION(BlueprintCallable, Category = "States")
	void SetIsInvincible(bool NewInvincible) {IsInvincible = NewInvincible; }
	
	UFUNCTION(BlueprintCallable, Category = "States")
	void SetIsBlocking(bool NewBlocking) {IsBlocking = NewBlocking; }
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetStartingHealth(float StartingHealth);
	
	
	// DELEGATES // 
	UPROPERTY(BlueprintAssignable, Category = "Damage Delegates")
	FOnDamageTaken OnDamageTaken;
	
	UPROPERTY(BlueprintAssignable, Category = "Damage Delegates")
	FOnDamageAvoided OnDamageAvoided;
	
	UPROPERTY(BlueprintAssignable, Category = "Damage Delegates")
	FOnDeath OnDeath;
	
	UPROPERTY(BlueprintAssignable, Category = "Damage Delegates")
	FOnHealReceived OnHealReceived;
	
	
};
