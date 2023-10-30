// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TankPawn.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class USplineComponent;
UCLASS()
class TACTICALTANKS_API ATankPawn : public APawn
{
	GENERATED_BODY()

public:
	ATankPawn();
	virtual void Tick(float DeltaTime) override;

	void SetNewMoveToDestination(const FVector& NewLocation);
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
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;


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
	void AutoMove();

};
