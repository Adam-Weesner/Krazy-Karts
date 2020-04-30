// Written by Adam Weesner @ 2020
#include "GoKart.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bReplicateMovement = false;

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("Movement Component"));
	ReplicationComponent = CreateDefaultSubobject<UGoKartReplicationComponent>(TEXT("Replication Component"));
}

void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}
