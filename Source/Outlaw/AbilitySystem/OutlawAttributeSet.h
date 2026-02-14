// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "OutlawAttributeSet.generated.h"

UCLASS()
class OUTLAW_API UOutlawAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public: 
	UOutlawAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing= OnRep_Health, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawAttributeSet, Health);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing= OnRep_MaxHealth, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawAttributeSet, MaxHealth);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing= OnRep_Stamina, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawAttributeSet, Stamina);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing= OnRep_MaxStamina, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawAttributeSet, MaxStamina);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing= OnRep_Strength, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawAttributeSet, Strength);
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing= OnRep_MaxStrength, Category = "Ability | Gameplay Attribute")
	FGameplayAttributeData MaxStrength;
	ATTRIBUTE_ACCESSORS_BASIC(UOutlawAttributeSet, MaxStrength);
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
	
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;
	
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;
	
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;
	
	UFUNCTION()
	void OnRep_MaxStrength(const FGameplayAttributeData& OldMaxStrength) const;
};
