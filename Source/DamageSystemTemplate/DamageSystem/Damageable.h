// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageSystemTypes.h"
#include "Damageable.generated.h"

class AActor;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Damageable interface: health, healing and damage handling.
 */
class DAMAGESYSTEMTEMPLATE_API IDamageable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable")
	float GetCurrentHealth();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable")
	float GetMaxHealth();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable")
	bool GetIsDead();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable")
	void Heal(float HealAmount, AActor* Healer);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable")
	bool ReceiveDamage(const FDamageInfo& DamageInfo);
};