// Automation tests for UDamageSystemComponent.
// Run in the editor: Tools > Session Frontend > Automation > "DamageSystemTemplate", or
// Automation RunTests DamageSystemTemplate from the console.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "DamageSystem/DamageSystemComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace DamageSystemTests
{
	// Builds a tiny standalone world with one actor (owner) that carries a UDamageSystemComponent
	// and tears everything down again. Standalone means the owner has authority.
	struct FTestContext
	{
		UWorld* World = nullptr;
		AActor* Owner = nullptr;
		UDamageSystemComponent* Damage = nullptr;

		explicit FTestContext(float StartingHealth = 100.0f)
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			World->AddToRoot();
			Owner = SpawnActorAt(FVector::ZeroVector);

			Damage = NewObject<UDamageSystemComponent>(Owner);
			Damage->RegisterComponent();
			Damage->SetStartingHealth(StartingHealth);
		}

		~FTestContext()
		{
			if (World)
			{
				World->RemoveFromRoot();
				World->DestroyWorld(false);
			}
		}

		// Spawns an actor with a root component so it has a location (used as damage causer).
		AActor* SpawnActorAt(const FVector& Location) const
		{
			AActor* Actor = World->SpawnActor<AActor>();
			USceneComponent* Root = NewObject<USceneComponent>(Actor, TEXT("Root"));
			Actor->SetRootComponent(Root);
			Root->RegisterComponent();
			Actor->SetActorLocation(Location);
			return Actor;
		}
	};

	FDamageInfo MakeDamage(float Amount, EDamageType Type = EDamageType::Physical)
	{
		FDamageInfo Info;
		Info.DamageAmount = Amount;
		Info.DamageType = Type;
		return Info;
	}
}

#define DAMAGE_TEST_FLAGS (EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageSystemTakesDamageTest, "DamageSystemTemplate.Component.TakesDamage", DAMAGE_TEST_FLAGS)
bool FDamageSystemTakesDamageTest::RunTest(const FString& Parameters)
{
	DamageSystemTests::FTestContext Ctx;

	TestTrue(TEXT("Damage is applied"), Ctx.Damage->HandleIncomingDamage(DamageSystemTests::MakeDamage(30.0f)));
	TestEqual(TEXT("Health after 30 damage"), Ctx.Damage->GetCurrentHealth(), 70.0f);
	TestFalse(TEXT("Still alive"), Ctx.Damage->GetIsDead());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageSystemDeathTest, "DamageSystemTemplate.Component.Death", DAMAGE_TEST_FLAGS)
