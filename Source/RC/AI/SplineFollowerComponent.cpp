// Fill out your copyright notice in the Description page of Project Settings.

#include "SplineFollowerComponent.h"

#include "Components/SplineComponent.h"

#include "RC/Debug/Debug.h"

// Sets default values for this component's properties
USplineFollowerComponent::USplineFollowerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;	
}

// Setup values after initial values have been loaded
void USplineFollowerComponent::PostLoad()
{
	Super::PostLoad();

	SplineComponent = Cast<USplineComponent>(SplineComponentRef.GetComponent(nullptr));	
}

// Called when the game starts
void USplineFollowerComponent::BeginPlay()
{
	Super::BeginPlay();
	if (SplineComponent != nullptr)
	{
		CurrentTargetPosition = SplineComponent->GetLocationAtSplinePoint(CurrentTargetPoint, ESplineCoordinateSpace::World);
	}
}


// Called every frame
void USplineFollowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

uint8 USplineFollowerComponent::GetClosestPoint()
{
	ASSERT_RETURN_VALUE(SplineComponent != nullptr, 0);
	return FMath::RoundHalfFromZero(SplineComponent->FindInputKeyClosestToWorldLocation(GetOwner()->GetActorLocation()));
}

FVector USplineFollowerComponent::GetClosestPositionOnSpline(uint8* ClosestKey)
{
	ASSERT_RETURN_VALUE(SplineComponent != nullptr, FVector::ZeroVector);

	// If we want the key
	if (ClosestKey != nullptr)
	{
		float Key = SplineComponent->FindInputKeyClosestToWorldLocation(GetOwner()->GetActorLocation());
		*ClosestKey = static_cast<uint8>(FMath::RoundHalfFromZero(Key));
		return SplineComponent->GetLocationAtSplinePoint(*ClosestKey, ESplineCoordinateSpace::World);
	}

	return SplineComponent->FindLocationClosestToWorldLocation(GetOwner()->GetActorLocation(), ESplineCoordinateSpace::World);
}

FVector USplineFollowerComponent::GetCurrentPatrolPosition() const
{
	return CurrentTargetPosition;
}

void USplineFollowerComponent::AdvanceToNextPatrolPoint()
{
	ASSERT_RETURN(SplineComponent != nullptr);
	int NumberOfPoints = SplineComponent->GetNumberOfSplinePoints();
	LOG_RETURN(NumberOfPoints != 0, LogAISpline, Warning, "Spline %s doesn't have any points", *SplineComponent->GetName());

	CurrentTargetPoint = (CurrentTargetPoint + 1) % SplineComponent->GetNumberOfSplinePoints();
	CurrentTargetPosition = SplineComponent->GetLocationAtSplinePoint(CurrentTargetPoint, ESplineCoordinateSpace::World);
}
