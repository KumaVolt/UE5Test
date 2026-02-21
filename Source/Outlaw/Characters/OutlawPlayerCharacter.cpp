// Fill out your copyright notice in the Description page of Project Settings.

#include "OutlawPlayerCharacter.h"
#include "AbilitySystem/OutlawAbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Player/OutlawPlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/OutlawCameraComponent.h"
#include "Camera/OutlawLockOnComponent.h"
#include "AbilitySystemComponent.h"
#include "Combat/OutlawDeathComponent.h"
#include "Combat/OutlawPlayerDeathHandler.h"
#include "Combat/OutlawCombatLogComponent.h"
#include "Combat/OutlawDamageNumberComponent.h"
#include "Animation/OutlawHitReactionComponent.h"
#include "Combat/OutlawStatusEffectComponent.h"
#include "Weapon/OutlawWeaponManagerComponent.h"
#include "Weapon/OutlawShooterWeaponData.h"
#include "Inventory/OutlawInventoryComponent.h"
#include "Inventory/OutlawItemInstance.h"
#include "Inventory/OutlawItemDefinition.h"
#include "Progression/OutlawProgressionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Game/OutlawDemoInputConfig.h"
#include "Game/OutlawDemoAbilitySets.h"
#include "Game/OutlawDemoDataSubsystem.h"
#include "Player/OutlawPlayerController.h"
#include "Progression/OutlawClassDefinition.h"
#include "UI/OutlawDemoDeathScreen.h"
#include "UI/OutlawDemoDamageNumber.h"
#include "GameFramework/CharacterMovementComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogOutlaw, Log, All);

AOutlawPlayerCharacter::AOutlawPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Spring Arm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 300.f;
	SpringArm->SocketOffset = FVector(0.f, 60.f, 60.f);
	SpringArm->bUsePawnControlRotation = true;

	// Camera
	CameraComponent = CreateDefaultSubobject<UOutlawCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArm);

	// Lock-On
	LockOnComponent = CreateDefaultSubobject<UOutlawLockOnComponent>(TEXT("LockOn"));

	// Combat
	DeathComponent = CreateDefaultSubobject<UOutlawDeathComponent>(TEXT("Death"));
	PlayerDeathHandler = CreateDefaultSubobject<UOutlawPlayerDeathHandler>(TEXT("PlayerDeathHandler"));
	PlayerDeathHandler->RespawnDelay = 3.f;
	PlayerDeathHandler->DeathScreenWidgetClass = UOutlawDemoDeathScreen::StaticClass();

	CombatLogComponent = CreateDefaultSubobject<UOutlawCombatLogComponent>(TEXT("CombatLog"));
	DamageNumberComponent = CreateDefaultSubobject<UOutlawDamageNumberComponent>(TEXT("DamageNumber"));
	DamageNumberComponent->DamageNumberWidgetClass = UOutlawDemoDamageNumber::StaticClass();

	HitReactionComponent = CreateDefaultSubobject<UOutlawHitReactionComponent>(TEXT("HitReaction"));
	StatusEffectComponent = CreateDefaultSubobject<UOutlawStatusEffectComponent>(TEXT("StatusEffects"));

	// Weapon
	WeaponManagerComponent = CreateDefaultSubobject<UOutlawWeaponManagerComponent>(TEXT("WeaponManager"));
	WeaponManagerComponent->ShooterWeaponSlotOrder.Add(FGameplayTag::RequestGameplayTag(TEXT("Weapon.Slot.Primary1")));
	WeaponManagerComponent->ShooterWeaponSlotOrder.Add(FGameplayTag::RequestGameplayTag(TEXT("Weapon.Slot.Primary2")));
	WeaponManagerComponent->ShooterWeaponSlotOrder.Add(FGameplayTag::RequestGameplayTag(TEXT("Weapon.Slot.Sidearm")));

	// Inventory
	InventoryComponent = CreateDefaultSubobject<UOutlawInventoryComponent>(TEXT("Inventory"));
	InventoryComponent->MaxSlots = 20;

	// Add equipment slots
	FOutlawEquipmentSlotInfo Primary1Slot;
	Primary1Slot.SlotTag = FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary1"));
	InventoryComponent->EquipmentSlots.Add(Primary1Slot);

	FOutlawEquipmentSlotInfo Primary2Slot;
	Primary2Slot.SlotTag = FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary2"));
	InventoryComponent->EquipmentSlots.Add(Primary2Slot);

	FOutlawEquipmentSlotInfo SidearmSlot;
	SidearmSlot.SlotTag = FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Sidearm"));
	InventoryComponent->EquipmentSlots.Add(SidearmSlot);

	// Progression
	ProgressionComponent = CreateDefaultSubobject<UOutlawProgressionComponent>(TEXT("Progression"));

	// Character Movement defaults for OTS shooter
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bOrientRotationToMovement = false;
	}
	bUseControllerRotationYaw = true;

	// Default ability set — use CDO of PlayerDefault
	DefaultAbilitySet = GetMutableDefault<UOutlawAbilitySet_PlayerDefault>();

	// Visual body stand-in (no skeletal mesh)
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(RootComponent);
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.76f));
	BodyMesh->CastShadow = true;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderFinder.Object);
	}

	// Weapon visual — simple rifle from cubes
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		// Gun body (receiver/stock)
		WeaponBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBody"));
		WeaponBodyMesh->SetupAttachment(RootComponent);
		WeaponBodyMesh->SetStaticMesh(CubeFinder.Object);
		WeaponBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponBodyMesh->SetRelativeLocation(FVector(25.f, 20.f, 30.f));
		WeaponBodyMesh->SetRelativeScale3D(FVector(0.30f, 0.07f, 0.10f));
		WeaponBodyMesh->CastShadow = true;

		// Barrel (long thin piece extending forward)
		WeaponBarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponBarrel"));
		WeaponBarrelMesh->SetupAttachment(WeaponBodyMesh);
		WeaponBarrelMesh->SetStaticMesh(CubeFinder.Object);
		WeaponBarrelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponBarrelMesh->SetRelativeLocation(FVector(120.f, 0.f, -10.f));
		WeaponBarrelMesh->SetRelativeScale3D(FVector(2.0f, 0.4f, 0.35f));
		WeaponBarrelMesh->CastShadow = true;
	}
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

	// Bind death component to ASC (deferred — ASC lives on PlayerState, not available at BeginPlay)
	if (DeathComponent && AbilitySystemComponent)
	{
		DeathComponent->BindToAbilitySystem(AbilitySystemComponent);
	}

	// Set up demo input mapping context
	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(UOutlawDemoInputConfig::GetOrCreateMappingContext(), 0);
		}
	}
}

void AOutlawPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitAbilitySystemComponent();
}

void AOutlawPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Color the body mesh blue for player
	if (BodyMesh)
	{
		UMaterialInstanceDynamic* DynMat = BodyMesh->CreateDynamicMaterialInstance(0);
		if (DynMat)
		{
			FLinearColor Blue(0.1f, 0.3f, 0.8f);
			DynMat->SetVectorParameterValue(TEXT("Color"), Blue);
			DynMat->SetVectorParameterValue(TEXT("BaseColor"), Blue);
			DynMat->SetVectorParameterValue(TEXT("Base Color"), Blue);
		}
	}

	// Color weapon dark gunmetal
	FLinearColor GunColor(0.12f, 0.12f, 0.14f);
	auto ColorMesh = [&GunColor](UStaticMeshComponent* Mesh)
	{
		if (!Mesh) return;
		UMaterialInstanceDynamic* Mat = Mesh->CreateDynamicMaterialInstance(0);
		if (Mat)
		{
			Mat->SetVectorParameterValue(TEXT("Color"), GunColor);
			Mat->SetVectorParameterValue(TEXT("BaseColor"), GunColor);
			Mat->SetVectorParameterValue(TEXT("Base Color"), GunColor);
		}
	};
	ColorMesh(WeaponBodyMesh);
	ColorMesh(WeaponBarrelMesh);

	if (HasAuthority())
	{
		SetupDemoDefaults();
	}

	// Set checkpoint at spawn location
	PlayerDeathHandler->SetCheckpoint(GetActorLocation(), GetActorRotation());
}

