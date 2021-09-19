// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/BlackboardAssetProvider.h"

#include "RC/Util/RCTypes.h"

#include "BaseAIController.generated.h"

/**
 * Result of state change attempt
 */
enum class EAIStateChangeResult : uint8
{
	Failed,
	InProgress,
	Succeeded,
};

/**
 * Base state change predicate
 */
struct FStateChangePredicateBase
{
	virtual ~FStateChangePredicateBase() {}
	virtual EAIStateChangeResult operator()() { return EAIStateChangeResult::Succeeded; }
};

/**
 * Struct to hold a predicate of any class for state changing
 * 
 * Example state transition function
 * EAIStateChangeResult test1() { return EAIStateChangeResult::Succeeded; }
 */
template <class AIControllerClass>
struct FStateChangePredicate : public FStateChangePredicateBase
{
	typedef EAIStateChangeResult(AIControllerClass::* StateChangeFunction)();
	AIControllerClass*	Self;
	StateChangeFunction	ChangeFunction = nullptr;

	FStateChangePredicate(AIControllerClass* InSelf, StateChangeFunction InChangeFunction)
		: Self(InSelf), ChangeFunction(InChangeFunction)
	{}

	EAIStateChangeResult operator() () { return (Self->*ChangeFunction)(); }
};

// Each AI state can have their own state change predicate to be called
typedef TMap<EAIState, TSharedPtr<FStateChangePredicateBase>> TNewStateChangeFunctionMap;

/**
 * Base state transition
 */
struct FStateTransitionBase
{
	// The mapping of predicates to call when transitioning to each state
	TNewStateChangeFunctionMap StateChangeMap;

	FStateTransitionBase() = default;
	FStateTransitionBase(TNewStateChangeFunctionMap& InStateChangeMap) : StateChangeMap(InStateChangeMap) {}
	virtual ~FStateTransitionBase() {}
	virtual bool HasTransitionFunctions() { return false; }
	virtual void Enter() {}
	virtual void Exit() {}
};

/**
 * Transition data for an AI state that has predicates for entering, exiting, and transitioning to other states
 */
template <class AIControllerClass>
struct FStateTransition : public FStateTransitionBase
{
	typedef void(AIControllerClass::* StateEnterFunction)();
	typedef void(AIControllerClass::* StateExitFunction)();

	FStateTransition(AIControllerClass* InSelf, StateEnterFunction InEnterFunction, TNewStateChangeFunctionMap& InStateChangeMap, StateExitFunction InExitFunction)
		: FStateTransitionBase(InStateChangeMap), Self(InSelf), EnterFunction(InEnterFunction), ExitFunction(InExitFunction)
	{}

	FStateTransition(AIControllerClass* InSelf, TNewStateChangeFunctionMap& InStateChangeMap)
		: FStateTransition(InSelf, nullptr, InStateChangeMap, nullptr)
	{}

	FStateTransition(AIControllerClass * InSelf, StateEnterFunction InEnterFunction, StateExitFunction InExitFunction)
		: Self(InSelf), EnterFunction(InEnterFunction), ExitFunction(InExitFunction)
	{}

	bool HasTransitionFunctions() { return EnterFunction != nullptr || ExitFunction != nullptr; }
	void Enter() override{ if (EnterFunction != nullptr) (Self->*EnterFunction)(); }
	void Exit() override{ if (ExitFunction != nullptr) (Self->*ExitFunction)(); }

	AIControllerClass* Self;

	// Predicate to call on enter
	StateEnterFunction	EnterFunction = nullptr;

	// Predicate to call on exit
	StateExitFunction	ExitFunction = nullptr;
};

// Map of AI states to their transition data
typedef TMap<EAIState, TSharedPtr<FStateTransitionBase>> TStateTransitionMap;

/**
 * Base controller for AI
 */
UCLASS(Abstract, Blueprintable)
class RC_API ABaseAIController : public AAIController, public IBlackboardAssetProvider
{
	GENERATED_BODY()
	
public:
	ABaseAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * When this controller is asked to possess a pawn
	 * @param InPawn The Pawn to be possessed
	 */
	void OnPossess(class APawn* const PossessedPawn) override;

	// Get the blackboard
	class UBlackboardComponent* GetBlackBoard() const { return BlackboardComponent; }

	/**
	 * Request the AI to change to a different state
	 * @param NewState	The state to change to
	 * @Return Whether the change was successful, failed, or in progress and will be completed once the transition function is done
	 */
	EAIStateChangeResult RequestState(EAIState NewState);	

	// Message to send out for nodes waiting on state chagnes
	static const FName AIMessage_StateChangeFinished;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	// BEGIN IBlackboardAssetProvider
	/// Get the blackboard asset
	virtual UBlackboardData* GetBlackboardAsset() const override;
	// END IBlackboardAssetProvider

	/** 
	 * Add the new state transition functions to the map.
	 * If there is an existing function for a state transition, it will be replaced with the new function
	 * 
	 * Transitions:	The map of new state transition
	 */
	void AddStateTransition(const TStateTransitionMap& Transitions);

	// Create new state transition function map. Call AssignStateTransitions to add them to the map
	virtual void SetupStateTransitions();

	// Send off events for behavior tree nodes that the state change has finished
	void OnStateTransitionFinsished();

	// Actually change the state
	void FinishStateChange();

private:
	/**
	 * When a target has been detected through stimulus
	 * @param DetectedActor	The actor that was detected
	 * @param Stimulus		The stimulus that was used to detect the actor
	 */ 
	UFUNCTION()
	void OnTargetDetected(AActor* DetectedActor, const FAIStimulus Stimulus);

	// Setup perception configs
	void SetupPerception();

	// Update the perception sight configs
	void UpdatePerceptionSight();

	// State to start in
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	EAIState DefaultState = EAIState::Idle;

	// Behavior tree component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	// Behavior tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	// Blackboard component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBlackboardComponent* BlackboardComponent;

	// Blackboard key for the current AI state
	UPROPERTY(EditDefaultsOnly, Category = Blackboard)
	struct FBlackboardKeySelector CurrentStateKey;

	// Blackboard key for the currently requested AI state
	UPROPERTY(EditDefaultsOnly, Category = Blackboard)
	struct FBlackboardKeySelector RequestedStateKey;

	// Sense configs
	class UAISenseConfig_Sight* SightConfig = nullptr;
	class UAISenseConfig_Damage* DamageConfig = nullptr;

	// Current AI state
	EAIState CurrentState = EAIState::NUM_STATES;

	// Requested AI state
	EAIState RequestedState = EAIState::NUM_STATES;

	// All the state transition predicates
	TStateTransitionMap StateTransitions;
};
