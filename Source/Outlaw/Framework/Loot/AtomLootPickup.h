// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AtomLootTypes.h"
#include "AtomLootPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UAtomLootBeamComponent;
class UAtomItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLootPickedUp, const UAtomItemDefinition*, ItemDef, int32, Quantity, AActor*, PickupActor);

UCLASS()
class OUTLAW_API AAtomLootPickup : public AActor
{
	GENERATED_BODY()

public:
	AAtomLootPickup(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAtomLootBeamComponent> LootBeamComponent;

	UPROPERTY(ReplicatedUsing = OnRep_LootDrop, BlueprintReadOnly, Category = "Loot")
	FAtomLootDrop LootDrop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Config")
	bool bAutoLoot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Config", meta = (EditCondition = "bAutoLoot"))
	float AutoLootRadius = 200.0f;

	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnLootPickedUp OnLootPickedUp;

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void InitializeLoot(const FAtomLootDrop& Drop);

	UFUNCTION()
	void OnRep_LootDrop();

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void AttemptPickup(AActor* PickupActor);
};
