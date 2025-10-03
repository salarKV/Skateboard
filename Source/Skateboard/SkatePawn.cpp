// Fill out your copyright notice in the Description page of Project Settings.


#include "SkatePawn.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"


// Sets default values
ASkatePawn::ASkatePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the skateboard mesh and make it the root
	Skateboard = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Skateboard"));
	RootComponent = Skateboard;

	// Create the arrow component as a child of the skateboard mesh
	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	ArrowComp->SetupAttachment(Skateboard);

	// Spring arm attached to the skateboard
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Skateboard);
	SpringArm->TargetArmLength = 300.f;          
	SpringArm->bUsePawnControlRotation = true;   

	// Camera attached to the spring arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;     
}

// Called when the game starts or when spawned
void ASkatePawn::BeginPlay()
{
	Super::BeginPlay();
	
}




// Called every frame
void ASkatePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	// Ground detection using sphere trace
	if (!ArrowComp || !GetWorld())
	{
		return;
	}

	FVector TraceStart = ArrowComp->GetComponentLocation();
	FVector TraceEnd = TraceStart + ArrowComp->GetForwardVector() * 60.0f;

	float Radius = bIsOnGround ? 12.0f : 4.0f;

	TArray<FHitResult> HitResults;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this); // Ignore self

	bIsOnGround = UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		TraceStart,
		TraceEnd,
		Radius,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,                         // trace complex (needed for BSP/geometry)
		ActorsToIgnore,
		EDrawDebugTrace::ForOneFrame, // debug
		HitResults,
		true                          
	) && HitResults.Num() > 0;


	// Move forward using constant velocity, unless S is pressed
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (!PC->IsInputKeyDown(EKeys::S))
		{
			if (Skateboard)
			{
				// Forward direction of skateboard, normalized
				FVector Forward = Skateboard->GetForwardVector().GetSafeNormal();

				// Constant horizontal velocity (X and Y only)
				FVector DesiredVelocity = Forward * 600.0f;

				// Preserve Z velocity from physics
				float ZValue = Skateboard->GetPhysicsLinearVelocity().Z;

				// Apply final velocity
				FVector FinalVelocity(DesiredVelocity.X, DesiredVelocity.Y, ZValue);
				Skateboard->SetPhysicsLinearVelocity(FinalVelocity, false);
			}
		}
	}


	// Turn skateboard using angular velocity, only if moving
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		bool bIsAPressed = PC->IsInputKeyDown(EKeys::A);
		bool bIsDPressed = PC->IsInputKeyDown(EKeys::D);

		float VelocityLength = Skateboard->GetPhysicsLinearVelocity().Size();

		if ((bIsAPressed || bIsDPressed) && VelocityLength > 10.0f)
		{
			// Get current angular velocity
			FVector AngularVel = Skateboard->GetPhysicsAngularVelocityInDegrees();

			// Overwrite Z depending on key
			AngularVel.Z = bIsDPressed ? 120.0f : -120.0f;

			// Apply new angular velocity (replace, not add)
			Skateboard->SetPhysicsAngularVelocityInDegrees(AngularVel, false);
		}
	}


	
	// turn skateboard using AddLocalRotation, Turn in place
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		bool bIsAPressed = PC->IsInputKeyDown(EKeys::A);
		bool bIsDPressed = PC->IsInputKeyDown(EKeys::D);

		float VelocityLength = Skateboard->GetPhysicsLinearVelocity().Size();

		if ((bIsAPressed || bIsDPressed) && VelocityLength <= 10.0f)
		{
			// Select rotation direction
			float YawStep = bIsDPressed ? 1.0f : -1.0f;

			// Add relative rotation (local Z)
			Skateboard->AddLocalRotation(FRotator(0.f, YawStep, 0.f));
		}
	}


	// Keep the spring arm above the skateboard
	if (Skateboard && SpringArm)
	{
		// Get the world location of the Skateboard (our root)
		FVector BoardLocation = Skateboard->GetComponentLocation();

		// Get X and Y from the board, and Z = Board.Z + 100
		FVector NewLocation(BoardLocation.X, BoardLocation.Y, BoardLocation.Z + 100.0f);

		// Update the SpringArm location
		SpringArm->SetWorldLocation(NewLocation);
	}
}

// Called to bind functionality to input
void ASkatePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

