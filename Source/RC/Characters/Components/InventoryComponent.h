// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"
#include "Containers/Map.h"

#include "RC/Util/RCTypes.h"

#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, class ABasePlayerWeapon*, Weapon);

USTRUCT(BlueprintType)
struct FLoadoutSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	EInventorySlot Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	TSubclassOf<class ABasePlayerWeapon> Weapon;
};

USTRUCT(BlueprintType)
struct FLoadout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	TArray<FLoadoutSlotInfo> LoadoutSlotInfos;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RC_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	static constexpr uint8 MAX_WEAPONS = static_cast<uint8>(EInventorySlot::NUM_SLOTS);
	static constexpr uint8 MAX_QUICKSLOTS = static_cast<uint8>(EQuickSlot::NUM_SLOTS);

	/** Save the inventory */
	friend FArchive& operator<<(FArchive& Ar, UInventoryComponent& SObj);

	/** 
	 * Assign a weapon to the slot. If that slot is currently equipped, it'll swap it out
	 * 
	 * @Param Slot The slot to set the weapon to
	 * @Param WeaponClass The weapon class to assign
	 */
	UFUNCTION(BlueprintCallable)
	void AssignSlot(EInventorySlot Slot, TSubclassOf<class ABasePlayerWeapon> WeaponClass);

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

	/** Equip the next slot numerically */
	class ABasePlayerWeapon* EquipNextSlot();

	/** Equip the previous slot numerically */
	class ABasePlayerWeapon* EquipPreviousSlot();

	// Return the currently equipped weapon
	UFUNCTION(BlueprintCallable)
	class ABasePlayerWeapon* GetEquippedWeapon() { return EquippedWeapon; }

	/** Get the Weapon Equipped delegate */
	FOnWeaponEquipped& OnWeaponEquipped() { return WeaponEquippedDelegate; }

protected:
	// Called when the game starts
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	class ABasePlayerWeapon* SpawnSlot(EInventorySlot NewSlot);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Loadout, meta = (AllowPrivateAccess = "true"))
	FLoadout DefaultLoadout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Loadout, meta = (AllowPrivateAccess = "true"))
	EInventorySlot DefaultSlot;

	UPROPERTY(SaveGame)
	TSubclassOf<class ABasePlayerWeapon> WeaponClasses[MAX_WEAPONS] = { NULL };

	UPROPERTY(SaveGame)
	EInventorySlot QuickSlots[MAX_QUICKSLOTS] = { EInventorySlot::NUM_SLOTS, EInventorySlot::NUM_SLOTS, EInventorySlot::NUM_SLOTS, EInventorySlot::NUM_SLOTS };

	UPROPERTY(SaveGame)
	EInventorySlot EquippedSlot = EInventorySlot::NUM_SLOTS;

	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnWeaponEquipped WeaponEquippedDelegate;

	class ABasePlayerWeapon* EquippedWeapon = nullptr;
};
