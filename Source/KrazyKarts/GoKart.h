// Written by Adam Weesner @ 2020
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKartMovementComponent.h"
#include "GoKart.generated.h"

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

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	AGoKart();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly)
	UGoKartMovementComponent* MovementComponent;

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	void SetupMove(float DeltaTime);
	void ClearAcknowledgedMoves(FGoKartMove LastMove);
	FString GetEnumText(ENetRole NetRole);

	// Replications
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedServerState)
	FGoKartState ServerState;

	UFUNCTION()
	void OnRep_ReplicatedServerState();

	TArray<FGoKartMove> UnacknowledgedMoves;
};
