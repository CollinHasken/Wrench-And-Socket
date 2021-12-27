// Fill out your copyright notice in the Description page of Project Settings.
#include "MovingTeleporter.h"

#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "EngineUtils.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/DataSingleton.h"
#include "RC/Util/RCStatics.h"

DEFINE_LOG_CATEGORY(LogTeleporter);

/*****************************
 *
 * AMovingTeleporter
 *
 *****************************/

// Static consts
const FName AMovingTeleporter::MaterialParamXMin	= FName(TEXT("XMin"));
const FName AMovingTeleporter::MaterialParamXMax	= FName(TEXT("XMax"));
const FName AMovingTeleporter::MaterialParamYMin	= FName(TEXT("YMin"));
const FName AMovingTeleporter::MaterialParamYMax	= FName(TEXT("YMax"));
const float AMovingTeleporter::TeleporterOffsetMag	= 315.f;
const float AMovingTeleporter::GrowScale			= 3.0f;
const float AMovingTeleporter::MinActiveDistanceSqr = FMath::Square(600);
const float AMovingTeleporter::MaxActiveDistanceSqr = FMath::Square(3500);
const float AMovingTeleporter::MaxActiveAngle		= FMath::DegreesToRadians(30);

// Sets default values
AMovingTeleporter::AMovingTeleporter()
{
	Base = CreateDefaultSubobject<USceneComponent>(TEXT("Base"));
	RootComponent = Base;

	IdleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IdleMesh"));
	IdleMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	IdleMesh->SetupAttachment(RootComponent);

	CaptureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CaptureMesh"));
	CaptureMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	CaptureMesh->SetVisibility(false);
	CaptureMesh->SetupAttachment(RootComponent);

	TeleportTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("TeleportTrigger"));
	TeleportTrigger->SetCollisionProfileName(URCStatics::Trigger_ProfileName);
	TeleportTrigger->SetupAttachment(CaptureMesh);

	TeleportLocation = CreateDefaultSubobject<USceneComponent>(TEXT("TeleportLocation"));
	TeleportLocation->SetupAttachment(RootComponent);

	CameraBaseLocation = CreateDefaultSubobject<USceneComponent>(TEXT("CameraBaseLocation"));
	CameraBaseLocation->SetupAttachment(TeleportLocation);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 430.f;
	CameraBoom->SocketOffset = FVector(0, 100, 60);
	CameraBoom->SetupAttachment(CameraBaseLocation);

	CameraCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CameraCapture"));
	CameraCapture->bUseRayTracingIfEnabled = true;
	CameraCapture->bCaptureEveryFrame = false;
	CameraCapture->bCaptureOnMovement = false;
	CameraCapture->SetupAttachment(CameraBoom);

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);
}

// Called when the game starts or when spawned
void AMovingTeleporter::BeginPlay()
{
	Super::BeginPlay();

	// Debug only checks
	ASSERT(UDataSingleton::Get().MovingTeleporterRotateCurve != nullptr, "Moving Teleporter Rotate Curve needs to be set in the singleton");
	ASSERT(UDataSingleton::Get().MovingTeleporterGrowCurve != nullptr, "Moving Teleporter Grow Curve needs to be set in the singleton");
	ASSERT(UDataSingleton::Get().MovingTeleporterMoveCurve != nullptr, "Moving Teleporter Move Curve needs to be set in the singleton");
	ASSERT(InactiveMaterial != nullptr, "Moving teleporter inactive material not set");
	ASSERT(ActiveMaterial != nullptr, "Moving teleporter active material not set");
	
	// Create the material we use to show the capture
	CaptureMI = CaptureMesh->CreateDynamicMaterialInstance(0, CaptureMaterial);
	ASSERT(CaptureMI != nullptr, "Unable to create dynamic material instance");

	// Stash the controller
	PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	ASSERT_RETURN(PlayerController != nullptr);

	// Set inactive
	IdleMesh->SetMaterial(0, InactiveMaterial);

	// Dont capture ourselves
	CameraCapture->HiddenActors.Add(this);
}
// Called every frame
void AMovingTeleporter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	StateTime += DeltaTime;
	switch (CurrentState)
	{
		case EMovingTeleporterState::Idle:
			LOG_CHECK(false, LogTeleporter, Error, "Moving teleporter shouldn't tick while idle");
			SetActorTickEnabled(false);
			break;
		case EMovingTeleporterState::Rotating:
			TickRotate(DeltaTime);
			break;
		case EMovingTeleporterState::Growing:
			TickGrow();
			break;
		case EMovingTeleporterState::Moving:
			TickMove();
			break;
		default:
			break;
	}
}

