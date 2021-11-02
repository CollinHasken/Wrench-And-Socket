// Fill out your copyright notice in the Description page of Project Settings.
#include "RCDataTypes.h"

#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/RCStatics.h"


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

// Serialize savegame properties
void UBaseData::Serialize(FArchive& Ar)
{
	if (Ar.IsLoading() || Ar.IsSaving())
	{
		UClass* DataClass = GetClass();
		DataClass->SerializeTaggedProperties(Ar, (uint8*)this, DataClass, nullptr);
	}
}
