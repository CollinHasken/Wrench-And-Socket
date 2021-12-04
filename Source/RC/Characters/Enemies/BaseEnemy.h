// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RC/AI/SplineFollowerComponent.h"
#include "RC/Characters/BaseCharacter.h"
#include "RC/Save/SaveGameInterface.h"

#include "BaseEnemy.generated.h"

/**
 * Base enemy with AI and spline following
 */
UCLASS(Abstract, Blueprintable)
class RC_API ABaseEnemy : public ABaseCharacter, public ISaveGameInterface
{
	GENERATED_BODY()

public:
	ABaseEnemy();

	// Attack the player
	void AttackPlayer();

	// Returns Spline Follower subobject
	FORCEINLINE class USplineFollowerComponent* GetSplineFollower() const { return SplineFollower; }

	// Get the currently equipped weapon
	class ABaseWeapon* GetEquippedWeapon() const override;

	// The enemy only saves itself once destroyed
	bool ActorNeedsSaving_Implementation() override { return false; }

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Called before the component is destroyed
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Called when the character dies
	 * @param Actor	The actor that's died
	 */
	void OnActorDied(AActor* Actor) override;

private:
	// Spawn and setup the weapon
	void SetupWeapon();

	// Health
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	class USplineFollowerComponent* SplineFollower;

	// Weapon Class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ABaseEnemyWeapon> WeaponClass;

	// Weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class ABaseEnemyWeapon* Weapon;
};
