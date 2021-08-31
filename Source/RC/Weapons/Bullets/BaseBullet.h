// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "RC/Util/RCTypes.h"

#include "BaseBullet.generated.h"

UCLASS(Abstract, Blueprintable)
class RC_API ABaseBullet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseBullet();

	/** initial setup */
	virtual void PostInitializeComponents() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Setup weapon properties */
	void Init(const FBulletData& BulletData);

	/** Handle hit */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

	/** Returns static mesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Mesh; }
	/** Returns Movement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetMovement() const { return Movement; }
	/** Returns Collision subobject **/
	FORCEINLINE class USphereComponent* GetCollision() const { return Collision; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* Movement;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collision;

	/** Static mesh for the bullet */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ABaseBulletHitEffect> HitEffectClass;

	int Damage = 0;
	TWeakObjectPtr<class ABaseCharacter> Shooter = nullptr;
	TWeakObjectPtr<class ABaseWeapon> Weapon = nullptr;
	UClass* WeaponClass = nullptr;
};
