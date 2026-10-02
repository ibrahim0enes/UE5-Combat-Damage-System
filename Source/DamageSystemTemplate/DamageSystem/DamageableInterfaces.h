// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageSystemTypes.h"
#include "DamageableInterfaces.generated.h"

class AActor;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UDamageableInterfaces : public UInterface
{
	GENERATED_BODY()
};

/**
 * Damageable interface: health, healing and damage handling.
 */
class DAMAGESYSTEMTEMPLATE_API IDamageableInterfaces
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	float GetCurrentHealth();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	float GetMaxHealth();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	bool GetIsDead();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	void Heal(float HealAmount, AActor* Healer);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	bool TakeDamage(const FDamageInfo& DamageInfo);
};