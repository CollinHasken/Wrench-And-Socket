// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/BlackboardAssetProvider.h"

#include "RC/Util/RCTypes.h"

#include "BaseAIController.generated.h"

enum class EAIStateChangeResult : uint8
{
	Failed,
	InProgress,
	Succeeded,
};

struct FStateChangePredicateBase
{
	virtual ~FStateChangePredicateBase() {}
	virtual EAIStateChangeResult operator()() { return EAIStateChangeResult::Succeeded; }
};

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

typedef TMap<EAIState, TSharedPtr<FStateChangePredicateBase>> TNewStateChangeFunctionMap;

struct FStateTransitionBase
{
	TNewStateChangeFunctionMap StateChangeMap;

	FStateTransitionBase() = default;
	FStateTransitionBase(TNewStateChangeFunctionMap& InStateChangeMap) : StateChangeMap(InStateChangeMap) {}
	virtual ~FStateTransitionBase() {}
	virtual bool HasTransitionFunctions() { return false; }
	virtual void Enter() {}
	virtual void Exit() {}
};

template <class AIControllerClass>
struct FStateTransition : public FStateTransitionBase
{
	typedef void(AIControllerClass::* StateEnterFunction)();
	typedef void(AIControllerClass::* StateExitFunction)();

	AIControllerClass* Self;
	StateEnterFunction	EnterFunction = nullptr;
	StateExitFunction	ExitFunction = nullptr;

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
};

typedef TMap<EAIState, TSharedPtr<FStateTransitionBase>> TStateTransitionMap;

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class RC_API ABaseAIController : public AAIController, public IBlackboardAssetProvider
{
	GENERATED_BODY()
	
public:
	static const FName AIMessage_StateChangeFinished;

	ABaseAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void BeginPlay() override;
	void OnPossess(class APawn* const PossessedPawn) override;
	void Tick(float DeltaTime) override;

	class UBlackboardComponent* GetBlackBoard() const { return BlackboardComponent; }

	EAIStateChangeResult RequestState(EAIState NewState);
	EAIStateChangeResult test1() { return EAIStateChangeResult::Succeeded; }

protected:
	// BEGIN IBlackboardAssetProvider
	/** @return blackboard asset */
	virtual UBlackboardData* GetBlackboardAsset() const override;
	// END IBlackboardAssetProvider

	/** 
	 * Add the new state transition functions to the map.
	 * If there is an existing function for a state transition, it will be replaced with the new function
	 * 
	 * Transitions:	The map of new state transition
	 */
	void AddStateTransition(const TStateTransitionMap& Transitions);

	/** Create new state transition function map. Call AssignStateTransitions to add them to the map */
	virtual void SetupStateTransitions();

	/** Send off events for behavior tree nodes that the state change has finished */
	void OnStateTransitionFinsished();

	/** Actually change the state */
	void FinishStateChange();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	EAIState DefaultState = EAIState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBlackboardComponent* BlackboardComponent;

	/** Blackboard key for the current AI state */
	UPROPERTY(EditDefaultsOnly, Category = Blackboard)
	struct FBlackboardKeySelector CurrentStateKey;

	/** Blackboard key for the currently requested AI state */
	UPROPERTY(EditDefaultsOnly, Category = Blackboard)
	struct FBlackboardKeySelector RequestedStateKey;

	class UAISenseConfig_Sight* SightConfig = nullptr;
	class UAISenseConfig_Damage* DamageConfig = nullptr;

	EAIState CurrentState = EAIState::NUM_STATES;
	EAIState RequestedState = EAIState::NUM_STATES;

	TStateTransitionMap StateTransitions;

	UFUNCTION()
	void OnTargetDetected(AActor* DetectedActor, const FAIStimulus stimulus);

	void SetupPerception();
	void UpdatePerceptionSight();
};