// Tick while rotating the player camera towards the teleporter
void AMovingTeleporter::TickRotate(float DeltaTime)
{
	// Still rotating
	if (StateTime < RotationDuration)
	{
		// Rotate the player towards the rotation it'll be at after the teleport
		ACharacter* Player = PlayerController->GetCharacter();
		ASSERT_RETURN(Player != nullptr);
		FRotator NewPlayerRot = Player->GetActorRotation();
		NewPlayerRot.Yaw += (PlayerDeltaPerSecRotYaw * DeltaTime);
		Player->SetActorRotation(NewPlayerRot);

		// Rotate the camera towards the teleporter
		float TimeAlpha = StateTime / RotationDuration;
		FQuat NewCameraQuat = FMath::Lerp(CameraStartQuat, CameraCapture->GetComponentQuat(), TimeAlpha);
		PlayerController->SetControlRotation(NewCameraQuat.Rotator()); 
	}
	else
	{
		// Done rotating
		CurrentState = EMovingTeleporterState::Growing;
		StateTime = 0.0f;

		// Make sure we got to the end states
		ACharacter* Player = PlayerController->GetCharacter();
		ASSERT_RETURN(Player != nullptr);
		Player->SetActorRotation(TeleportLocation->GetComponentQuat());
		PlayerController->SetControlRotation(CameraCapture->GetComponentRotation());

		// Show the capture now
		CameraCapture->bCaptureEveryFrame = true;
		CaptureMesh->SetVisibility(true);
		IdleMesh->SetVisibility(false);
	}
}

// Tick while growing the teleporter
void AMovingTeleporter::TickGrow()
{
	// Set the scale for the mesh
	float Alpha = UDataSingleton::Get().MovingTeleporterGrowCurve->GetFloatValue(StateTime);
	float NewScale = FMath::Lerp(1.0f, GrowScale, Alpha);
	CaptureMesh->SetWorldScale3D(FVector(1.78f * NewScale, NewScale, 1.0f));

	// Update the material
	UpdateMaterialUVs();

	// Done growing
	if (Alpha >= 1.0f)
	{
		CurrentState = EMovingTeleporterState::Moving;
		StateTime = 0.0f;

		// Store the move time for the tick
		float unused;
		UDataSingleton::Get().MovingTeleporterMoveCurve->GetTimeRange(unused, MoveTime);

		// Listen for when the trigger overlapps with the player
		TeleportTrigger->OnComponentBeginOverlap.AddDynamic(this, &AMovingTeleporter::OnTeleportBeginOverlap);
	}
}

// Tick while moving the teleporter to the player
void AMovingTeleporter::TickMove()
{
	// Move the capture mesh
	float TimeAlpha = StateTime / MoveTime;
	FVector NewLocation = FMath::Lerp(StartLocation, EndLocation, TimeAlpha);
	CaptureMesh->SetWorldLocation(NewLocation);

	// Keep scaling the mesh
	float NewScale = UDataSingleton::Get().MovingTeleporterMoveCurve->GetFloatValue(StateTime);
	CaptureMesh->SetWorldScale3D(FVector(1.78f * NewScale, NewScale, 1));

	// Update the material
	UpdateMaterialUVs();

	// Finished moving
	if (TimeAlpha >= 1.f)
	{
		URCStatics::LockCamera(this, false);
		Reset();
	}
}

