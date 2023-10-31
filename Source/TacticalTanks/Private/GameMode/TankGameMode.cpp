// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/TankGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

AActor* ATankGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    for (APlayerStart* PlayerStart : AvailablePlayerStarts)
    {
        if (IsPlayerStartAvailable(PlayerStart))
        {
            return PlayerStart;
        }
    }
    return Super::ChoosePlayerStart_Implementation(Player);
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
    FCollisionShape CollisionShape = FCollisionShape::MakeSphere(100.0f);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(PlayerStart);
 
    return !GetWorld()->OverlapAnyTestByChannel(PlayerStart->GetActorLocation(), FQuat::Identity, ECC_Pawn, CollisionShape, QueryParams);
}
