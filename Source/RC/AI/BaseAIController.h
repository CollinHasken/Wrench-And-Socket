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
	StateChangeFunction Function;
	AIControllerClass* Self;

	FStateChangePredicate(AIControllerClass* InSelf, StateChangeFunction InFunction)
		: Function(InFunction), Self(InSelf)
	{}

	EAIStateChangeResult operator() () { return (Self->*Function)(); }
};

typedef TMap<EAIState, TSharedPtr<FStateChangePredicateBase>> TNewStateChangeFunctionMap;
typedef TMap<EAIState, TNewStateChangeFunctionMap> TStateChangeFunctionMap;

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
	 * Functions:	The map of new state transition functions
	 */
	void AddStateChangeFunctions(const TStateChangeFunctionMap& Functions);

	/** Create new state transition function map. Call AssignStateChangeFunctions to add them to the map */
	virtual void SetupStateChangeFunctions();

	/** Send off events that the state change has finished */
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

	TStateChangeFunctionMap StateChangeFunctions;

	UFUNCTION()
	void OnTargetDetected(AActor* DetectedActor, const FAIStimulus stimulus);

	void SetupPerception();
};
