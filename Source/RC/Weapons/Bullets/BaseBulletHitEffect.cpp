// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseBulletHitEffect.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseBulletHitEffect::ABaseBulletHitEffect()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseBulletHitEffect::BeginPlay()
{
	Super::BeginPlay();

	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionFX, GetActorLocation(), GetActorRotation());
	}

	SetLifeSpan(Lifetime);
}

// Called every frame
void ABaseBulletHitEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

