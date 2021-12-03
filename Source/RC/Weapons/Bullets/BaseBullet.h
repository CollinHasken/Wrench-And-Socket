// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "RC/Weapons/RCWeaponTypes.h"

#include "BaseBullet.generated.h"

/**
 * Base bullet to spawn from a weapon when shot
 */
UCLASS(Abstract, Blueprintable)
class RC_API ABaseBullet : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseBullet();

	//  Initial setup
	virtual void PostInitializeComponents() override;

	/**
	 * Setup weapon properties
	 * @param InBulletData	The data to init with
	 */
	void Init(const FBulletData& InBulletData);

	// Handle hit
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

	// Returns static mesh subobject
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Mesh; }
	// Returns Movement subobject
	FORCEINLINE class UProjectileMovementComponent* GetMovement() const { return Movement; }
	// Returns Collision subobject
	FORCEINLINE class USphereComponent* GetCollision() const { return Collision; }

protected:
	// movement component
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* Movement;

	// collisions
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collision;

	// Static mesh for the bullet
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;

	// Effect to spawn when hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ABaseBulletHitEffect> HitEffectClass;

	// Bullet data
	FBulletData BulletData;

	// The Asset Id of the weapon that shot this
	FPrimaryAssetId WeaponId = FPrimaryAssetId();
};
