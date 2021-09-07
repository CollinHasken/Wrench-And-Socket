// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Engine/World.h"

#include "RC/Characters/Enemies/BaseEnemy.h"
#include "RC/AI/BlackBoardKeys.h"
#include "RC/Debug/Debug.h"
#include "RC/Characters/Player/RCCharacter.h"

const FName ABaseAIController::AIMessage_StateChangeFinished = TEXT("StateChangeFinished");

ABaseAIController::ABaseAIController(const FObjectInitializer& ObjectInitializer/* = FObjectInitializer::Get()*/)
{
	BehaviorTreeComponent = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorTreeComponent"));
	BlackboardComponent = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackboardComponent"));

#if WITH_EDITOR
	CurrentStateKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(ABaseAIController, CurrentStateKey), StaticEnum<EAIState>());
	RequestedStateKey.AddEnumFilter(this, GET_MEMBER_NAME_CHECKED(ABaseAIController, RequestedStateKey), StaticEnum<EAIState>());
#endif

	SetupPerception();
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();

	ASSERT_RETURN(BehaviorTree != nullptr);
	ASSERT_RETURN(RunBehaviorTree(BehaviorTree));

	CurrentState = DefaultState;

	SetupStateChangeFunctions();
}

void ABaseAIController::OnPossess(APawn* const PossessedPawn)
{
	Super::OnPossess(PossessedPawn);

	if (Blackboard)
	{
		ASSERT_RETURN(BehaviorTree != nullptr);
		ASSERT_RETURN(Blackboard->InitializeBlackboard(*BehaviorTree->BlackboardAsset));
	}
}

/** Return the blackboard asset */
UBlackboardData* ABaseAIController::GetBlackboardAsset() const
{
	ASSERT_RETURN_VALUE(BlackboardComponent != nullptr, nullptr);
	if (BlackboardComponent->HasValidAsset())
	{
		return BlackboardComponent->GetBlackboardAsset();
	}
	return BehaviorTree != nullptr ? BehaviorTree->GetBlackboardAsset() : nullptr;
}

/** Add the new state transition functions to the map */
void ABaseAIController::AddStateChangeFunctions(const TStateChangeFunctionMap& Functions)
{
	for (const TPair<EAIState, TNewStateChangeFunctionMap> NewFunctionMapPair : Functions)
	{
		// If there's an existing map for a state, then we need to add the new transitions to that map
		TNewStateChangeFunctionMap* CurrentFunctionMap = StateChangeFunctions.Find(NewFunctionMapPair.Key);
		if (CurrentFunctionMap != nullptr)
		{
			CurrentFunctionMap->Append(NewFunctionMapPair.Value);
		}
		else
		{
			// Add map for new states
			StateChangeFunctions.Add(NewFunctionMapPair);
		}
	}
}

/** Create new state transition function map */
void ABaseAIController::SetupStateChangeFunctions()
{
	TNewStateChangeFunctionMap IdleMap;	
	IdleMap.Emplace(EAIState::Patrol, new FStateChangePredicate<ABaseAIController>(this, &ABaseAIController::test1));
	StateChangeFunctions.Add(EAIState::Idle, IdleMap);
}

void ABaseAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

EAIStateChangeResult ABaseAIController::RequestState(EAIState NewState)
{
	ASSERT_RETURN_VALUE(BlackboardComponent != nullptr, EAIStateChangeResult::Failed);

	TNewStateChangeFunctionMap* NewStateFunctionMap = StateChangeFunctions.Find(CurrentState);
	if (NewStateFunctionMap != nullptr)
	{
		TSharedPtr<FStateChangePredicateBase>* StateChangePredicate = NewStateFunctionMap->Find(NewState);
		if (StateChangePredicate != nullptr && *StateChangePredicate != nullptr)
		{
			EAIStateChangeResult Result = (**StateChangePredicate)();

			switch (Result)
			{
				case EAIStateChangeResult::Failed:
					LOG_RETURN_VALUE(false, Result, LogAI, Warning, "State change for %s failed from %s to %s", *(GetOwner()->GetName()), *(StaticEnum<EAIState>()->GetValueAsString(CurrentState)), *(StaticEnum<EAIState>()->GetValueAsString(NewState)));
				case EAIStateChangeResult::InProgress:
					// The state transition will actually change the state
					BlackboardComponent->SetValue<UBlackboardKeyType_Enum>(RequestedStateKey.GetSelectedKeyID(), static_cast<UBlackboardKeyType_Enum::FDataType>(NewState));
					RequestedState = NewState;
					return Result;
				case EAIStateChangeResult::Succeeded:
				default:
					break;
			}
		}
	}
	// Changed states immediately, set both the requested and current state
	BlackboardComponent->SetValue<UBlackboardKeyType_Enum>(RequestedStateKey.GetSelectedKeyID(), static_cast<UBlackboardKeyType_Enum::FDataType>(NewState));
	BlackboardComponent->SetValue<UBlackboardKeyType_Enum>(CurrentStateKey.GetSelectedKeyID(), static_cast<UBlackboardKeyType_Enum::FDataType>(NewState));
	CurrentState = NewState;
	return EAIStateChangeResult::Succeeded;
}

void ABaseAIController::FinishStateChange()
{
	FAIMessage Msg(ABaseAIController::AIMessage_StateChangeFinished, this, true);
	FAIMessage::Send(static_cast<AController*>(this), Msg);

	// Update state
	ASSERT_RETURN(BlackboardComponent != nullptr);
	BlackboardComponent->SetValue<UBlackboardKeyType_Enum>(CurrentStateKey.GetSelectedKeyID(), static_cast<UBlackboardKeyType_Enum::FDataType>(RequestedState));
	CurrentState = RequestedState;
}

void ABaseAIController::OnTargetDetected(AActor* DetectedActor, const FAIStimulus stimulus)
{
	if (stimulus.WasSuccessfullySensed())
	{
		GetBlackBoard()->SetValueAsObject(BlackBoardKeys::LAST_DETECTED_TARGET_ACTOR, DetectedActor);
	}

	if (stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (stimulus.WasSuccessfullySensed())
		{
			RequestState(EAIState::Combat);
		}
	}
	else if (stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		ARCCharacter* Player = Cast<ARCCharacter>(DetectedActor);
		if (Player != nullptr)
		{
			if (stimulus.WasSuccessfullySensed())
			{
				RequestState(EAIState::Combat);
			}
			else
			{
				RequestState(EAIState::Search);
			}
			GetBlackBoard()->SetValueAsBool(BlackBoardKeys::CAN_SEE_PLAYER, stimulus.WasSuccessfullySensed());
		}
	}
}

void ABaseAIController::SetupPerception()
{
	// Create and initialize sight configuration object
	UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component"));
	ASSERT_RETURN(Perception != nullptr);
	SetPerceptionComponent(*Perception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	ASSERT_RETURN(SightConfig != nullptr);
	SightConfig->SightRadius = 800.f;
	SightConfig->LoseSightRadius = SightConfig->SightRadius + 50.f;
	SightConfig->PeripheralVisionAngleDegrees = 75.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("Damage Config"));
	ASSERT_RETURN(DamageConfig != nullptr);

	// Add config to component
	Perception->ConfigureSense(*SightConfig);
	Perception->ConfigureSense(*DamageConfig);
	Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnTargetDetected);
}
