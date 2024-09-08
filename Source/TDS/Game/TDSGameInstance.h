// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "../FuncLibrary/Types.h"
#include "../Weapons/WeaponActor_Base.h"

#include "TDSGameInstance.generated.h"

UCLASS()
class TDS_API UTDSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	// table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSettings")
	UDataTable* weaponInfoTable = nullptr;
	UFUNCTION(BlueprintCallable)
	FWeaponInfo GetWeaponInfoByName(FName nameWeapon);
};
