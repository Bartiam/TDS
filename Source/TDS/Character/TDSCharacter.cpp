// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ATDSCharacter::ATDSCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
	CameraBoom->bUsePawnControlRotation = false;

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/Blueprints/Character/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Setting the default speed settings
	currentSpeed = movementSpeedInfo.runSpeed;
	GetCharacterMovement()->MaxWalkSpeed = currentSpeed;
	currentStamina = maxStamina;
}

void ATDSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
				FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
				Params.AddIgnoredActor(this);
				World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
				FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
				CursorToWorld->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
			}
		}
		else if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}

	MovementTick(DeltaSeconds);
}

void ATDSCharacter::SetupPlayerInputComponent(UInputComponent* newInputComponent)
{
	Super::SetupPlayerInputComponent(newInputComponent);

	newInputComponent->BindAxis(TEXT("MoveForward"), this, &ATDSCharacter::InputAxisX);
	newInputComponent->BindAxis(TEXT("MoveRight"), this, &ATDSCharacter::InputAxisY);
	newInputComponent->BindAxis(TEXT("MouseWheel"), this, &ATDSCharacter::MouseWheelCameraSlide);
}

// ================================ Functions for movement character ================================
void ATDSCharacter::InputAxisX(const float value)
{ axisX = value; }

void ATDSCharacter::InputAxisY(const float value)
{ axisY = value; }

void ATDSCharacter::MovementTick(const float deltaTime)
{
	if (CursorToWorld != nullptr)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			AddMovementInput(FVector(1.f, 0.f, 0.f), axisX);
			AddMovementInput(FVector(0.f, 1.f, 0.f), axisY);
			FHitResult traceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, traceHitResult);
			FVector cursorFV = traceHitResult.ImpactNormal;
			FRotator cursorR = cursorFV.Rotation();
			CursorToWorld->SetWorldLocation(traceHitResult.Location);
			CursorToWorld->SetWorldRotation(cursorR);
		}
	}

	auto myPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!(IsValid(myPlayerController)))
		return;

	FHitResult resultHit;
	myPlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, resultHit);

	if (bIsFastRunning)
	{
		numberWhichStaminaChanges = (decreaseStamina * deltaTime);
		ReducesStamina();
	}
	else 
	{
		auto newActorRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), resultHit.Location);
		SetActorRotation(FRotator(0.f, newActorRotation.Yaw, 0.f));
	}
	
	if (bIsCanIncreaseStamina)
	{
		numberWhichStaminaChanges = (increaseStamina * deltaTime);
		AugmentStamina();
	}
}



// ============================ Changes the current state of the character ============================
void ATDSCharacter::CharacterUpdateSpeed()
{
	GetWorldTimerManager().ClearTimer(timerToAccelirationSpeed);

	switch (currentStateOfMove)
	{
	case EMovementState::AIM_WALK_STATE:
		currentSpeed = movementSpeedInfo.aimWalkSpeed;
		break;
	case EMovementState::WALK_STATE:
		currentSpeed = movementSpeedInfo.simpleWalkSpeed;
		break;
	case EMovementState::AIM_RUN_STATE:
		currentSpeed = movementSpeedInfo.aimRunSpeed;
		break;
	case EMovementState::RUN_STATE:
		currentSpeed = movementSpeedInfo.runSpeed;
		break;
	case EMovementState::FAST_RUN_STATE:
		currentSpeed = movementSpeedInfo.fastRunSpeed;
		break;
	}

	GetWorldTimerManager().SetTimer(timerToAccelirationSpeed, this, &ATDSCharacter::AccelerationAndDeccelerationToMove, 0.002f, true);
}

void ATDSCharacter::ChangeMovementState()
{
	if (bIsFastRunning)
	{
		bIsWalking = false;
		bIsAiming = false;
		currentStateOfMove = EMovementState::FAST_RUN_STATE;
	}
	else if (bIsWalking && bIsAiming)
		currentStateOfMove = EMovementState::AIM_WALK_STATE;
	else if (!bIsWalking && bIsAiming)
		currentStateOfMove = EMovementState::AIM_RUN_STATE;
	else if (bIsWalking && !bIsAiming)
		currentStateOfMove = EMovementState::WALK_STATE;
	else
		currentStateOfMove = EMovementState::RUN_STATE;

	CharacterUpdateSpeed();
}

