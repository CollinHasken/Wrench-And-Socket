// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Gameplay/StatusEffects/BaseStatusEffect.h"
#include "StatusEffectStun.generated.h"

/**
 * Prevents the character from moving
 */
UCLASS()
class RC_API UStatusEffectStun : public UBaseStatusEffect
{
	GENERATED_BODY()
	
public:
	// Called when the first stack has been added
	void OnAdded_Implementation() override;

	// Called when the last stack has been removed 
	void OnRemoved_Implementation() override;

private:
	// Mask to apply to the player when they're stunned
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UInputMaskInfo* StunMask;

	// Montage to play on the character when stunned
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* StunMontage = nullptr;
};
