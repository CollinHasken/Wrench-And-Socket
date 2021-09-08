// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "RC/Util/RCTypes.h"

#include "BaseCharacter.generated.h"

UCLASS(Abstract, Blueprintable)
class RC_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	virtual void Serialize(FArchive& Ar) override;

	/**
	 * Request for this character to be damaged
	 * @param Params	The request params
	 */
	void RequestDamage(FDamageRequestParams& Params);

	/**
	 * Called when the character has given damage to someone else
	 * @param Params	The received params
	 */
	virtual void OnDamageGiven(const FDamageReceivedParams& Params) {};


	/** Returns Health subobject **/
	FORCEINLINE class UHealthComponent* GetHealth() const { return Health; }

protected:
	/**
	 * Called when the character dies
	 * @param Actor	The actor that's died
	 */
	UFUNCTION()
	virtual void OnActorDied(AActor* Actor);

private:

	/** Health */
	//component is null on load. need to special serialize? check google for saving component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	class UHealthComponent* Health;

	/** Health */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = XP, meta = (AllowPrivateAccess = "true"))
	float XPForKilling = 0;

	bool bIsRagdolling = false;
};
