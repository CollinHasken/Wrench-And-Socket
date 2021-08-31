// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_AdvanceToNextPatrolPoint.h"

#include "AIModule/Classes/AIController.h"

#include "RC/Characters/Enemies/BaseEnemy.h"
#include "RC/Debug/Debug.h"
#include "RC/AI/SplineFollowerComponent.h"

UBTTask_AdvanceToNextPatrolPoint::UBTTask_AdvanceToNextPatrolPoint(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("Advance To Next Patrol Point");
}

EBTNodeResult::Type UBTTask_AdvanceToNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory)
{
	AAIController* Controller = Cast<AAIController>(OwnerComponent.GetOwner());
	ASSERT_RETURN_VALUE(Controller != nullptr, EBTNodeResult::Failed);

	AActor* Pawn = Controller->GetPawn();
	ASSERT_RETURN_VALUE(Pawn != nullptr, EBTNodeResult::Failed);
	
	ABaseEnemy* BaseEnemy = Cast<ABaseEnemy>(Pawn);

	USplineFollowerComponent* SplineFollower = BaseEnemy != nullptr ? BaseEnemy->GetSplineFollower() : Pawn->FindComponentByClass<USplineFollowerComponent>();
	LOG_RETURN_VALUE(SplineFollower != nullptr, EBTNodeResult::Failed, LogAISpline, Warning, "No spline follower for actor %s", *Pawn->GetName());

	SplineFollower->AdvanceToNextPatrolPoint();

	return EBTNodeResult::Succeeded;
}
