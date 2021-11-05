// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputMaskInfo.generated.h"

/**
 * List of actions and axes to mask
 */
UCLASS()
class RC_API UInputMaskInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UInputMaskInfo();

	static constexpr uint8 INVALID_MASK_HANDLE = 0;

	// Get the Input Mask's UID
	uint8 GetHandle() const { return Handle; }

	// Get the action masks
	const TArray<FName>& GetActionMasks() const { return ActionMasks; }

	// Get the axis masks
	const TArray<FName>& GetAxisMasks() const { return AxisMasks; }
	
private:
	// The actions to mask
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta=(AllowPrivateAccess="True"))
	TArray<FName> ActionMasks;

	// THe axes to mask
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", Meta = (AllowPrivateAccess = "True"))
	TArray<FName> AxisMasks;

	// UID
	uint8 Handle = INVALID_MASK_HANDLE;
};
