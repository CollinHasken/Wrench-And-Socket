// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h" 	
#include "Containers/StaticArray.h"

#include "RC/Util/RCTypes.h"
#include "RC/Util/TimeStamp.h"

#include "BaseWeapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponConfig
{
	GENERATED_USTRUCT_BODY()
	
	// The bullet class to spawn when shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bullet)
	TSubclassOf<class ABaseBullet> BulletClass;

	/** The delay before each shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	float StartFiringDelay = 0.1f;

	/** The cooldown after each shot*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	float CooldownDelay = 0.5f;

	/** The cooldown after each shot*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	float BaseDamage = 1;
};

UCLASS(Abstract, Blueprintable, config=Game)
class RC_API ABaseWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	static constexpr uint8 MAX_LEVELS = 10;

	// Sets default values for this actor's properties
	ABaseWeapon();

	void ApplyConfig(const FWeaponConfig& Config);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * Set the new wielder for this weapon
	 * @param NewWielder		The pawn that is now wielding this weapon
	 */
	UFUNCTION(BlueprintCallable)
	void SetWielder(class ABaseCharacter* NewWielder);

	/** 
	 * Get the current wielder
	 */
	UFUNCTION(BlueprintCallable)
	class ABaseCharacter* GetWielder() { return Wielder; }

	/** Returns whether the weapon can start shooting **/
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CanStartShooting();

	/** Returns whether the weapon can be shot now **/
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CanShoot();

	// Shoot the weapon
	UFUNCTION()
	virtual bool Shoot();

	/** Shoot the weapon at the specified target */
	virtual bool ShootAtTarget(const FVector& TargetLocation);

	/** Shoot the weapon at the specified target */
	virtual bool ShootAtTarget(class ABaseCharacter* Target);

	/** Returns Mesh subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh() const { return Mesh; }

	/** Returns the Socket Name **/
	FORCEINLINE FName GetSocketName() const { return SocketName; }

protected:
	virtual void PostInitProperties() override;

	/** Called when the shot cooldown has expired **/
	void CooldownExpired();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	FWeaponConfig WeaponConfig;

	/** The main skeletal mesh associated with this weapon. */
	UPROPERTY(Category = Mesh, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh = nullptr;

	/** The socket that we'll attach this weapon to */
	UPROPERTY(Category = Mesh, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FName SocketName = NAME_None;

	// Timer to keep track of when to fire while shoot is held down
	FTimerHandle ShootTimer;

	// Timer to keep track of cooling down
	FTimeStamp CooldownTimer;

	// The socket where the bullet should spawn from
	const class USkeletalMeshSocket* BulletOffsetSocket = nullptr;

	// The actor wielding this weapon
	class ABaseCharacter* Wielder = nullptr;
private:
};

