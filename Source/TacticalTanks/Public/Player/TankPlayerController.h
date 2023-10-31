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

	void HandleRespawn(ATankPawn* InNewTank);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	UPROPERTY()
	TObjectPtr<ATankPawn> NewTank;
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
	
	UFUNCTION()
	void FinishRespawn();

	UFUNCTION()
	void BlendCameraToNewTank();

	UFUNCTION(Client,Reliable)
	void Client_SetViewTarget(AActor* NewViewTarget);

};