void ATDSCharacter::AccelerationAndDeccelerationToMove()
{
	if (GetCharacterMovement()->MaxWalkSpeed < currentSpeed)
		GetCharacterMovement()->MaxWalkSpeed += movementSpeedInfo.acceleration;
	else if (GetCharacterMovement()->MaxWalkSpeed > currentSpeed)
		GetCharacterMovement()->MaxWalkSpeed -= movementSpeedInfo.acceleration;
	else
		GetWorldTimerManager().ClearTimer(timerToAccelirationSpeed);
}



// ===================== Zooming in and out of the camera by the teddy bear wheel =====================
void ATDSCharacter::MouseWheelCameraSlide(const float value)
{
	float springArmLength = CameraBoom->TargetArmLength;

	if (value < 0.f && (springArmLength + changeDistanceSpringArm <= maxCameraHeight) && bIsSlideDone)
	{
		GetWorldTimerManager().SetTimer(timerToSmooth, this, &ATDSCharacter::AddsSmoothnessToTheCamera, 0.001, true);
		isSlideUp = 1;
		bIsSlideDone = false;
	}
	else if (value > 0.f && (springArmLength - changeDistanceSpringArm >= minCameraHeight) && bIsSlideDone)
	{
		GetWorldTimerManager().SetTimer(timerToSmooth, this, &ATDSCharacter::AddsSmoothnessToTheCamera, 0.001, true);
		isSlideUp = -1;
		bIsSlideDone = false;
	}
}

void ATDSCharacter::AddsSmoothnessToTheCamera()
{
	float changeSlideStepDistance = 1.f;
	currentSlideDistance += changeSlideStepDistance;

	CameraBoom->TargetArmLength += isSlideUp;
	if (currentSlideDistance >= changeDistanceSpringArm)
	{
		bIsSlideDone = true;
		currentSlideDistance = 0.f;
		GetWorldTimerManager().ClearTimer(timerToSmooth);
	}

}



// ============================================= STAMINA ==============================================
void ATDSCharacter::ReducesStamina()
{
	if (!GetVelocity().IsZero())
	{
		bIsCanIncreaseStamina = false;
		bIsStartsTimerToIncreaseStamina = false;
		currentStamina -= numberWhichStaminaChanges;
	}

	if (!bIsStartsTimerToIncreaseStamina)
	{
		GetWorldTimerManager().ClearTimer(timerToAugmentStamina);

		if (currentStamina <= 0.f)
		{
			currentStamina = 0.f;

			bIsFastRunning = false;
			bIsCharacterTired = true;
			ChangeMovementState();

			GetWorldTimerManager().SetTimer(timerToAugmentStamina, this, &ATDSCharacter::ChangeCanIncreaseStamina, timeToRecoverStaminaAfterZero, false);
			return;
		}

		GetWorldTimerManager().SetTimer(timerToAugmentStamina, this, &ATDSCharacter::ChangeCanIncreaseStamina, timeToRecoverStamina, false);

		bIsStartsTimerToIncreaseStamina = true;
	}
}

void ATDSCharacter::AugmentStamina()
{
	currentStamina += numberWhichStaminaChanges;

	if (currentStamina >= recoveryFromTired)
	{
		bIsCharacterTired = false;

		auto myPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		FKey shiftKey = EKeys::LeftShift;

		if (myPlayerController->IsInputKeyDown(shiftKey))
		{
			bIsFastRunning = true;
			ChangeMovementState();
		}
	}

	if (currentStamina >= maxStamina)
	{
		currentStamina = maxStamina;
		bIsCanIncreaseStamina = false;
	}
}

void ATDSCharacter::ChangeCanIncreaseStamina()
{ bIsCanIncreaseStamina = true; }



// ===================================== Getters and setters ==========================================
float ATDSCharacter::GetCurrentStamina() const
{ return currentStamina; }
