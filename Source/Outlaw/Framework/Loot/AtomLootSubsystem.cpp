// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomLootSubsystem.h"
#include "AtomLootTable.h"
#include "AtomLootPickup.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomLootSubsystem, Log, All);

UAtomLootSubsystem::UAtomLootSubsystem()
{
}

void UAtomLootSubsystem::SpawnLoot(const FVector& DeathLocation, UAtomLootTable* LootTable, int32 EnemyLevel, float RarityBonus, int32 NumDrops)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetAuthGameMode() || !LootTable || !LootPickupClass)
	{
		return;
	}

	TArray<FAtomLootDrop> Drops = LootTable->RollLoot(EnemyLevel, NumDrops, RarityBonus);

	if (Drops.Num() == 0)
	{
		return;
	}

	const float AngleIncrement = 360.0f / FMath::Max(Drops.Num(), 1);
	
	for (int32 Index = 0; Index < Drops.Num(); ++Index)
	{
		const FAtomLootDrop& Drop = Drops[Index];
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

void UAtomLootSubsystem::SpawnSinglePickup(const FAtomLootDrop& Drop, const FVector& SpawnLocation)
{
	UWorld* World = GetWorld();
	if (!World || !LootPickupClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AAtomLootPickup* Pickup = World->SpawnActor<AAtomLootPickup>(LootPickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (Pickup)
	{
		Pickup->InitializeLoot(Drop);
		Pickup->bAutoLoot = bAutoLootPickups;
	}
}
