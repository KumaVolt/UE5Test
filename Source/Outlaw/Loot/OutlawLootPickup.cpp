// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawLootPickup.h"
#include "OutlawLootBeamComponent.h"
#include "Inventory/OutlawInventoryComponent.h"
#include "Inventory/OutlawItemDefinition.h"
#include "Inventory/OutlawItemInstance.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlawLootPickup, Log, All);

AOutlawLootPickup::AOutlawLootPickup(const FObjectInitializer& ObjectInitializer)
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

	LootBeamComponent = CreateDefaultSubobject<UOutlawLootBeamComponent>(TEXT("LootBeamComponent"));
	LootBeamComponent->SetupAttachment(RootComponent);
}

void AOutlawLootPickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AOutlawLootPickup::OnOverlapBegin);
	}

	if (LootDrop.ItemDefinition)
	{
		LootBeamComponent->InitForRarity(LootDrop.RolledRarity);
	}
}

void AOutlawLootPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOutlawLootPickup, LootDrop);
}

void AOutlawLootPickup::InitializeLoot(const FOutlawLootDrop& Drop)
{
	LootDrop = Drop;

	if (LootBeamComponent && LootDrop.ItemDefinition)
	{
		LootBeamComponent->InitForRarity(LootDrop.RolledRarity);
	}
}

void AOutlawLootPickup::OnRep_LootDrop()
{
	if (LootBeamComponent && LootDrop.ItemDefinition)
	{
		LootBeamComponent->InitForRarity(LootDrop.RolledRarity);
	}
}

void AOutlawLootPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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

void AOutlawLootPickup::AttemptPickup(AActor* PickupActor)
{
	if (!HasAuthority() || !LootDrop.ItemDefinition || !PickupActor)
	{
		return;
	}

	UOutlawInventoryComponent* InventoryComp = PickupActor->FindComponentByClass<UOutlawInventoryComponent>();
	if (!InventoryComp)
	{
		return;
	}

	int32 AddedCount = InventoryComp->AddItem(const_cast<UOutlawItemDefinition*>(LootDrop.ItemDefinition.Get()), LootDrop.Quantity);

	if (AddedCount > 0)
	{
		// TODO: Auto-roll affixes on weapon pickup
		// Requires inventory API enhancement to return instance ID from AddItem
		// For now, affixes can be rolled manually via Blueprint or in a future crafting system

		OnLootPickedUp.Broadcast(LootDrop.ItemDefinition, AddedCount, this);
		Destroy();
	}
	else
	{
		UE_LOG(LogOutlawLootPickup, Warning, TEXT("AttemptPickup: Inventory full, cannot pick up %s"), *LootDrop.ItemDefinition->DisplayName.ToString());
	}
}
