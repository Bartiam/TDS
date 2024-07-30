// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "../FuncLibrary/Types.h"

#include "TDSCharacter.generated.h"

UCLASS(Blueprintable)
class ATDSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATDSCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* newInputComponent) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;

public:

	// Current state of character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EMovementState currentStateOfMove = EMovementState::RUN_STATE;
	/////////////////////////////

	// Struct info about speed of character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FCharacterSpeed movementSpeedInfo;
	///////////////////////////////////////
	
	// Variables for zoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Zoom")
	float minCameraHeight = 700.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Zoom")
	float maxCameraHeight = 1200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Zoom")
	float changeDistanceSpringArm = 100.f;
	/////////////////////

	// Variables for character movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsWalking = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsFastRunning = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIsAiming = false;
	///////////////////////////////////

private:
	// Functions for movement character
	UFUNCTION()
	void InputAxisX(const float value);
	UFUNCTION()
	void InputAxisY(const float value);
	UFUNCTION()
	void MovementTick(const float value);
	///////////////////////////////////

	// Changes the current state of the character
	UFUNCTION(BlueprintCallable)
	void CharacterUpdate();

	UFUNCTION(BlueprintCallable)
	void ChangeMovementState();
	/////////////////////////////////////////////

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
		/////////////////////////////

	UFUNCTION()
	void MouseWheelCameraSlide(const float value);

	UFUNCTION()
	void AddsSmoothnessToTheCamera();
	//////////////////////////////////////////////////////////
};

