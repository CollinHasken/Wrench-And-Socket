// Fill out your copyright notice in the Description page of Project Settings.
#include "Collectible.h"

#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Util/RCStatics.h"
#include "RC/Util/RCTypes.h"

#if DEBUG_ENABLED
static TAutoConsoleVariable<bool> CCollectibleDebugDisplay(
	TEXT("Collectible.DebugDisplay"),
	false,
	TEXT("Toggle the collectible debug display.\n")
	TEXT("0: Don't display debug info (default)\n")
	TEXT("1: Display debug info"),
	ECVF_Default
	);
#endif

// Sets default values
ACollectible::ACollectible()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(URCStatics::CollectiblePre_ProfileName);
	Mesh->SetSimulatePhysics(true);
	Mesh->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	RootComponent = Mesh;

	CollectTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Collect Trigger"));
	CollectTrigger->SetCollisionProfileName(URCStatics::Collectible_ProfileName);
	CollectTrigger->SetupAttachment(Mesh);
}

// Called when the game starts or when spawned
void ACollectible::BeginPlay()
{
	Super::BeginPlay();

	// Start the delay
	if (CollectibleInfo != nullptr)
	{
		CollectDelayHandle.Set(CollectibleInfo->CollectionDelay);
	}
}

// Called every frame
void ACollectible::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if DEBUG_ENABLED
	Debug(DeltaTime);
