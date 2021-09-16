// Fill out your copyright notice in the Description page of Project Settings.
#include "RC/Characters/Components/InventoryComponent.h"

#include "Camera/CameraComponent.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Weapons/Weapons/BasePlayerWeapon.h"
#include "RC/Debug/Debug.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"

FArchive& operator<<(FArchive& Ar, UInventoryComponent& SObj)
{
	if (Ar.IsLoading() || Ar.IsSaving())
	{
		UClass* Class = UInventoryComponent::StaticClass();
		Class->SerializeTaggedProperties(Ar, (uint8*)&SObj, Class, nullptr);
	}
	return Ar;
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Equip defaults
	for (const FLoadoutSlotInfo& LoadoutSlotInfo : DefaultLoadout.LoadoutSlotInfos)
	{
		AssignSlot(LoadoutSlotInfo.Slot, LoadoutSlotInfo.WeaponClass);
	}

	// Either equipped the saved slot or equip the default
	EquipSlot(EquippedSlot != EInventorySlot::NUM_SLOTS ? EquippedSlot : DefaultSlot);
}

// Called before the component is destroyed
void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Destroy the spawned weapon
	if (EquippedWeapon != nullptr)
	{
		EquippedWeapon->Destroy();
	}
}

// Assign a weapon to the slot. If that slot is currently equipped, it'll swap it out
void UInventoryComponent::AssignSlot(EInventorySlot Slot, TSubclassOf<ABasePlayerWeapon> WeaponClass)
{
	WeaponClasses[static_cast<uint8>(Slot)] = WeaponClass;

	if (Slot == EquippedSlot)
	{
		/****************
		*
		*HACK REMOVE
		*
		****************/
		EquippedSlot = EInventorySlot::NUM_SLOTS;
		EquipSlot(Slot);
	}
}

// Equip a slot as the current weapon. If there's no weapon in that slot, then it won't equip
ABasePlayerWeapon* UInventoryComponent::EquipSlot(EInventorySlot NewSlot)
{
	// Equipping invalid slot
	if (NewSlot == EInventorySlot::NUM_SLOTS)
	{
		return nullptr;
	}

	// Equipping the same slot
	if (NewSlot == EquippedSlot)
	{
		return GetEquippedWeapon();
	}

	// New slot has no weapon
	TSubclassOf<ABasePlayerWeapon>& WeaponClass = WeaponClasses[static_cast<uint8>(NewSlot)];
	if (WeaponClass == NULL)
	{
		return nullptr;
	}

	AActor* Owner = GetOwner();
	ASSERT_RETURN_VALUE(Owner != nullptr, nullptr);

	ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(Owner);
	ASSERT_RETURN_VALUE(BaseCharacter != nullptr, nullptr, "Inventory isn't on a base character?");

	// * TODO *
	// delete old weapon
	if (EquippedWeapon != nullptr)
	{
		// Rename so we can name the new weapon
		EquippedWeapon->Rename();
		EquippedWeapon->Destroy();
	}

	ABasePlayerWeapon* NewWeapon = SpawnSlot(NewSlot);
	ASSERT_RETURN_VALUE(NewWeapon != nullptr, nullptr, "New weapon wasn't able to be spawned");

	EquippedWeapon = NewWeapon;
	PreviouslyEquippedSlot = EquippedSlot;
	EquippedSlot = NewSlot;

	UCameraComponent* CharacterCamera = BaseCharacter->FindComponentByClass<UCameraComponent>();

	// Tell weapon who the owner is
	EquippedWeapon->SetWielder(BaseCharacter, CharacterCamera);

	ARCCharacter* Player = Cast<ARCCharacter>(BaseCharacter);
	if (Player)
	{
		ARCPlayerState* PlayerState = Player->GetPlayerState<ARCPlayerState>();
		if (PlayerState)
		{
			EquippedWeapon->SetData(PlayerState->FindOrAddWeaponDataForClass(WeaponClass));
		}
		else
		{
			ASSERT(PlayerState != nullptr, "Player with no state");
		}
	}

	WeaponEquippedDelegate.Broadcast(EquippedWeapon);

	return EquippedWeapon;
}

// Equip a slot as the current weapon through the quick slot.
ABasePlayerWeapon* UInventoryComponent::EquipQuickSlot(EQuickSlot QuickSlot)
{
	return EquipSlot(QuickSlots[static_cast<uint8>(QuickSlot)]);
}

/** Equip a weapon to a quickslot by mapping the inventory slot */
void UInventoryComponent::EquipToQuickSlot(EQuickSlot QuickSlot, EInventorySlot InventorySlot)
{
	QuickSlots[static_cast<uint8>(QuickSlot)] = InventorySlot;
}

