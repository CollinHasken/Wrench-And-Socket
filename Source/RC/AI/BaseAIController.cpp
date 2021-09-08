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

	SetupStateTransitions();
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
void ABaseAIController::AddStateTransition(const TStateTransitionMap& Transitions)
{
	for (const TPair<EAIState, TSharedPtr<FStateTransitionBase>> NewFunctionTransitionPair : Transitions)
	{
		// If there's an existing transition for a state, then we need to add the new functions to that map
		TSharedPtr<FStateTransitionBase>* CurrentFunctionTransition = StateTransitions.Find(NewFunctionTransitionPair.Key);
		if (CurrentFunctionTransition != nullptr && (NewFunctionTransitionPair.Value) != nullptr)
		{
			if ((NewFunctionTransitionPair.Value)->HasTransitionFunctions())
			{
				/*Need to somehow copy over the self and functions
				maybe append the new change functions to the current, then make the new equal to the current or swap, then set the current equal to the new?
				then can't be const*/
				TNewStateChangeFunctionMap CombinedMap = (*CurrentFunctionTransition)->StateChangeMap;
				CombinedMap.Append(NewFunctionTransitionPair.Value->StateChangeMap);
				(*CurrentFunctionTransition) = NewFunctionTransitionPair.Value;
				(*CurrentFunctionTransition)->StateChangeMap = CombinedMap;
			}
			else
			{
				// No transition functions to worry about
				(*CurrentFunctionTransition)->StateChangeMap.Append((NewFunctionTransitionPair.Value)->StateChangeMap);
			}
		}
		else
		{
			// Add map for new states
			StateTransitions.Add(NewFunctionTransitionPair);
		}
	}
}

/** Create new state transition function map */
void ABaseAIController::SetupStateTransitions()
{
	//TNewStateChangeFunctionMap ChaseMap;
	//ChaseMap.Emplace(EAIState::Patrol, new FStateChangePredicate<ABaseAIController>(this, &ABaseAIController::test1));
	//StateTransitions.Emplace(EAIState::Combat, new FStateTransition<ABaseAIController>(this, &ABaseAIController::UpdatePerceptionSight, CombatMap, &ABaseAIController::UpdatePerceptionSight));
	
	StateTransitions.Emplace(EAIState::Combat, new FStateTransition<ABaseAIController>(this, &ABaseAIController::UpdatePerceptionSight, &ABaseAIController::UpdatePerceptionSight));
	StateTransitions.Emplace(EAIState::Chase, new FStateTransition<ABaseAIController>(this, &ABaseAIController::UpdatePerceptionSight, &ABaseAIController::UpdatePerceptionSight));
}

void ABaseAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

EAIStateChangeResult ABaseAIController::RequestState(EAIState NewState)
{
	if (NewState == CurrentState)
	{
		return EAIStateChangeResult::Succeeded;
	}

	ASSERT_RETURN_VALUE(BlackboardComponent != nullptr, EAIStateChangeResult::Failed);

	TSharedPtr<FStateTransitionBase>* NewStateTransition = StateTransitions.Find(CurrentState);
	if (NewStateTransition != nullptr)
	{
		TSharedPtr<FStateChangePredicateBase>* StateChangePredicate = (*NewStateTransition)->StateChangeMap.Find(NewState);
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
	RequestedState = NewState;
	FinishStateChange();
	return EAIStateChangeResult::Succeeded;
}

void ABaseAIController::OnStateTransitionFinsished()
{
	FAIMessage Msg(ABaseAIController::AIMessage_StateChangeFinished, this, true);
	FAIMessage::Send(static_cast<AController*>(this), Msg);

	FinishStateChange();
}

/** Actually change the state */
void ABaseAIController::FinishStateChange()
{
	// On Exit function
	TSharedPtr<FStateTransitionBase>* OldStateTransition = StateTransitions.Find(CurrentState);
	if (OldStateTransition != nullptr && *OldStateTransition != nullptr)
	{
		(*OldStateTransition)->Exit();
	}	

	CurrentState = RequestedState;
	RequestedState = EAIState::NUM_STATES;

	// Update blackboard keys
	BlackboardComponent->SetValue<UBlackboardKeyType_Enum>(CurrentStateKey.GetSelectedKeyID(), static_cast<UBlackboardKeyType_Enum::FDataType>(CurrentState));
	BlackboardComponent->SetValue<UBlackboardKeyType_Enum>(RequestedStateKey.GetSelectedKeyID(), static_cast<UBlackboardKeyType_Enum::FDataType>(RequestedState));

	// On Enter function
	TSharedPtr<FStateTransitionBase>* NewStateTransition = StateTransitions.Find(CurrentState);
	if (NewStateTransition != nullptr && *NewStateTransition != nullptr)
	{
		(*NewStateTransition)->Enter();
	}
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
	// Updating the sight will configure it
	UpdatePerceptionSight();
	Perception->ConfigureSense(*DamageConfig);
	Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseAIController::OnTargetDetected);
}

void ABaseAIController::UpdatePerceptionSight()
{
	UAIPerceptionComponent* Perception = GetPerceptionComponent();
	ASSERT_RETURN(Perception != nullptr);

	switch (CurrentState)
	{
		case EAIState::Idle:
		case EAIState::Patrol:
		case EAIState::Search:
		case EAIState::NUM_STATES:
			SightConfig->SightRadius = 800.f;
			SightConfig->LoseSightRadius = SightConfig->SightRadius + 50.f;
			break;
		case EAIState::Chase:
		case EAIState::Combat:
			SightConfig->SightRadius = 1400.f;
			SightConfig->LoseSightRadius = SightConfig->SightRadius + 100.f;
			break;
		default:
			break;
	}

	// Need to reconfigure to let the sense know values have been changed
	Perception->ConfigureSense(*SightConfig);

	// Update BB
	BlackboardComponent->SetValueAsFloat(BlackBoardKeys::SIGHT_DISTANCE, SightConfig->SightRadius);
}
