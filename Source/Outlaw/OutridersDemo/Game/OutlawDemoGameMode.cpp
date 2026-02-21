#include "Game/OutlawDemoGameMode.h"
#include "Characters/OutlawPlayerCharacter.h"
#include "Characters/OutlawEnemyCharacter.h"
#include "Player/OutlawPlayerState.h"
#include "Player/OutlawPlayerController.h"
#include "AI/OutlawEnemySpawner.h"
#include "AI/OutlawAITypes.h"
#include "Loot/OutlawLootSubsystem.h"
#include "Loot/OutlawLootPickup.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerStart.h"

AOutlawDemoGameMode::AOutlawDemoGameMode()
{
	DefaultPawnClass = AOutlawPlayerCharacter::StaticClass();
	PlayerStateClass = AOutlawPlayerState::StaticClass();
	PlayerControllerClass = AOutlawPlayerController::StaticClass();
}

void AOutlawDemoGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogTemp, Log, TEXT("OutlawDemoGameMode: Pure C++ demo initialized on map %s"), *MapName);

	// Build arena before players spawn so PlayerStart and floor exist
	BuildArena();
	SpawnLighting();
	SpawnEnemySpawner();
}

void AOutlawDemoGameMode::BeginPlay()
{
	Super::BeginPlay();

	ConfigureLootSubsystem();
}

void AOutlawDemoGameMode::BuildArena()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!CubeMesh || !PlaneMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("OutlawDemoGameMode: Failed to load basic shape meshes"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Helper to spawn a static mesh actor with given transform and scale
	auto SpawnMeshActor = [&](UStaticMesh* Mesh, FVector Location, FVector Scale) -> AStaticMeshActor*
	{
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParams);
		if (Actor)
		{
			Actor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
			Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
			Actor->SetActorScale3D(Scale);
		}
		return Actor;
	};

	// ── Floor ──────────────────────────────────────────────────────
	// 3600x2400 arena. Cube (100^3) scale (36,24,1) → surface at Z=0.
	SpawnMeshActor(CubeMesh, FVector(0.f, 0.f, -50.f), FVector(36.f, 24.f, 1.f));

	// ── Walls ──────────────────────────────────────────────────────
	// 500 tall, 50 thick. Center at Z=250 so base on floor, top at 500.
	SpawnMeshActor(CubeMesh, FVector(0.f, 1200.f, 250.f),   FVector(36.f, 0.5f, 5.f));  // North
	SpawnMeshActor(CubeMesh, FVector(0.f, -1200.f, 250.f),  FVector(36.f, 0.5f, 5.f));  // South
	SpawnMeshActor(CubeMesh, FVector(1800.f, 0.f, 250.f),   FVector(0.5f, 24.f, 5.f));  // East
	SpawnMeshActor(CubeMesh, FVector(-1800.f, 0.f, 250.f),  FVector(0.5f, 24.f, 5.f));  // West

	// ── Buildings (4 corner structures, 600x400x300) ───────────────
	// Cube scale (6,4,3), center at Z=150 so base on floor, top at 300.
	SpawnMeshActor(CubeMesh, FVector(-1200.f, 600.f, 150.f),  FVector(6.f, 4.f, 3.f));  // Bldg A (NW)
	SpawnMeshActor(CubeMesh, FVector(1200.f, 600.f, 150.f),   FVector(6.f, 4.f, 3.f));  // Bldg B (NE)
	SpawnMeshActor(CubeMesh, FVector(-1200.f, -600.f, 150.f), FVector(6.f, 4.f, 3.f));  // Bldg C (SW)
	SpawnMeshActor(CubeMesh, FVector(1200.f, -600.f, 150.f),  FVector(6.f, 4.f, 3.f));  // Bldg D (SE)

	// ── Trucks (2 center-lane blockers, 400x150x150) ──────────────
	// Cube scale (4,1.5,1.5), center at Z=75.
	SpawnMeshActor(CubeMesh, FVector(0.f, 350.f, 75.f),  FVector(4.f, 1.5f, 1.5f));  // Truck N
	SpawnMeshActor(CubeMesh, FVector(0.f, -350.f, 75.f), FVector(4.f, 1.5f, 1.5f));  // Truck S

	// ── Barrels (4 small cover, 100x100x120) ───────────────────────
	// Cube scale (1,1,1.2), center at Z=60.
	SpawnMeshActor(CubeMesh, FVector(-500.f, 150.f, 60.f),  FVector(1.f, 1.f, 1.2f));
	SpawnMeshActor(CubeMesh, FVector(500.f, 150.f, 60.f),   FVector(1.f, 1.f, 1.2f));
	SpawnMeshActor(CubeMesh, FVector(-500.f, -150.f, 60.f), FVector(1.f, 1.f, 1.2f));
	SpawnMeshActor(CubeMesh, FVector(500.f, -150.f, 60.f),  FVector(1.f, 1.f, 1.2f));

	// ── Crates (4 medium cover, 200x200x100) ──────────────────────
	// Cube scale (2,2,1), center at Z=50.
	SpawnMeshActor(CubeMesh, FVector(-600.f, 700.f, 50.f),  FVector(2.f, 2.f, 1.f));
	SpawnMeshActor(CubeMesh, FVector(600.f, 700.f, 50.f),   FVector(2.f, 2.f, 1.f));
	SpawnMeshActor(CubeMesh, FVector(-600.f, -700.f, 50.f), FVector(2.f, 2.f, 1.f));
	SpawnMeshActor(CubeMesh, FVector(600.f, -700.f, 50.f),  FVector(2.f, 2.f, 1.f));

	// ── Player Start ───────────────────────────────────────────────
	// Dead center, facing north (toward enemy spawner's opposite side)
	World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
		FVector(0.f, 0.f, 100.f), FRotator(0.f, 90.f, 0.f), SpawnParams);

	UE_LOG(LogTemp, Log, TEXT("OutlawDemoGameMode: Nuketown arena built — floor, 4 walls, 4 buildings, 2 trucks, 4 barrels, 4 crates, PlayerStart"));
}

