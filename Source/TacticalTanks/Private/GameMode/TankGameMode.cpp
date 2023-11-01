// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TankGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameMode/TankGameState.h"
#include "Player/TankPlayerController.h"
#include "Actor/TankPawn.h"
#include "Kismet/KismetArrayLibrary.h"

AActor* ATankGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    if (AvailablePlayerStarts.Num() == 0)
        return Super::ChoosePlayerStart_Implementation(Player);

    int32 RandomIndex;
    int32 NumberOfRandomTries = 10;
    for (int32 i = 0; i < NumberOfRandomTries; i++)
    {
        RandomIndex = FMath::RandRange(0, AvailablePlayerStarts.Num() - 1);
        if (IsPlayerStartAvailable(AvailablePlayerStarts[RandomIndex]))
        {
            return AvailablePlayerStarts[RandomIndex];
        }
    }
    // Fallback to the default behavior if no suitable player start is found
    return Super::ChoosePlayerStart_Implementation(Player);
}

void ATankGameMode::OnPlayerKilled(APlayerController* ScoringPlayer, APlayerController* Victim)
{
    // Handle Score for the Scoring Player
    ATankGameState* TankGameState = GetGameState<ATankGameState>();
    if (TankGameState && ScoringPlayer) TankGameState->IncrementPlayersKills(ScoringPlayer->PlayerState);

    // Spawn a New tank and Possess
    UWorld* World = GetWorld();
    if (World && Victim)
    {
        AActor* PlayerStart = ChoosePlayerStart(Victim);
        FVector SpawnLocation = PlayerStart ? PlayerStart->GetActorLocation() : FVector(0);  
        FRotator SpawnRotation = PlayerStart ? PlayerStart->GetActorRotation() : FRotator(0);  

        // Spawn the tank
        ATankPawn* NewTank = World->SpawnActor<ATankPawn>(DefaultPawnClass, SpawnLocation, SpawnRotation);
        ATankPlayerController* TankPC = Cast<ATankPlayerController>(Victim);
        if (NewTank == nullptr)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn new tank!!!"));
        }
        if (NewTank && TankPC)
        {
            NewTank->SetTankColor(TankPC->GetPlayerColor());
            TankPC->HandleRespawn(NewTank);
        }
    }
}

void ATankGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    // Assign color to the new player.
    ATankPlayerController* NewPC = Cast<ATankPlayerController>(NewPlayer);
    FLinearColor NewPlayersColor = FLinearColor::White;
    if (PlayerColors.Contains(CurrentPlayerCount))
    {
        NewPlayersColor = PlayerColors[CurrentPlayerCount];
    }
    if (NewPC) NewPC->SetPlayerColor(NewPlayersColor);

    CurrentPlayerCount++;
}

void ATankGameMode::BeginPlay()
{
	Super::BeginPlay();

    // Populate the array with all PlayerStart actors in the level
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<APlayerStart> It(World); It; ++It)
        {
            AvailablePlayerStarts.Add(*It);
        }
    }
}

bool ATankGameMode::IsPlayerStartAvailable(APlayerStart* PlayerStart) const
{
    FCollisionShape CollisionShape = FCollisionShape::MakeSphere(50.f);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(PlayerStart);
    bool bOverlapTestResult = GetWorld()->OverlapAnyTestByChannel(PlayerStart->GetActorLocation(), FQuat::Identity, ECC_Pawn, CollisionShape, QueryParams);
    return !GetWorld()->OverlapAnyTestByChannel(PlayerStart->GetActorLocation(), FQuat::Identity, ECC_Pawn, CollisionShape, QueryParams);
}
