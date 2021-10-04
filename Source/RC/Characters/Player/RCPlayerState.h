// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "RC/Util/RCTypes.h"

#include "RCPlayerState.generated.h"


/**
 * Player state data that will be saved
 */
USTRUCT()
struct FPlayerStateData
{
	GENERATED_USTRUCT_BODY()

public:
	// Save player data
	friend FArchive& operator<<(FArchive& Ar, FPlayerStateData& StateData);

	SaveDataStructMap DataStructMap;
};

/**
 * Player state
 */
UCLASS()
class RC_API ARCPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// Save player data
	virtual void Serialize(FArchive& Ar) override;

	/**
	 * Save the player data for a level transition 
	 * @param SaveGame	The Save to save to
	 */
	void SaveForLevelTransition(class URCLevelTransitionSave* SaveGame);

	/**
	 * Load the player data for a level transition
	 * @param SaveGame	The Save to load from
	 */
	void LoadForLevelTransition(const class URCLevelTransitionSave* SaveGame);

	/**
	 * Find or add the data for a specific primary asset
	 * @param AssetId	The Id of the primary asset associated with the data
	 *
	 * Returns the data
	 */
	template<class DataClass>
	DataClass* FindOrAddDataForAsset(const FPrimaryAssetId& AssetId);

	/**
	 * Find the data for a specific primary asset
	 * @param AssetId	The Id of the primary asset associated with the data
	 *
	 * Returns the data if found
	 */
	template<class DataClass> 
	DataClass* FindDataForAsset(const FPrimaryAssetId& AssetId);

	/**
	 * Add a data for a specific primary asset, checking to make sure it's unique
	 * @param AssetId	The Id of the primary asset associated with the data
	 *
	 * Returns the data that was added
	 */
	template<class DataClass>
	DataClass* AddDataForAsset(const FPrimaryAssetId& AssetId);

private:
	/**
	 * Add a data for a specific primary asset, skips checking for uniqueness
	 * @param AssetId	The Id of the primary asset associated with the data
	 *
	 * Returns the data that was added
	 */
	template<class DataClass>
	DataClass* AddDataForAssetQuick(const FPrimaryAssetId& AssetId);

	// Player save data
	UPROPERTY(SaveGame)
	FPlayerStateData SaveData;
};

#if CPP
#include "RCPlayerState.inl"
#endif
