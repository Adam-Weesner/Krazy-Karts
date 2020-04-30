// Written by Adam Weesner @ 2020

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.h"
#include "GoKartReplicationComponent.generated.h"

USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FGoKartMove LastMove;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGoKartReplicationComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	void SetupMove(float DeltaTime);
	void ClearAcknowledgedMoves(FGoKartMove LastMove);
	FString GetEnumText(ENetRole NetRole);
	void UpdateServerState(const FGoKartMove& Move);

	// Replications
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedServerState)
	FGoKartState ServerState;

	UFUNCTION()
	void OnRep_ReplicatedServerState();

	TArray<FGoKartMove> UnacknowledgedMoves;

	UPROPERTY()
	UGoKartMovementComponent* MovementComponent;
		
};
