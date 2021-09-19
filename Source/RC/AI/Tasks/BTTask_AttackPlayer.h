// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_AttackPlayer.generated.h"

/**
 * Attack the player
 */
UCLASS()
class RC_API UBTTask_AttackPlayer : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_AttackPlayer(const FObjectInitializer& ObjectInitializer);
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory) override;
};
