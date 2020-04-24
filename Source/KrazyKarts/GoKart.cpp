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
	SteeringThrow = Axis;
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetVelocity(DeltaTime);
	SetOffset(DeltaTime);
	AddRotation(DeltaTime);
}

void AGoKart::GetVelocity(float DeltaTime)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetResistance();
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

FVector AGoKart::GetResistance()
{
	float AirResistance = -Velocity.SizeSquared() * DRAG_COEFFECIENT;
	return GetActorForwardVector().GetSafeNormal() * AirResistance;
}

void AGoKart::AddRotation(float DeltaTime)
{
	float RotationAngle = MaxDegreesPerSecond * SteeringThrow * DeltaTime;
	FQuat RotationDelta(GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta, true);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

