// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile_Base.h"

// Sets default values
AProjectile_Base::AProjectile_Base()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bulletCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));

	bulletCollisionSphere->SetSphereRadius(16.f);

	bulletCollisionSphere->OnComponentHit.AddDynamic(this, &AProjectile_Base::BulletCollisionSphereHit);
	bulletCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectile_Base::BulletCollisionSphereBeginOverlap);
	bulletCollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AProjectile_Base::BulletCollisionSphereEndOverlap);

	bulletCollisionSphere->bReturnMaterialOnMove = true;//hit event return physMaterial

	bulletCollisionSphere->SetCanEverAffectNavigation(false);//collision not affect navigation (P keybord on editor)

	RootComponent = bulletCollisionSphere;

	bulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Projectile Mesh"));
	bulletMesh->SetupAttachment(RootComponent);
	bulletMesh->SetCanEverAffectNavigation(false);

	bulletFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Bullet FX"));
	bulletFX->SetupAttachment(RootComponent);

	//BulletSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Bullet Audio"));
	//BulletSound->SetupAttachment(RootComponent);

	bulletProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Bullet ProjectileMovement"));
	bulletProjectileMovement->UpdatedComponent = RootComponent;
	bulletProjectileMovement->InitialSpeed = 1.f;
	bulletProjectileMovement->MaxSpeed = 0.f;

	bulletProjectileMovement->bRotationFollowsVelocity = true;
	bulletProjectileMovement->bShouldBounce = true;

}

// Called when the game starts or when spawned
void AProjectile_Base::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AProjectile_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile_Base::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
}

void AProjectile_Base::BulletCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AProjectile_Base::BulletCollisionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}
