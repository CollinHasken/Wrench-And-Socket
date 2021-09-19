// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_GetClosestPatrolPosition.h"

#include "AIModule/Classes/AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "RC/Characters/Enemies/BaseEnemy.h"
#include "RC/Debug/Debug.h"
#include "RC/AI/SplineFollowerComponent.h"

UBTTask_GetClosestPatrolPosition::UBTTask_GetClosestPatrolPosition(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("Get Closest Patrol Position");

	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_GetClosestPatrolPosition, BlackboardKey));
}

// Get the closest patrol point of the AI's spline follower
EBTNodeResult::Type UBTTask_GetClosestPatrolPosition::ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory)
{
	UBlackboardComponent* BlackBoardComponent = OwnerComponent.GetBlackboardComponent();
	ASSERT_RETURN_VALUE(BlackBoardComponent != nullptr, EBTNodeResult::Failed);

	AAIController* Controller = Cast<AAIController>(OwnerComponent.GetOwner());
	ASSERT_RETURN_VALUE(Controller != nullptr, EBTNodeResult::Failed);

	AActor* Pawn = Controller->GetPawn();
	ASSERT_RETURN_VALUE(Pawn != nullptr, EBTNodeResult::Failed);

	ABaseEnemy* BaseEnemy = Cast<ABaseEnemy>(Pawn);

	USplineFollowerComponent* SplineFollower = BaseEnemy != nullptr ? BaseEnemy->GetSplineFollower() : Pawn->FindComponentByClass<USplineFollowerComponent>();
	LOG_RETURN_VALUE(SplineFollower != nullptr, EBTNodeResult::Failed, LogAISpline, Warning, "No spline follower for actor %s", *Pawn->GetName());

	BlackBoardComponent->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), SplineFollower->GetClosestPositionOnSpline());

	return EBTNodeResult::Succeeded;
}
