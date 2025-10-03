// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/ArrowComponent.h" 
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "SkatePawn.generated.h"



// Delegate signature: bool parameter tells if pawn is on ground
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroundStateChanged, bool, bIsOnGround);


UCLASS()
class SKATEBOARD_API ASkatePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASkatePawn();

	// Event Dispatcher
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGroundStateChanged OnGroundStateChanged;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;




public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Static mesh root representing the skateboard
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Skateboard;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") bool	bIsOnGround = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") bool 	bIsOnRamp = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") bool 	bIsJumping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") bool 	bPrevIsOnGround = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") FVector 	UpVector = FVector::UpVector; // default (0,0,1)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") FVector 	RampInitialVelocity = FVector::UpVector; // default (0,0,1)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") FRotator 	RampInitialRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") FRotator 	RampInAirRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") float 	SkateSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skate") float 	MidAirRotation = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
UArrowComponent* ArrowComp;

};