void AOutlawDemoGameMode::SpawnLighting()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// Directional light (sun) — angled for good shadow definition
	ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(
		FVector::ZeroVector, FRotator(-50.f, -45.f, 0.f), SpawnParams);
	if (Sun && Sun->GetLightComponent())
	{
		Sun->GetLightComponent()->SetMobility(EComponentMobility::Movable);
		Sun->GetLightComponent()->SetIntensity(3.f);
	}

	// Sky light for ambient fill
	ASkyLight* Sky = World->SpawnActor<ASkyLight>(
		FVector(0.f, 0.f, 500.f), FRotator::ZeroRotator, SpawnParams);
	if (Sky)
	{
		USkyLightComponent* SkyComp = Sky->FindComponentByClass<USkyLightComponent>();
		if (SkyComp)
		{
			SkyComp->SetMobility(EComponentMobility::Movable);
			SkyComp->SetIntensity(1.f);
			SkyComp->bLowerHemisphereIsBlack = false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("OutlawDemoGameMode: Lighting spawned — directional + sky light"));
}

void AOutlawDemoGameMode::SpawnEnemySpawner()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AOutlawEnemySpawner* Spawner = World->SpawnActor<AOutlawEnemySpawner>(
		FVector(0.f, -1100.f, 100.f), FRotator::ZeroRotator, SpawnParams);

	if (!Spawner)
	{
		UE_LOG(LogTemp, Error, TEXT("OutlawDemoGameMode: Failed to spawn enemy spawner"));
		return;
	}

	Spawner->SpawnerMode = EOutlawSpawnerMode::WaveBased;
	Spawner->SpawnRadius = 500.f;
	Spawner->MaxActiveEnemies = 10;

	// Wave 1: 3 enemies
	FOutlawWaveSpawnData Wave1;
	Wave1.EnemyClass = AOutlawEnemyCharacter::StaticClass();
	Wave1.Count = 3;
	Wave1.SpawnInterval = 1.0f;
	Spawner->Waves.Add(Wave1);

	// Wave 2: 5 enemies
	FOutlawWaveSpawnData Wave2;
	Wave2.EnemyClass = AOutlawEnemyCharacter::StaticClass();
	Wave2.Count = 5;
	Wave2.SpawnInterval = 0.8f;
	Spawner->Waves.Add(Wave2);

	// Wave 3: 8 enemies
	FOutlawWaveSpawnData Wave3;
	Wave3.EnemyClass = AOutlawEnemyCharacter::StaticClass();
	Wave3.Count = 8;
	Wave3.SpawnInterval = 0.5f;
	Spawner->Waves.Add(Wave3);

	UE_LOG(LogTemp, Log, TEXT("OutlawDemoGameMode: Enemy spawner configured — 3 waves (3, 5, 8 enemies)"));
}

void AOutlawDemoGameMode::ConfigureLootSubsystem()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UOutlawLootSubsystem* LootSub = World->GetSubsystem<UOutlawLootSubsystem>();
	if (LootSub)
	{
		LootSub->LootPickupClass = AOutlawLootPickup::StaticClass();
		LootSub->bAutoLootPickups = true;
		UE_LOG(LogTemp, Log, TEXT("OutlawDemoGameMode: Loot subsystem configured — auto-loot enabled"));
	}
}
