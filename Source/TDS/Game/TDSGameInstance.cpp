// Fill out your copyright notice in the Description page of Project Settings.


#include "TDSGameInstance.h"

bool UTDSGameInstance::GetWeaponInfoByName(FName nameWeapon, FWeaponInfo& outInfoWeapon)
{
	bool bIsFind = false;
	FWeaponInfo* weaponInfoRow = nullptr;

	if (weaponInfoTable)
	{
		weaponInfoRow = weaponInfoTable->FindRow<FWeaponInfo>(nameWeapon, "", false);
		if (weaponInfoRow)
		{
			bIsFind = true;
			outInfoWeapon = *weaponInfoRow;
		}
	}

	return bIsFind;
}
