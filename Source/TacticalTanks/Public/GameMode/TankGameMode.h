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
	void OnPlayerKilled(APlayerController* ScoringPlayer, APlayerController* Victim);
	virtual void PostLogin(APlayerController* NewPlayer) override;
protected:

	virtual void BeginPlay() override;
private:
	TArray<TObjectPtr<APlayerStart>> AvailablePlayerStarts;

	bool PlayerStartsInitialized = false;
	bool IsPlayerStartAvailable(APlayerStart* PlayerStart) const;

	/* Player Colors*/
	UPROPERTY(EditAnywhere, Category = "Player Colors")
	TMap<int32, FLinearColor> PlayerColors;

	int32 CurrentPlayerCount = 0;

};
