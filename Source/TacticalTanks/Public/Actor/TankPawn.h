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

	virtual void PossessedBy(AController* NewController) override;
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

	/* Implement Click to move behaviour */
	

	FVector CachedDestination = FVector::ZeroVector;

	// Spline from the controlled pawn to the Cached destination
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> NavigationSpline;

	// The pawn will get this close to the destination
	UPROPERTY(EditDefaultsOnly, Category = "Tank|Movement")
	float AutoMoveAcceptanceRadius = 50.f;

	// When true, we interpolate the tank's location and rotation along the spline
	bool bAutoMove = false;
	void HandleTankMovement(float DeltaTime);

	// Send Path calculated on the Client to the server 
	UFUNCTION(Server,Reliable,WithValidation)
	void ServerSetNewMoveToDestination(const TArray<FVector>& PathPoints);

	// PathPoints determined on the LocallyControlled Client, set with Server RPC
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedPathPoints)
	TArray<FVector> ReplicatedPathPoints;

	// Using the Pathpoints generated by the Navigation system we create a spline and start mowing along it
	void CreateSplineAndStartMoving(const TArray<FVector>& PathPoints);

	UFUNCTION()
	void OnRep_ReplicatedPathPoints();

	/* Tank Movement related variable*/

	// Speed of the tank in units per second
	UPROPERTY(EditAnywhere, Category = "Tank|Movement")
	float TankSpeed = 200.f;  

	// Rotation speed in degrees per second
	UPROPERTY(EditAnywhere, Category = "Tank|Movement")
	float TankRotationRate = 45.f;  

	// Track how far along the spline the tank has moved
	float DistanceTraveled = 0.f;  

	// Helper variable, to stay above the ground
	UPROPERTY(EditAnywhere, Category = "Tank|Movement")
	float TankHeight = 50.f;

	/* Aiming related functionality*/

	// Set on the Locally Controlled Tank, we calculate the ClientTargetTopYaw based of this
	FVector AimTarget;

	// Defines how fast the rotation of the barrel interpolates towards the mouse direction
	UPROPERTY(EditDefaultsOnly, Category = "Tank|Aiming")
	float AimRotationRate = 5.f;

	// We interpolate the TopMesh's Yaw rotation towards this value, only on the LocallyControlled Tank
	float ClientTargetTopYaw = 0.0f;

	// We interpolate the client/server Tank's TopMesh's yaw rotation 
	UPROPERTY(Replicated)
	float ServerTargetTopYaw = 0.0f;

	FTimerHandle UpdateServerAimYawTimer;

	// With this frequecy we update the server's TargetTopYaw using RPC
	UPROPERTY(EditDefaultsOnly, Category = "Tank|Aiming")
	float UpdateServerTopYawFrequency = 0.2f;

	UFUNCTION()
	void TimedTopYawUpdate();

	UFUNCTION(Server, Reliable)
	void ServerUpdateServerTargetTopYaw(float ClientTopYaw);

	// Smoothly update the TopMesh representing aiming towards the mouse
	void InterpolateBarrelTowardsAimTarget(float DeltaTime);

	/* Firing functionality*/
	// Projectile class this Tank will use to Fire
	UPROPERTY(EditDefaultsOnly, Category = "Tank|Combat")
	TSubclassOf<ATankProjectile> ProjectileClass;

	UFUNCTION(Server,Reliable,WithValidation)
	void ServerFire();

	/* Tank color related functionality */

	UPROPERTY(EditAnywhere, Category = "Tank|Materials")
	TObjectPtr<UMaterialInstance> TankMaterialInstance;

	UPROPERTY(ReplicatedUsing = OnRep_TankColor)
	FLinearColor TankColor = FLinearColor::White;

	UFUNCTION()
	void OnRep_TankColor();

	UFUNCTION()
	void HandleColorChange(FLinearColor Color);
	void SetColor(FLinearColor Color);
};
