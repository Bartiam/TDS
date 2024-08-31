// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	AIM_WALK_STATE UMETA(DisplayName = "Aim Walk State"),
	WALK_STATE UMETA(DisplayName = "Walk State"),
	AIM_RUN_STATE UMETA(DisplayName = "Aim Run State"),
	RUN_STATE UMETA(DisplayName = "Run State"),
	FAST_RUN_STATE UMETA(DisplayName = "Fast Run State")
};

USTRUCT(BlueprintType)
struct FCharacterSpeed
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float aimWalkSpeed = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float simpleWalkSpeed = 300.f;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float aimRunSpeed = 400.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float runSpeed = 500.f;



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float acceleration = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float fastRunSpeed = 900.f;
};

UCLASS()
class TDS_API UTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


};