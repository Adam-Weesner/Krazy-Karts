// Written by Adam Weesner @ 2020
#include "GoKart.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}

void AGoKart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoKart, ServerState);
}

void AGoKart::MoveForward(float Axis)
{
	Throttle = FMath::Clamp(Axis, -1.0f, 1.0f);
}

void AGoKart::MoveRight(float Axis)
{
	SteeringThrow = FMath::Clamp(Axis, -1.0f, 1.0f);
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	return true; // TODO: Make better validation
}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
		break;
	case  ROLE_SimulatedProxy:
		return "Simulated Proxy";
		break;
	case ROLE_AutonomousProxy:
		return "Autonomous Proxy";
		break;
	case ROLE_Authority:
		return "Authority";
		break;
	default:
		return "";
		break;
	}
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetVehicleVelocity(DeltaTime, Throttle);
	SetOffset(DeltaTime);
	AddRotation(DeltaTime, SteeringThrow);
	CreateMove(DeltaTime);

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role), this, FColor::White, DeltaTime);
}

void AGoKart::CreateMove(float DeltaTime)
{
	if (IsLocallyControlled())
	{
		FGoKartMove Move;
		Move.DeltaTime = DeltaTime;
		Move.SteeringThrow = SteeringThrow;
		Move.Throttle = Throttle;
		Move.TimeStamp = GetWorld()->TimeSeconds;

		Server_SendMove(Move);
		SimulateMove(Move);
	}
}

void AGoKart::GetVehicleVelocity(float DeltaTime, float InThrottle)
{
	FVector Force = GetActorForwardVector() * MaxDrivingForce * InThrottle;
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

void AGoKart::OnRep_ReplicatedServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;
}

void AGoKart::SimulateMove(FGoKartMove Move)
{
	GetVehicleVelocity(Move.DeltaTime, Move.Throttle);
	SetOffset(Move.DeltaTime);
	AddRotation(Move.DeltaTime, Move.SteeringThrow);
}

void AGoKart::AddRotation(float DeltaTime, float InSteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = (DeltaLocation / MinTurningRadius) * InSteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}

void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

