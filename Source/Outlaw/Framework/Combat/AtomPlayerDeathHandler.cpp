// Fill out your copyright notice in the Description page of Project Settings.

#include "AtomPlayerDeathHandler.h"
#include "AtomDeathComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AtomAttributeSet.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Blueprint/UserWidget.h"
#include "Animation/AtomAnimationTypes.h"

UAtomPlayerDeathHandler::UAtomPlayerDeathHandler(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAtomPlayerDeathHandler::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (UAtomDeathComponent* DeathComp = Owner->FindComponentByClass<UAtomDeathComponent>())
	{
		DeathComp->OnDeathStarted.AddDynamic(this, &UAtomPlayerDeathHandler::OnDeathStarted);
	}
}

void UAtomPlayerDeathHandler::SetCheckpoint(FVector Location, FRotator Rotation)
{
	CheckpointLocation = Location;
	CheckpointRotation = Rotation;
}

void UAtomPlayerDeathHandler::OnDeathStarted(AActor* Killer)
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
		&UAtomPlayerDeathHandler::RespawnAtCheckpoint,
		RespawnDelay,
		false
	);
}

void UAtomPlayerDeathHandler::RespawnAtCheckpoint()
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
		ASC->RemoveLooseGameplayTag(AtomAnimTags::Dead);

		if (const UAtomAttributeSet* AttributeSet = ASC->GetSet<UAtomAttributeSet>())
		{
			ASC->SetNumericAttributeBase(
				UAtomAttributeSet::GetHealthAttribute(),
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
