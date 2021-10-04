// Fill out your copyright notice in the Description page of Project Settings.


#include "RCTypes.h"

#include "Engine/AssetManager.h"

#include "RC/Debug/Debug.h"
#include "RC/Weapons/Weapons/BasePlayerWeapon.h"


FArchive& operator<<(FArchive& Ar, FPrimaryAssetId& SObj)
{
	// Need to serialize with the string representation as the FName is only reliable for the current session
	if (Ar.IsLoading())
	{
		FString InString;
		int32 InNumber;
		Ar << InString << InNumber;
		FPrimaryAssetType AssetType = FPrimaryAssetType(FName(*InString, InNumber));
		Ar << InString << InNumber;
		SObj = FPrimaryAssetId(AssetType, FName(*InString, InNumber));
	}
	else
	{
		FString OutString = SObj.PrimaryAssetType.ToString();
		int32 OutNumber = SObj.PrimaryAssetType.GetName().GetNumber();
		Ar << OutString << OutNumber;
		OutString = SObj.PrimaryAssetName.ToString();
		OutNumber = SObj.PrimaryAssetName.GetNumber();
		Ar << OutString << OutNumber;
	}
	return Ar;
}

FArchive& operator<<(FArchive& Ar, SaveDataStructMap& SObj)
{
	if (Ar.IsLoading())
	{
		// Get how many data types
		int32 NumElements;
		Ar << NumElements;
		for (int32 CurElement = 0; CurElement < NumElements; ++CurElement)
		{
			// Get the data struct
			UScriptStruct* DataStruct = nullptr;
			Ar << DataStruct;

			SaveDataMap DataMap;

			// Get how many entries for the data type
			int32 NumDatas;
			Ar << NumDatas;
			for (int32 CurData = 0; CurData < NumDatas; ++CurData)
			{
				// Get the asset Id
				FPrimaryAssetId AssetId;
				Ar << AssetId;

				// Create a new data, initialize, setup the defaults, and load the saved data
				FBaseData* NewData = (FBaseData*)FMemory::Malloc(DataStruct->GetCppStructOps()->GetSize());
				DataStruct->InitializeStruct(NewData);
				NewData->SetDefaults(AssetId);
				NewData->Serialize(Ar);

				DataMap.Add(AssetId, TSharedPtr<FBaseData>(NewData));
			}

			SObj.Add(DataStruct, DataMap);
		}
	}
	else
	{
		// Save off how many data types
		int32 NumElements = SObj.Num();
		Ar << NumElements;
		for (TPair< UScriptStruct*, SaveDataMap>& DataMapPair : SObj)
		{
			// Save the data struct
			Ar << DataMapPair.Key;

			// Save how many entries for this data type
			NumElements = DataMapPair.Value.Num();
			Ar << NumElements;
			for (TPair<FPrimaryAssetId, TSharedPtr<FBaseData>>& DataPair : DataMapPair.Value)
			{
				FBaseData* Data = DataPair.Value.Get();
				if (Data == nullptr)
				{
					continue;
				}
				// Save the asset Id and serialize the data
				Ar << DataPair.Key;
				Data->Serialize(Ar);
			}
		}
	}

	return Ar;
}

// Serialize savegame properties
void FBaseData::Serialize(FArchive& Ar)
{
	if (Ar.IsLoading() || Ar.IsSaving())
	{
		ASSERT(DataStruct != FBaseData::StaticStruct(), "The Struct wasn't set in the constructor for the data");
		DataStruct->SerializeTaggedProperties(Ar, (uint8*)this, DataStruct, nullptr);
	}
}

// Set the default values the data should have
void FWeaponData::SetDefaults(const FPrimaryAssetId& Id)
{
	Super::SetDefaults(Id);

	// Set the values to the default
	if (UAssetManager* Manager = UAssetManager::GetIfValid())
	{
		const UObject* AssetObject = Manager->GetPrimaryAssetObject(AssetId);
		ASSERT_RETURN(AssetObject != nullptr, "Unable to get asset for %s", *AssetId.ToString());

		const UPlayerWeaponInfo* PlayerWeaponInfo = Cast<UPlayerWeaponInfo>(AssetObject);
		ASSERT_RETURN(PlayerWeaponInfo != nullptr, "Asset wasn't the correct type of UPlayerWeaponInfo %s", *AssetId.ToString());
		
		CurrentAmmo = PlayerWeaponInfo->BaseMaxAmmo;
		MaxAmmo = PlayerWeaponInfo->BaseMaxAmmo;
		XPTotalForNextLevel = PlayerWeaponInfo->WeaponLevelConfigs[1].XPNeeded;
	}	
}

// Grant XP
void FWeaponData::GrantDamageXP(float XP)
{
	CurrentXP += XP;

	// If the weapon still exists, let it know
	ABasePlayerWeapon* Weapon = CurrentWeapon.Get();
	if (Weapon != nullptr)
	{
		Weapon->OnXPGained(XP);
	}
}
