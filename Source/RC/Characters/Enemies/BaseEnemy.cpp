// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseEnemy.h"

#include "RC/AI/SplineFollowerComponent.h"

// Sets default values
ABaseEnemy::ABaseEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SplineFollower = CreateDefaultSubobject<USplineFollowerComponent>(TEXT("Spline Follower"));
}

