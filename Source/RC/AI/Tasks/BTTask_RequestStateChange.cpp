// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RequestStateChange.h"

#include "AIModule/Classes/AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "RC/AI/BaseAIController.h"
#include "RC/Util/RCTypes.h"
#include "RC/Debug/Debug.h"

UBTTask_RequestStateChange::UBTTask_RequestStateChange(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("Request State Change");
}

// Request the AI to have a state change
EBTNodeResult::Type UBTTask_RequestStateChange::ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory)
{
	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComponent.GetAIOwner());
	ASSERT_RETURN_VALUE(AIController != nullptr, EBTNodeResult::Failed);

	EAIStateChangeResult Result = AIController->RequestState(RequestedState);

	switch (Result)
	{
		case EAIStateChangeResult::Failed:
			return EBTNodeResult::Failed;
		case EAIStateChangeResult::InProgress:
			// Set the requested state and process
			WaitForMessage(OwnerComponent, ABaseAIController::AIMessage_StateChangeFinished);
			return EBTNodeResult::InProgress;
		case EAIStateChangeResult::Succeeded:
		default:
			break;
	}
	return EBTNodeResult::Succeeded;
}
