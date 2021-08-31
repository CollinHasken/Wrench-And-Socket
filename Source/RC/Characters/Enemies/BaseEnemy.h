// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Characters/BaseCharacter.h"
#include "RC/AI/SplineFollowerComponent.h"
#include "BaseEnemy.generated.h"

UCLASS(Abstract, Blueprintable)
class RC_API ABaseEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseEnemy();

	/** Returns Spline Follower subobject **/
	FORCEINLINE class USplineFollowerComponent* GetSplineFollower() const { return SplineFollower; }

private:
	/** Health */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	class USplineFollowerComponent* SplineFollower;
};
