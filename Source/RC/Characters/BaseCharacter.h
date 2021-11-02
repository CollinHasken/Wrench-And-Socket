// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "RC/Framework/DamageInterface.h"
#include "RC/Util/RCTypes.h"

#include "BaseCharacter.generated.h"

/**
 * Base character with health
 */
UCLASS(Abstract, Blueprintable)
class RC_API ABaseCharacter : public ACharacter, public IDamageInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	// Save character's data
	virtual void Serialize(FArchive& Ar) override;

	/**
	 * Request for this character to be damaged
	 * @param Params	The request params
	 */
	void RequestDamage(FDamageRequestParams& Params) override;

	/**
	 * Called when the character has given damage to someone else
	 * @param Params	The received params
	 */
	virtual void OnDamageGiven(const FDamageReceivedParams& Params) {};

	// Get the currently equipped weapon
	UFUNCTION(BlueprintPure)
	virtual class ABaseWeapon* GetEquippedWeapon() const { return nullptr; }

	// Returns Health subobject
	FORCEINLINE class UHealthComponent* GetHealth() const { return Health; }

	// Returns the anim instance
	FORCEINLINE class UAnimInstance* GetAnimInstance() const { return GetMesh() != nullptr ? GetMesh()->GetAnimInstance() : nullptr; }

protected:
	/**
	 * Called when the character dies
	 * @param Actor	The actor that's died
	 */
	UFUNCTION()
	virtual void OnActorDied(AActor* Actor);

private:

	/** Health */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	class UHealthComponent* Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = XP, meta = (AllowPrivateAccess = "true"))
	float XPForKilling = 0;

	bool bIsRagdolling = false;
};
