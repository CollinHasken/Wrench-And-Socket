// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h" 	
#include "Containers/StaticArray.h"

#include "RC/Util/RCTypes.h"
#include "RC/Util/TimeStamp.h"

#include "BaseWeapon.generated.h"

UENUM(BlueprintType, Category = "Weapon")
enum class ETriggerStatus : uint8
{
	NONE UMETA(DisplayName = "No Trigger"),
	HALF UMETA(DisplayName = "Half Pulled Trigger"),
	FULL UMETA(DisplayName = "Full Pulled Trigger"),
};

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
};

USTRUCT(BlueprintType)
struct FWeaponLevelInfo
{
	GENERATED_USTRUCT_BODY()

	/** The XP needed to get to this level from the previous*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float XPNeeded = 100.0f;

	/** The base damage from this weapon at this level */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float BaseDamage = 10.0f;
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
	 * @param NewWielderCamera	The camera of the new wielder to be used for aiming
	 */
	UFUNCTION(BlueprintCallable)
	void SetWielder(class ABaseCharacter* NewWielder, class UCameraComponent* NewWielderCamera = nullptr);

	UFUNCTION(BlueprintCallable)
	void SetData(FWeaponData& InWeaponData) { WeaponData = &InWeaponData; WeaponData->CurrentWeapon = this; }

	/** 
	 * Get the current wielder
	 */
	UFUNCTION(BlueprintCallable)
	class ABaseCharacter* GetWielder() { return Wielder; }

	/**
	 * Start shooting
	 */
	UFUNCTION()
	void UpdateTriggerStatus(const ETriggerStatus& NewTriggerStatus);

	/** Returns whether the weapon can start shooting **/
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CanStartShooting();

	/** Returns whether the weapon can be shot now **/
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool CanShoot();

	// Shoot the weapon
	UFUNCTION()
	virtual bool Shoot();

	/** Returns Mesh subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh() const { return Mesh; }

	/** Returns the Socket Name **/
	FORCEINLINE FName GetSocketName() const { return SocketName; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	static ETriggerStatus TriggerValueToStatus(float Value) { return Value < 0.3f ? ETriggerStatus::NONE : (Value < 0.75f ? ETriggerStatus::HALF : ETriggerStatus::FULL); }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	ETriggerStatus GetTriggerStatus() { return CurrentTriggerStatus; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	FWeaponData& GetWeaponData() { return *WeaponData; }

	void GrantDamageXP(float XP);

protected:
	virtual void PostInitProperties() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MaybeShoot();

	/** Perform a shot when the trigger is fully held **/
	bool ShootFull();

	/** Perform a shot when the trigger is held halfway **/
	bool ShootHalf();

	/** Called when the shot cooldown has expired **/
	void CooldownExpired();

	/** Get current level data */
	const FWeaponLevelInfo& GetCurrentLevelData() const;

	bool CanLevelUp() const;

	/** Get the total XP needed to get to the next level */
	float GetXPTotalForNextLevel() const;

	/** Level up the weapon */
	void LevelUp();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	FWeaponConfig WeaponConfig;

	UPROPERTY(EditDefaultsOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	FWeaponLevelInfo WeaponLevelConfigs[MAX_LEVELS];

	UPROPERTY(EditDefaultsOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	float XPPerHit = 5.0f;

	/** The main skeletal mesh associated with this weapon. */
	UPROPERTY(Category = Mesh, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh = nullptr;

	/** The socket that we'll attach this weapon to */
	UPROPERTY(Category = Mesh, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FName SocketName = NAME_None;

	/** Whether the weapon is currently shooting */
	UPROPERTY(Category = Weapon, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ETriggerStatus CurrentTriggerStatus = ETriggerStatus::NONE;
		
	FWeaponData* WeaponData = nullptr;

	// Timer to keep track of when to fire while shoot is held down
	FTimerHandle ShootTimer;

	// Timer to keep track of cooling down
	FTimeStamp CooldownTimer;

	// Timer to keep track of level up slowmo
	FTimeStamp LevelUpTimer;

	// The socket where the bullet should spawn from
	const class USkeletalMeshSocket* BulletOffsetSocket = nullptr;

	// The actor wielding this weapon
	class ABaseCharacter* Wielder = nullptr;

	// The camera of the wielder to use for aiming
	class UCameraComponent* WielderCamera = nullptr;

	class UCurveFloat* LevelDilationCurve = nullptr;
private:
};

