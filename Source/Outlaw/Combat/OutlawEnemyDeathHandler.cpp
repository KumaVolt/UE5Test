// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawEnemyDeathHandler.h"
#include "OutlawDeathComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Loot/OutlawLootSubsystem.h"
#include "Loot/OutlawLootTable.h"
#include "Progression/OutlawProgressionComponent.h"

UOutlawEnemyDeathHandler::UOutlawEnemyDeathHandler(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOutlawEnemyDeathHandler::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (UOutlawDeathComponent* DeathComp = Owner->FindComponentByClass<UOutlawDeathComponent>())
	{
		DeathComp->OnDeathStarted.AddDynamic(this, &UOutlawEnemyDeathHandler::OnDeathStarted);
	}
}

void UOutlawEnemyDeathHandler::OnDeathStarted(AActor* Killer)
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
		&UOutlawEnemyDeathHandler::StartDissolve,
		RagdollDuration,
		false
	);
}

void UOutlawEnemyDeathHandler::StartDissolve()
{
	GetWorld()->GetTimerManager().SetTimer(
		DissolveTimerHandle,
		this,
		&UOutlawEnemyDeathHandler::SpawnLootAndDestroy,
		DissolveDuration,
		false
	);
}

void UOutlawEnemyDeathHandler::SpawnLootAndDestroy()
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
		if (UOutlawProgressionComponent* ProgressionComp = KillerActor->FindComponentByClass<UOutlawProgressionComponent>())
		{
			ProgressionComp->AwardXP(BaseXPReward);
			OnXPAwarded.Broadcast(BaseXPReward);
		}
	}

	if (LootTable)
	{
		if (UOutlawLootSubsystem* LootSubsystem = World->GetSubsystem<UOutlawLootSubsystem>())
		{
			LootSubsystem->SpawnLoot(DeathLocation, LootTable, EnemyLevel, RarityBonus, NumLootDrops);
			OnLootDropRequested.Broadcast(DeathLocation, EnemyLevel);
		}
	}

	Owner->Destroy();
}
