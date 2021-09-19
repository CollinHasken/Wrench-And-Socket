// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AdvanceToNextPatrolPoint.generated.h"

/**
 * Advance the AI's spline follower to the next patrol point
 */
UCLASS()
class RC_API UBTTask_AdvanceToNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_AdvanceToNextPatrolPoint(const FObjectInitializer& ObjectInitializer);
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory) override;	
};
