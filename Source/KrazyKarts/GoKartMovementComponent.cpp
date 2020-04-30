// Written by Adam Weesner @ 2020
#include "GoKartMovementComponent.h"
#include "Engine/World.h"
#include "GameFrameWork/GameState.h"

UGoKartMovementComponent::UGoKartMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetupMove(DeltaTime);
}

void UGoKartMovementComponent::SetupMove(float DeltaTime)
{
	// We are in control of the pawn
	if (GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		LastMove = CreateMove(DeltaTime);
		SimulateMove(LastMove);
	}
}

void UGoKartMovementComponent::MoveForward(float Axis)
{
	Throttle = FMath::Clamp(Axis, -1.0f, 1.0f);
}

void UGoKartMovementComponent::MoveRight(float Axis)
{
	SteeringThrow = FMath::Clamp(Axis, -1.0f, 1.0f);
}

FGoKartMove UGoKartMovementComponent::CreateMove(float DeltaTime)
{
	FGoKartMove Move;

	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.TimeStamp = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	return Move;
}

void UGoKartMovementComponent::GetVehicleVelocity(float DeltaTime, float InThrottle)
{
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * InThrottle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity += Acceleration * DeltaTime;
}

void UGoKartMovementComponent::SetOffset(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult hit;
	GetOwner()->AddActorWorldOffset(Translation, true, &hit);

	if (hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

FVector UGoKartMovementComponent::GetAirResistance()
{
	FVector AirResistance = -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoeffecient;
	return AirResistance;
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	FVector RollingResistance = -Velocity.GetSafeNormal() * RollingResistanceCoeffecient * NormalForce;
	return RollingResistance;
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove& Move)
{
	GetVehicleVelocity(Move.DeltaTime, Move.Throttle);
	SetOffset(Move.DeltaTime);
	AddRotation(Move.DeltaTime, Move.SteeringThrow);
}

void UGoKartMovementComponent::AddRotation(float DeltaTime, float InSteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = (DeltaLocation / MinTurningRadius) * InSteeringThrow;
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	GetOwner()->AddActorWorldRotation(RotationDelta);
}