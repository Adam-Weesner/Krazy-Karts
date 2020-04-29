// Written by Adam Weesner @ 2020
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float TimeStamp;
};

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

	UFUNCTION(BlueprintCallable)
	void MoveForward(float Axis);

	UFUNCTION(BlueprintCallable)
	void MoveRight(float Axis);

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	void SetupMove(float DeltaTime);
	FGoKartMove CreateMove(float DeltaTime);
	void ClearAcknowledgedMoves(FGoKartMove LastMove);
	void SimulateMove(const FGoKartMove& Move);
	void AddRotation(float DeltaTime, float InSteeringThrow);
	void GetVehicleVelocity(float DeltaTime, float InThrottle);
	void SetOffset(float DeltaTime);
	FVector GetAirResistance();
	FVector GetRollingResistance();

	// Force applied to car when throttle is fully down (N)
	UPROPERTY(Category = "Setup", EditDefaultsOnly)
	float MaxDrivingForce = 10000; 

	UPROPERTY(Category = "Setup", EditDefaultsOnly)
	float Mass = 1000; // kg

	// Minimum radius of the car turning cricle at full lock (m)
	UPROPERTY(Category = "Setup", EditDefaultsOnly)
	float MinTurningRadius = 16.0f;

	// Higher means more drag
	UPROPERTY(Category = "Setup", EditDefaultsOnly)
	float DragCoeffecient = 16.0f;

	// Higher means more rolling resistance
	UPROPERTY(Category = "Setup", EditDefaultsOnly)
	float RollingResistanceCoeffecient = 0.015f;

	FVector Velocity;
	float Throttle;
	float SteeringThrow;
	TArray<FGoKartMove> UnacknowledgedMoves;

	// Replications
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedServerState)
	FGoKartState ServerState;

	UFUNCTION()
	void OnRep_ReplicatedServerState();
};
