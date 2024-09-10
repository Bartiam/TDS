// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "../FuncLibrary/Types.h"
#include "../Weapons/WeaponActor_Base.h"

#include "TDSCharacter.generated.h"

//USTRUCT(BlueprintType)
//struct FCharacterSpeedInfo
//{
//	GENERATED_BODY()
//
//
//};

UCLASS(Blueprintable)
class ATDSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATDSCharacter();

	// Called once at the beginning of the game
	virtual void BeginPlay() override;

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* newInputComponent) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:

	// ============================= Cursor =============================
	UPROPERTY();
	UDecalComponent* cursorToWorld = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* cursorMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FVector cursorSize = FVector();

	// =============== Current state of character =======================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EMovementState currentStateOfMove = EMovementState::RUN_STATE;


	// ============== Struct info about speed of character ==============
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FCharacterSpeed movementSpeedInfo;

	
	// ======================= Variables for zoom =======================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Zoom")
	float minCameraHeight = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Zoom")
	float maxCameraHeight = 1200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Zoom")
	float changeDistanceSpringArm = 100.f;


	// ================ Variables for movement character ================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsWalking = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsFastRunning = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsAiming = false;

	float axisX = 0.f;
	float axisY = 0.f;


	// ===================== Variables for stamina ======================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina")
	float decreaseStamina = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina")
	float increaseStamina = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina")
	float recoveryFromTired = 40.f;
	UPROPERTY(BlueprintReadOnly, Category = "Stamina")
	bool bIsCharacterTired = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina")
	float timeToRecoverStamina = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stamina")
	float timeToRecoverStaminaAfterZero = 2.f;

	//============================== for demo ===============================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	FName initWeaponName;

	// ============================ Public Weapon ===========================
	UFUNCTION(BlueprintCallable)
	void InitWeapon(FName idWeapon); //ToDo Init by id row by table
	UFUNCTION(BlueprintCallable)
	void TryReloadWeapon();

	// ============================= Public fire ============================
	UFUNCTION(BlueprintCallable)
	void AttackCharEvent(bool bIsFiring);

private:
	// ================ Functions for movement character ================
	UFUNCTION()
	void InputAxisX(const float value);
	UFUNCTION()
	void InputAxisY(const float value);
	UFUNCTION()
	void MovementTick(const float deltaTime);

	// =========== Changes the current state of the character ============
	UFUNCTION(BlueprintCallable)
	void CharacterUpdateSpeed();

	UFUNCTION(BlueprintCallable)
	void ChangeMovementState();
	UFUNCTION()
	void AccelerationAndDeccelerationToMove();
		// ============ Variables for change movement
		UPROPERTY()
		float currentSpeed;
		UPROPERTY()
		FTimerHandle timerToAccelirationSpeed;


	// Zooming in and out of the camera by the teddy bear wheel
		// Variables for smooth slide
		UPROPERTY()
		bool bIsSlideDone = true;
		UPROPERTY()
		int isSlideUp;
		UPROPERTY()
		float currentSlideDistance = 0.f;
		UPROPERTY()
		FTimerHandle timerToSmooth;


	UFUNCTION()
	void MouseWheelCameraSlide(const float value);

	UFUNCTION()
	void AddsSmoothnessToTheCamera();


	// ============================= STAMINA ================================
		// Variables for stamina
		const float maxStamina = 100.f;
		UPROPERTY()
		float currentStamina = 0.f;
		UPROPERTY()
		float numberWhichStaminaChanges;
		UPROPERTY()
		FTimerHandle timerToAugmentStamina;
		UPROPERTY()
		bool bIsCanIncreaseStamina = false;
		UPROPERTY()
		bool bIsStartsTimerToIncreaseStamina = false;

	UFUNCTION()
	void ReducesStamina();
	UFUNCTION()
	void AugmentStamina();
	UFUNCTION()
	void ChangeCanIncreaseStamina();

	// =========================== Weapon private ===========================
	UPROPERTY()
	AWeaponActor_Base* currentWeapon = nullptr;

	// ============================ Fire private ============================
	UFUNCTION()
	void InputAttackPressed();
	void InputAttackReleased();

public: // ===================== Getters and setters ========================

	UFUNCTION(BlueprintCallable)
	float GetCurrentStamina() const;

	UFUNCTION(BlueprintCallable)
	UDecalComponent* GetCursorToWorld();

	UFUNCTION(BlueprintCallable)
	AWeaponActor_Base* GetCurrentWeapon() const;
};

