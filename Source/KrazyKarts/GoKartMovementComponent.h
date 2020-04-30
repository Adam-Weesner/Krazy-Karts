// Written by Adam Weesner @ 2020
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGoKartMovementComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SimulateMove(const FGoKartMove& Move);
	void SetVelocity(FVector InVelocity) { Velocity = InVelocity; };
	FVector GetVelocity() { return Velocity; };
	FGoKartMove GetLastMove() { return LastMove; };

protected:
	UFUNCTION(BlueprintCallable)
	void MoveForward(float Axis);

	UFUNCTION(BlueprintCallable)
	void MoveRight(float Axis);

private:
	FGoKartMove CreateMove(float DeltaTime);
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
	FGoKartMove LastMove;
};
