// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "UObject/NoExportTypes.h"

#include "RC/Util/RCTypes.h"

#include "BaseStatusEffect.generated.h"

/**
 * Base status effect class to derive from
 */
UCLASS(BlueprintType, Blueprintable)
class RC_API UBaseStatusEffect : public UObject
{
	GENERATED_BODY()

public:
	// Set the owner of this status effect
	void SetOwner(AActor* InOwner) { Owner = InOwner; }

	// Called when the first stack has been added
	UFUNCTION(BlueprintNativeEvent)
	void OnAdded();

	// Called when the last stack has been removed 
	UFUNCTION(BlueprintNativeEvent)
	void OnRemoved();

	// Called when a stack has been added
	UFUNCTION(BlueprintNativeEvent)
	void OnStackAdded();

	// Called when a stack has been removed
	UFUNCTION(BlueprintNativeEvent)
	void OnStackRemoved();

	// The request type to use by default when adding stacks for this status effect
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EStatusEffectRequestType DefaultRequestType = EStatusEffectRequestType::Default;

	// The owner of this status effect
	UPROPERTY(BlueprintReadOnly)
	AActor* Owner = nullptr;
};