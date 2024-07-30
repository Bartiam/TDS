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
}

void ATDSCharacter::SetupPlayerInputComponent(UInputComponent* newInputComponent)
{
	Super::SetupPlayerInputComponent(newInputComponent);

	newInputComponent->BindAxis(TEXT("MoveForward"), this, &ATDSCharacter::InputAxisX);
	newInputComponent->BindAxis(TEXT("MoveRight"), this, &ATDSCharacter::InputAxisY);
	newInputComponent->BindAxis(TEXT("MouseWheel"), this, &ATDSCharacter::MouseWheelCameraSlide);
}

// Function for movement character
void ATDSCharacter::InputAxisX(const float value)
{
	AddMovementInput(FVector(1.0f, 0.0f, 0.f), value);
	MovementTick(value);
}

void ATDSCharacter::InputAxisY(const float value)
{
	AddMovementInput(FVector(0.0f, 1.0f, 0.f), value);
	MovementTick(value);
}

void ATDSCharacter::MovementTick(const float value)
{
	auto myPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!(IsValid(myPlayerController)))
		return;

	FHitResult resultHit;
	myPlayerController->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery6, false, resultHit);

	auto newActorRotationYaw = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), resultHit.Location).Yaw;
	SetActorRotation(FRotator(0.f, newActorRotationYaw, 0.f));
}
///////////////////////////////////

// Changes the current state of the character
void ATDSCharacter::CharacterUpdate()
{
	float resultSpeed;
	switch (currentStateOfMove)
	{
	case EMovementState::AIM_WALK_STATE:
		resultSpeed = movementSpeedInfo.aimWalkSpeed;
		break;
	case EMovementState::WALK_STATE:
		resultSpeed = movementSpeedInfo.simpleWalkSpeed;
		break;
	case EMovementState::AIM_RUN_STATE:
		resultSpeed = movementSpeedInfo.aimRunSpeed;
		break;
	case EMovementState::RUN_STATE:
		resultSpeed = movementSpeedInfo.runSpeed;
		break;
	case EMovementState::FAST_RUN_STATE:
		resultSpeed = movementSpeedInfo.fastRunSpeed;
		break;
	}

	GetCharacterMovement()->MaxWalkSpeed = resultSpeed;
}

void ATDSCharacter::ChangeMovementState(EMovementState movementState)
{
	currentStateOfMove = movementState;
	CharacterUpdate();
}
/////////////////////////////////////////////

// Zooming in and out of the camera by the teddy bear wheel
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
////////////////////////////////////////////////////////////
