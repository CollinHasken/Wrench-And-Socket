// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StreamableManager.h"
#include "DataSingleton.generated.h"

static const char* COLLISION_PRESET_PLAYERBULLET = "PlayerBullet";

/**
 * Singleton to hold global data from the editor
 */
UCLASS(Blueprintable, BlueprintType)
class RC_API UDataSingleton : public UObject
{
	GENERATED_BODY()
public:
	// Get the singleton
	static UDataSingleton& Get();

	// Asset loader
	FStreamableManager AssetLoader;

	// Curve to dilate time during a level up
	UPROPERTY(EditAnywhere, Category = LODSetting)
	class UCurveFloat* LevelDilationCurve;
};
