// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TankPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorChangedDelegate, FLinearColor, NewColor);

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
	void SetPlayerColor(FLinearColor InColor);
	FLinearColor GetPlayerColor() const { return PlayerColor;}
	bool IsPlayerColorSet() const { return bPlayerColorSet;}

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnColorChangedDelegate OnColorChanged;
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> TankContext;

	UPROPERTY()
	TObjectPtr<ATankPawn> ControlledTank;

	UPROPERTY()
	TObjectPtr<ATankPawn> NewTank;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LMBAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> FireAction;

	FLinearColor PlayerColor = FLinearColor::White;
	bool bPlayerColorSet = false;

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
