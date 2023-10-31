// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TankPawn.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class USplineComponent;
class ATankProjectile;
UCLASS()
class TACTICALTANKS_API ATankPawn : public APawn
{
	GENERATED_BODY()

public:
	ATankPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetNewMoveToDestination(const FVector& NewLocation);
	void SetNewAimTarget(const FVector& NewAimTarget);
	void FireButtonPressed();

protected:
	virtual void BeginPlay() override;


private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TankRoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> TopMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BarrelMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> ProjectileSpawnScene;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATankProjectile> ProjectileClass;

	/* Implement Click to move behaviour */
	// Location under the mouse cursor, cached click
	FVector CachedDestination = FVector::ZeroVector;

	// Spline from the controlled pawn to the Cached destination
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> NavigationSpline;

	// The pawn will get this close to the destination
	UPROPERTY(EditDefaultsOnly)
	float AutoMoveAcceptanceRadius = 50.f;

	bool bAutoMove = false;
	void AutoMove(float DeltaTime);
	void HandleTankMovement(float DeltaTime);

	// Send Path calculated on the Client to the server 

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerSetNewMoveToDestination(const TArray<FVector>& PathPoints);

	/* Tank Movement related variable*/
	UPROPERTY(EditAnywhere, Category = "Movement")
	float TankSpeed = 200.f;  // Speed of the tank in units per second

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TankRotationRate = 45.f;  // Rotation speed in degrees per second

	float DistanceTraveled = 0.f;  // Track how far along the spline the tank has moved

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TankHeight = 50.f;

	/* Aiming related functionality*/
	FVector AimTarget;

	UPROPERTY(EditDefaultsOnly)
	float AimRotationRate = 5.f;

	float ClientTargetTopYaw = 0.0f;

	UPROPERTY(Replicated)
	float ServerTargetTopYaw = 0.0f;

	void InterpolateBarrelTowardsAimTarget(float DeltaTime);

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerFire();

	float UpdateServerTopYawFrequency = 0.2f;
	
	UFUNCTION()
	void TimedTopYawUpdate();

	UFUNCTION(Server,Reliable)
	void ServerUpdateServerTargetTopYaw(float ClientTopYaw);

	FTimerHandle UpdateServerAimYawTimer;
};
