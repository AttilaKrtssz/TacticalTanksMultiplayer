// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TankGameState.generated.h"

USTRUCT(BlueprintType)
struct FPlayerScore
{
    GENERATED_USTRUCT_BODY()

    TObjectPtr<APlayerState> PS = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    FText PlayerName = FText::FromString("");

    UPROPERTY(BlueprintReadWrite, Category = "Scoring")
    int32 Kills = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScoresChanged);


/**
 * 
 */
UCLASS()
class TACTICALTANKS_API ATankGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:

    ATankGameState();

    UPROPERTY(BlueprintAssignable, Category = "Scoring")
    FOnScoresChanged OnScoresChanged;

    // Getter function to get the scores
    UFUNCTION(BlueprintCallable, Category = "Scoring")
    TArray<FPlayerScore> GetScores() const { return Scores;};

    void IncrementPlayersKills(APlayerState* InPS); 

protected:
    virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

    UPROPERTY(ReplicatedUsing = OnRep_Scores, BlueprintReadOnly, Category = "Scoring")
    TArray<FPlayerScore> Scores;

    UFUNCTION()
    void OnRep_Scores();

};
