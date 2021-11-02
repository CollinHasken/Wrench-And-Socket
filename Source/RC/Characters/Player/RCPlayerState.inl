#pragma once

#include "RC/Debug/Debug.h"

class ARCPlayerstate;

// Find the data for a specific primary asset
template<class DataClass>
DataClass* ARCPlayerState::FindDataForAsset(const FPrimaryAssetId& AssetId)
{ 
	UBaseData* Data = FindDataForAsset(DataClass::StaticClass(), AssetId);
	return Data != nullptr ? Cast<DataClass>(Data) : nullptr;
}

// Find or add the data for a specific primary asset
template<class DataClass>
DataClass* ARCPlayerState::FindOrAddDataForAsset(const FPrimaryAssetId& AssetId)
{
	return Cast<DataClass>(FindOrAddDataForAsset(DataClass::StaticClass(), AssetId));
}

// Add a data for a specific primary asset, checking to make sure it's unique
template<class DataClass>
DataClass* ARCPlayerState::AddDataForAsset(const FPrimaryAssetId& AssetId)
{
	// Make sure it's unique
	return Cast<DataClass>(AddDataForAsset(DataClass::StaticClass(), AssetId));
}

// Add a data for a specific primary asset, checking to make sure it's unique
template<class DataClass>
DataClass* ARCPlayerState::AddDataForAssetQuick(const FPrimaryAssetId& AssetId)
{
	return Cast<DataClass>(AddDataForAssetQuick(DataClass::StaticClass(), AssetId));
}
