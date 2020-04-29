// Written by Adam Weesner @ 2020
#include "GoKart.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "GameFrameWork/GameState.h"
#include "DrawDebugHelpers.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
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

	SetupMove(DeltaTime);

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role), this, FColor::White, DeltaTime);
}

void AGoKart::SetupMove(float DeltaTime)
{
	FGoKartMove Move = CreateMove(DeltaTime);

	// We are the client
	if (Role == ROLE_AutonomousProxy)
	{
		SimulateMove(Move); 
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}
	else if (Role == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove);
	}
	// We are server and in control of the pawn
	else if (Role == ROLE_Authority && IsLocallyControlled())
	{
		Server_SendMove(Move);
	}
}

FGoKartMove AGoKart::CreateMove(float DeltaTime)
{
	FGoKartMove Move;

	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.TimeStamp = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	return Move;
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

void AGoKart::ClearAcknowledgedMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.TimeStamp > LastMove.TimeStamp)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void AGoKart::OnRep_ReplicatedServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;
	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
}

void AGoKart::SimulateMove(const FGoKartMove& Move)
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