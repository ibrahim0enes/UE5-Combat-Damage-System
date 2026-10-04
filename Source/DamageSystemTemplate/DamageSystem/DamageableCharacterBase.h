// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterfaces.h"
#include "GameFramework/Character.h"
#include "DamageableCharacterBase.generated.h"

class UDamageSystemComponent;

UCLASS()
class DAMAGESYSTEMTEMPLATE_API ADamageableCharacterBase : public ACharacter, public IDamageableInterfaces
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ADamageableCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void RespondToDamageTaken(const FDamageInfo& DamageInfo);
	
	UFUNCTION()
	void RespondTDamageAvoided(const FDamageInfo& DamageInfo);
	
	UFUNCTION()
	void RespondToHealRecieved(float HealAmount, AActor* Healer);
	
	UFUNCTION()
	void RespondToDeath();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// Damageable Interface Implementations
	virtual float GetMaxHealth_Implementation() override;
	virtual float GetCurrentHealth_Implementation() override;
	virtual bool GetIsDead_Implementation() override;
	virtual  void Heal_Implementation(float HealAmount, AActor* Healer) override;
	virtual bool TakeDamage_Implementation(const FDamageInfo& DamageInfo) override;
	
	// Damage System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDamageSystemComponent> DamageSystemComponent;
};
