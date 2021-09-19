// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"

#include "RC/Util/RCTypes.h"

#include "BTTask_RequestStateChange.generated.h"

/**
 * Request the AI to have a state change
 */
UCLASS()
class RC_API UBTTask_RequestStateChange : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_RequestStateChange(const FObjectInitializer& ObjectInitializer);
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory) override;

	/** blackboard key selector */
	UPROPERTY(EditAnywhere, Category = AI)
	EAIState RequestedState;
};
