// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TankPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ATankPawn;
/**
 * 
 */
UCLASS()
class TACTICALTANKS_API ATankPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ATankPlayerController();
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> TankContext;

	UPROPERTY()
	TObjectPtr<ATankPawn> ControlledTank;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LMBAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> FireAction;


	void LeftMouseButtonPressed();
	void FireButtonPressed();
	ATankPawn* GetControlledTank();
};