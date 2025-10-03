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

	// Create a spring arm attached to the skateboard
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Skateboard);
	SpringArm->TargetArmLength = 400.f;          // default camera distance
	SpringArm->bUsePawnControlRotation = true;   // rotate arm based on controller

	// Create a camera attached to the spring arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;     // don't rotate camera itself, spring arm does it

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

	


	// Ground detection
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
        EDrawDebugTrace::None, // debug
        HitResults,
        true                          
    ) && HitResults.Num() > 0;




	// If skateboard is upside down, flip it
FVector CurrentVelocity = GetVelocity(); 

if (!bIsOnGround && !bIsOnRamp && CurrentVelocity.Size() <= 2.0f)
{
    if (Skateboard)
    {
        FRotator CurrentRotation = Skateboard->GetComponentRotation();
        FRotator NewRotation(0.0f, CurrentRotation.Yaw, 0.0f);

        Skateboard->SetWorldRotation(NewRotation);

    }
}
// Set the linear velocity to zero when pressing S on ground
if (bIsOnGround)
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (PC && PC->IsInputKeyDown(EKeys::S))
    {
        if (Skateboard)
        {
            Skateboard->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
            
        }
    }
}


// Ramp 
if (!bIsOnGround && bIsOnRamp)
{
    if (Skateboard)
    {
        // Apply current ramp velocity
        Skateboard->SetPhysicsLinearVelocity(RampInitialVelocity, false);

        // Reduce the ramp velocity
        RampInitialVelocity = RampInitialVelocity - (UpVector * 5.0f);

        // Apply ramp in-air rotation
        Skateboard->SetWorldRotation(RampInAirRotation);
    }
}






// when the skate is coming back to ramp, if it's velocity is in oppisite direction to it's forward vector, flip it 180 degrees 
if (bIsOnGround && bIsOnRamp)
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (!PC->IsInputKeyDown(EKeys::S))
        {
            if (Skateboard)
            {
                FVector Forward = Skateboard->GetForwardVector();
                FVector Velocity = Skateboard->GetPhysicsLinearVelocity();

                // Avoid divide by zero
                if (!Velocity.IsNearlyZero())
                {
                    FVector VelocityDir = Velocity.GetSafeNormal();
                    float Dot = FVector::DotProduct(Forward, VelocityDir);

                    if (Dot <= 0.0f)
                    {
                        // rotate 180 degrees around local Z (yaw) — AddLocalRotation applies in local space
                        Skateboard->AddLocalRotation(FRotator(0.f, 180.f, 0.f));
                    }
                }
            }
        }
    }
}





// Main skate linear velocity control
if (bIsOnGround)
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (!PC->IsInputKeyDown(EKeys::S))
        {
            if (Skateboard)
            {
                // Forward direction of skateboard
                FVector Forward = Skateboard->GetForwardVector();

                // Base velocity (X and Y from forward vector * speed)
                FVector DesiredVelocity = Forward * SkateSpeed;

                // Z velocity selection
                float ZValue;
                if (bIsJumping)
                {
                    ZValue = 500.0f; // jump override
                }
                else if (bIsOnGround)
                {
                    ZValue = DesiredVelocity.Z; // ground case
                }
                else
                {
                    ZValue = Skateboard->GetPhysicsLinearVelocity().Z; // preserve Z
                }

                // Apply final velocity
                FVector FinalVelocity(DesiredVelocity.X, DesiredVelocity.Y, ZValue);
                Skateboard->SetPhysicsLinearVelocity(FinalVelocity, false);
            }
        }
    }
}







// keep the spring arm 100 units above the skateboard
if (Skateboard && SpringArm)
{
    // Get the world location of the Skateboard (our root)
    FVector BoardLocation = Skateboard->GetComponentLocation();

    // Get X and Y from the board, and Z = Board.Z + 100
    FVector NewLocation(BoardLocation.X, BoardLocation.Y, BoardLocation.Z + 100.0f);

    // Update the SpringArm location
    SpringArm->SetWorldLocation(NewLocation);
}


//====
// rest of stuff are for rotation and angular velocity
//====

// keep angular velocity Z (yaw) zero when no input
if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
{
    bool bIsAPressed = PC->IsInputKeyDown(EKeys::A);
    bool bIsDPressed = PC->IsInputKeyDown(EKeys::D);

    if (!bIsAPressed && !bIsDPressed)
    {
        // Get current angular velocity (in degrees)
        FVector AngularVel = Skateboard->GetPhysicsAngularVelocityInDegrees();

        // Zero out Z (yaw) while keeping X and Y the same
        AngularVel.Z = 0.f;

        // Apply back to the skateboard
        Skateboard->SetPhysicsAngularVelocityInDegrees(AngularVel, false);
    }
}





// Turn in place, on ground 
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





// Turn while moving on ground
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




// Turn in air, after taking off from ramp
if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
{
    bool bIsAPressed = PC->IsInputKeyDown(EKeys::A);
    bool bIsDPressed = PC->IsInputKeyDown(EKeys::D);

    float VelocityLength = Skateboard->GetPhysicsLinearVelocity().Size();

    if ((bIsAPressed || bIsDPressed) && VelocityLength > 10.0f)
    {
        // Axis = Up vector of RampInitialRotation
        FVector Axis = RampInitialRotation.RotateVector(FVector::UpVector).GetSafeNormal();

        // Pick +3 or -3 degrees depending on input
        float AngleDeg = bIsDPressed ? 3.0f : -3.0f;

        // Make a delta quaternion from axis & angle
        FQuat DeltaQuat = FQuat(Axis, FMath::DegreesToRadians(AngleDeg));

        // Combine with current RampInAirRotation
        FQuat CurrentQuat = RampInAirRotation.Quaternion();
        FQuat NewQuat = DeltaQuat * CurrentQuat;

        RampInAirRotation = NewQuat.Rotator();
    }
}




// Turn in air
if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
{
    bool bIsAPressed = PC->IsInputKeyDown(EKeys::A);
    bool bIsDPressed = PC->IsInputKeyDown(EKeys::D);

    float VelocityLength = Skateboard->GetPhysicsLinearVelocity().Size();

    if ((bIsAPressed || bIsDPressed) && VelocityLength > 10.0f && !bIsOnGround)
    {
        // Choose rotation direction: +2 for D, -2 for A
        float YawStep = bIsDPressed ? 2.0f : -2.0f;

        // Apply local yaw rotation
        Skateboard->AddLocalRotation(FRotator(0.f, YawStep, 0.f));
    }
}





}

// Called to bind functionality to input
void ASkatePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

