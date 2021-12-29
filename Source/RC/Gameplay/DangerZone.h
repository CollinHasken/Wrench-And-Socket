// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DangerZone.generated.h"

/**
 * Different ways to apply the impulse to the actors inside
 */
UENUM()
enum class EDangerZoneImpulseTypes : uint8
{
	Center	UMETA(DisplayName = "From The Center"),
	BoostUp	UMETA(DisplayName = "Boost Forward and Upwards")
};

/**
 * Zone that can damage and/or impulse actors that enter it
 */
UCLASS()
class RC_API ADangerZone : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADangerZone();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Returns Trigger subobject 
	FORCEINLINE class UBoxComponent* GetTrigger() const { return Trigger; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// On an actor entering the danger zone trigger
	UFUNCTION()
	void OnDangerZoneEntered(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// On an actor exiting the danger zone trigger
	UFUNCTION()
	void OnDangerZoneExited(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Apply the impulse to the given actor
	void ApplyImpulse(AActor* Actor) const;

	// Should this danger zone tick
	bool ShouldTick() const;

	// Should damage be dealt once an actor enters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	bool bDealsDamageOnEnter;

	// Amount of damage dealt once an actor enters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true", EditCondition = "bDealsDamageOnEnter", EditConditionHides))
	float DamageOnEnter;

	// Should damage be dealt each tick
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true"))
	bool bDealsDamageEachTick;

	// Damage dealt each tick
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (AllowPrivateAccess = "true", EditCondition = "bDealsDamageEachTick", EditConditionHides))
	float DamageEachTick;

	// Should an impulse be applied to the actors inside
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impulse", meta = (AllowPrivateAccess = "true"))
	bool bApplyImpulse;

	// Should an impulse be applied to the actors inside each tick
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impulse", meta = (AllowPrivateAccess = "true", EditCondition = "bApplyImpulse", EditConditionHides))
	bool bApplyImpulseEachTick;

	// Should the actor's movement be stopped once they enter
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impulse", meta = (AllowPrivateAccess = "true", EditCondition = "bApplyImpulse", EditConditionHides))
	bool bStopMovementOnEnter;

	// Impulse magnitude applied to the actors inside
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impulse", meta = (AllowPrivateAccess = "true", EditCondition = "bApplyImpulse", EditConditionHides))
	float ImpulseMagnitude;

	// How to determine the impulse direction
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impulse", meta = (AllowPrivateAccess = "true", EditCondition = "bApplyImpulse", EditConditionHides))
	EDangerZoneImpulseTypes ImpulseType = EDangerZoneImpulseTypes::BoostUp;

	// Trigger to listen for actors entering/exiting the danger zone
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Teleport, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* Trigger;

	// List of actors inside the danger zone
	UPROPERTY()
	TArray<AActor*> ActorsInside;
};
