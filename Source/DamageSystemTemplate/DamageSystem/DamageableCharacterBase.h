// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DamageableInterfaces.h"
#include "GameFramework/Character.h"
#include "DamageableCharacterBase.generated.h"

class UDamageSystemComponent;
class UAnimMontage;

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
	
	UFUNCTION(BlueprintNativeEvent)
	void RespondToDamageTaken(const FDamageInfo& DamageInfo);
	
	UFUNCTION(BlueprintNativeEvent)
	void RespondTDamageAvoided(const FDamageInfo& DamageInfo);
	
	UFUNCTION(BlueprintNativeEvent)
	void RespondToHealRecieved(float HealAmount, AActor* Healer);
	
	UFUNCTION(BlueprintNativeEvent)
	void RespondToDeath();

	// Optional montages played for the matching EDamageResponse.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Response")
	TObjectPtr<UAnimMontage> HitReactionMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Response")
	TObjectPtr<UAnimMontage> StaggerMontage;

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

private:
	FTimerHandle StunTimerHandle;

	void PlayResponseMontage(UAnimMontage* Montage, bool bForceInterrupt);
	void ApplyKnockback(const FDamageInfo& DamageInfo);
	void ApplyStun(float Duration);
	void EndStun();
};
