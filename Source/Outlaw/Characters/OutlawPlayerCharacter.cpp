// Fill out your copyright notice in the Description page of Project Settings.


#include "OutlawPlayerCharacter.h"
#include "AbilitySystem/OutlawAbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "Player/OutlawPlayerState.h"
DEFINE_LOG_CATEGORY_STATIC(LogOutlaw, Log, All);


// Sets default values
AOutlawPlayerCharacter::AOutlawPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AOutlawPlayerCharacter::InitAbilitySystemComponent()
{
	AOutlawPlayerState* OutlawPlayerState = GetPlayerState<AOutlawPlayerState>();
	check(OutlawPlayerState);
	AbilitySystemComponent = CastChecked<UOutlawAbilitySystemComponent>(
		OutlawPlayerState->GetAbilitySystemComponent());
	AbilitySystemComponent->InitAbilityActorInfo(OutlawPlayerState, this);
}

void AOutlawPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	InitAbilitySystemComponent();
	GrantDefaultAbilitySet();
}

void AOutlawPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	InitAbilitySystemComponent();
}

void AOutlawPlayerCharacter::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AOutlawPlayerCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AOutlawPlayerCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOutlawPlayerCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOutlawPlayerCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AOutlawPlayerCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogOutlaw, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AOutlawPlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AOutlawPlayerCharacter::LookInput(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AOutlawPlayerCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AOutlawPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AOutlawPlayerCharacter::DoJumpStart()
{
	Jump();
}

void AOutlawPlayerCharacter::DoJumpEnd()
{
	StopJumping();
}
