// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AtomAnimationTypes.h"
#include "AtomHitReactionComponent.generated.h"

class UAbilitySystemComponent;
class UAnimMontage;
struct FGameplayTag;
struct FOnAttributeChangeData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OUTLAW_API UAtomHitReactionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtomHitReactionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atom|HitReaction")
	TArray<FAtomHitReactionConfig> HitReactionMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atom|HitReaction")
	float LightHitThresholdPercent = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atom|HitReaction")
	float MediumHitThresholdPercent = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Atom|HitReaction")
	bool bCanBeStaggered = true;

	UFUNCTION(BlueprintCallable, Category = "Atom|HitReaction")
	void PlayHitReaction(float DamageAmount, AActor* DamageSource);

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	EAtomHitReactionType DetermineReactionType(float DamageAmount, float MaxHealth) const;
	EAtomHitDirection DetermineHitDirection(AActor* DamageSource) const;
	UAnimMontage* FindHitReactionMontage(EAtomHitReactionType ReactionType, EAtomHitDirection Direction) const;

private:
	void OnIncomingDamageChanged(const FOnAttributeChangeData& Data);

	FDelegateHandle IncomingDamageHandle;
};
