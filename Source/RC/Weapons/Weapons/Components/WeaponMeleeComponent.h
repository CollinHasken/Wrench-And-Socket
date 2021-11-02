// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/CapsuleComponent.h"

#include "RC/Weapons/Weapons/Components/WeaponComponent.h"
#include "RC/Weapons/RCWeaponTypes.h"

#include "WeaponMeleeComponent.generated.h"

/**
 * 
 */
UCLASS()
class RC_API UWeaponMeleeComponent : public UWeaponComponent
{
	GENERATED_BODY()
	
public:
	using UWeaponComponent::Init;

	// Initialize the weapon component
	void Init(const UWeaponInfo& WeaponInfo, UCapsuleComponent& CapsuleComponent);

protected:
	// Called when the anim notify state attack begins
	void OnAnimNotifyStateAttack_Begin() override;

	// Called when the anim notify state attack ends
	void OnAnimNotifyStateAttack_End() override;

	// On actor overlapping with the weapon's trigger
	UFUNCTION()
	void OnWeaponTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UCapsuleComponent* Capsule = nullptr;
};
