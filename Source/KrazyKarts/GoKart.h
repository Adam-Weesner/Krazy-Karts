// Written by Adam Weesner @ 2020
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKartMovementComponent.h"
#include "GoKartReplicationComponent.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	AGoKart();
	virtual void BeginPlay() override;

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UGoKartMovementComponent* MovementComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UGoKartReplicationComponent* ReplicationComponent;
};
