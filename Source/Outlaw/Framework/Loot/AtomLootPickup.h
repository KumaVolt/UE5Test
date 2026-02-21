// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OutlawLootTypes.h"
#include "OutlawLootPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UOutlawLootBeamComponent;
class UOutlawItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLootPickedUp, const UOutlawItemDefinition*, ItemDef, int32, Quantity, AActor*, PickupActor);

UCLASS()
class OUTLAW_API AOutlawLootPickup : public AActor
{
	GENERATED_BODY()

public:
	AOutlawLootPickup(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UOutlawLootBeamComponent> LootBeamComponent;

	UPROPERTY(ReplicatedUsing = OnRep_LootDrop, BlueprintReadOnly, Category = "Loot")
	FOutlawLootDrop LootDrop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Config")
	bool bAutoLoot = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot|Config", meta = (EditCondition = "bAutoLoot"))
	float AutoLootRadius = 200.0f;

	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnLootPickedUp OnLootPickedUp;

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void InitializeLoot(const FOutlawLootDrop& Drop);

	UFUNCTION()
	void OnRep_LootDrop();

private:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void AttemptPickup(AActor* PickupActor);
};
