// Written by Adam Weesner @ 2020
#include "GoKart.h"

AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("Movement Component"));
	ReplicationComponent = CreateDefaultSubobject<UGoKartReplicationComponent>(TEXT("Replication Component"));
}

void AGoKart::BeginPlay()
{
	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}
