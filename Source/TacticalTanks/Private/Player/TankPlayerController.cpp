// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/TankPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "DrawDebugHelpers.h"
#include "Actor/TankPawn.h"

ATankPlayerController::ATankPlayerController()
{
	bReplicates = true;

}

void ATankPlayerController::HandleRespawn(ATankPawn* InNewTank)
{
	ControlledTank = nullptr;
	NewTank = InNewTank;
	if (NewTank)
	{
		FTimerHandle CameraBlendTimer;
		GetWorldTimerManager().SetTimer(CameraBlendTimer, this, &ATankPlayerController::BlendCameraToNewTank, 0.2f, false);

		FTimerHandle RespawnTimer;
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &ATankPlayerController::FinishRespawn, 2.f, false);
	}
}


void ATankPlayerController::BeginPlay()
{
	check(TankContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (Subsystem)
	{
		Subsystem->AddMappingContext(TankContext, 0);
	}

	// Setup top down Mouse behaviour
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void ATankPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

	EnhancedInputComponent->BindAction(LMBAction, ETriggerEvent::Started, this, &ATankPlayerController::LeftMouseButtonPressed);
	EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ATankPlayerController::FireButtonPressed);
}

void ATankPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	
	if (CursorHit.bBlockingHit && GetControlledTank())
	{
		ControlledTank->SetNewAimTarget(CursorHit.ImpactPoint);
	}
}


void ATankPlayerController::LeftMouseButtonPressed()
{
	UE_LOG(LogTemp, Warning,TEXT("LeftMouseButtonPressed"));
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	if (CursorHit.bBlockingHit && GetControlledTank())
	{
		ControlledTank->SetNewMoveToDestination(CursorHit.ImpactPoint);
	}
}


void ATankPlayerController::FireButtonPressed()
{
	if (GetControlledTank())
	{
		ControlledTank->FireButtonPressed();
	}
}

ATankPawn* ATankPlayerController::GetControlledTank()
{
	// First time it will return false, then we return the stored reference to avoid casting
	if (!IsValid(ControlledTank))
	{
		ControlledTank = Cast<ATankPawn>(GetPawn());
	}
	return ControlledTank;
}

void ATankPlayerController::FinishRespawn()
{
	if (NewTank)
	{
		Possess(NewTank);
	}
}

void ATankPlayerController::BlendCameraToNewTank()
{
	if (NewTank) Client_SetViewTarget(NewTank);
}

void ATankPlayerController::Client_SetViewTarget_Implementation(AActor* NewViewTarget)
{
	SetViewTargetWithBlend(NewViewTarget,1.5f);
}
