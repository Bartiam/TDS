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
#include "../Game/TDSGameInstance.h"

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

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Setting the default speed settings
	currentSpeed = movementSpeedInfo.runSpeed;
	GetCharacterMovement()->MaxWalkSpeed = currentSpeed;
	currentStamina = maxStamina;
}

void ATDSCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (cursorMaterial)
		cursorToWorld = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), cursorMaterial, cursorSize, FVector());

	InitWeapon(initWeaponName);
}

void ATDSCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (cursorToWorld)
	{
		if (APlayerController* myPC = Cast<APlayerController>(GetController()))
		{
			FHitResult traceHitResult;
			myPC->GetHitResultUnderCursor(ECC_Visibility, true, traceHitResult);
			FVector CursorFV = traceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();

			cursorToWorld->SetWorldLocation(traceHitResult.Location);
			cursorToWorld->SetWorldRotation(CursorR);
		}
	}

	MovementTick(DeltaSeconds);
}

void ATDSCharacter::SetupPlayerInputComponent(UInputComponent* newInputComponent)
{
	Super::SetupPlayerInputComponent(newInputComponent);

	// Events to movement
	newInputComponent->BindAxis(TEXT("MoveForward"), this, &ATDSCharacter::InputAxisX);
	newInputComponent->BindAxis(TEXT("MoveRight"), this, &ATDSCharacter::InputAxisY);

	// Event to camera slide
	newInputComponent->BindAxis(TEXT("MouseWheel"), this, &ATDSCharacter::MouseWheelCameraSlide);

	// Events to fire
	newInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Pressed, this, &ATDSCharacter::InputAttackPressed);
	newInputComponent->BindAction(TEXT("FireEvent"), EInputEvent::IE_Released, this, &ATDSCharacter::InputAttackReleased);
}

// ================================ Functions for movement character ================================
void ATDSCharacter::InputAxisX(const float value)
{ axisX = value; }

void ATDSCharacter::InputAxisY(const float value)
{ axisY = value; }

void ATDSCharacter::MovementTick(const float deltaTime)
{
	AddMovementInput(FVector(1.f, 0.f, 0.f), axisX);
	AddMovementInput(FVector(0.f, 1.f, 0.f), axisY);

	auto myPlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!(IsValid(myPlayerController)))
		return;

	FHitResult resultHit;
	myPlayerController->GetHitResultUnderCursor(ECC_GameTraceChannel1, false, resultHit);

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

	// Weapon state update
	AWeaponActor_Base* myWeapon = GetCurrentWeapon();

	if (IsValid(myWeapon))
		myWeapon->UpdateStateWeapon(currentStateOfMove);
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

// =========================================== Weapon =================================================

void ATDSCharacter::InitWeapon(FName idWeapon) //ToDo Init by id row by table
{
	UTDSGameInstance* myGameInstance = Cast<UTDSGameInstance>(GetGameInstance());
	FWeaponInfo myWeaponInfo;

	if (myGameInstance)
	{
		if (myGameInstance->GetWeaponInfoByName(idWeapon, myWeaponInfo))
		{
			if (myWeaponInfo.weaponClass)
			{
				FVector SpawnLocation = FVector(0);
				FRotator SpawnRotation = FRotator(0);

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();

				AWeaponActor_Base* myWeapon = Cast<AWeaponActor_Base>(GetWorld()->SpawnActor(myWeaponInfo.weaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (myWeapon)
				{
					FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					myWeapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					currentWeapon = myWeapon;

					myWeapon->SetWeaponSettings(myWeaponInfo);
					myWeapon->UpdateStateWeapon(currentStateOfMove);
				}
			}
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("InitWeapon = ERROR! - Weapon nor found in table. "));
	}
}

// ============================================ Fire ==================================================

void ATDSCharacter::InputAttackPressed()
{ AttackCharEvent(true); }

void ATDSCharacter::InputAttackReleased()
{ AttackCharEvent(false); }

void ATDSCharacter::AttackCharEvent(bool bIsFiring)
{
	AWeaponActor_Base* myWeapon = nullptr;
	myWeapon = GetCurrentWeapon();

	if (myWeapon)
	{
		// ToDo Check melee or range
		myWeapon->SetWeaponStateFire(bIsFiring);
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("ATDCharacter::AttachCharEvent - CurrentWeapon - NULL"));
}

// ===================================== Getters and setters ==========================================
float ATDSCharacter::GetCurrentStamina() const
{ return currentStamina; }

UDecalComponent* ATDSCharacter::GetCursorToWorld()
{ return cursorToWorld; }

AWeaponActor_Base* ATDSCharacter::GetCurrentWeapon() const
{ return currentWeapon; }
