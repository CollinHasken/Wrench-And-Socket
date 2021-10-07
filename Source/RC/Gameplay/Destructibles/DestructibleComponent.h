// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DestructibleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RC_API UDestructibleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Apply hit to destructible	 
	UFUNCTION(BlueprintCallable)
	void ApplyHit();

private:
	// Destroy this destructible
	void Destroy();

	// The max health
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Destructible, meta = (AllowPrivateAccess = "true"))
	int HitsToDestruction = 1;

	// The current health
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Destructible, meta = (AllowPrivateAccess = "true"))
	int CurrentHits = 0;		

	// The class to spawn once destroyed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Destructible, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> HuskClass = NULL;
};
