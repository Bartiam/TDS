// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"

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

USTRUCT(BlueprintType)
struct FProjectileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSettings")
	TSubclassOf<class AProjectile_Base> projectile = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSettings")
	float projectileDamage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSettings")
	float projectileLifeTime = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSettings")
	float projectileInitSpeed = 2000.f;

	// Hit fx actor ?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSettings")
	bool bIsLikeBomp = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileSettings")
	float projectileMaxRadiusDamage = 200.f;
};

USTRUCT(BlueprintType)
struct FWeaponDispersion
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	float dispersionAimStart = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	float dispersionAimMax = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	float dispersionAimMin = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	float dispersionAimShootCoef = 1.f;
};

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<class AWeaponActor_Base> weaponClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float rateOfFire = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	float reloadTime = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 maxRound = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dispersion")
	FWeaponDispersion dispersionWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* soundFireWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* soundReloadWeapon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* effectFireWeapon = nullptr;
	// If null use trace logic (TSubclassOf<class AWeaponActor_Base> weaponClass = nullptr)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FProjectileInfo projectileSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float weaponDamage = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	float distanceTrace = 2000.f;
	// One decal or all ?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitEffect")
	UDecalComponent* decalOnHit = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	UAnimMontage* animCharFire = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	UAnimMontage* animCharReload = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* magazineDrop = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UStaticMesh* sleeveBullets = nullptr;
};

USTRUCT(BlueprintType)
struct FAddicionalWeaponInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon stats")
	int32 round = 10;
};

UCLASS()
class TDS_API UTypes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


};