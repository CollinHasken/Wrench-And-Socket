// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RC/Weapons/Weapons/Components/WeaponComponent.h"

#include "WeaponRaycastComponent.generated.h"

/**
 *  Weapon that can fire and damage enemies with a raycast
 */
UCLASS()
class RC_API UWeaponRaycastComponent : public UWeaponComponent
{
	GENERATED_BODY()

public:
	// Initialize the weapon component
	void Init(const UWeaponInfo& InWeaponInfo) override;

	// Attack with the weapon
	bool Attack() override;

	// Attack with the weapon at the given target
	bool AttackTarget(class ABaseCharacter* Target) override;

	// Returns Mesh subobject
	FORCEINLINE const class USkeletalMeshSocket* GetVFXOffsetSocket() const { return VFXOffsetSocket; }

private:
	/** 
	 * Shoot the weapon towards the specified target
	 * @param TargetLocation	The location to shoot towards
	 */
	bool ShootTowardsTarget(const FVector& TargetDirection);

	// The half size of the close trace to create a cone
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FVector CloseTraceHalfSize;

	// The half size of the far trace to create a cone
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	FVector FarTraceHalfSize;

	// The socket where the VFX should spawn from
	const class USkeletalMeshSocket* VFXOffsetSocket = nullptr;

	// The VFX class to spawn when shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, Meta = (AllowPrivateAccess = "True"))
	TSubclassOf<class AActor> VFXClass;

	// Reference to the mesh of the weapon
	const class USkeletalMeshComponent* WeaponMesh = nullptr;
};
