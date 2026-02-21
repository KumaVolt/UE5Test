// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AtomHitReactionComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AtomAttributeSet.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffectTypes.h"
#include "Kismet/KismetMathLibrary.h"

UAtomHitReactionComponent::UAtomHitReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAtomHitReactionComponent::BeginPlay()
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
			UAtomAttributeSet::GetIncomingDamageAttribute()
		).AddUObject(this, &UAtomHitReactionComponent::OnIncomingDamageChanged);
	}
}

void UAtomHitReactionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AbilitySystemComponent && IncomingDamageHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UAtomAttributeSet::GetIncomingDamageAttribute()
		).Remove(IncomingDamageHandle);
		IncomingDamageHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void UAtomHitReactionComponent::OnIncomingDamageChanged(const FOnAttributeChangeData& Data)
{
	if (!bCanBeStaggered || Data.NewValue <= 0.f)
	{
		return;
	}

	PlayHitReaction(Data.NewValue, nullptr);
}

void UAtomHitReactionComponent::PlayHitReaction(float DamageAmount, AActor* DamageSource)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UAtomAttributeSet::GetMaxHealthAttribute());
	if (MaxHealth <= 0.f)
	{
		return;
	}

	EAtomHitReactionType ReactionType = DetermineReactionType(DamageAmount, MaxHealth);
	if (ReactionType == EAtomHitReactionType::None)
	{
		return;
	}

	EAtomHitDirection HitDir = DetermineHitDirection(DamageSource);
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

	AbilitySystemComponent->AddLooseGameplayTag(AtomAnimTags::Staggered);
	AnimInstance->Montage_Play(Montage);

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindLambda([this](UAnimMontage* CompletedMontage, bool bInterrupted)
	{
		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(AtomAnimTags::Staggered);
		}
	});
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);
}

EAtomHitReactionType UAtomHitReactionComponent::DetermineReactionType(float DamageAmount, float MaxHealth) const
{
	float Percent = (DamageAmount / MaxHealth) * 100.f;

	if (Percent < LightHitThresholdPercent)
	{
		return EAtomHitReactionType::None;
	}
	else if (Percent < MediumHitThresholdPercent)
	{
		return EAtomHitReactionType::Light;
	}
	else
	{
		return EAtomHitReactionType::Medium;
	}
}

EAtomHitDirection UAtomHitReactionComponent::DetermineHitDirection(AActor* DamageSource) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !DamageSource)
	{
		return EAtomHitDirection::Front;
	}

	FVector ToSource = (DamageSource->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
	FVector OwnerForward = Owner->GetActorForwardVector();
	FVector OwnerRight = Owner->GetActorRightVector();

	float ForwardDot = FVector::DotProduct(ToSource, OwnerForward);
	float RightDot = FVector::DotProduct(ToSource, OwnerRight);

	if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
	{
		return (ForwardDot > 0.f) ? EAtomHitDirection::Front : EAtomHitDirection::Back;
	}
	else
	{
		return (RightDot > 0.f) ? EAtomHitDirection::Right : EAtomHitDirection::Left;
	}
}

UAnimMontage* UAtomHitReactionComponent::FindHitReactionMontage(EAtomHitReactionType ReactionType, EAtomHitDirection Direction) const
{
	for (const FAtomHitReactionConfig& Config : HitReactionMontages)
	{
		if (Config.ReactionType == ReactionType && Config.Direction == Direction && Config.Montage)
		{
			return Config.Montage;
		}
	}

	for (const FAtomHitReactionConfig& Config : HitReactionMontages)
	{
		if (Config.ReactionType == ReactionType && Config.Montage)
		{
			return Config.Montage;
		}
	}

	return nullptr;
}
