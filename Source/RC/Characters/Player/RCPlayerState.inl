#pragma once

#include "RC/Debug/Debug.h"

class ARCPlayerstate;

// Find the data for a specific primary asset
template<class DataClass>
DataClass* ARCPlayerState::FindDataForAsset(const FPrimaryAssetId& AssetId)
{ 
	SaveDataMap* DataMap = SaveData.DataStructMap.Find(DataClass::StaticStruct());
	if (DataMap == nullptr)
	{
		return nullptr;
	}

	TSharedPtr<FBaseData>* Data = DataMap->Find(AssetId);
	return Data != nullptr ? StaticCastSharedPtr<DataClass, FBaseData>(*Data).Get() : nullptr;
}

// Find or add the data for a specific primary asset
template<class DataClass>
DataClass* ARCPlayerState::FindOrAddDataForAsset(const FPrimaryAssetId& AssetId)
{
	DataClass* Data = FindDataForAsset<DataClass>(AssetId);
	return Data != nullptr ? Data : AddDataForAssetQuick<DataClass>(AssetId);
}

// Add a data for a specific primary asset, checking to make sure it's unique
template<class DataClass>
DataClass* ARCPlayerState::AddDataForAsset(const FPrimaryAssetId& AssetId)
{
	// Make sure it's unique
	DataClass* ExistingData = FindDataForAsset<DataClass>(Class);
	if (ExistingData != nullptr)
	{
		ASSERT(ExistingData != nullptr, "Trying to add data for existing class");
		return ExistingData;
	}

	return AddDataForAssetQuick(AssetId);
}

// Add a data for a specific primary asset, checking to make sure it's unique
template<class DataClass>
DataClass* ARCPlayerState::AddDataForAssetQuick(const FPrimaryAssetId& AssetId)
{
	// Get the data map for this data's struct
	UScriptStruct* DataStruct = DataClass::StaticStruct();
	SaveDataMap& DataMap = SaveData.DataStructMap.FindOrAdd(DataStruct);

	// Add data to the map
	TSharedPtr<FBaseData>& DataPtr = DataMap.Emplace(AssetId, new DataClass());
	FBaseData* Data = DataPtr.Get();
	ASSERT_RETURN_VALUE(Data != nullptr, nullptr, "Unable to add data");

	Data->SetDefaults(AssetId);

	return StaticCast<DataClass*>(Data);
}
