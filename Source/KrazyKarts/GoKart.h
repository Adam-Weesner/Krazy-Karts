// Written by Adam Weesner @ 2020
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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
	void AddRotation(float DeltaTime);
	void GetVelocity(float DeltaTime);
	void SetOffset(float DeltaTime);

	UPROPERTY(Category = "Setup", EditDefaultsOnly)
	float MaxDrivingForce = 10000; //N: Force applied to car when throttle is fully down

	UPROPERTY(Category = "Setup", EditDefaultsOnly)
	float MaxDegreesPerSecond = 90.0f;

	UPROPERTY(Category = "Setup", EditDefaultsOnly)
	float Mass = 1000; // kg

	FVector Velocity;
	float Throttle;
	float SteeringThrow;
};
