// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_FindPlayerLocation.h"

#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

#include "RC/AI/BaseAIController.h"
#include "RC/AI/BlackBoardKeys.h"
#include "RC/Debug/Debug.h"

UBTTask_FindPlayerLocation::UBTTask_FindPlayerLocation(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("Find Player Location");
}

// Find the player's current location
EBTNodeResult::Type UBTTask_FindPlayerLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory)
{
	UBlackboardComponent* BlackBoardComponent = OwnerComponent.GetBlackboardComponent();
	ASSERT_RETURN_VALUE(BlackBoardComponent != nullptr, EBTNodeResult::Failed);

	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	ASSERT_RETURN_VALUE(Player != nullptr, EBTNodeResult::Failed);
	
	AAIController* AIController = OwnerComponent.GetAIOwner();
	ASSERT_RETURN_VALUE(AIController != nullptr, EBTNodeResult::Failed);
	const FNavAgentProperties& AgentProps = AIController->GetNavAgentPropertiesRef();

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	ASSERT_RETURN_VALUE(NavSys != nullptr, EBTNodeResult::Failed);

	FNavLocation ProjectedLocation;
	EBTNodeResult::Type Result = EBTNodeResult::Failed;
	if (NavSys && NavSys->ProjectPointToNavigation(Player->GetActorLocation(), ProjectedLocation, INVALID_NAVEXTENT, &AgentProps))
	{
		BlackBoardComponent->SetValue<UBlackboardKeyType_Vector>(BlackboardKey.GetSelectedKeyID(), ProjectedLocation);
		Result = EBTNodeResult::Succeeded;
	}

	return Result;
}