// Activate the teleporter
void AMovingTeleporter::Activate()
{
	CurrentState = EMovingTeleporterState::Rotating;
	StateTime = 0.0f;

	// Get the starting params
	StartLocation = CaptureMesh->GetComponentLocation();
	CameraStartQuat = PlayerController->GetControlRotation().Quaternion();

	// Rotate the whole teleporter actor towards the camera
	ARCCharacter* Player = PlayerController->GetPawn<ARCCharacter>();
	ASSERT_RETURN(Player != nullptr);
	FRotator TeleporterRotation = FRotationMatrix::MakeFromX(StartLocation - Player->GetActorLocation()).Rotator();
	TeleporterRotation.Pitch = 0;
	TeleporterRotation.Roll = 0;
	SetActorRotation(TeleporterRotation);

	// Save off the rotation we need to rotate the player each second
	PlayerDeltaPerSecRotYaw = (TeleportLocation->GetComponentRotation() - Player->GetActorRotation()).Yaw / RotationDuration;

	// Get the local to world for the final camera position
	FTransform LocalToFinalWorld = FTransform(CameraCapture->GetComponentQuat(), Player->GetActorLocation(), Player->GetActorScale());

	// Transform the default camera position to this end rotation and save as our end location
	USpringArmComponent* PlayerBoom = Player->GetCameraBoom();
	ASSERT_RETURN(PlayerBoom != nullptr);
	FVector FinalOffset = PlayerBoom->SocketOffset + PlayerBoom->TargetOffset;
	FinalOffset.X -= PlayerBoom->TargetArmLength;
	FinalOffset = LocalToFinalWorld.TransformPosition(FinalOffset);
	EndLocation = FinalOffset + ((StartLocation - FinalOffset).GetSafeNormal() * TeleporterOffsetMag);

	// Start ticking
	SetActorTickEnabled(true);
}

// On player overlapping with the teleport trigger
void AMovingTeleporter::OnTeleportBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	// Check if it's the player
	if (!URCStatics::IsActorPlayer(OtherActor))
	{
		return;
	}

	// Stop listening
	TeleportTrigger->OnComponentBeginOverlap.RemoveDynamic(this, &AMovingTeleporter::OnTeleportBeginOverlap);

	// Prevent the camera from teleporting
	URCStatics::LockCamera(this, true);

	// Teleport player
	ACharacter* Character = PlayerController->GetCharacter();
	ASSERT_RETURN(Character != nullptr);
	Character->SetActorLocationAndRotation(TeleportLocation->GetComponentLocation(), TeleportLocation->GetComponentQuat());
}

// Reset the teleporter
void AMovingTeleporter::Reset()
{
	CurrentState = EMovingTeleporterState::Idle;
	IdleMesh->SetVisibility(false);
	CaptureMesh->SetVisibility(false);
	CaptureMesh->SetWorldLocation(StartLocation);
	CaptureMesh->SetWorldScale3D(FVector(1.78f, 1, 1));
	bIsIdleHidden = true;

	TeleportTrigger->OnComponentBeginOverlap.RemoveAll(this);
	CameraCapture->bCaptureEveryFrame = false;
	StateTime = 0.0f;

	// Stop ticking
	SetActorTickEnabled(false);
}

// Set whether the idle mesh should be visible
void AMovingTeleporter::SetIdleVisibility(bool bIsVisible)
{
	// No change
	if (bIsIdleHidden != bIsVisible)
	{
		return;
	}

	IdleMesh->SetVisibility(bIsVisible);
	bIsIdleHidden = !bIsVisible;
}

// Set whether this can be activated
void AMovingTeleporter::SetCanBeActivated(bool bCanBeActivatedIn)
{
	// No change
	if (bCanBeActivated == bCanBeActivatedIn)
	{
		return;
	}

	bCanBeActivated = bCanBeActivatedIn;
	IdleMesh->SetMaterial(0, bCanBeActivated ? ActiveMaterial : InactiveMaterial);
}

