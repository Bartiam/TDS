// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponActor_Base.h"

// Sets default values
AWeaponActor_Base::AWeaponActor_Base()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	sceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = sceneComponent;

	skeletalMeshWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	skeletalMeshWeapon->SetGenerateOverlapEvents(false);
	skeletalMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	skeletalMeshWeapon->SetupAttachment(RootComponent);

	staticMeshWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh "));
	staticMeshWeapon->SetGenerateOverlapEvents(false);
	staticMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	staticMeshWeapon->SetupAttachment(RootComponent);

	shootLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	shootLocation->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeaponActor_Base::BeginPlay()
{
	Super::BeginPlay();
	
	WeaponInit();
}

// Called every frame
void AWeaponActor_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FireTick(DeltaTime);
}

void AWeaponActor_Base::FireTick(float DeltaTime)
{
	if (weaponFiring)
		if (FireTime < 0.f)
			Fire();
		else
			FireTime -= DeltaTime;

}

void AWeaponActor_Base::WeaponInit()
{
	if (skeletalMeshWeapon && !skeletalMeshWeapon->SkeletalMesh)
	{
		skeletalMeshWeapon->DestroyComponent(true);
	}

	if (staticMeshWeapon && !staticMeshWeapon->GetStaticMesh())
	{
		staticMeshWeapon->DestroyComponent();
	}
}

void AWeaponActor_Base::SetWeaponStateFire(bool bIsFire)
{
	if (GetWeaponCanFire())
		weaponFiring = bIsFire;
	else
		weaponFiring = false;
}

bool AWeaponActor_Base::GetWeaponCanFire()
{
	return false;
}

FProjectileInfo AWeaponActor_Base::GetProjectile()
{
	return FProjectileInfo();
}

void AWeaponActor_Base::Fire()
{
	FireTime = weaponSetting.rateOfFire;

	if (shootLocation)
	{
		FVector SpawnLocation = shootLocation->GetComponentLocation();
		FRotator SpawnRotation = shootLocation->GetComponentRotation();
		FProjectileInfo ProjectileInfo;
		ProjectileInfo = GetProjectile();

		if (ProjectileInfo.projectile)
		{
			//Projectile Init ballistic fire

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = GetInstigator();

			AProjectile_Base* myProjectile = Cast<AProjectile_Base>(GetWorld()->SpawnActor(ProjectileInfo.projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
			if (myProjectile)
			{
				//ToDo Init Projectile settings by id in table row(or keep in weapon table)
				myProjectile->InitialLifeSpan = 20.0f;
				//Projectile->BulletProjectileMovement->InitialSpeed = 2500.0f;
			}
		}
		else
		{
			//ToDo Projectile null Init trace fire			
		}
	}

}

void AWeaponActor_Base::UpdateStateWeapon(EMovementState NewMovementState)
{
	//ToDo Dispersion
	ChangeDispersion();
}

void AWeaponActor_Base::ChangeDispersion()
{

}