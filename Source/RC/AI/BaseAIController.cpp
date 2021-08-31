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
#include "RC/Util/RCTypes.h"


ABaseAIController::ABaseAIController(const FObjectInitializer& ObjectInitializer/* = FObjectInitializer::Get()*/)
{
	BehaviorTreeComponent = ObjectInitializer.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorTreeComponent"));
	BlackboardComponent = ObjectInitializer.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackboardComponent"));

	SetupPerception();
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();

	ASSERT_RETURN(BehaviorTree != nullptr);
	ASSERT_RETURN(RunBehaviorTree(BehaviorTree));
	//BehaviorTreeComponent->StartTree(*BehaviorTree);
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

void ABaseAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
			GetBlackBoard()->SetValue<UBlackboardKeyType_Enum>(BlackBoardKeys::STATE, static_cast<UBlackboardKeyType_Enum::FDataType>(EAIState::CHASE));
		}
	}
	else if (stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		ARCCharacter* Player = Cast<ARCCharacter>(DetectedActor);
		if (Player != nullptr)
		{
			if (stimulus.WasSuccessfullySensed())
			{
				GetBlackBoard()->SetValue<UBlackboardKeyType_Enum>(BlackBoardKeys::STATE, static_cast<UBlackboardKeyType_Enum::FDataType>(EAIState::CHASE));
			}
			else
			{
				GetBlackBoard()->SetValue<UBlackboardKeyType_Enum>(BlackBoardKeys::STATE, static_cast<UBlackboardKeyType_Enum::FDataType>(EAIState::SEARCH));
			}
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