void AOutlawPlayerCharacter::SetupDemoDefaults()
{
	// Get data subsystem and give the player a starting weapon
	UOutlawDemoDataSubsystem* DataSub = GetWorld()->GetSubsystem<UOutlawDemoDataSubsystem>();
	if (!DataSub)
	{
		return;
	}

	// Add assault rifle to inventory and equip to Primary1
	if (InventoryComponent)
	{
		int32 Added = InventoryComponent->AddItem(DataSub->GetAssaultRifleItemDef(), 1);
		if (Added > 0)
		{
			TArray<FOutlawInventoryEntry> Weapons = InventoryComponent->FindItemsForSlot(
				FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary1")));
			if (Weapons.Num() > 0)
			{
				InventoryComponent->EquipItem(Weapons[0].InstanceId);
			}
		}

		// Add hand cannon to inventory and equip to Primary2
		Added = InventoryComponent->AddItem(DataSub->GetHandCannonItemDef(), 1);
		if (Added > 0)
		{
			TArray<FOutlawInventoryEntry> Weapons = InventoryComponent->FindItemsForSlot(
				FGameplayTag::RequestGameplayTag(TEXT("Equipment.Slot.Primary2")));
			if (Weapons.Num() > 0)
			{
				InventoryComponent->EquipItem(Weapons[0].InstanceId);
			}
		}

		// Add some health packs and ammo
		InventoryComponent->AddItem(DataSub->GetHealthPackItemDef(), 5);
		InventoryComponent->AddItem(DataSub->GetAmmoPackItemDef(), 30);
	}

	// Set up demo class + progression
	if (ProgressionComponent && DataSub->GetDemoClassDef())
	{
		ProgressionComponent->DefaultLevelingConfig = DataSub->GetDemoLevelingConfig();
		ProgressionComponent->AvailableClasses.Empty();
		ProgressionComponent->AvailableClasses.Add(DataSub->GetDemoClassDef());
		ProgressionComponent->SelectClass(DataSub->GetDemoClassDef()->ClassTag);
		ProgressionComponent->AwardXP(250);
	}
}

void AOutlawPlayerCharacter::SetupPlayerInputComponent(UInputComponent* InputComp)
{
	// Bind demo input actions from the config
	JumpAction = UOutlawDemoInputConfig::GetJumpAction();
	MoveAction = UOutlawDemoInputConfig::GetMoveAction();
	LookAction = UOutlawDemoInputConfig::GetLookAction();
	MouseLookAction = UOutlawDemoInputConfig::GetLookAction();
	FireAction = UOutlawDemoInputConfig::GetFireAction();
	ADSAction = UOutlawDemoInputConfig::GetADSAction();
	ReloadAction = UOutlawDemoInputConfig::GetReloadAction();
	CycleWeaponAction = UOutlawDemoInputConfig::GetCycleWeaponAction();
	InventoryToggleAction = UOutlawDemoInputConfig::GetInventoryToggleAction();
	FullInventoryAction = UOutlawDemoInputConfig::GetFullInventoryAction();
	SkillTreeAction = UOutlawDemoInputConfig::GetSkillTreeAction();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComp))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AOutlawPlayerCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AOutlawPlayerCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOutlawPlayerCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOutlawPlayerCharacter::LookInput);

		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AOutlawPlayerCharacter::OnFireStarted);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AOutlawPlayerCharacter::OnFireCompleted);

		// ADS
		EnhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Started, this, &AOutlawPlayerCharacter::OnADSStarted);
		EnhancedInputComponent->BindAction(ADSAction, ETriggerEvent::Completed, this, &AOutlawPlayerCharacter::OnADSCompleted);

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &AOutlawPlayerCharacter::OnReloadStarted);

		// Cycle Weapon
		EnhancedInputComponent->BindAction(CycleWeaponAction, ETriggerEvent::Started, this, &AOutlawPlayerCharacter::OnCycleWeaponStarted);

		// Inventory Toggle (Outriders-style equipment popup)
		EnhancedInputComponent->BindAction(InventoryToggleAction, ETriggerEvent::Started, this, &AOutlawPlayerCharacter::OnInventoryToggle);

		// Full Inventory (Destiny-style)
		EnhancedInputComponent->BindAction(FullInventoryAction, ETriggerEvent::Started, this, &AOutlawPlayerCharacter::OnFullInventoryToggle);

		// Skill Tree
		EnhancedInputComponent->BindAction(SkillTreeAction, ETriggerEvent::Started, this, &AOutlawPlayerCharacter::OnSkillTreeToggle);
	}
	else
	{
		UE_LOG(LogOutlaw, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
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
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AOutlawPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
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

void AOutlawPlayerCharacter::OnFireStarted()
{
	if (AbilitySystemComponent)
	{
		FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Ability.Fire"));
		AbilitySystemComponent->AbilityInputTagPressed(FireTag);
		AbilitySystemComponent->ProcessAbilityInput();
	}
}

void AOutlawPlayerCharacter::OnFireCompleted()
{
	if (AbilitySystemComponent)
	{
		FGameplayTag FireTag = FGameplayTag::RequestGameplayTag(TEXT("Input.Ability.Fire"));
		AbilitySystemComponent->AbilityInputTagReleased(FireTag);
		AbilitySystemComponent->ProcessAbilityInput();
	}
}

void AOutlawPlayerCharacter::OnADSStarted()
{
	if (CameraComponent)
	{
		CameraComponent->EnterADS();
	}
}

void AOutlawPlayerCharacter::OnADSCompleted()
{
	if (CameraComponent)
	{
		CameraComponent->ExitADS();
	}
}

void AOutlawPlayerCharacter::OnReloadStarted()
{
	// For the demo, simply reload the active weapon
	if (WeaponManagerComponent)
	{
		if (UOutlawItemInstance* Weapon = WeaponManagerComponent->GetActiveWeapon())
		{
			if (Weapon->ItemDef && Weapon->ItemDef->ShooterWeaponData)
			{
				Weapon->CurrentAmmo = Weapon->ItemDef->ShooterWeaponData->MagazineSize;
			}
		}
	}
}

void AOutlawPlayerCharacter::OnCycleWeaponStarted()
{
	if (WeaponManagerComponent)
	{
		WeaponManagerComponent->CycleWeapon();
	}
}

void AOutlawPlayerCharacter::OnInventoryToggle()
{
	if (AOutlawPlayerController* PC = Cast<AOutlawPlayerController>(GetController()))
	{
		PC->ToggleEquipmentPopup();
	}
}

void AOutlawPlayerCharacter::OnFullInventoryToggle()
{
	if (AOutlawPlayerController* PC = Cast<AOutlawPlayerController>(GetController()))
	{
		PC->ToggleInventoryScreen();
	}
}

void AOutlawPlayerCharacter::OnSkillTreeToggle()
{
	if (AOutlawPlayerController* PC = Cast<AOutlawPlayerController>(GetController()))
	{
		PC->ToggleSkillTree();
	}
}
