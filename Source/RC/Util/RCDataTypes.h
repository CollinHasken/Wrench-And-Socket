// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedStruct.h"
#include "RCDataTypes.generated.h"


/**
 * Base data all info datas should derive from
 */
UCLASS()
class UBaseData : public UUserDefinedStruct
{
	GENERATED_BODY()

public:
	virtual ~UBaseData() {};

	// Serialize savegame properties
	virtual void Serialize(FArchive& Ar);

	/**
	 * Set the default values the data should have
	 *
	 * @param Id	The asset Id for the data
	 */
	virtual void SetDefaults(const FPrimaryAssetId& Id) { AssetId = Id; };

protected:
	FPrimaryAssetId AssetId = FPrimaryAssetId();
};

/**
 * Map of a UClass's instances to their data
 * Needs to be a struct to be a UPROPERTY to not be GCed
 */
USTRUCT()
struct FDataMap
{
	GENERATED_USTRUCT_BODY()
public:
	TMap<FPrimaryAssetId, UBaseData*>* operator->() { return &DataMap; }

private:
	// Actual map
	UPROPERTY()
	TMap<FPrimaryAssetId, UBaseData*> DataMap;
};

typedef TMap<FPrimaryAssetId, UBaseData*> SaveDataMap;
typedef TMap<const UClass*, FDataMap> SaveDataClassMap;

FArchive& operator<<(FArchive& Ar, FPrimaryAssetId& SObj);;
