// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomLootPickup.h"
#include "AtomLootBeamComponent.h"
#include "Inventory/AtomInventoryComponent.h"
#include "Inventory/AtomItemDefinition.h"
#include "Inventory/AtomItemInstance.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogAtomLootPickup, Log, All);

AAtomLootPickup::AAtomLootPickup(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->SetSphereRadius(100.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LootBeamComponent = CreateDefaultSubobject<UAtomLootBeamComponent>(TEXT("LootBeamComponent"));
	LootBeamComponent->SetupAttachment(RootComponent);
}

void AAtomLootPickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAtomLootPickup::OnOverlapBegin);
	}

	if (LootDrop.ItemDefinition)
	{
		LootBeamComponent->InitForRarity(LootDrop.RolledRarity);
	}
}

void AAtomLootPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAtomLootPickup, LootDrop);
}

void AAtomLootPickup::InitializeLoot(const FAtomLootDrop& Drop)
{
	LootDrop = Drop;

	if (LootBeamComponent && LootDrop.ItemDefinition)
	{
		LootBeamComponent->InitForRarity(LootDrop.RolledRarity);
	}
}

void AAtomLootPickup::OnRep_LootDrop()
{
	if (LootBeamComponent && LootDrop.ItemDefinition)
	{
		LootBeamComponent->InitForRarity(LootDrop.RolledRarity);
	}
}

void AAtomLootPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !LootDrop.ItemDefinition)
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	if (bAutoLoot && CollisionSphere->GetScaledSphereRadius() <= AutoLootRadius)
	{
		AttemptPickup(Pawn);
	}
}

void AAtomLootPickup::AttemptPickup(AActor* PickupActor)
{
	if (!HasAuthority() || !LootDrop.ItemDefinition || !PickupActor)
	{
		return;
	}

	UAtomInventoryComponent* InventoryComp = PickupActor->FindComponentByClass<UAtomInventoryComponent>();
	if (!InventoryComp)
	{
		return;
	}

	int32 AddedCount = InventoryComp->AddItem(const_cast<UAtomItemDefinition*>(LootDrop.ItemDefinition.Get()), LootDrop.Quantity);

	if (AddedCount > 0)
	{
		// Auto-equip into empty equipment slot if applicable
		const UAtomItemDefinition* ItemDef = LootDrop.ItemDefinition.Get();
		if (ItemDef && ItemDef->bCanBeEquipped && ItemDef->EquipmentSlotTag.IsValid())
		{
			if (!InventoryComp->IsSlotOccupied(ItemDef->EquipmentSlotTag))
			{
				TArray<FAtomInventoryEntry> Matching = InventoryComp->FindItemsForSlot(ItemDef->EquipmentSlotTag);
				for (const auto& Entry : Matching)
				{
					if (Entry.ItemDef == ItemDef)
					{
						InventoryComp->EquipItem(Entry.InstanceId);
						break;
					}
				}
			}
		}

		OnLootPickedUp.Broadcast(LootDrop.ItemDefinition, AddedCount, this);
		Destroy();
	}
	else
	{
		UE_LOG(LogAtomLootPickup, Warning, TEXT("AttemptPickup: Inventory full, cannot pick up %s"), *LootDrop.ItemDefinition->DisplayName.ToString());
	}
}
