// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseBulletHitEffect.generated.h"

/**
 * Effect to spawn when a bullet hits an actor
 */
UCLASS(Abstract, Blueprintable)
class RC_API ABaseBulletHitEffect : public AActor
{
	GENERATED_BODY()
	
public:	
	ABaseBulletHitEffect();

	// Returns exploxion fx subobject
	FORCEINLINE class UParticleSystem* GetExplosionFX() const { return ExplosionFX; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** explosion FX */
	UPROPERTY(EditDefaultsOnly, Category = Effect, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ExplosionFX;

	/** Lifetime of effect in seconds */
	UPROPERTY(EditDefaultsOnly, Category = Effect, meta = (AllowPrivateAccess = "true"))
	float Lifetime = 2;
};
