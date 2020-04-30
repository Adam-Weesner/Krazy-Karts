// Written by Adam Weesner @ 2020
#include "GoKartReplicationComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

UGoKartReplicationComponent::UGoKartReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}

void UGoKartReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
}

void UGoKartReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetupMove(DeltaTime);	
	
	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetOwnerRole()), GetOwner(), FColor::White, DeltaTime);
}

void UGoKartReplicationComponent::SetupMove(float DeltaTime)
{
	if (!ensure(MovementComponent)) return;

	FGoKartMove LastMove = MovementComponent->GetLastMove();

	// We are the client
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}
	// We are server and in control of the pawn
	else if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(LastMove);
	}
	// We are a non-player-controlled entity
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}
}

void UGoKartReplicationComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGoKartReplicationComponent, ServerState);
}

bool UGoKartReplicationComponent::Server_SendMove_Validate(FGoKartMove Move)
{
	return true; // TODO: Make better validation
}

FString UGoKartReplicationComponent::GetEnumText(ENetRole NetRole)
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

void UGoKartReplicationComponent::UpdateServerState(const FGoKartMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UGoKartReplicationComponent::ClearAcknowledgedMoves(FGoKartMove LastMove)
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

void UGoKartReplicationComponent::OnRep_ReplicatedServerState()
{
	if (!MovementComponent) return;

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);
	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoKartReplicationComponent::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (!ensure(MovementComponent)) return;

	MovementComponent->SimulateMove(Move);

	UpdateServerState(Move);
}