// Update the scalar parameters for the material to change the crop based on the screen space of the teleporter
void AMovingTeleporter::UpdateMaterialUVs()
{
	ASSERT_RETURN(CaptureMI != nullptr);

	// Get the world location of the min and max of the capture mesh
	const FTransform& CaptureTransform = CaptureMesh->GetComponentTransform();
	FVector CaptureMin(0), CaptureMax(0);
	CaptureMesh->GetLocalBounds(CaptureMin, CaptureMax);
	CaptureMin = CaptureTransform.TransformPosition(CaptureMin);
	CaptureMax = CaptureTransform.TransformPosition(CaptureMax);

	// Get the location in screen space
	FVector2D CaptureMinScreen, CaptureMaxScreen;
	if (!PlayerController->ProjectWorldLocationToScreen(CaptureMin, CaptureMinScreen) || 
		!PlayerController->ProjectWorldLocationToScreen(CaptureMax, CaptureMaxScreen))
	{
		LOG_RETURN(false, LogTeleporter, Error, "Unable to project world location to screen. The material will be messed up.");
	}

	// Get the size of the screen
	int32 ViewportX = 0, ViewportY = 0;
	PlayerController->GetViewportSize(ViewportX, ViewportY);

	// Calculate the percentage from 0.0 - 1.0 across the screen
	CaptureMinScreen.Set(CaptureMinScreen.X / ViewportX, CaptureMinScreen.Y / ViewportY);
	CaptureMaxScreen.Set(CaptureMaxScreen.X / ViewportX, CaptureMaxScreen.Y / ViewportY);

	// Set the params
	CaptureMI->SetScalarParameterValue(MaterialParamXMin, CaptureMinScreen.X);
	CaptureMI->SetScalarParameterValue(MaterialParamXMax, CaptureMaxScreen.X);
	CaptureMI->SetScalarParameterValue(MaterialParamYMin, CaptureMinScreen.Y);
	CaptureMI->SetScalarParameterValue(MaterialParamYMax, CaptureMaxScreen.Y);
}

/*****************************
 *
 * UTeleporterSubsystem
 * 
 *****************************/

 // Listen for when the world is done loading
void UTeleporterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PostWorldInitDH = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UTeleporterSubsystem::OnPostWorldInit);
}

// Make sure to remove our listener
void UTeleporterSubsystem::Deinitialize()
{
	Super::Deinitialize();
	if (PostWorldInitDH.IsValid())
	{
		FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitDH);
	}
}

// Called once the world has initialized
void UTeleporterSubsystem::OnPostWorldInit(UWorld* World, const UWorld::InitializationValues)
{
	if (World == nullptr || !World->IsGameWorld())
	{
		return;
	}

	// Cache all the teleporters to prevent gathering all of them each tick
	for (TActorIterator<AMovingTeleporter> Iter(World); Iter; ++Iter)
	{
		Teleporters.Add(*Iter);
	}
	bTeleportersInitialized = true;
	FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitDH);
}

// Whether this subsystem should tick
bool UTeleporterSubsystem::IsTickable() const
{
	// Don't tick if we've checked and there's no teleporters
	return (!bTeleportersInitialized || Teleporters.Num() != 0) && Super::IsTickable();
}

// Tells the best teleporter to activate
bool UTeleporterSubsystem::ActivateBestTeleporter()
{
	// No teleporter to activate
	if (BestTeleporter == nullptr)
	{
		return false;
	}

	ActiveTeleporter = BestTeleporter;
	ActiveTeleporter->Activate();
	return true;
}

