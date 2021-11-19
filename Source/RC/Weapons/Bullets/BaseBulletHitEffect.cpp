// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseBulletHitEffect.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

ABaseBulletHitEffect::ABaseBulletHitEffect()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseBulletHitEffect::BeginPlay()
{
	Super::BeginPlay();

	// Spawn emitter
	if (ExplosionFX)
	{
		SpawnedFX = UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionFX, GetActorLocation(), GetActorRotation());
	}

	// Set how long to live
	SetLifeSpan(Lifetime);
}

void ABaseBulletHitEffect::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SpawnedFX)
	{
		SpawnedFX->Deactivate();
	}

	Super::EndPlay(EndPlayReason);
}
