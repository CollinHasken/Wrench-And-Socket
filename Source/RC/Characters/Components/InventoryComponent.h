// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Templates/SubclassOf.h"
#include "Containers/Map.h"

#include "RC/Util/RCTypes.h"

#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FLoadoutSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	EInventorySlot Slot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Loadout)
	TSubclassOf<class ABaseWeapon> Weapon;
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

	// Sets default values for this component's properties
	UInventoryComponent();

	friend FArchive& operator<<(FArchive& Ar, UInventoryComponent& SObj);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 
	 * Assign a weapon to the slot. If that slot is currently equipped, it'll swap it out
	 * 
	 * @Param Slot The slot to set the weapon to
	 * @Param WeaponClass The weapon class to assign
	 */
	UFUNCTION(BlueprintCallable)
	void AssignSlot(EInventorySlot Slot, TSubclassOf<class ABaseWeapon> WeaponClass);

	/**
	 * Equip a slot as the current weapon. If there's no weapon in that slot, then it won't equip
	 *
	 * @Param NewSlot The slot to switch to
	 *
	 * @Return True if the weapon was properly equipped
	 */
	UFUNCTION(BlueprintCallable)
	bool EquipSlot(EInventorySlot NewSlot);

	/** Equip the next slot numerically */
	void EquipNextSlot();

	/** Equip the previous slot numerically */
	void EquipPreviousSlot();

	// Return the currently equipped weapon
	UFUNCTION(BlueprintCallable)
	class ABaseWeapon* GetEquippedWeapon() { return EquippedWeapon; }

	/** Test weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Test)
	TSubclassOf<class ABaseWeapon> TestWeaponClass;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	class ABaseWeapon* SpawnSlot(EInventorySlot NewSlot);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Loadout, meta = (AllowPrivateAccess = "true"))
	FLoadout DefaultLoadout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Loadout, meta = (AllowPrivateAccess = "true"))
	EInventorySlot DefaultSlot;

	TSubclassOf<class ABaseWeapon> WeaponClasses[MAX_WEAPONS] = { NULL };

	EInventorySlot EquippedSlot = EInventorySlot::NUM_SLOTS;
	class ABaseWeapon* EquippedWeapon = nullptr;

public:
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
};
