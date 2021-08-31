// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StreamableManager.h"
#include "DataSingleton.generated.h"

static const char* COLLISION_PRESET_PLAYERBULLET = "PlayerBullet";

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class RC_API UDataSingleton : public UObject
{
	GENERATED_BODY()
public:
	static UDataSingleton& Get();
	FStreamableManager AssetLoader;

	UPROPERTY(EditAnywhere, Category = LODSetting)
	class UCurveFloat* LevelDilationCurve;

private:
};
