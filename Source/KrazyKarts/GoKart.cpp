// Written by Adam Weesner @ 2020
#include "GoKart.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

void AGoKart::MoveForward(float Axis)
{
	Throttle = Axis;
}

void AGoKart::MoveRight(float Axis)
{
	Velocity.X = Axis;
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetVelocity(DeltaTime);
	SetOffset(DeltaTime);
}

void AGoKart::GetVelocity(float DeltaTime)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * DeltaTime;
}

void AGoKart::SetOffset(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult hit;
	AddActorWorldOffset(Translation, true, &hit);

	if (hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