/** Equip the next slot numerically */
ABasePlayerWeapon* UInventoryComponent::EquipNextSlot()
{
	uint8 InventorySlotIndex = (static_cast<uint8>(EquippedSlot) + 1) % static_cast<uint8>(EInventorySlot::NUM_SLOTS);
	for (; InventorySlotIndex != static_cast<uint8>(EquippedSlot); InventorySlotIndex = (InventorySlotIndex + 1) % static_cast<uint8>(EInventorySlot::NUM_SLOTS))
	{
		if (WeaponClasses[InventorySlotIndex] == NULL)
		{
			continue;
		}

		return EquipSlot(static_cast<EInventorySlot>(InventorySlotIndex));
	}
	return nullptr;
}

/** Equip the previous slot numerically */
ABasePlayerWeapon* UInventoryComponent::EquipPreviousSlot()
{
	uint8 InventorySlotIndex = static_cast<uint8>(EquippedSlot) == 0 ? static_cast<uint8>(EInventorySlot::NUM_SLOTS) - 1 : (static_cast<uint8>(EquippedSlot) - 1);
	for (; InventorySlotIndex != static_cast<uint8>(EquippedSlot); InventorySlotIndex = InventorySlotIndex == 0 ? static_cast<uint8>(EInventorySlot::NUM_SLOTS) - 1 : InventorySlotIndex - 1)
	{
		if (InventorySlotIndex == -1)
		{
			InventorySlotIndex = static_cast<uint8>(EInventorySlot::NUM_SLOTS) - 1;
		}

		if (WeaponClasses[InventorySlotIndex] == nullptr)
		{
			continue;
		}

		return EquipSlot(static_cast<EInventorySlot>(InventorySlotIndex));
	}
	return nullptr;
}

// Equip the previous weapon that was equipped
ABasePlayerWeapon* UInventoryComponent::EquipPreviousWeapon()
{
	return EquipSlot(PreviouslyEquippedSlot);
}

// Get the weapon data for the inventory slot
bool UInventoryComponent::GetWeaponData(FWeaponData& WeaponData, EInventorySlot InventorySlot)
{
	if (InventorySlot == EInventorySlot::NUM_SLOTS)
	{
		return false;
	}

	// No weapon in that slot
	TSubclassOf<class ABasePlayerWeapon>& WeaponClass = WeaponClasses[static_cast<uint8>(InventorySlot)];
	if (WeaponClass == NULL)
	{
		return false;
	}

	// Inventory not on player
	ARCCharacter* Player = GetOwner<ARCCharacter>();
	if (Player == nullptr)
	{
		return false;
	}

	ARCPlayerState* PlayerState = Player->GetPlayerState<ARCPlayerState>();
	ASSERT_RETURN_VALUE(PlayerState != nullptr, false, "Player doesn't have RCPlayerState");

	WeaponData = PlayerState->FindOrAddWeaponDataForClass(WeaponClass);

	return true;
}

// Get weapon class for the slot
bool UInventoryComponent::GetWeaponClass(TSubclassOf<class ABasePlayerWeapon>& WeaponClass, EInventorySlot InventorySlot)
{
	if (InventorySlot == EInventorySlot::NUM_SLOTS)
	{
		return false;
	}

	WeaponClass = WeaponClasses[static_cast<uint8>(InventorySlot)];
	return WeaponClass != NULL;
}

// Is there a weapon in the given slot
bool UInventoryComponent::IsSlotOccupied(EInventorySlot Slot)
{
	return (Slot != EInventorySlot::NUM_SLOTS) && (WeaponClasses[static_cast<uint8>(Slot)] != NULL);
}

// Spawn the weapon from the slot
ABasePlayerWeapon* UInventoryComponent::SpawnSlot(EInventorySlot NewSlot)
{
	ASSERT_RETURN_VALUE(NewSlot != EInventorySlot::NUM_SLOTS, nullptr, "Trying to equip invalid slot");

	TSubclassOf<ABasePlayerWeapon>& WeaponClass = WeaponClasses[static_cast<uint8>(NewSlot)];
	ASSERT_RETURN_VALUE(WeaponClass != NULL, nullptr, "Trying to equip slot that has no weapon");

	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, nullptr);

	AActor* Owner = GetOwner();
	ASSERT_RETURN_VALUE(Owner != nullptr, nullptr);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("CurrentWeapon");
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Cast<APawn>(Owner);
	SpawnParams.bNoFail = true;

	FTransform SpawnTransform = FTransform::Identity;

	return World->SpawnActor<ABasePlayerWeapon>(WeaponClasses[static_cast<uint8>(NewSlot)], SpawnTransform, SpawnParams);
}