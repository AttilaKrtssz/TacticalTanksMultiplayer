// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/TankPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Components/SplineComponent.h"
#include "Actor/TankProjectile.h"
#include <Net/UnrealNetwork.h>
#include "Engine/Engine.h"

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

	ProjectileSpawnScene = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawnScene"));
	ProjectileSpawnScene->SetupAttachment(BarrelMesh);
	
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

void ATankPawn::HandleTankMovement(float DeltaTime)
{
	// Calculate how far the tank should move this frame
	float DistanceToMove = TankSpeed * DeltaTime;

	// Update the distance traveled along the spline
	DistanceTraveled += DistanceToMove;

	// Get the new location and tangent (forward direction) on the spline based on the distance traveled
	FVector NewLocation = NavigationSpline->GetLocationAtDistanceAlongSpline(DistanceTraveled, ESplineCoordinateSpace::World);
	NewLocation.Z += TankHeight;
	FVector ForwardDirection = NavigationSpline->GetDirectionAtDistanceAlongSpline(DistanceTraveled, ESplineCoordinateSpace::World);

	// Interpolate the tank's position smoothly to the new location
	SetActorLocation(FMath::VInterpTo(GetActorLocation(), NewLocation, DeltaTime, 5.0f));

	// Interpolate the tank's rotation smoothly to face the forward direction
	FRotator TargetRotation = ForwardDirection.Rotation();
	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, TankRotationRate);
	SetActorRotation(NewRotation);

	// Check if the tank has reached the end of the spline (or is very close to it)
	if (DistanceTraveled >= NavigationSpline->GetSplineLength() - AutoMoveAcceptanceRadius)
	{
		bAutoMove = false;
		DistanceTraveled = 0.f;  // Reset for the next movement
	}
}

void ATankPawn::InterpolateBarrelTowardsAimTarget(float DeltaTime)
{
	FString DebugMessage = FString::Printf(TEXT("ServerTargetTopYaw = %f"), ServerTargetTopYaw);
	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, DebugMessage);
	float TargetYaw = IsLocallyControlled() ? ClientTargetTopYaw : ServerTargetTopYaw;
	FRotator CurrentTopRotation = TopMesh->GetComponentRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentTopRotation, FRotator(CurrentTopRotation.Pitch, TargetYaw, CurrentTopRotation.Roll), DeltaTime, AimRotationRate);
	TopMesh->SetWorldRotation(FRotator(NewRotation));
}

void ATankPawn::TimedTopYawUpdate()
{
	UE_LOG(LogTemp, Warning, TEXT("TimedTopYawUpdatecalled with client targetTopYaw: %f"), ClientTargetTopYaw);
	ServerUpdateServerTargetTopYaw(ClientTargetTopYaw);
}

void ATankPawn::ServerUpdateServerTargetTopYaw_Implementation(float ClientTopYaw)
{
	ServerTargetTopYaw = ClientTopYaw;
}

void ATankPawn::ServerFire_Implementation()
{
	if (ProjectileClass)
	{

		FVector SpawnLocation = ProjectileSpawnScene->GetComponentLocation();
		FRotator SpawnRotation = ProjectileSpawnScene->GetComponentRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetController();
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Spawn the projectile
		ATankProjectile* SpawnedProjectile = GetWorld()->SpawnActor<ATankProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	}
}
bool ATankPawn::ServerFire_Validate()
{
	// Check if the player can fire... has ammo, not spamming
	return true;
}


void ATankPawn::AutoMove(float DeltaTime)
{
	if (!bAutoMove) return;
	
	HandleTankMovement(DeltaTime);
}

void ATankPawn::ServerSetNewMoveToDestination_Implementation(const TArray<FVector>& PathPoints)
{
	DistanceTraveled = 0;
	NavigationSpline->ClearSplinePoints();
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
bool ATankPawn::ServerSetNewMoveToDestination_Validate(const TArray<FVector>& PathPoints)
{
	// We can implement anti-cheat here, check if the Player was cheating
	return true;
}

void ATankPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AutoMove(DeltaTime);
	InterpolateBarrelTowardsAimTarget(DeltaTime);
}

void ATankPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ATankPawn, ServerTargetTopYaw);
}


void ATankPawn::SetNewMoveToDestination(const FVector& NewLocation)
{
	DistanceTraveled = 0.f;
	CachedDestination = NewLocation;

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
			// Use the last point found by the NavigationSystem 
			CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
			bAutoMove = true;

			if (IsLocallyControlled() && !HasAuthority())
			{
				ServerSetNewMoveToDestination(NavPath->PathPoints);
			}
		}
	}
}

void ATankPawn::SetNewAimTarget(const FVector& NewAimTarget)
{
	AimTarget = NewAimTarget;

	FVector DirectionToTarget = (AimTarget - GetActorLocation()).GetSafeNormal();
	if (HasAuthority())
	{
		ServerTargetTopYaw = DirectionToTarget.Rotation().Yaw;
		if (IsLocallyControlled())
		{
			ClientTargetTopYaw = ServerTargetTopYaw;
		}
	}
	else
	{
		ClientTargetTopYaw = DirectionToTarget.Rotation().Yaw;
		if (!GetWorld()->GetTimerManager().IsTimerActive(UpdateServerAimYawTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(UpdateServerAimYawTimer, this, &ATankPawn::TimedTopYawUpdate, UpdateServerTopYawFrequency, true);
		}
	}

}

void ATankPawn::FireButtonPressed()
{
	ServerFire();
}

