// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawPlayerDeathHandler.h"
#include "OutlawDeathComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/OutlawAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/OutlawAnimationTypes.h"

UOutlawPlayerDeathHandler::UOutlawPlayerDeathHandler(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOutlawPlayerDeathHandler::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (UOutlawDeathComponent* DeathComp = Owner->FindComponentByClass<UOutlawDeathComponent>())
	{
		DeathComp->OnDeathStarted.AddDynamic(this, &UOutlawPlayerDeathHandler::OnDeathStarted);
	}
}

void UOutlawPlayerDeathHandler::SetCheckpoint(FVector Location, FRotator Rotation)
{
	CheckpointLocation = Location;
	CheckpointRotation = Rotation;
}

void UOutlawPlayerDeathHandler::OnDeathStarted(AActor* Killer)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	if (DeathScreenWidgetClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(Cast<APawn>(Owner)->GetController()))
		{
			DeathScreenWidget = CreateWidget<UUserWidget>(PC, DeathScreenWidgetClass);
			if (DeathScreenWidget)
			{
				DeathScreenWidget->AddToViewport();
			}
		}
	}

	GetWorld()->GetTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&UOutlawPlayerDeathHandler::RespawnAtCheckpoint,
		RespawnDelay,
		false
	);
}

void UOutlawPlayerDeathHandler::RespawnAtCheckpoint()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	Owner->SetActorLocationAndRotation(CheckpointLocation, CheckpointRotation);

	UAbilitySystemComponent* ASC = nullptr;
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		ASC = ASI->GetAbilitySystemComponent();
	}

	if (ASC)
	{
		ASC->RemoveLooseGameplayTag(OutlawAnimTags::Dead);

		if (const UOutlawAttributeSet* AttributeSet = ASC->GetSet<UOutlawAttributeSet>())
		{
			ASC->SetNumericAttributeBase(
				UOutlawAttributeSet::GetHealthAttribute(),
				AttributeSet->GetMaxHealth()
			);
		}
	}

	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			Movement->SetMovementMode(MOVE_Walking);
		}

		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			Capsule->SetCollisionResponseToAllChannels(ECR_Block);
			Capsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		}

		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			Character->EnableInput(PC);
		}
	}

	if (DeathScreenWidget)
	{
		DeathScreenWidget->RemoveFromParent();
		DeathScreenWidget = nullptr;
	}
}
