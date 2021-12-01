// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "Engine/UserDefinedStruct.h"

#include "RC/Gameplay/StatusEffects/BaseStatusEffect.h"
#include "RC/Util/RCTypes.h"
#include "RC/Util/TimeStamp.h"

#include "StatusEffectComponent.generated.h"

/**
 * Struct to contain info needed for requesting a status effect stack
 */
USTRUCT(BlueprintType)
struct FStatusEffectRequest
{
	GENERATED_BODY()

public:
	FStatusEffectRequest() {}
	FStatusEffectRequest(const TSubclassOf<class UBaseStatusEffect>& InStatusEffectClass) : StatusEffectClass(InStatusEffectClass) {};

	// The class of the status effect to add a stack to
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<class UBaseStatusEffect> StatusEffectClass = NULL;

	// The request type to use
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EStatusEffectRequestType RequestType = EStatusEffectRequestType::Default;
};

/**
 * Struct to contain info needed for requesting a timed status effect stack
 */
USTRUCT(BlueprintType, Blueprintable)
struct FStatusEffectTimedRequest : public FStatusEffectRequest
{
	GENERATED_BODY()

public:
	FStatusEffectTimedRequest() {}
	FStatusEffectTimedRequest(const TSubclassOf<class UBaseStatusEffect>& InStatusEffectClass, float InTime) : FStatusEffectRequest(InStatusEffectClass), Time(InTime) {};

	// The time for the request
	UPROPERTY(BlueprintReadWrite)
	float Time = 0.0f;
};

/**
 * Component to hold active status effects
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RC_API UStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UStatusEffectComponent();	

	// Add a stack for the requested status effect
	UFUNCTION(BlueprintCallable)
	void AddStatusEffect(const FStatusEffectRequest& StatusEffectRequest);

	// Add a timed stack for the requested status effect
	UFUNCTION(BlueprintCallable)
	void AddStatusEffectTimed(const FStatusEffectTimedRequest& StatusEffectTimedRequest);

	// Return the number of stacks a status effect currently has
	UFUNCTION(BlueprintCallable)
	int GetStatusEffectStackCount(const TSubclassOf<class UBaseStatusEffect> Class) const;

	// Return the time remaining for the timed status effect
	UFUNCTION(BlueprintCallable)
	float GetTimedStatusEffectTimeRemaining(const TSubclassOf<class UBaseStatusEffect> Class) const;

	// Remove a stack for the given status effect class
	UFUNCTION(BlueprintCallable)
	void RemoveStatusEffect(const TSubclassOf<class UBaseStatusEffect>& Class, bool bRemoveAllStacks = false);

	// Remove a timed stack for the given status effect class
	UFUNCTION(BlueprintCallable)
	void RemoveStatusEffectTimed(const TSubclassOf<class UBaseStatusEffect> Class);

protected:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Keep track of how many stacks for a status effect class
	 */
	struct FStatusEffect
	{
		FStatusEffect() = default;
		FStatusEffect(const FStatusEffectRequest& Request) : StatusEffectClass(Request.StatusEffectClass) {}

		// Add a stack
		void AddStack(EStatusEffectRequestType RequestType);

		// Remove a stack
		void RemoveStack();

		bool operator==(const FStatusEffect& rhs) const
		{
			return StatusEffectClass == rhs.StatusEffectClass;
		}

		bool operator==(const TSubclassOf<class UBaseStatusEffect>& rhs) const
		{
			return StatusEffectClass == rhs;
		}

		// The actual status effect
		class UBaseStatusEffect* StatusEffect = nullptr;

		// The class of the status effect
		TSubclassOf<class UBaseStatusEffect> StatusEffectClass = NULL;

		// The amount of stacks of the status effect
		int Stacks = 0;
	};

	/**
	 * Keep track of time remaining for a status effect
	 */
	struct FStatusEffectTimed : public FStatusEffect
	{
		FStatusEffectTimed(const FStatusEffectTimedRequest& Request) : FStatusEffect(Request) {};

		// Add a stack with a given time before it expires
		void AddTimedStack(float Time, EStatusEffectRequestType RequestType);

		// Time stamp for when the status effect expires and will be removed
		FTimeStamp ExpireTime;
	};

	// Active status effects
	TArray<FStatusEffect> ActiveStatusEffects;

	// Active timed status effects
	TArray<FStatusEffectTimed> ActiveTimedStatusEffects;
};
