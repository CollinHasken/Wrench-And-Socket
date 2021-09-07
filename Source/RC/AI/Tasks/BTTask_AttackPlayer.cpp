// Fill out your copyright notice in the Description page of Project Settings.
#include "RC/AI/Tasks/BTTask_AttackPlayer.h"

#include "AIController.h"

#include "RC/Debug/Debug.h"
#include "RC/Characters/Enemies/BaseEnemy.h"

UBTTask_AttackPlayer::UBTTask_AttackPlayer(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("Attack Player");
}

EBTNodeResult::Type UBTTask_AttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory)
{
	AAIController* Controller = Cast<AAIController>(OwnerComponent.GetOwner());
	ASSERT_RETURN_VALUE(Controller != nullptr, EBTNodeResult::Failed);

	AActor* Pawn = Controller->GetPawn();
	ASSERT_RETURN_VALUE(Pawn != nullptr, EBTNodeResult::Failed);

	ABaseEnemy* Enemy = Cast<ABaseEnemy>(Pawn);
	ASSERT_RETURN_VALUE(Enemy != nullptr, EBTNodeResult::Failed);

	Enemy->AttackPlayer();

	return EBTNodeResult::Succeeded;
}
