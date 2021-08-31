// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "SplineFollowerComponent.generated.h"

UCLASS( ClassGroup=(AI), config = Game, meta=(BlueprintSpawnableComponent) )
class RC_API USplineFollowerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USplineFollowerComponent();


	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*
	 * Get the position on the spline that is closest to the actor of this component
	 *
	 * @param ClosestKey The closest key the position on the spline is
	 * Returns the closest position
	 */
	FVector GetClosestPositionOnSpline(uint8* ClosestKey = nullptr);

	/*
	 * Get the position of the current patrol point
	 *
	 * Returns the closest position
	 */
	FVector GetCurrentPatrolPosition() const;

	/* Advance the follower to the next point on the spline */
	void AdvanceToNextPatrolPoint();

private:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (UseComponentPicker, AllowedClasses = "SplineComponent", AllowAnyActor, AllowPrivateAccess = "true"))
	FComponentReference SplineComponentRef;
	class USplineComponent* SplineComponent;

	uint8 CurrentTargetPoint = 0;
	FVector CurrentTargetPosition = FVector::ZeroVector;

	// After the component has initialized
	virtual void PostLoad() override;

	// Called when the game starts
	virtual void BeginPlay() override;

	// Get the spline point index that is closest to the actor of this compoennt
	uint8 GetClosestPoint();
};
