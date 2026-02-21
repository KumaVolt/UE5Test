// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomEnemyDeathHandler.h"
#include "AtomDeathComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Loot/AtomLootSubsystem.h"
#include "Loot/AtomLootTable.h"
#include "Progression/AtomProgressionComponent.h"

UAtomEnemyDeathHandler::UAtomEnemyDeathHandler(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAtomEnemyDeathHandler::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (UAtomDeathComponent* DeathComp = Owner->FindComponentByClass<UAtomDeathComponent>())
	{
		DeathComp->OnDeathStarted.AddDynamic(this, &UAtomEnemyDeathHandler::OnDeathStarted);
	}
}

void UAtomEnemyDeathHandler::OnDeathStarted(AActor* Killer)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	KillerActor = Killer;
	DeathLocation = Owner->GetActorLocation();

	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		// Try skeletal mesh ragdoll first
		bool bHasSkeletalRagdoll = false;
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			if (Mesh->GetSkeletalMeshAsset())
			{
				Mesh->SetSimulatePhysics(true);
				Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				if (DeathImpulse.SizeSquared() > 0.f)
				{
					Mesh->AddImpulseAtLocation(DeathImpulse, DeathLocation);
				}
				bHasSkeletalRagdoll = true;
			}
		}

		// Fallback: tumble the static body mesh for graybox characters
		if (!bHasSkeletalRagdoll)
		{
			if (UStaticMeshComponent* BodyMesh = Character->FindComponentByClass<UStaticMeshComponent>())
			{
				BodyMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
				BodyMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				BodyMesh->SetCollisionResponseToAllChannels(ECR_Block);
				BodyMesh->SetSimulatePhysics(true);
				if (DeathImpulse.SizeSquared() > 0.f)
				{
					BodyMesh->AddImpulse(DeathImpulse, NAME_None, true);
				}
			}
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		RagdollTimerHandle,
		this,
		&UAtomEnemyDeathHandler::StartDissolve,
		RagdollDuration,
		false
	);
}

void UAtomEnemyDeathHandler::StartDissolve()
{
	GetWorld()->GetTimerManager().SetTimer(
		DissolveTimerHandle,
		this,
		&UAtomEnemyDeathHandler::SpawnLootAndDestroy,
		DissolveDuration,
		false
	);
}

void UAtomEnemyDeathHandler::SpawnLootAndDestroy()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (KillerActor.IsValid())
	{
		if (UAtomProgressionComponent* ProgressionComp = KillerActor->FindComponentByClass<UAtomProgressionComponent>())
		{
			ProgressionComp->AwardXP(BaseXPReward);
			OnXPAwarded.Broadcast(BaseXPReward);
		}
	}

	if (LootTable)
	{
		if (UAtomLootSubsystem* LootSubsystem = World->GetSubsystem<UAtomLootSubsystem>())
		{
			LootSubsystem->SpawnLoot(DeathLocation, LootTable, EnemyLevel, RarityBonus, NumLootDrops);
			OnLootDropRequested.Broadcast(DeathLocation, EnemyLevel);
		}
	}

	Owner->Destroy();
}
