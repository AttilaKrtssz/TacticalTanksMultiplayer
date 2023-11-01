// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/TankProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Actor/TankPawn.h"
#include "GameMode/TankGameMode.h"
#include "GameFramework/PlayerController.h"

ATankProjectile::ATankProjectile()
{
 
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->ProjectileGravityScale = GravityScale;
}


void ATankProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetLifeSpan(LifeSpan);
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &ATankProjectile::OnSphereOverlap);
	}

}

void ATankProjectile::Destroyed()
{
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactParticles, GetActorLocation());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	Super::Destroyed();
}

void ATankProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetInstigator() == OtherActor || OtherActor == nullptr) return;
	// Called only on the server
	if (ATankPawn* OtherTank = Cast<ATankPawn>(OtherActor)) // Would implement a combat interface, but this should be OK for now
	{
		ATankGameMode* GameMode = Cast<ATankGameMode>(GetWorld()->GetAuthGameMode());
		APlayerController* ScoringPC = Cast<APlayerController>(GetOwner());
		APlayerController* VictimPC = Cast<APlayerController>(OtherTank->GetController());
		if (GameMode)
		{
			GameMode->OnPlayerKilled(ScoringPC, VictimPC);
		}
		OtherTank->Destroy();
	}
	Destroy();

}

