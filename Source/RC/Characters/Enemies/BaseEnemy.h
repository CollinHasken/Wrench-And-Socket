// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Characters/BaseCharacter.h"
#include "RC/AI/SplineFollowerComponent.h"
#include "BaseEnemy.generated.h"

UCLASS(Abstract, Blueprintable)
class RC_API ABaseEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseEnemy();

	/** Attack the player */
	void AttackPlayer();

	/** Returns Spline Follower subobject **/
	FORCEINLINE class USplineFollowerComponent* GetSplineFollower() const { return SplineFollower; }

	/** Returns Weapn subobject **/
	FORCEINLINE class ABaseEnemyWeapon* GetWeapon() const { return Weapon; }

protected:
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Called when the character dies
	 * @param Actor	The actor that's died
	 */
	void OnActorDied(AActor* Actor) override;

private:
	/** Health */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	class USplineFollowerComponent* SplineFollower;

	/** Weapon Class*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ABaseEnemyWeapon> WeaponClass;

	/** Weapon*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class ABaseEnemyWeapon* Weapon;

	/** Spawn and setup the weapon */
	void SetupWeapon();
};
