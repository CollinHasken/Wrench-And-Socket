// Fill out your copyright notice in the Description page of Project Settings.
#include "StatusEffectComponent.h"

#include "RC/Debug/Debug.h"

// Sets default values for this component's properties
UStatusEffectComponent::UStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void UStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Remove expired status effects
	for (int StatusEffectIndex = ActiveTimedStatusEffects.Num() - 1; StatusEffectIndex >= 0; --StatusEffectIndex)
	{
		if (ActiveTimedStatusEffects[StatusEffectIndex].ExpireTime.Elapsed())
		{
			ActiveTimedStatusEffects[StatusEffectIndex].RemoveStack();
			ActiveTimedStatusEffects.RemoveAtSwap(StatusEffectIndex);
		}
	}
}

// Add a stack for the requested status effect
void UStatusEffectComponent::AddStatusEffect(const FStatusEffectRequest& StatusEffectRequest)
{
	int StatusEffectIndex = ActiveStatusEffects.AddUnique(FStatusEffect(StatusEffectRequest));
	ASSERT_RETURN(StatusEffectIndex != -1);

	FStatusEffect& StatusEffect = ActiveStatusEffects[StatusEffectIndex];
	StatusEffect.AddStack(StatusEffectRequest.RequestType);
}

// Add a timed stack for the requested status effect
void UStatusEffectComponent::AddStatusEffectTimed(const FStatusEffectTimedRequest& StatusEffectTimedRequest)
{
	int StatusEffectIndex = ActiveTimedStatusEffects.AddUnique(FStatusEffectTimed(StatusEffectTimedRequest));
	ASSERT_RETURN(StatusEffectIndex != -1);

	FStatusEffectTimed& TimedStatusEffect = ActiveTimedStatusEffects[StatusEffectIndex];
	TimedStatusEffect.AddTimedStack(StatusEffectTimedRequest.Time, StatusEffectTimedRequest.RequestType);
}

// Return the number of stacks a status effect currently has
int UStatusEffectComponent::GetStatusEffectStackCount(const TSubclassOf<UBaseStatusEffect> Class) const
{
	int StatusEffectIndex = ActiveStatusEffects.IndexOfByKey(Class);
	return StatusEffectIndex != INDEX_NONE ? ActiveStatusEffects[StatusEffectIndex].Stacks : -1;
}

// Return the time remaining for the timed status effect
float UStatusEffectComponent::GetTimedStatusEffectTimeRemaining(const TSubclassOf<UBaseStatusEffect> Class) const
{
	int StatusEffectIndex = ActiveTimedStatusEffects.IndexOfByKey(Class);
	return StatusEffectIndex != INDEX_NONE ? ActiveTimedStatusEffects[StatusEffectIndex].ExpireTime.GetTimeRemaining() : -1.0f;
}

// Remove a stack for the given status effect class
void UStatusEffectComponent::RemoveStatusEffect(const TSubclassOf<UBaseStatusEffect>& Class, bool bRemoveAllStacks/* = false*/)
{
	// Find the effect
	int StatusEffectIndex = ActiveStatusEffects.IndexOfByKey(Class);
	if (StatusEffectIndex == INDEX_NONE)
	{
		return;
	}
	FStatusEffect& StatusEffect = ActiveStatusEffects[StatusEffectIndex];
	
	// Remove one or all stacks
	StatusEffect.RemoveStack();
	if (bRemoveAllStacks)
	{
		while (StatusEffect.Stacks > 0)
		{
			StatusEffect.RemoveStack();
		}
	}	

	// Remove if there are no more stacks
	if (StatusEffect.Stacks == 0)
	{
		ActiveStatusEffects.RemoveAtSwap(StatusEffectIndex);
	}
}

// Remove a timed stack for the given status effect class
void UStatusEffectComponent::RemoveStatusEffectTimed(const TSubclassOf<UBaseStatusEffect> Class)
{
	// Find the effect
	int StatusEffectIndex = ActiveTimedStatusEffects.IndexOfByKey(Class);
	if (StatusEffectIndex == INDEX_NONE)
	{
		return;
	}
	FStatusEffectTimed& StatusEffect = ActiveTimedStatusEffects[StatusEffectIndex];

	// Remove a stack
	StatusEffect.RemoveStack();

	// Remove if there are no more stacks
	if (StatusEffect.Stacks == 0)
	{
		ActiveTimedStatusEffects.RemoveAtSwap(StatusEffectIndex);
	}
}

// Add a stack
void UStatusEffectComponent::FStatusEffect::AddStack(EStatusEffectRequestType RequestType)
{
	switch (RequestType)
	{
		case EStatusEffectRequestType::Override:
			// If we're overriding then remove
			RemoveStack();
			break;
		case EStatusEffectRequestType::Add:
		case EStatusEffectRequestType::Default:
			break;
		case EStatusEffectRequestType::Ignore:
			// Return if there's already a stack
			if (Stacks > 0)
			{
				return;
			}
			break;
		default:
			break;
	}

	// Add stack
	++Stacks;

	// First stack
	if (StatusEffect == nullptr)
	{
		ASSERT(Stacks == 1);

		StatusEffect = NewObject<UBaseStatusEffect>(GetTransientPackage(), StatusEffectClass);
		ASSERT_RETURN(StatusEffect != nullptr);

		StatusEffect->OnAdded();
	}

	StatusEffect->OnStackAdded();
}

// Remove a stack
void UStatusEffectComponent::FStatusEffect::RemoveStack()
{
	// No stacks to remove
	if (Stacks <= 0)
	{
		return;
	}

	--Stacks;

	ASSERT_RETURN(StatusEffect != nullptr);

	StatusEffect->OnStackRemoved();

	if (Stacks == 0)
	{
		StatusEffect->OnRemoved();
	}
}

// Add a stack with a given time before it expires
void UStatusEffectComponent::FStatusEffectTimed::AddTimedStack(float Time, EStatusEffectRequestType RequestType)
{
	bool FirstStack = Stacks == 0;

	// Add a stack
	AddStack(RequestType);

	// If this is the first stack, just set the time
	if (FirstStack)
	{
		ExpireTime.Set(Time);
		return;
	}
	
	ASSERT_RETURN(StatusEffect != nullptr);

	// If we're doing the default then get the status effect's default type
	RequestType = RequestType == EStatusEffectRequestType::Default ? StatusEffect->DefaultRequestType : RequestType;
	switch (RequestType)
	{
		case EStatusEffectRequestType::Default:
		case EStatusEffectRequestType::Override:
			ExpireTime.Set(Time);
			break;
		case EStatusEffectRequestType::Add:
			ExpireTime.AddTime(Time);
			break;
		case EStatusEffectRequestType::Ignore:		
			// Don't change the time since there was already a stack
			break;
		default:
			break;
	}
}
