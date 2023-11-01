// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/TankPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Actor/TankProjectile.h"
#include <Net/UnrealNetwork.h>
#include "Player/TankPlayerController.h"
#include "Actor/Component/TankMovementComponent.h"
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

	MovementComponent = CreateDefaultSubobject<UTankMovementComponent>(TEXT("TankMovementComponent"));
	MovementComponent->SetIsReplicated(true);
	
}



void ATankPawn::BeginPlay()
{
	Super::BeginPlay();
	
}
void ATankPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATankPawn, ServerTargetTopYaw);
	DOREPLIFETIME(ATankPawn, TankColor);
}

void ATankPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	InterpolateBarrelTowardsAimTarget(DeltaTime);
}


void ATankPawn::InterpolateBarrelTowardsAimTarget(float DeltaTime)
{
	float TargetYaw = IsLocallyControlled() ? ClientTargetTopYaw : ServerTargetTopYaw;
	FRotator CurrentTopRotation = TopMesh->GetComponentRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentTopRotation, FRotator(CurrentTopRotation.Pitch, TargetYaw, CurrentTopRotation.Roll), DeltaTime, AimRotationRate);
	TopMesh->SetWorldRotation(FRotator(NewRotation));
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

void ATankPawn::TimedTopYawUpdate()
{
	ServerUpdateServerTargetTopYaw(ClientTargetTopYaw);
}
void ATankPawn::ServerUpdateServerTargetTopYaw_Implementation(float ClientTopYaw)
{
	ServerTargetTopYaw = ClientTopYaw;
}

void ATankPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	ATankPlayerController* PC = Cast<ATankPlayerController>(NewController);
	if (PC)
	{
		PC->OnColorChanged.AddDynamic(this, &ATankPawn::HandleColorChange);
		if (PC->IsPlayerColorSet())
		{
			TankColor = PC->GetPlayerColor();
			OnRep_TankColor();
		}
	}
}

void ATankPawn::SetTankColor(FLinearColor NewColor)
{
	TankColor = NewColor;
	OnRep_TankColor();
}
void ATankPawn::HandleColorChange(FLinearColor Color)
{
	SetTankColor(Color);
}

void ATankPawn::OnRep_TankColor()
{
	SetColor(TankColor);
}

void ATankPawn::SetColor(FLinearColor Color)
{
	if (TankMaterialInstance == nullptr) return;
	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(TankMaterialInstance, this);
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(FName("Color"), Color);
		BaseMesh->SetMaterial(0, DynamicMaterial);
		TopMesh->SetMaterial(0, DynamicMaterial);
		BarrelMesh->SetMaterial(0, DynamicMaterial);
	}

}



void ATankPawn::FireButtonPressed()
{
	if (!bCanFire) return;

	ServerFire();
	// Limit firing functionality, so the clients cannot spam server RPC's
	bCanFire = false;
	GetWorldTimerManager().SetTimer(ResetCanFireHandle, this, &ATankPawn::ResetCanFire, FireRate, false);
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
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// Spawn the projectile
		ATankProjectile* SpawnedProjectile = GetWorld()->SpawnActor<ATankProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	}
}
bool ATankPawn::ServerFire_Validate()
{
	// Check if the player can fire... has ammo, not spamming
	return true;
}



void ATankPawn::SetNewMoveToDestination(const FVector& NewLocation)
{
	if (MovementComponent) MovementComponent->SetMoveToDestination(NewLocation);
}

