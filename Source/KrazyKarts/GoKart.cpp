// Written by Adam Weesner @ 2020
#include "GoKart.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("Movement Component"));
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetupMove(DeltaTime);

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role), this, FColor::White, DeltaTime);
}

void AGoKart::SetupMove(float DeltaTime)
{
	if (!ensure(MovementComponent)) return;
	FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);

	// We are the client
	if (Role == ROLE_AutonomousProxy)
	{
		MovementComponent->SimulateMove(Move);
		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}
	else if (Role == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}
	// We are server and in control of the pawn
	else if (Role == ROLE_Authority && IsLocallyControlled())
	{
		Server_SendMove(Move);
	}
}

void AGoKart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoKart, ServerState);
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	return true; // TODO: Make better validation
}

FString AGoKart::GetEnumText(ENetRole NetRole)
{
	switch (NetRole)
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
	if (!ensure(MovementComponent)) return;

	SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);
	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (!ensure(MovementComponent)) return;

	MovementComponent->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}