#endif

	if (bDelayCollection)
	{
		if (CollectDelayHandle.Elapsed())
		{
			CollectDelayHandle.Invalidate();
			bDelayCollection = false;

			StartCollecting(CurrentTargetW.Get());
		}
	}

	if (bIsTraveling)
	{
		CurrentTravelTime += DeltaTime;
		float DistancePercentage = CurrentTravelTime * CurrentTravelTimeMaxInv;

		FVector NextLocation(FVector::ZeroVector);
		if (!FindTravelLocation(&NextLocation, DistancePercentage))
		{
			bIsTraveling = false;
			Destroy();
			return;
		}

		SetActorLocation(NextLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

// Start the collection process of moving towards the target
void ACollectible::StartCollecting(class AActor* Target)
{
	if (Target == nullptr)
	{
		return;
	}

	// If the delay is going, we'll call this function again once it's done
	if (CollectDelayHandle.IsActive())
	{
		bDelayCollection = true;

		// Store the target for when we recall this
		CurrentTargetW = Target;
		return;
	}

	// Allow us to go through things
	Mesh->SetSimulatePhysics(false);
	Mesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	// Cache and reset values
	CollectStartLocation = GetActorLocation();
	CurrentTargetW = Target;
	CurrentTravelTime = 0;
	bIsTraveling = true;

	ASSERT_RETURN(CollectibleInfo != nullptr);
	CurrentTravelTimeMaxInv = 1 / FMath::FRandRange(CollectibleInfo->TravelTimeMin, CollectibleInfo->TravelTimeMax);

	// Store off the collectible data for when we get collected
	ARCCharacter* TargetPlayer = Cast<ARCCharacter>(Target);
	if (TargetPlayer != nullptr)
	{
		ARCPlayerState* PlayerState = TargetPlayer->GetPlayerState<ARCPlayerState>();
		ASSERT_RETURN(PlayerState != nullptr, "Player doesn't have player state");

		CollectibleData = Cast<UCollectibleData>(PlayerState->FindOrAddDataForAsset(CollectibleDataClass, CollectibleInfo->GetPrimaryAssetId()));
		ASSERT_RETURN(CollectibleData != nullptr, "Weapon Data not able to be added");
	}
}

// Called when this collectible has been collected
int ACollectible::Collect()
{
	// If we aren't traveling yet, then it was collected during the initial delay
	if (!bIsTraveling)
	{
		ARCPlayerState* PlayerState = URCStatics::GetPlayerState(GetWorld());
		if (PlayerState == nullptr)
		{
			ASSERT(PlayerState != nullptr, "Player doesn't have player state");
		}
		else
		{
			CollectibleData = Cast<UCollectibleData>(PlayerState->FindOrAddDataForAsset(CollectibleDataClass, CollectibleInfo->GetPrimaryAssetId()));
			ASSERT(CollectibleData != nullptr, "Weapon Data not able to be added");		
		}
	}

	int CollectionAmount = 0;
	if (CollectibleData != nullptr)
	{
		CollectionAmount = GetCollectionAmount();
		CollectibleData->GrantCollectible(CollectionAmount);
	}

	Destroy();
	return CollectionAmount;
}

// Find the location along the quadratic given the percentage along the arc
void ACollectible::FindRelativeTravelLocation(FVector* FoundLocation, float DistancePercentage, const FVector2D& StartToTargetXY, float TargetDist, float A, float B) const
{
	ASSERT_RETURN(FoundLocation != nullptr);

	// Calculate the point location dependent on the percentage along the arc
	float PointULength = DistancePercentage * TargetDist;
	FVector2D PointU = StartToTargetXY * PointULength;
	float PointZ = (A * FMath::Square(PointULength)) + (B * PointULength);
	FoundLocation->Set(PointU.X, PointU.Y, PointZ);
}

// Find the location along the quadratic given the percentage along the arc
bool ACollectible::FindRelativeTravelLocation(FVector* FoundLocation, float DistancePercentage, const FVector& StartLocation) const
{
	ASSERT_RETURN_VALUE(FoundLocation != nullptr, false);

	AActor* CurrentTarget = CurrentTargetW.Get();
	if (CurrentTarget == nullptr)
	{
		return false;
	}

	const FVector& TargetLocation = CurrentTarget->GetActorLocation();
	FVector2D StartToTargetXY(TargetLocation - StartLocation);
	float TargetDist = StartToTargetXY.Size();
	StartToTargetXY.Normalize();

	float A = 0;
	float B = 0;

	if (!FindQuadraticCoefficients(&A, &B))
	{
		return false;
	}
	
	FindRelativeTravelLocation(FoundLocation, DistancePercentage, StartToTargetXY, TargetDist, A, B);
	return true;
}

// Find the location along the quadratic given the percentage along the arc in world space
void ACollectible::FindTravelLocation(FVector* FoundLocation, float DistancePercentage, const FVector& StartLocation, const FVector2D& StartToTargetXY, float TargetToStartDist, float A, float B) const
{
	ASSERT_RETURN(FoundLocation != nullptr);

	FindRelativeTravelLocation(FoundLocation, DistancePercentage, StartToTargetXY, TargetToStartDist, A,  B);
	//is start location right?
	*FoundLocation += StartLocation;
}

// Find the location along the quadratic given the percentage along the arc in world space
bool ACollectible::FindTravelLocation(FVector* FoundLocation, float DistancePercentage, const FVector& StartLocation) const
{
	ASSERT_RETURN_VALUE(FoundLocation != nullptr, false);

	if (!FindRelativeTravelLocation(FoundLocation, DistancePercentage, StartLocation))
	{
		return false;
	}

	*FoundLocation += StartLocation;
	return true;
}

// Solve the quadratic y = Ax^2 + Bx for the stored start position to the target
void ACollectible::FindQuadraticCoefficients(float* A, float* B, const FVector& StartLocation, const FVector& TargetLocation) const
{
	ASSERT_RETURN(A != nullptr);
	ASSERT_RETURN(B != nullptr);

	// Using the length of the XY vector (U) as x
	// Using the height Z as y
	
	// Get target location U and Z
	const FVector StartToTarget = TargetLocation - StartLocation;
	FVector2D StartToTargetXY(StartToTarget);
	const float TargetUSquared = StartToTargetXY.SizeSquared();

	// Normalize to get the direction from the start to the target
	FVector2D& StartToTargetDir = StartToTargetXY;
	StartToTargetDir.Normalize();

	// Make a 3rd point for the quadratic to go through
	const float ThroughLocationU = 50;
	const float ThroughLocationZ = 50;
	StartToTargetDir *= ThroughLocationU;
	const float ThroughUSquared = StartToTargetDir.SizeSquared();

	// Create matrices to solve
	FMatrix2x2 UMatrix(TargetUSquared, FMath::Sqrt(TargetUSquared), ThroughUSquared, FMath::Sqrt(ThroughUSquared));
	const FMatrix2x2 ZMatrix(StartToTarget.Z, 0, ThroughLocationZ, 0);

	// Solve for AB
	const FMatrix2x2 ABMatrix = UMatrix.Inverse().Concatenate(ZMatrix);

	// Get AB
	float unused;
	ABMatrix.GetMatrix(*A, unused, *B, unused);
}

// Solve the quadratic y = Ax^2 + Bx for the stored start position to the current target
bool ACollectible::FindQuadraticCoefficients(float* A, float* B) const
{
	const AActor* CurrentTarget = CurrentTargetW.Get();
	if (CurrentTarget == nullptr)
	{
		return false;
	}

	FindQuadraticCoefficients(A, B, CollectStartLocation, CurrentTarget->GetActorLocation());
	return true;
}

#if DEBUG_ENABLED
#include "DrawDebugHelpers.h"
void ACollectible::Debug(float DeltaTime)
{
	if (!CCollectibleDebugDisplay.GetValueOnGameThread())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (PlayerCharacter == nullptr)
	{
		return;
	}

	if (DebugStartLocation == FVector::ZeroVector)
	{
		DebugStartLocation = GetActorLocation();
	}

	float A = 0;
	float B = 0;
	const FVector& TargetLocation = PlayerCharacter->GetActorLocation();
	FindQuadraticCoefficients(&A, &B, DebugStartLocation, TargetLocation);
	UE_LOG(LogCollectible, Verbose, TEXT("Collectible A: %f B: %f"), A, B);

#if ENABLE_DRAW_DEBUG
	//const FVector& StartLocation = GetActorLocation();
	FVector2D StartToTargetXY(TargetLocation - DebugStartLocation);
	float TargetDist = StartToTargetXY.Size();
	StartToTargetXY.Normalize();

	static constexpr int MAX_POINTS = 50;
	static constexpr int MAX_H_VALUE = 360;
	for (int PointIndex = 0; PointIndex < MAX_POINTS; ++PointIndex)
	{
		const float PointPercentage = (float)PointIndex / MAX_POINTS;

		// Create an HSV color using the percentage across the arc and store as RGB
		FLinearColor PointColor(PointPercentage * MAX_H_VALUE, 1, 1, 1);
		PointColor = PointColor.HSVToLinearRGB();

		FVector PointLocation(FVector::ZeroVector);
		FindTravelLocation(&PointLocation, PointPercentage, DebugStartLocation, StartToTargetXY, TargetDist, A, B);

		// debug end Z is off. hook blueprint back up and check
		// collect was very fast, couldn't tell
		DrawDebugPoint(World, PointLocation, 30, PointColor.ToFColor(true), false, DeltaTime + 0.01f, SDPG_World);
	}
#endif
}
#endif
