// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "RC/Framework/DamageInterface.h"

#include "BaseDestructible.generated.h"

UCLASS()
class RC_API ABaseDestructible : public AActor, public IDamageInterface
{
	GENERATED_BODY()
	
public:	
	ABaseDestructible();

	/**
	 * Request for this character to be damaged
	 * @param Params	The request params
	 */
	void RequestDamage(FDamageRequestParams& Params) override;

	/** Returns Destructible subobject **/
	FORCEINLINE class UDestructibleComponent* GetDestructible() const { return Destructible; }

	/** Returns Mesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Mesh; }

private:
	/** Destructible */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Destructible, meta = (AllowPrivateAccess = "true"))
	class UDestructibleComponent* Destructible;

	/** Mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;
};
