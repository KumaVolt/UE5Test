// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/OutlawHitReactionComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/OutlawAttributeSet.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffectTypes.h"
#include "Kismet/KismetMathLibrary.h"

UOutlawHitReactionComponent::UOutlawHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOutlawHitReactionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		AbilitySystemComponent = ASI->GetAbilitySystemComponent();
	}
	else if (APawn* Pawn = Cast<APawn>(Owner))
	{
		if (APlayerState* PS = Pawn->GetPlayerState())
		{
			if (IAbilitySystemInterface* PSI = Cast<IAbilitySystemInterface>(PS))
			{
				AbilitySystemComponent = PSI->GetAbilitySystemComponent();
			}
		}
	}

	if (AbilitySystemComponent)
	{
		IncomingDamageHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UOutlawAttributeSet::GetIncomingDamageAttribute()
		).AddUObject(this, &UOutlawHitReactionComponent::OnIncomingDamageChanged);
	}
}

void UOutlawHitReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AbilitySystemComponent && IncomingDamageHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UOutlawAttributeSet::GetIncomingDamageAttribute()
		).Remove(IncomingDamageHandle);
		IncomingDamageHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void UOutlawHitReactionComponent::OnIncomingDamageChanged(const FOnAttributeChangeData& Data)
{
	if (!bCanBeStaggered || Data.NewValue <= 0.f)
	{
		return;
	}

	PlayHitReaction(Data.NewValue, nullptr);
}

void UOutlawHitReactionComponent::PlayHitReaction(float DamageAmount, AActor* DamageSource)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UOutlawAttributeSet::GetMaxHealthAttribute());
	if (MaxHealth <= 0.f)
	{
		return;
	}

	EOutlawHitReactionType ReactionType = DetermineReactionType(DamageAmount, MaxHealth);
	if (ReactionType == EOutlawHitReactionType::None)
	{
		return;
	}

	EOutlawHitDirection HitDir = DetermineHitDirection(DamageSource);
	UAnimMontage* Montage = FindHitReactionMontage(ReactionType, HitDir);
	if (!Montage)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !Character->GetMesh())
	{
		return;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	AbilitySystemComponent->AddLooseGameplayTag(OutlawAnimTags::Staggered);
	AnimInstance->Montage_Play(Montage);

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindLambda([this](UAnimMontage* CompletedMontage, bool bInterrupted)
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(OutlawAnimTags::Staggered);
		}
	});
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);
}

EOutlawHitReactionType UOutlawHitReactionComponent::DetermineReactionType(float DamageAmount, float MaxHealth) const
{
	float Percent = (DamageAmount / MaxHealth) * 100.f;

	if (Percent < LightHitThresholdPercent)
	{
		return EOutlawHitReactionType::None;
	}
	else if (Percent < MediumHitThresholdPercent)
	{
		return EOutlawHitReactionType::Light;
	}
	else
	{
		return EOutlawHitReactionType::Medium;
	}
}

EOutlawHitDirection UOutlawHitReactionComponent::DetermineHitDirection(AActor* DamageSource) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !DamageSource)
	{
		return EOutlawHitDirection::Front;
	}

	FVector ToSource = (DamageSource->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
	FVector OwnerForward = Owner->GetActorForwardVector();
	FVector OwnerRight = Owner->GetActorRightVector();

	float ForwardDot = FVector::DotProduct(ToSource, OwnerForward);
	float RightDot = FVector::DotProduct(ToSource, OwnerRight);

	if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
	{
		return (ForwardDot > 0.f) ? EOutlawHitDirection::Front : EOutlawHitDirection::Back;
	}
	else
	{
		return (RightDot > 0.f) ? EOutlawHitDirection::Right : EOutlawHitDirection::Left;
	}
}

UAnimMontage* UOutlawHitReactionComponent::FindHitReactionMontage(EOutlawHitReactionType ReactionType, EOutlawHitDirection Direction) const
{
	for (const FOutlawHitReactionConfig& Config : HitReactionMontages)
	{
		if (Config.ReactionType == ReactionType && Config.Direction == Direction && Config.Montage)
		{
			return Config.Montage;
		}
	}

	for (const FOutlawHitReactionConfig& Config : HitReactionMontages)
	{
		if (Config.ReactionType == ReactionType && Config.Montage)
		{
			return Config.Montage;
		}
	}

	return nullptr;
}
