// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "MovingTeleporter.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTeleporter, Log, All);

// Executed when the teleport has finished
DECLARE_DELEGATE(FOnTeleportFinished);

/**
 * States the moving teleporter can be in while teleporting
 */
UENUM()
enum class EMovingTeleporterState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Rotating	UMETA(DisplayName = "Rotating"),
	Growing		UMETA(DisplayName = "Growing"),
	Moving		UMETA(DisplayName = "Moving"),
};

/**
 * A teleporter that will show a preview of the teleport location as the
 * teleporter moves towards the camera, causing a seamless teleport
 */
UCLASS()
class RC_API AMovingTeleporter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMovingTeleporter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Activate the teleporter
	UFUNCTION(BlueprintCallable)
	void Activate();

	// Whether this teleporter is currently active
	UFUNCTION(BlueprintPure)
	bool IsActive() const { return CurrentState != EMovingTeleporterState::Idle; }

	// Returns Idle Mesh subobject 
	FORCEINLINE class UStaticMeshComponent* GetIdleMesh() const { return IdleMesh; }

	// Returns Capture Plane Mesh subobject 
	FORCEINLINE class UStaticMeshComponent* GetCaptureMesh() const { return CaptureMesh; }

	// Returns Teleport Trigger subobject 
	FORCEINLINE class UBoxComponent* GetTeleportTrigger() const { return TeleportTrigger; }

	// Returns Teleport Location subobject 
	FORCEINLINE class USceneComponent* GetTeleportLocation() const { return TeleportLocation; }

	// Returns Camera Base Loction subobject 
	FORCEINLINE class USceneComponent* GetCameraBaseLocation() const { return CameraBaseLocation; }

	// Returns Camera Boom subobject 
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	// Returns Camera Capture subobject 
	FORCEINLINE class USceneCaptureComponent2D* GetCameraCapture() const { return CameraCapture; }

protected:
	friend class UTeleporterSubsystem;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Tick while rotating the player camera towards the teleporter
	void TickRotate(float DeltaTime);

	// Tick while growing the teleporter
	void TickGrow();

	// Tick while moving the teleporter to the player
	void TickMove();
	
	// On player overlapping with the teleport trigger
	UFUNCTION()
	void OnTeleportBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Reset the teleporter
	void Reset();

	// Set whether the idle mesh should be visible
	void SetIdleVisibility(bool bIsVisible);

	// Return whether the idle mesh is visible
	bool IsIdleVisibile() const { return !bIsIdleHidden; }

	// Set whether this can be activated
	void SetCanBeActivated(bool bCanBeActivatedIn);

	// Update the scalar parameters for the material to change the crop based on the screen space of the teleporter
	void UpdateMaterialUVs();

	// The dynamic material instance showing the capture
	UPROPERTY()
	class UMaterialInstanceDynamic* CaptureMI = nullptr;
	
	// Current state
	UPROPERTY(BlueprintReadOnly, Category = State, meta = (AllowPrivateAccess = "true"))
	EMovingTeleporterState CurrentState = EMovingTeleporterState::Idle;
	
	// The starting location to lerp from
	FVector StartLocation;

	// The end location to lerp to
	FVector EndLocation;

	// The starting quat for the camerea to lerp from
	FQuat CameraStartQuat;

	// The delta needed to add each second to rotate the player towards the post teleport state
	float PlayerDeltaPerSecRotYaw;

	// The time in the state
	float StateTime = 0.0f;

private:
	// Base 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Base;

	// Idle Mesh to show when not teleporting
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* IdleMesh;

	// Capture Plane Mesh to show the capture on
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* CaptureMesh;

	// Teleport Trigger to tell when the player should teleport
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Teleport, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* TeleportTrigger;

	// Teleport Location the transform to teleport the player to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Teleport, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* TeleportLocation;

	// Camera Base Loction for camera position
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* CameraBaseLocation;

	// Camera Boom for camera positioning
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	// Camera Capture to capture the world at the teleport location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USceneCaptureComponent2D* CameraCapture;

	// Inactive Material the material to show when the teleporter isn't active
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* InactiveMaterial;

	// Active Material the material to show when the teleporter is active
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* ActiveMaterial;

	// Capture Material the material that's showing the capture
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UMaterialInterface* CaptureMaterial;

	// Duration to rotate the camera and player towards the teleporter
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float RotationDuration = 0.5f;

	// Bool for whether this can be activated right now
	bool bCanBeActivated = false;

	// Bool for whether to hide the idle mesh
	bool bIsIdleHidden = false;

	// Reference to the player controller
	class APlayerController* PlayerController = nullptr;

	// Time it takes to move the teleporter. Matches the duration of the move curve
	float MoveTime;

	// Material scalar param names
	static const FName MaterialParamXMin;
	static const FName MaterialParamXMax;
	static const FName MaterialParamYMin;
	static const FName MaterialParamYMax;

	// Teleporter end offset
	static const float TeleporterOffsetMag;

	// End scale for growing
	static const float GrowScale;

	// Minimum distance the player has to be for this to be active
	static const float MinActiveDistanceSqr;

	// Maximum distance the player has to be for this to be active
	static const float MaxActiveDistanceSqr;

	// Maximum angle the player can look away from this to be active
	static const float MaxActiveAngle;
};

/**
 * A subsystem to compare the teleporters in the world and choose
 * the best candidate that the player can teleport to
 */
UCLASS()
class UTeleporterSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	// Begin USubsystem
	// Listen for when the world is done loading
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Make sure to remove our listener
	virtual void Deinitialize() override;
	// End USubsystem
	
	// FTickableGameObject implementation Begin
	// Whether this subsystem should tick
	virtual bool IsTickable() const override;

	// Update the activatable teleporter
	virtual void Tick(float DeltaTime) override;

	// Needed for tickables
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UTeleporterSubsystem, STATGROUP_Tickables); }
	// FTickableGameObject implementation End

	/*
	 * Tells the best teleporter to activate
	 * Returns whether a teleporter was activated
	 */
	UFUNCTION(BlueprintCallable)
	bool ActivateBestTeleporter();

	// Get the Teleport Finished delegate
	FOnTeleportFinished& OnTeleportFinished() { return TeleportFinishedDelegate; }

private:
	// Called once the world has initialized
	void OnPostWorldInit(UWorld* World, const UWorld::InitializationValues);

	// Post world init delegate handle
	FDelegateHandle PostWorldInitDH;

	// Our current best teleporter
	AMovingTeleporter* BestTeleporter = nullptr;

	// The current active teleporter
	AMovingTeleporter* ActiveTeleporter = nullptr;

	// Array of teleporters in the level
	TArray<AMovingTeleporter*> Teleporters;

	// Whether we've initialized the teleporter array
	bool bTeleportersInitialized = false;

	// Called once the active teleporter has finished
	FOnTeleportFinished TeleportFinishedDelegate;
};