bool FDamageSystemDeathTest::RunTest(const FString& Parameters)
{
	DamageSystemTests::FTestContext Ctx;

	Ctx.Damage->HandleIncomingDamage(DamageSystemTests::MakeDamage(500.0f));
	TestEqual(TEXT("Health is clamped to 0"), Ctx.Damage->GetCurrentHealth(), 0.0f);
	TestTrue(TEXT("Dead"), Ctx.Damage->GetIsDead());

	// A dead character takes no further damage and cannot be healed.
	TestFalse(TEXT("No damage when dead"), Ctx.Damage->HandleIncomingDamage(DamageSystemTests::MakeDamage(10.0f)));
	Ctx.Damage->HandleIncomingHeal(50.0f, nullptr);
	TestEqual(TEXT("No healing when dead"), Ctx.Damage->GetCurrentHealth(), 0.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageSystemHealTest, "DamageSystemTemplate.Component.Heal", DAMAGE_TEST_FLAGS)
bool FDamageSystemHealTest::RunTest(const FString& Parameters)
{
	DamageSystemTests::FTestContext Ctx;

	Ctx.Damage->HandleIncomingDamage(DamageSystemTests::MakeDamage(60.0f));
	Ctx.Damage->HandleIncomingHeal(20.0f, nullptr);
	TestEqual(TEXT("Health after heal"), Ctx.Damage->GetCurrentHealth(), 60.0f);

	Ctx.Damage->HandleIncomingHeal(1000.0f, nullptr);
	TestEqual(TEXT("Heal is clamped to MaxHealth"), Ctx.Damage->GetCurrentHealth(), 100.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageSystemInvincibleTest, "DamageSystemTemplate.Component.Invincible", DAMAGE_TEST_FLAGS)
bool FDamageSystemInvincibleTest::RunTest(const FString& Parameters)
{
	DamageSystemTests::FTestContext Ctx;
	Ctx.Damage->SetIsInvincible(true);

	TestFalse(TEXT("Invincible avoids damage"), Ctx.Damage->HandleIncomingDamage(DamageSystemTests::MakeDamage(40.0f)));
	TestEqual(TEXT("Health untouched"), Ctx.Damage->GetCurrentHealth(), 100.0f);

	FDamageInfo Piercing = DamageSystemTests::MakeDamage(40.0f);
	Piercing.ShouldDamageInvincible = true;
	TestTrue(TEXT("ShouldDamageInvincible pierces"), Ctx.Damage->HandleIncomingDamage(Piercing));
	TestEqual(TEXT("Health after piercing damage"), Ctx.Damage->GetCurrentHealth(), 60.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageSystemResistanceTest, "DamageSystemTemplate.Component.ResistanceAndArmor", DAMAGE_TEST_FLAGS)
bool FDamageSystemResistanceTest::RunTest(const FString& Parameters)
{
	DamageSystemTests::FTestContext Ctx;
	Ctx.Damage->Resistances.Add(EDamageType::Magical, 0.5f);
	Ctx.Damage->Armor = 5.0f;

	TestEqual(TEXT("50% magic resistance"), Ctx.Damage->CalculateFinalDamage(DamageSystemTests::MakeDamage(40.0f, EDamageType::Magical)), 20.0f);
	TestEqual(TEXT("Armor reduces physical damage"), Ctx.Damage->CalculateFinalDamage(DamageSystemTests::MakeDamage(30.0f, EDamageType::Physical)), 25.0f);
	TestEqual(TEXT("Armor ignores environment damage"), Ctx.Damage->CalculateFinalDamage(DamageSystemTests::MakeDamage(30.0f, EDamageType::Environment)), 30.0f);
	TestEqual(TEXT("Damage never goes below zero"), Ctx.Damage->CalculateFinalDamage(DamageSystemTests::MakeDamage(3.0f, EDamageType::Physical)), 0.0f);

	// A hit that is fully absorbed counts as avoided and costs no health.
	TestFalse(TEXT("Fully absorbed hit"), Ctx.Damage->HandleIncomingDamage(DamageSystemTests::MakeDamage(3.0f, EDamageType::Physical)));
	TestEqual(TEXT("Health untouched"), Ctx.Damage->GetCurrentHealth(), 100.0f);

	TestTrue(TEXT("Magic hit is applied"), Ctx.Damage->HandleIncomingDamage(DamageSystemTests::MakeDamage(40.0f, EDamageType::Magical)));
	TestEqual(TEXT("Health after resisted magic hit"), Ctx.Damage->GetCurrentHealth(), 80.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageSystemBlockAngleTest, "DamageSystemTemplate.Component.BlockAngle", DAMAGE_TEST_FLAGS)
bool FDamageSystemBlockAngleTest::RunTest(const FString& Parameters)
{
	DamageSystemTests::FTestContext Ctx;
	Ctx.Damage->SetIsBlocking(true);

	// The owner stands at the origin facing +X.
	AActor* InFront = Ctx.SpawnActorAt(FVector(300.0f, 0.0f, 0.0f));
	AActor* Behind = Ctx.SpawnActorAt(FVector(-300.0f, 0.0f, 0.0f));

	FDamageInfo FrontHit = DamageSystemTests::MakeDamage(20.0f);
	FrontHit.DamageCauser = InFront;
	TestFalse(TEXT("Hit from the front is blocked"), Ctx.Damage->HandleIncomingDamage(FrontHit));
	TestEqual(TEXT("No damage from blocked hit"), Ctx.Damage->GetCurrentHealth(), 100.0f);

	FDamageInfo BackHit = DamageSystemTests::MakeDamage(20.0f);
	BackHit.DamageCauser = Behind;
	TestTrue(TEXT("Hit from behind is not blocked"), Ctx.Damage->HandleIncomingDamage(BackHit));
	TestEqual(TEXT("Damage from unblocked hit"), Ctx.Damage->GetCurrentHealth(), 80.0f);

	FDamageInfo Unblockable = DamageSystemTests::MakeDamage(20.0f);
	Unblockable.DamageCauser = InFront;
	Unblockable.CanBeBlocked = false;
	TestTrue(TEXT("CanBeBlocked = false goes through"), Ctx.Damage->HandleIncomingDamage(Unblockable));
	TestEqual(TEXT("Damage from unblockable hit"), Ctx.Damage->GetCurrentHealth(), 60.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageSystemParryTest, "DamageSystemTemplate.Component.Parry", DAMAGE_TEST_FLAGS)
bool FDamageSystemParryTest::RunTest(const FString& Parameters)
{
	DamageSystemTests::FTestContext Ctx;
	Ctx.Damage->SetIsParrying(true);

	FDamageInfo Parryable = DamageSystemTests::MakeDamage(25.0f);
	Parryable.CanBeParried = true;
	TestFalse(TEXT("Parryable hit is negated"), Ctx.Damage->HandleIncomingDamage(Parryable));
	TestEqual(TEXT("No damage from parried hit"), Ctx.Damage->GetCurrentHealth(), 100.0f);

	FDamageInfo Unparryable = DamageSystemTests::MakeDamage(25.0f);
	Unparryable.CanBeParried = false;
	TestTrue(TEXT("Unparryable hit goes through"), Ctx.Damage->HandleIncomingDamage(Unparryable));
	TestEqual(TEXT("Damage from unparryable hit"), Ctx.Damage->GetCurrentHealth(), 75.0f);
	return true;
}

#undef DAMAGE_TEST_FLAGS

#endif // WITH_DEV_AUTOMATION_TESTS
