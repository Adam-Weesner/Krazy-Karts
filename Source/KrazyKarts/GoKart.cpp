// Written by Adam Weesner @ 2020
#include "GoKart.h"
#include "Engine/World.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

void AGoKart::Server_MoveForward_Implementation(float Axis)
{
	Throttle = FMath::Clamp(Axis, -1.0f, 1.0f);
}

bool AGoKart::Server_MoveForward_Validate(float Axis)
{
	return FMath::Abs(Axis) <= 1;
}

void AGoKart::Server_MoveRight_Implementation(float Axis)
{
	SteeringThrow = FMath::Clamp(Axis, -1.0f, 1.0f);
}

bool AGoKart::Server_MoveRight_Validate(float Axis)
{
	return FMath::Abs(Axis) <= 1;
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetVehicleVelocity(DeltaTime);
	SetOffset(DeltaTime);
	AddRotation(DeltaTime);
}

void AGoKart::GetVehicleVelocity(float DeltaTime)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

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

FVector AGoKart::GetAirResistance()
{
	FVector AirResistance = -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoeffecient;
	return AirResistance;
}

FVector AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	FVector RollingResistance = -Velocity.GetSafeNormal() * RollingResistanceCoeffecient * NormalForce;
	return RollingResistance;
}

void AGoKart::AddRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = (DeltaLocation / MinTurningRadius) * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}

void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

