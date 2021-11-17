// Fill out your copyright notice in the Description page of Project Settings.
#include "RCPlayerState.h"

#include "Engine/AssetManager.h"

#include "RC/Debug/Debug.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Collectibles/Collectible.h"
#include "RC/Framework/RCGameInstance.h"
#include "RC/Save/RCSaveGame.h"
#include "RC/Weapons/Weapons/PlayerWeapons/BasePlayerWeapon.h"


void FPlayerStateData::Serialize(FArchive& Ar, ARCPlayerState& PlayerState)
{
	if (Ar.IsLoading())
	{
		// Get how many data types
		int32 NumElements;
		Ar << NumElements;
		for (int32 CurElement = 0; CurElement < NumElements; ++CurElement)
		{
			// Get the data class
			UClass* DataClass = nullptr;
			Ar << DataClass;

			FDataMap DataMap;

			// Get how many entries for the data type
			int32 NumDatas;
			Ar << NumDatas;
			for (int32 CurData = 0; CurData < NumDatas; ++CurData)
			{
				// Get the asset Id
				FPrimaryAssetId AssetId;
				Ar << AssetId;

				// Create a new data, initialize, setup the defaults, and load the saved data				
				UBaseData* NewData = DataMap->Emplace(AssetId, NewObject<UBaseData>(&PlayerState, DataClass));
				NewData->SetDefaults(AssetId);
				NewData->Serialize(Ar);
			}

			DataClassMap.Add(DataClass, DataMap);
		}
	}
	else
	{
		// Save off how many data types
		int32 NumElements = DataClassMap.Num();
		Ar << NumElements;
		for (TPair<const UClass*, FDataMap>& DataMapPair : DataClassMap)
		{
			// Save the data class. Safe to const cast as we're saving it off, not loading
			UClass* Class = const_cast<UClass*>(DataMapPair.Key);
			Ar << Class;

			// Save how many entries for this data type
			NumElements = DataMapPair.Value->Num();
			Ar << NumElements;
			for (TPair<FPrimaryAssetId, UBaseData*>& DataPair : *DataMapPair.Value.operator->())
			{
				UBaseData* Data = DataPair.Value;
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
}

// Save player data
void ARCPlayerState::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsSaveGame())
	{
		UClass* Class = ARCPlayerState::StaticClass();
		Class->SerializeTaggedProperties(Ar, (uint8*)this, Class, nullptr);
		SaveData.Serialize(Ar, *this);
	}
}

// Set the current checkpoint to respawn from
void ARCPlayerState::SetCheckpoint(const AActor* Checkpoint)
{ 
	SaveData.CheckpointPath = FSoftObjectPath(Checkpoint); 

	URCGameInstance* GameInstance = GetGameInstance<URCGameInstance>();
	ASSERT_RETURN(GameInstance != nullptr);

	bool bSucceeded = GameInstance->AutoSave();
	LOG_CHECK(bSucceeded, LogSave, Error, "Unable to auto save after checkpoint");
}

// Get the current checkpoint to respawn from
AActor* ARCPlayerState::GetCheckpoint() const
{
	if (SaveData.CheckpointPath.IsNull())
	{
		return nullptr;
	}

	UAssetManager& AssetManager = UAssetManager::Get();
	FAssetData CheckpointData;
	AssetManager.GetAssetDataForPath(SaveData.CheckpointPath, CheckpointData);

	if (!CheckpointData.IsAssetLoaded())
	{
		return nullptr;
	}

	return Cast<AActor>(CheckpointData.GetAsset());
}

// Collect the given collectible
void ARCPlayerState::CollectCollectible(ACollectible* Collectible)
{
	// Let the collectible actor know they've been collected
	ASSERT_RETURN(Collectible != nullptr);
	int AmountCollected = Collectible->Collect();

	// Let the collectible data know it's been collected
	FPrimaryAssetId CollectibleInfoId = Collectible->GetInfoId();
	UCollectibleData* CollectibleData = FindOrAddDataForAsset<UCollectibleData>(CollectibleInfoId);
	ASSERT_RETURN(CollectibleData != nullptr, "Weapon Data not able to be added");

	// Let others know it's been collected
	CollectibleCollectedDelegate.Broadcast(CollectibleInfoId, AmountCollected, CollectibleData->CurrentAmount);
}

// Find the data for a specific primary asset
UBaseData* ARCPlayerState::FindDataForAsset(const UClass* DataClass, const FPrimaryAssetId& AssetId)
{
	FDataMap* DataMap = SaveData.DataClassMap.Find(DataClass);
	if (DataMap == nullptr)
	{
		return nullptr;
	}

	UBaseData** Data = (*DataMap)->Find(AssetId);
	return Data != nullptr ? *Data : nullptr;
}

// Find or add the data for a specific primary asset
UBaseData* ARCPlayerState::FindOrAddDataForAsset(const UClass* DataClass, const FPrimaryAssetId& AssetId)
{
	UBaseData* Data = FindDataForAsset(DataClass, AssetId);
	return Data != nullptr ? Data : AddDataForAssetQuick(DataClass, AssetId);
}

// Add a data for a specific primary asset, checking to make sure it's unique
UBaseData* ARCPlayerState::AddDataForAsset(const UClass* DataClass, const FPrimaryAssetId& AssetId)
{
	// Make sure it's unique
	UBaseData* ExistingData = FindDataForAsset(DataClass, AssetId);
	if (ExistingData != nullptr)
	{
		ASSERT(ExistingData != nullptr, "Trying to add data for existing class");
		return ExistingData;
	}

	return AddDataForAssetQuick(DataClass, AssetId);
}

// Add a data for a specific primary asset, checking to make sure it's unique
UBaseData* ARCPlayerState::AddDataForAssetQuick(const UClass* DataClass, const FPrimaryAssetId& AssetId)
{
	// Get the data map for this data's class
	FDataMap& DataMap = SaveData.DataClassMap.FindOrAdd(DataClass);

	// Add data to the map
	UBaseData* Data = DataMap->Emplace(AssetId, NewObject<UBaseData>(this, DataClass));
	ASSERT_RETURN_VALUE(Data != nullptr, nullptr);

	Data->SetDefaults(AssetId);
	return Data;
}

// Save the player data for a level transition 
void ARCPlayerState::SaveForLevelTransition(URCLevelTransitionSave* SaveGame)
{
	ASSERT_RETURN(SaveGame != nullptr);

	// Pass the array to fill with data from Player State
	FMemoryWriter MemWriter(SaveGame->SavedPlayer.PlayerStateByteData);
	FSaveGameArchive Ar(MemWriter, true);
	Serialize(Ar);

	ARCCharacter* ControlledCharacter = Cast<ARCCharacter>(GetPawn());
	ASSERT_RETURN(ControlledCharacter != nullptr);

	SaveGame->SavePlayer(ControlledCharacter);
}

// Load the player data for a level transition
void ARCPlayerState::LoadForLevelTransition(const URCLevelTransitionSave* SaveGame)
{
	// Convert binary array back into actor's variables
	FMemoryReader MemReader(SaveGame->SavedPlayer.PlayerStateByteData);
	FSaveGameArchive Ar(MemReader, true);
	Serialize(Ar);

	ARCCharacter* ControlledCharacter = Cast<ARCCharacter>(GetPawn());
	ASSERT_RETURN(ControlledCharacter != nullptr);

	SaveGame->LoadPlayer(ControlledCharacter);
}

// Find the data map for a data class
const TMap<FPrimaryAssetId, UBaseData*>* ARCPlayerState::FindDataMap(const UClass* DataClass)
{
	return SaveData.DataClassMap.Find(DataClass)->operator->();
}
