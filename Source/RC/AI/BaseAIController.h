// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"

#include "BaseAIController.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class RC_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABaseAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void BeginPlay() override;
	void OnPossess(class APawn* const PossessedPawn) override;
	void Tick(float DeltaTime) override;

	class UBlackboardComponent* GetBlackBoard() const { return BlackboardComponent; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UBlackboardComponent* BlackboardComponent;

	class UAISenseConfig_Sight* SightConfig = nullptr;
	class UAISenseConfig_Damage* DamageConfig = nullptr;

	UFUNCTION()
	void OnTargetDetected(AActor* DetectedActor, const FAIStimulus stimulus);

	void SetupPerception();
};