// Update the activatable teleporter
void UTeleporterSubsystem::Tick(float DeltaTime)
{
	// If there's an active teleporter, check to see if it's done
	if (ActiveTeleporter != nullptr)
	{
		if (ActiveTeleporter->IsActive())
		{
			return;
		}
		OnTeleportFinished().ExecuteIfBound();		
		ActiveTeleporter = nullptr;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController == nullptr || PlayerController->PlayerCameraManager == nullptr)
	{
		return;
	}

	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Player == nullptr)
	{
		return;
	}

	FVector CamForward = PlayerController->GetControlRotation().Vector();
	const FVector& PlayerLocation = Player->GetActorLocation();
	const FVector& CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();

	float BestDistance = TNumericLimits<float>::Max();
	float BestAngle = BestDistance;

	// Hide or show the idle mesh dependent on distance
	// Rotate the idle mesh towards the player if applicable
	// Returns true if this is the best teleporter so far
	auto TickTeleporter = [&](AMovingTeleporter* Teleporter, float& Distance, float& Angle) -> bool
	{
		// If the teleporter is currently hidden, then see if the player is far enough to unhide
		if (!Teleporter->IsIdleVisibile())
		{
			FVector TeleporterToPlayer = PlayerLocation - Teleporter->GetActorLocation();
			float DistSquared = TeleporterToPlayer.SizeSquared();
			if (DistSquared >= AMovingTeleporter::MinActiveDistanceSqr)
			{
				Teleporter->SetIdleVisibility(true);
			}
			// No more processing will be done this frame to make sure the player is looking at it
			return false;
		}

		// If this isn't hidden and not rendered, then don't do anything
		if (!Teleporter->WasRecentlyRendered())
		{
			return false;
		}

		const FVector& TeleporterLocation = Teleporter->GetActorLocation();
		FVector TeleporterToPlayer = PlayerLocation - TeleporterLocation;
		Distance = TeleporterToPlayer.SizeSquared();

		// Hide the mesh if the player is too close
		if (Distance < AMovingTeleporter::MinActiveDistanceSqr)
		{
			Teleporter->SetIdleVisibility(false);
			return false;
		}

		// Rotate the idle towards the player
		FRotator RotatorToPlayer = FRotationMatrix::MakeFromX(TeleporterToPlayer).Rotator();
		RotatorToPlayer.Pitch = 0;
		RotatorToPlayer.Roll = 0;
		Teleporter->SetActorRotation(RotatorToPlayer);

		// If the player is too far away
		if (Distance > AMovingTeleporter::MaxActiveDistanceSqr)
		{
			return false;
		}

		// If they are look too far away
		FVector CamToTeleporter = TeleporterLocation - CameraLocation;
		CamToTeleporter.Normalize();
		Angle = FMath::Acos((CamForward | CamToTeleporter) / (CamForward.Size() * CamToTeleporter.Size()));
		if (Angle > AMovingTeleporter::MaxActiveAngle)
		{
			return false;
		}

		// Determine if this teleporter is the best
		// If the angles are close to the center
		// or their angles are very similar
		static const float CloseAngle = FMath::DegreesToRadians(2.5f);
		if ((FMath::Abs(Angle) <= CloseAngle && FMath::Abs(BestAngle) <= CloseAngle) ||
			(FMath::Abs(Angle - BestAngle) <= CloseAngle))
		{
			// Go with the closer one
			if (Distance >= BestDistance)
			{
				return false;
			}
		}
		else if (FMath::Abs(Angle) >= FMath::Abs(BestAngle))
		{
			// Go with the better angle
			return false;

		}
		return true;
	};

	// Loop through all teleporters and see which, if any, are activatable
	// Also rotate the idle mesh towards the player if it's on screen
	BestTeleporter = nullptr;
	float CurrentDistance, CurrentAngle;
	for (AMovingTeleporter* Teleporter : Teleporters)
	{
		if (TickTeleporter(Teleporter, CurrentDistance, CurrentAngle))
		{
			// Make the no longer best teleporter unactivatable
			if (BestTeleporter != nullptr)
			{
				BestTeleporter->SetCanBeActivated(false);
			}
			BestTeleporter = Teleporter;
			BestDistance = CurrentDistance;
			BestAngle = CurrentAngle;
		}
		else
		{
			// Isn't the best, set can't be activated
			Teleporter->SetCanBeActivated(false);
		}
	}

	// Set the best teleporter as activatable
	if (BestTeleporter != nullptr)
	{
		BestTeleporter->SetCanBeActivated(true);
	}
}