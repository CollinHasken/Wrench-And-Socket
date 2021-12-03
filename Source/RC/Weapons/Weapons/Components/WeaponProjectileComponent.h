// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RC/Weapons/Weapons/Components/WeaponComponent.h"
#include "WeaponProjectileComponent.generated.h"

/**
 * Weapon that can fire a projectile towards a target character or location
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RC_API UWeaponProjectileComponent : public UWeaponComponent
{
	GENERATED_BODY()

public:
	void Init(const UWeaponInfo& InWeaponInfo) override;

	// Attack with the weapon
	bool Attack() override;

	// Attack with the weapon at the given target
	bool AttackTarget(class ABaseCharacter* Target) override;

	void SetAccuracy(float InAccuracy) { Accuracy = InAccuracy; }

	// Returns Mesh subobject
	FORCEINLINE const class USkeletalMeshSocket* GetBulletOffsetSocket() const { return BulletOffsetSocket; }

protected:
	/** Shoot the weapon at the specified target
	 * @param TargetLocation	The location to shoot at
	 */
	virtual bool ShootAtTarget(const FVector& TargetLocation);

	/** Shoot the weapon at the specified target
	 * @param Target	The target character to shoot at
	 */
	virtual bool ShootAtTarget(class ABaseCharacter* Target);

private:
	// The socket where the bullet should spawn from
	const class USkeletalMeshSocket* BulletOffsetSocket = nullptr;

	// The projectile class to spawn when shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile, Meta=(AllowPrivateAccess="True"))
	TSubclassOf<class ABaseBullet> ProjectileClass;

	const class USkeletalMeshComponent* WeaponMesh = nullptr;

	float Accuracy = 1;
};
