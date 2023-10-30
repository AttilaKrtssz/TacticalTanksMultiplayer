// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/TankPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Components/SplineComponent.h"

ATankPawn::ATankPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	TankRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TankRoot"));
	RootComponent = TankRoot;

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(GetRootComponent());

	TopMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopMesh"));
	TopMesh->SetupAttachment(GetRootComponent());

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMesh->SetupAttachment(TopMesh);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);

	NavigationSpline = CreateDefaultSubobject<USplineComponent>(TEXT("NavigationSpline"));

}

void ATankPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATankPawn::AutoMove()
{
	if (!bAutoMove) return;
	const FVector LocationOnSpline = NavigationSpline->FindLocationClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);
	const FVector Direction = NavigationSpline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
	AddMovementInput(Direction);
	const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
	if (DistanceToDestination <= AutoMoveAcceptanceRadius)
	{
		bAutoMove = false;
	}
}

void ATankPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AutoMove();
	// Get the current movement input vector
	FVector CurrentInput = ConsumeMovementInputVector();

	// Update the pawn's position based on the input
	FVector NewLocation = GetActorLocation() + (CurrentInput * 200.f * DeltaTime);
	SetActorLocation(NewLocation);
}


void ATankPawn::SetNewMoveToDestination(const FVector& NewLocation)
{
	CachedDestination = NewLocation;
	UE_LOG(LogTemp, Warning, TEXT("Blocking Hit and Pawn is valid"));
	if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, GetActorLocation(), CachedDestination))
	{
		NavigationSpline->ClearSplinePoints();
		for (const FVector& PointLoc : NavPath->PathPoints)
		{
			NavigationSpline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
			DrawDebugSphere(GetWorld(), PointLoc, 10, 10, FColor::Green, false, 10.f);
		}
		if (NavPath->PathPoints.Num() > 0)
		{
			CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
			bAutoMove = true;
		}
	}
}

