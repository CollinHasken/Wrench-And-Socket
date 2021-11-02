// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "Templates/SubclassOf.h"

#include "RC/Util/RCTypes.h"

#include "InventoryComponent.generated.h"

// Broadcasted when a weapon is equipped
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponEquipped, class ABasePlayerWeapon*, Weapon, EInventorySlot, Slot);

/**
 * Info for a weapon in the inventory
 */
USTRUCT(BlueprintType)
struct FInventoryWeapon
{
	GENERATED_BODY()

	FInventoryWeapon() = default;

	// Stream handle when loading it
	TSharedPtr<struct FStreamableHandle> StreamHandle;

	// Asset Id for the weapon
	UPROPERTY(SaveGame)
	FPrimaryAssetId AssetId = FPrimaryAssetId();

	// Cached weapon info
	const class UPlayerWeaponInfo* WeaponInfo = nullptr;

	bool IsValid() const { return WeaponInfo != nullptr; }
};

/**
 * Pairing of a slot to a weapon class
 */
USTRUCT(BlueprintType)
struct FLoadoutSlotInfo
{
	GENERATED_BODY()

	// Slot the weapon is in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	EInventorySlot Slot;

	// Id of the weapon info for the slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	FPrimaryAssetId WeaponInfoId;
};

/**
 * Array of slot and weapon pairings
 */
USTRUCT(BlueprintType)
struct FLoadout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	TArray<FLoadoutSlotInfo> LoadoutSlotInfos;
};

/**
 * The inventory component on the player to manage the equippable and equipped weapon
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RC_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	static constexpr uint8 MAX_WEAPONS = static_cast<uint8>(EInventorySlot::NUM_SLOTS);
	static constexpr uint8 MAX_QUICKSLOTS = static_cast<uint8>(EQuickSlot::NUM_SLOTS);

	// Save the inventory
	friend FArchive& operator<<(FArchive& Ar, UInventoryComponent& SObj);

	/** 
	 * Assign a weapon to the slot. If that slot is currently equipped, it'll swap it out
	 * 
	 * @Param Slot The slot to set the weapon to
	 * @Param WeaponInfoId The asset Id for the weapon info
	 */
	UFUNCTION(BlueprintCallable)
	void AssignSlot(EInventorySlot Slot, const FPrimaryAssetId& WeaponInfoId);

	/**
	 * Equip a slot as the current weapon. If there's no weapon in that slot, then it won't equip
	 *
	 * @Param NewSlot The slot to switch to
	 *
	 * @Return The newely equipped weapon
	 */
	UFUNCTION(BlueprintCallable)
	class ABasePlayerWeapon* EquipSlot(EInventorySlot NewSlot);

	/**
	 * Equip a slot as the current weapon through the quick slot. If there's no inventory slot assigned to that quick slot or no weapon in that slot, then it won't equip
	 *
	 * @Param QuickSlot The quick slot to switch to
	 *
	 * @Return The newely equipped weapon
	 */
	UFUNCTION(BlueprintCallable)
	class ABasePlayerWeapon* EquipQuickSlot(EQuickSlot QuickSlot);

	/**
	 * Equip a weapon to a quickslot by mapping the inventory slot
	 *
	 * @Param Quickslot		The quickslot to map for
	 * @Param InventorySlot The slot of the weapon to map to
	 */
	UFUNCTION(BlueprintCallable)
	void EquipToQuickSlot(EQuickSlot QuickSlot, EInventorySlot InventorySlot);

	// Equip the next slot numerically
	class ABasePlayerWeapon* EquipNextSlot();

	// Equip the previous slot numerically
	class ABasePlayerWeapon* EquipPreviousSlot();

	// Equip the previous weapon that was equipped
	class ABasePlayerWeapon* EquipPreviousWeapon();

	/**
	 * Return the currently equipped weapon
	 * 
	 * @Return the equipped weapon
	 */
	UFUNCTION(BlueprintPure)
	class ABasePlayerWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	/**
	 * Return the currently equipped weapon slot
	 *
	 * @Return the equipped weapon slot
	 */
	UFUNCTION(BlueprintPure)
	EInventorySlot GetEquippedWeaponSlot() const { return EquippedSlot; }

	/**
	 * Get weapon info for the slot
	 *
	 * @Param InventorySlot		The slot to get the class from
	 * @Return The weapon info
	 */
	UFUNCTION(BlueprintPure)
	const UPlayerWeaponInfo* GetWeaponInfo(EInventorySlot InventorySlot) const;

	/**
	 * Get the weapon data for the inventory slot
	 *
	 * @Param InventorySlot		The slot to get the data for
	 * @Return The Data retrieved
	 */
	UFUNCTION(BlueprintCallable)
	UPlayerWeaponData* GetWeaponData(EInventorySlot InventorySlot);

	/**
	 * Get weapon class for the slot
	 *
	 * @Param Class			The class retrieved
	 * @Param InventorySlot The slot to get the class from
	 * @Return Whether getting it was successful
	 */
	UFUNCTION(BlueprintPure)
	bool GetWeaponClass(TSubclassOf<class ABaseWeapon>& WeaponClass, EInventorySlot InventorySlot) const;

	/**
	 * Is there a weapon in the given slot
	 *
	 * @Param Slot	The slot to check
	 * @Return Whether there is a weapon
	 */
	UFUNCTION(BlueprintPure)
	bool IsSlotOccupied(EInventorySlot Slot) const;

	// Get the Weapon Equipped delegate
	FOnWeaponEquipped& OnWeaponEquipped() { return WeaponEquippedDelegate; }

protected:
	// Called when the game starts
	void BeginPlay() override;

	// Called before the component is destroyed
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/**
	 * Called once the weapon info has been loaded
	 * 
	 * @Slot	The slot that was loaded
	 */
	void OnWeaponInfoLoaded(EInventorySlot Slot);

	/**
	 * Spawn the weapon in the given slot
	 *
	 * @Param Slot	The slot to get the weapon class to spawn with
	 * @Return The spawaned weapon
	 */
	class ABasePlayerWeapon* SpawnSlot(EInventorySlot NewSlot);

	// The loadout to spawn the player with
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Loadout, meta = (AllowPrivateAccess = "true"))
	FLoadout DefaultLoadout;

	// The slot to start equipped with
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Loadout, meta = (AllowPrivateAccess = "true"))
	EInventorySlot DefaultSlot;

	// The weapons, indexed by their associated slot
	UPROPERTY(SaveGame)
	FInventoryWeapon Weapons[MAX_WEAPONS] = { FInventoryWeapon() };

	// The quick slots, storing which inventory slot they map to
	UPROPERTY(SaveGame)
	EInventorySlot QuickSlots[MAX_QUICKSLOTS] = { EInventorySlot::NUM_SLOTS, EInventorySlot::NUM_SLOTS, EInventorySlot::NUM_SLOTS, EInventorySlot::NUM_SLOTS };

	// The currently equipped slot
	UPROPERTY(SaveGame)
	EInventorySlot EquippedSlot = EInventorySlot::NUM_SLOTS;

	// The previously equipped slot
	EInventorySlot PreviouslyEquippedSlot = EInventorySlot::NUM_SLOTS;

	// The currently equipped weapon
	class ABasePlayerWeapon* EquippedWeapon = nullptr;

	// Broadcasted when a weapon is equipped
	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnWeaponEquipped WeaponEquippedDelegate;
};
