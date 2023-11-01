// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TankProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
UCLASS()
class TACTICALTANKS_API ATankProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ATankProjectile();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "TankProjectile|Parameters")
	float LifeSpan = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "TankProjectile|Parameters")
	float InitialSpeed = 700.f;

	UPROPERTY(EditDefaultsOnly, Category = "TankProjectile|Parameters")
	float MaxSpeed = 700.f;

	UPROPERTY(EditDefaultsOnly, Category = "TankProjectile|Parameters")
	float GravityScale = 0.25f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;


	UPROPERTY(EditDefaultsOnly, Category = "TankProjectile|Parameters")
	TObjectPtr<UParticleSystem>  ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "TankProjectile|Parameters")
	TObjectPtr<USoundBase> ImpactSound;

};
