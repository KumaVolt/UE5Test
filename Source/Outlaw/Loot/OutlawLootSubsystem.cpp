// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawLootSubsystem.h"
#include "OutlawLootTable.h"
#include "OutlawLootPickup.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlawLootSubsystem, Log, All);

UOutlawLootSubsystem::UOutlawLootSubsystem()
{
}

void UOutlawLootSubsystem::SpawnLoot(const FVector& DeathLocation, UOutlawLootTable* LootTable, int32 EnemyLevel, float RarityBonus, int32 NumDrops)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetAuthGameMode() || !LootTable || !LootPickupClass)
	{
		return;
	}

	TArray<FOutlawLootDrop> Drops = LootTable->RollLoot(EnemyLevel, NumDrops, RarityBonus);

	if (Drops.Num() == 0)
	{
		return;
	}

	const float AngleIncrement = 360.0f / FMath::Max(Drops.Num(), 1);
	
	for (int32 Index = 0; Index < Drops.Num(); ++Index)
	{
		const FOutlawLootDrop& Drop = Drops[Index];
		if (!Drop.ItemDefinition)
		{
			continue;
		}

		float Angle = AngleIncrement * Index;
		float RadialOffset = ScatterRadius * FMath::FRand();
		FVector Offset = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * RadialOffset,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * RadialOffset,
			DropHeight
		);

		FVector SpawnLocation = DeathLocation + Offset;
		SpawnSinglePickup(Drop, SpawnLocation);
	}
}

void UOutlawLootSubsystem::SpawnSinglePickup(const FOutlawLootDrop& Drop, const FVector& SpawnLocation)
{
	UWorld* World = GetWorld();
	if (!World || !LootPickupClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AOutlawLootPickup* Pickup = World->SpawnActor<AOutlawLootPickup>(LootPickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (Pickup)
	{
		Pickup->InitializeLoot(Drop);
		Pickup->bAutoLoot = bAutoLootPickups;
	}
}
