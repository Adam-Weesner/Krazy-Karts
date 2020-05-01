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
	else if (Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		UpdateServerState(LastMove);
	}
	// We are a non-player-controlled entity
	else if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
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

void UGoKartReplicationComponent::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	// Avoiding floating point number division with very small numbers
	if (ClientTimeBetweenLastUpdate < KINDA_SMALL_NUMBER) return;
	if (!ensure(MovementComponent)) return;

	FVector StartLocation = ClientStartTransform.GetLocation();
	FVector TargetLocation = ServerState.Transform.GetLocation();
	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdate;
	float VelocityToDerative = ClientTimeBetweenLastUpdate * 100; // 100 to convert between meters and cm
	FVector StartDerivative = ClientStartVelocity * VelocityToDerative;
	FVector TargetDerivative = ServerState.Velocity * VelocityToDerative;

	FVector NewLocation = FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	GetOwner()->SetActorLocation(NewLocation);

	FVector NewDerivative = FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerative;
	MovementComponent->SetVelocity(NewVelocity);

	FQuat StartRotation = ClientStartTransform.GetRotation();
	FQuat TargetRotation = ServerState.Transform.GetRotation();

	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
	GetOwner()->SetActorRotation(NewRotation);
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

// This is an update from the server
void UGoKartReplicationComponent::OnRep_ReplicatedServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ReplicatedServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ReplicatedServerState();
		break;
	default:
		break;
	}
}

void UGoKartReplicationComponent::SimulatedProxy_OnRep_ReplicatedServerState()
{
	if (!ensure(MovementComponent)) return;

	ClientTimeBetweenLastUpdate = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;
	ClientStartTransform = GetOwner()->GetActorTransform();
	ClientStartVelocity = MovementComponent->GetVelocity();
}

void UGoKartReplicationComponent::AutonomousProxy_OnRep_ReplicatedServerState()
{
	if (MovementComponent == nullptr) return;

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