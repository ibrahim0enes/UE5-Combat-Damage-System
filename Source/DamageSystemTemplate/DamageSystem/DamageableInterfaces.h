// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamageSystemTypes.h"
#include "DamageableInterfaces.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UDamageableInterfaces : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class DAMAGESYSTEMTEMPLATE_API IDamageableInterfaces
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(Blueprintable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	float GetCurrentHealth();
	
	UFUNCTION(Blueprintable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	float GetMaxHealth();
	
	UFUNCTION(Blueprintable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	bool GetIsDead();
	
	UFUNCTION(Blueprintable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	void Healt(float HealAmount, AActor* Healer);

	UFUNCTION(Blueprintable, BlueprintNativeEvent, Category = "Damageable Interfaces")
	bool TakeDamage(const FDamageInfo& DamageInfo);
};
