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
	// Sets default values for this pawn's properties
	AGoKart();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable)
	void MoveForward(float Axis);

	UFUNCTION(BlueprintCallable)
	void MoveRight(float Axis);

private:
	UPROPERTY(EditDefaultsOnly)
	float MaxDrivingForce = 10000; //N: Force applied to car when throttle is fully down

	UPROPERTY(EditDefaultsOnly)
	float Mass = 1000; // kg

	FVector Velocity;
	float Throttle;
};
