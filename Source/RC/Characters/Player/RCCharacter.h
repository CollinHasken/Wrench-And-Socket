// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Util/RCTypes.h"
#include "RC/Util/TimeStamp.h"

#include "RCCharacter.generated.h"

/**
 * The player
 */
UCLASS(config=Game)
class ARCCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	~ARCCharacter();
	ARCCharacter();

	// Save player components
	virtual void Serialize(FArchive& Ar) override;

	// Called each frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * Called when the character has given damage to someone else
	 * @param Params	The received params
	 */
	virtual void OnDamageGiven(const FDamageReceivedParams& Params) override;

	// Base turn rate, in deg/sec. Other scaling may affect final turn rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	// Base look up/down rate, in deg/sec. Other scaling may affect final rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	// Returns CameraBoom subobject
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	// Returns FollowCamera subobject
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	// Returns Inventory subobject
	FORCEINLINE class UInventoryComponent* GetInventory() const { return Inventory; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// APawn interface
	/**
	 * Setup the player inputs
	 * @param PlayerInputComponent	The input component to bind to
	 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	// Called via input for forwards/backward input
	void MoveForward(float Value);

	// Called via input for side to side input
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** 
	 * Called via input to update shooting the current weapon 
	 * @param Value	This is a normalized value of the percentage the shoot trigger is held
	 */
	void Shoot(float Value);

	// Called via input to equip the next weapon in our inventory
	void EquipNextWeapon();

	// Called via input to equip the previous weapon in our inventory
	void EquipPreviousWeapon();

	// Called via input to swap to previous weapon or open the weapon wheel
	void SelectWeapon();

	// Called via input once the swap weapon key isn't pressed
	void SelectWeaponEnd();

	// Called via input to open the pause HUD
	void OpenPauseHUD();

	// Called via input to open the pause settings
	void OpenPauseSettings();

	/**
	 * Called when a weapon is equipped
	 * @param Weapon	The weapon that was equipped
	 */
	UFUNCTION()
	void OnWeaponEquipped(class ABasePlayerWeapon* Weapon);

	/**
	 * Called when the equipped weapon levels up
	 * @param Weapon		The weapon that leveld up
	 * @param CurrentLevel	The current level of the weapon
	 */
	UFUNCTION()
	void OnWeaponLevelUp(class ABasePlayerWeapon* Weapon, uint8 CurrentLevel);

	/**
	 * Called when the character dies
	 * @param Actor	The actor that's died
	 */
	void OnActorDied(AActor* Actor) override;

private:
	// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	// Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	class UInventoryComponent* Inventory;

	// Amount of kills
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, meta = (AllowPrivateAccess = "true"))
	float Kills;

	// Stimuli Source for AI perception
	class UAIPerceptionStimuliSourceComponent* Stimulus;

	// Timer to keep track of level up slowmo
	FTimeStamp LevelUpTimer;

	// Curve for the time dilation during a levelup
	class UCurveFloat* LevelDilationCurve = nullptr;

	// Timer to decide between swapping between weapons vs opening the weapon select
	FTimeStamp WeaponSelectTimer;
};

/**
 * Structure to hold the save data for the player that can persist through respawning
 */
USTRUCT()
struct FCharacterSaveData
{
	GENERATED_BODY()

public:

	// Identifier for which Actor this belongs to
	UPROPERTY()
	FName ActorName;

	// For movable Actors, keep location,rotation,scale
	UPROPERTY()
	FTransform Transform;

	// Contains all 'SaveGame' marked variables of the Actor
	UPROPERTY()
	TArray<uint8> ByteData;
};