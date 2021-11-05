// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InputComponent.h"
#include "RC/Debug/Debug.h"
#include "MaskableInputComponent.generated.h"

/**
 * Struct to keep track of setters for a given mask
 */
struct FSetterActionIndecies
{
	FSetterActionIndecies() = default;
	FSetterActionIndecies(const FName& Setter) { PushSetter(Setter); }

	// Push a setter
	void PushSetter(const FName& Setter)
	{
#if DEBUG_ENABLED
		SetterNames.Add(Setter);
#endif
		++Setters;
	}

	// Pop a setter
	void PopSetter(const FName& Setter)
	{
#if DEBUG_ENABLED
		int Removed = SetterNames.RemoveSingle(Setter);
		ASSERT(Removed == 1, "Trying to remove mask from %s when they did not set", Setter);
#endif
		--Setters;
	}

	// Is there any more setters
	bool HasSetters() const { return Setters != 0; }

	// List of action binding handles for this mask
	TArray<int32> ActionIndecies;

	// Amount of setters for this mask
	uint8 Setters = 0;

#if DEBUG_ENABLED
	// Debug list of names of the setters
	TArray<FName> SetterNames;
#endif
};

/**
 * An input component that has functions to allow masking input
 * This is accomplished by having the highest priority 
 */
UCLASS()
class RC_API UMaskableInputComponent : public UInputComponent
{
	GENERATED_BODY()

public:
	/*
	 * Push the mask
	 * @param MaskInfo The mask to push
	 * @param Setter The setter of the mask
	*/ 
	UFUNCTION(BlueprintCallable)
	void PushMask(const class UInputMaskInfo* MaskInfo, const FName& Setter);

	/*
	 * Pop the mask
	 * @param MaskInfo The mask to pop
	 * @param Setter The setter of the mask
	*/
	UFUNCTION(BlueprintCallable)
	void PopMask(const class UInputMaskInfo* MaskInfo, const FName& Setter);

private:
	// Empty action callback to bind to
	void EmptyActionCallback() {};

	// Map for mask to list of action indecies
	TMap<uint8, FSetterActionIndecies> MaskActionIndecies;
};
