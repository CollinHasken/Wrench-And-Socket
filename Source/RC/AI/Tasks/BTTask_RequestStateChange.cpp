// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_RequestStateChange.h"

#include "AIModule/Classes/AIController.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "RC/Util/RCTypes.h"
#include "RC/Debug/Debug.h"
#include "RC/AI/SplineFollowerComponent.h"

UBTTask_RequestStateChange::UBTTask_RequestStateChange(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("Request State Change");

	BlackboardKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RequestStateChange, BlackboardKey), StaticEnum<EAIState>());
}

EBTNodeResult::Type UBTTask_RequestStateChange::ExecuteTask(UBehaviorTreeComponent& OwnerComponent, uint8* NodeMemory)
{
	UBlackboardComponent* BlackBoardComponent = OwnerComponent.GetBlackboardComponent();
	ASSERT_RETURN_VALUE(BlackBoardComponent != nullptr, EBTNodeResult::Failed);

	BlackBoardComponent->SetValue<UBlackboardKeyType_Enum>(BlackboardKey.GetSelectedKeyID(), static_cast<UBlackboardKeyType_Enum::FDataType>(RequestedState));

	return EBTNodeResult::Succeeded;
}
