// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/OutlawAnimationTypes.h"
#include "OutlawHitReactionComponent.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;
struct FGameplayTag;
struct FOnAttributeChangeData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UOutlawHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOutlawHitReactionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outlaw|HitReaction")
	TArray<FOutlawHitReactionConfig> HitReactionMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outlaw|HitReaction")
	float LightHitThresholdPercent = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outlaw|HitReaction")
	float MediumHitThresholdPercent = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outlaw|HitReaction")
	bool bCanBeStaggered = true;

	UFUNCTION(BlueprintCallable, Category = "Outlaw|HitReaction")
	void PlayHitReaction(float DamageAmount, AActor* DamageSource);

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	EOutlawHitReactionType DetermineReactionType(float DamageAmount, float MaxHealth) const;
	EOutlawHitDirection DetermineHitDirection(AActor* DamageSource) const;
	UAnimMontage* FindHitReactionMontage(EOutlawHitReactionType ReactionType, EOutlawHitDirection Direction) const;

private:
	void OnIncomingDamageChanged(const FOnAttributeChangeData& Data);

	FDelegateHandle IncomingDamageHandle;
};
