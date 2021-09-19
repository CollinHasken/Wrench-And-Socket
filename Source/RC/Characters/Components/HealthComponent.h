// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "RC/Util/RCTypes.h"

#include "HealthComponent.generated.h"

// Broadcasts when the actor this component is on has been damaged
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorDamaged, class AActor*, Actor, float, CurrentHealth);

// Broadcasts when the actor this component is on has died
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorDied, class AActor*, Actor);

/**
 * Component to track health
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RC_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

	// Save off component's save data
	friend FArchive& operator<<(FArchive& Ar, UHealthComponent& SObj);

	/**
	 * Apply damage requested
	 * @param DamageParams	Params for the damage to apply
	 * 
	 * Returns the actual damage inflicted
	 */
	UFUNCTION(BlueprintCallable)
	float ApplyDamage(const FDamageRequestParams& DamageParams);

	// Get the Actor Damaged delegate
	FOnActorDamaged& OnActorDamaged() { return ActorDamagedDelegate; }

	// Get the Actor Died delegate
	FOnActorDied& OnActorDied() { return ActorDiedDelegate; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// Once we've ran out of health
	void OnKilled();
	
	// The max health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health, meta = (AllowPrivateAccess = "true"))
	int MaxHealth = 100;

	// The current health
	UPROPERTY(BlueprintReadOnly , Category = Health, SaveGame, meta = (AllowPrivateAccess = "true"))
	int CurrentHealth = 100;

	// Whether they're dead
	UPROPERTY(BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

	// Broadcasts when the actor this component is on has been damaged
	UPROPERTY(BlueprintAssignable, Category = Health, meta = (AllowPrivateAccess))
	FOnActorDamaged ActorDamagedDelegate;

	// Broadcasts when the actor this component is on has died
	UPROPERTY(BlueprintAssignable, Category = Health, meta = (AllowPrivateAccess))
	FOnActorDied ActorDiedDelegate;
};
