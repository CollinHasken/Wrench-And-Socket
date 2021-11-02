// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "RC/Util/RCTypes.h"
#include "RC/Util/TimeStamp.h"
#include "RC/Weapons/RCWeaponTypes.h"

#include "WeaponComponent.generated.h"


UCLASS(Abstract, ClassGroup=(Custom) )
class RC_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Initialize the weapon component
	virtual void Init(const UWeaponInfo& WeaponInfo);

	/**
	 * Set the new wielder for this weapon
	 * @param NewWielder		The character that is now wielding this weapon
	 */
	void SetWielder(ABaseCharacter* NewWielder);

	// Attack with the weapon
	UFUNCTION(BlueprintCallable, Category = "Weapon|Attack")
	virtual bool Attack() { return true; }

	// Attack with the weapon at the given target
	UFUNCTION(BlueprintCallable, Category = "Weapon|Attack")
	virtual bool AttackTarget(class ABaseCharacter* Target) { return true; }
		
	// Set the damage this weapon does
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetDamage(int NewDamage) { Damage = NewDamage; }

	// Get the current damage this weapon should deal
	FORCEINLINE float GetDamage() { return Damage; }

protected:
	friend class ABaseWeapon;

	// Called when the anim notify attack is triggered
	virtual void OnAnimNotifyAttack() { Attack(); }

	// Called when the anim notify state attack begins
	virtual void OnAnimNotifyStateAttack_Begin() { Attack(); }

	// Called when the anim notify state attack ends
	virtual void OnAnimNotifyStateAttack_End() {}

	// Damage this weapon does
	UPROPERTY(BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	int Damage;

	// Wielder of this weapon
	class ABaseCharacter* Wielder = nullptr;

	FPrimaryAssetId WeaponInfoId = FPrimaryAssetId();

	// Camera of the wielder of this weapon
	class UCameraComponent* WielderCamera = nullptr;
};
