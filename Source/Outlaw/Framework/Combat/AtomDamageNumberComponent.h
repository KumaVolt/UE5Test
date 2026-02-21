// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystemComponent.h"
#include "OutlawDamageNumberComponent.generated.h"

class UOutlawDamageNumberWidget;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawDamageNumberComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawDamageNumberComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	TSubclassOf<UOutlawDamageNumberWidget> DamageNumberWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Number")
	FVector SpawnOffset = FVector(0.f, 0.f, 100.f);

private:
	void OnDamageReceived(const FOnAttributeChangeData& Data);

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle IncomingDamageDelegateHandle;
	bool bWasCritical = false;
};
