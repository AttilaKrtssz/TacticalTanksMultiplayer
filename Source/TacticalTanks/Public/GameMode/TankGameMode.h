// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TankGameMode.generated.h"

class APlayerStart;
/**
 * 
 */
UCLASS()
class TACTICALTANKS_API ATankGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
protected:

	virtual void BeginPlay() override;
private:
	TArray<TObjectPtr<APlayerStart>> AvailablePlayerStarts;
	bool IsPlayerStartAvailable(APlayerStart* PlayerStart) const;
};
