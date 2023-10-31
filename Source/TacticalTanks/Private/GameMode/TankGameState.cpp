// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TankGameState.h"
#include <Net/UnrealNetwork.h>
#include "GameFramework/PlayerState.h"

ATankGameState::ATankGameState()
{
	bReplicates = true;
}

void ATankGameState::IncrementPlayersKills(APlayerState* InPS)
{
	if (InPS == nullptr) return;
	TArray<FPlayerScore> TempScores = Scores;
	Scores.Empty();
	bool bPlayerFound = false;
	for (FPlayerScore& PlayerScore : TempScores)
	{
		if (PlayerScore.PS == InPS)
		{
			bPlayerFound = true;
			PlayerScore.Kills++;
		}
		Scores.Add(PlayerScore);
	}
	if (!bPlayerFound)
	{
		FPlayerScore NewEntry;
		NewEntry.PS = InPS;
		NewEntry.PlayerName = FText::FromString(InPS->GetPlayerName());
		NewEntry.Kills = 1;
		Scores.Add(NewEntry);
	}
	OnRep_Scores();
}

void ATankGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATankGameState, Scores);
}

void ATankGameState::OnRep_Scores()
{
	OnScoresChanged.Broadcast();
}
