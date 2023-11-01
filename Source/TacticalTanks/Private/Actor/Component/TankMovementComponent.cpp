// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Component/TankMovementComponent.h"
#include <Net/UnrealNetwork.h>
#include "Components/SplineComponent.h"
#include "Actor/TankPawn.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UTankMovementComponent::UTankMovementComponent()
{

	PrimaryComponentTick.bCanEverTick = true;
	NavigationSpline = CreateDefaultSubobject<USplineComponent>(TEXT("NavigationSpline"));
}

void UTankMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UTankMovementComponent, ReplicatedPathPoints);

}
void UTankMovementComponent::SetMoveToDestination(const FVector& NewTargetLocation)
{
	CachedDestination = NewTargetLocation;

	if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, GetOwner()->GetActorLocation(), CachedDestination))
	{
		CreateSplineAndStartMoving(NavPath->PathPoints); // We handle the movement for the locally controlled tank
		if (GetOwningTank()->HasAuthority())
		{
			ReplicatedPathPoints = NavPath->PathPoints; // Set the replicated variable, so the clients start moving as well
		}
		else
		{
			ServerSetNewMoveToDestination(NavPath->PathPoints); // Send the PathPoints to the server and start moving
		}
	}
}
void UTankMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateTankGrounding();
	FVector SpawnLocation = GetOwner()->GetActorLocation();
	SpawnLocation.Z = TankZ;
	GetOwner()->SetActorLocation(SpawnLocation);

}


void UTankMovementComponent::OnRep_ReplicatedPathPoints()
{
	if (GetOwningTank() && GetOwningTank()->IsLocallyControlled()) return; // We handle the Locally Controlled tank when we generate the points
	CreateSplineAndStartMoving(ReplicatedPathPoints);
}

void UTankMovementComponent::CreateSplineAndStartMoving(const TArray<FVector>& PathPoints)
{
	NavigationSpline->ClearSplinePoints();
	DistanceTraveled = 0;

	for (const FVector& PointLoc : PathPoints)
	{
		NavigationSpline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
	}

	if (PathPoints.Num() > 0)
	{
		CachedDestination = PathPoints[PathPoints.Num() - 1];
		bAutoMove = true;
	}
}

void UTankMovementComponent::HandleTankMovement(float DeltaTime)
{
	// Calculate how far the tank should move this frame
	float DistanceToMove = TankSpeed * DeltaTime;

	// Update the distance traveled along the spline
	DistanceTraveled += DistanceToMove;

	// Get the new location and tangent (forward direction) on the spline based on the distance traveled
	FVector NewLocation = NavigationSpline->GetLocationAtDistanceAlongSpline(DistanceTraveled, ESplineCoordinateSpace::World);
	NewLocation.Z = TankZ;
	FVector ForwardDirection = NavigationSpline->GetDirectionAtDistanceAlongSpline(DistanceTraveled, ESplineCoordinateSpace::World);

	// Interpolate the tank's position smoothly to the new location
	GetOwner()->SetActorLocation(FMath::VInterpTo(GetOwner()->GetActorLocation(), NewLocation, DeltaTime, 5.0f));

	// Interpolate the tank's rotation smoothly to face the forward direction
	FRotator TargetRotation = ForwardDirection.Rotation();
	TargetRotation.Pitch = TankPitch;
	FRotator CurrentRotation = GetOwner()->GetActorRotation();
	FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, TankRotationRate);
	GetOwner()->SetActorRotation(NewRotation);

	// Check if the tank has reached the end of the spline (or is very close to it)
	if (DistanceTraveled >= NavigationSpline->GetSplineLength() - AutoMoveAcceptanceRadius)
	{
		bAutoMove = false;
		DistanceTraveled = 0.f;  // Reset for the next movement
	}
}

ATankPawn* UTankMovementComponent::GetOwningTank()
{
	if (!IsValid(OwningTank))
	{
		OwningTank = Cast<ATankPawn>(GetOwner());
	}
	return OwningTank;
}

void UTankMovementComponent::UpdateTankGrounding()
{
	FVector Start = GetOwner()->GetActorLocation();
	Start.Z += TankHeight;
	Start.X += 100.f;
	FVector End = Start - FVector(0, 0, 1000); // Adjust as needed
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility,Params))
	{
		TankZ = HitResult.Location.Z + TankHeight;
		FVector Normal = HitResult.Normal;
		TankPitch = FMath::Atan2(Normal.Y, Normal.Z) * (180.f / PI);
	}
}

void UTankMovementComponent::ServerSetNewMoveToDestination_Implementation(const TArray<FVector>& PathPoints)
{
	ReplicatedPathPoints = PathPoints;
	OnRep_ReplicatedPathPoints();
}
bool UTankMovementComponent::ServerSetNewMoveToDestination_Validate(const TArray<FVector>& PathPoints)
{
	return true;
}



void UTankMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTankGrounding();
	if (bAutoMove) HandleTankMovement(DeltaTime);

}



