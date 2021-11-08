// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"

#include "RC/Debug/Debug.h"
#include "RC/Framework/AssetDataInterface.h"
#include "RC/Util/TimeStamp.h"

#include "Collectible.generated.h"

/**
* Config for every weapon
*/
UCLASS(BlueprintType)
class RC_API UCollectibleInfo : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// The amount granted upon collecting this
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Collectible, meta = (AllowPrivateAccess))
	int CollectionAmount = 1;

	// The minimum amount of time the collectible will travel in the air
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Collectible, meta = (AllowPrivateAccess))
	float TravelTimeMin = .8f;

	// The maximum amount of time the collectible will travel in the air
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Collectible, meta = (AllowPrivateAccess))
	float TravelTimeMax = 1.2f;

	// Time to delay between spawning and when it can be sucked in for collection
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Collectible, meta = (AllowPrivateAccess))
	float CollectionDelay = 2;
};

UCLASS()
class RC_API ACollectible : public AActor, public IAssetDataInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACollectible();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*
	 * Start the collection process of moving towards the target
	 * 
	 * @param Target	The target to collect towards
	 */	
	UFUNCTION(BlueprintCallable)
	void StartCollecting(class AActor* Target);

	/*
	 * Called to collect this collectible
	 * @Return The amount collected
	 */
	virtual int Collect();

	// Returns Mesh subobject
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Mesh; }

	// Get the Primary Data Asset Id associated with this data actor's data.
	UFUNCTION(BlueprintPure)
	FPrimaryAssetId GetInfoId() const override { return CollectibleInfo != nullptr ? CollectibleInfo->GetPrimaryAssetId() : FPrimaryAssetId(); }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Pointer to our target we're moving towards
	TWeakObjectPtr<class AActor> CurrentTargetW = nullptr;

private:
	// Get the amount to be granted when this has been collected
	virtual int GetCollectionAmount() const { return CollectibleInfo != nullptr ? CollectibleInfo->CollectionAmount : 0; }

	/**
	 * Find the location along the quadratic given the percentage along the arc
	 *
	 * @param FoundLocation			(Output) The found location
	 //* @param DistancePercentage	The percentage along the arc to find the location
	 //* @param StartToTargetXY		The XY direction (normalized) from the start of the arc to the target
	 * @param TargetDist			The distance from the start of the arc to the target
	 * @param A						the A coefficient for the quadratic
	 * @param B						the B coefficient for the quadratic
	 */
	void FindRelativeTravelLocation(FVector* FoundLocation, float DistancePercentage, const FVector2D& StartToTargetXY, float TargetDist, float A, float B) const;

	/**
	 * Find the location along the quadratic given the percentage along the arc
	 *
	 * @param FoundLocation			(Output) The found location
	 * @param DistancePercentage	The percentage along the arc to find the location
	 * @param StartLocation			The start location for the arc
	 * @Return True if the location was found. Will fail if we're unable to find the A and B for the quadratic or the current target isn't valid
	 */
	bool FindRelativeTravelLocation(FVector* FoundLocation, float DistancePercentage, const FVector& StartLocation) const;

	/**
	 * Find the location along the quadratic given the percentage along the arc using the Collect Start Location
	 *
	 * @param FoundLocation			(Output) The found location
	 * @param DistancePercentage	The percentage along the arc to find the location
	 * @Return True if the location was found. Will fail if we're unable to find the A and B for the quadratic or the current target isn't valid
	 */
	bool FindRelativeTravelLocation(FVector* FoundLocation, float DistancePercentage) const { return FindRelativeTravelLocation(FoundLocation, DistancePercentage, CollectStartLocation); }

	/**
	 * Find the location along the quadratic given the percentage along the arc in world space
	 *
	 * @param FoundLocation			(Output) The found location
	 * @param DistancePercentage	The percentage along the arc to find the location
	 * @param StartLocation			The start location for the arcTargetDist
	 * @param TargetDist			The distance from the start of the arc to the target
	 * @param A						the A coefficient for the quadratic
	 * @param B						the B coefficient for the quadratic
	 */
	void FindTravelLocation(FVector* FoundLocation, float DistancePercentage, const FVector& StartLocation, const FVector2D& StartToTargetXY, float TargetDist, float A, float B) const;

	/**
	 * Find the location along the quadratic given the percentage along the arc in world space
	 *
	 * @param FoundLocation			(Output) The found location
	 * @param DistancePercentage	The percentage along the arc to find the location
	 * @param StartLocation			The start location for the arc
	 * @Return True if the location was found. Will fail if we're unable to find the A and B for the quadratic or the current target isn't valid
	 */
	bool FindTravelLocation(FVector* FoundLocation, float DistancePercentage, const FVector& StartLocation) const;

	/**
	 * Find the location along the quadratic given the percentage along the arc using the Collect Start Location in world space
	 *
	 * @param FoundLocation			(Output) The found location
	 * @param DistancePercentage	The percentage along the arc to find the location
	 * @Return True if the location was found. Will fail if we're unable to find the A and B for the quadratic or the current target isn't valid
	 */
	bool FindTravelLocation(FVector* FoundLocation, float DistancePercentage) const { return FindTravelLocation(FoundLocation, DistancePercentage, CollectStartLocation);}

	/**
	 * Solve the quadratic y = Ax^2 + Bx for the stored start position to the target
	 * 
	 * @param A					(Output) the A coefficient
	 * @param B					(Output) the B coefficient
	 * @param StartLocation		The start location for the arc
	 * @param TargetLocation	The target location we're going to
	 */
	void FindQuadraticCoefficients(float* A, float* B, const FVector& StartLocation, const FVector& TargetLocation) const;

	/**
	 * Solve the quadratic y = Ax^2 + Bx for the stored start position to the current target
	 *
	 * @param A	(Output) the A coefficient
	 * @param B	(Output) the B coefficient
	 * @Return true if the value was retrieved, false if the target is no longer valid
	 */
	bool FindQuadraticCoefficients(float* A, float* B) const;

	// The collectible config
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	UCollectibleInfo* CollectibleInfo = nullptr;

	// The collectible data
	class UCollectibleData* CollectibleData = nullptr;

	// Start location for the collection
	FVector CollectStartLocation = FVector::ZeroVector;

	// Current time that this has been collecting
	float CurrentTravelTime = -1;

	// Inverse of the max time for multiplying instead of dividing
	float CurrentTravelTimeMaxInv = -1;

	// Whether it's currently traveling
	bool bIsTraveling = false;

	// Whether this was collected early and should start the collection once the delay has ended
	bool bDelayCollection = false;

	// Timer for delaying when we can be collected
	FTimeStamp CollectDelayHandle;

	// The collectible mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh = nullptr;

	// The trigger to detect the collectible
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollectTrigger = nullptr;

#if DEBUG_ENABLED
	FVector DebugStartLocation = FVector::ZeroVector;
	void Debug(float DeltaTime);
#endif
};
