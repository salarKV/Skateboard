#include "RampCollision.h"
#include "Components/BoxComponent.h"
#include "SkatePawn.h"

// Sets default values
ARampCollision::ARampCollision()
{
	PrimaryActorTick.bCanEverTick = true;

	
}

void ARampCollision::SetPawnOffRamp(AActor* TargetActor)
{
    if (ASkatePawn* SkatePawn = Cast<ASkatePawn>(TargetActor))
    {
        SkatePawn->bIsOnRamp = false;
    }
}

void ARampCollision::SetPawnOnRamp(AActor* TargetActor)
{
    if (ASkatePawn* SkatePawn = Cast<ASkatePawn>(TargetActor))
    {
        // Set ramp state
        SkatePawn->bIsOnRamp = true;
        SkatePawn->UpVector = GetActorUpVector();

        if (SkatePawn->Skateboard)
        {
            // Get current velocity
            FVector Velocity = SkatePawn->Skateboard->GetPhysicsLinearVelocity();

            // Ramp forward vector
            FVector RampForward = GetActorForwardVector();

            // Project velocity onto ramp forward
            FVector ForwardComponent = FVector::DotProduct(Velocity, RampForward) * RampForward;

            // Remove forward component from velocity
            FVector FinalVelocity = Velocity - ForwardComponent;

            // Store result in pawn
            SkatePawn->RampInitialVelocity = FinalVelocity;

            // Get world rotation of skateboard
            FRotator BoardRotation = SkatePawn->Skateboard->GetComponentRotation();

            // Store in pawn rotation vars
            SkatePawn->RampInAirRotation = BoardRotation;
            SkatePawn->RampInitialRotation = BoardRotation;
        }
    }
}





// Called when the game starts or when spawned
void ARampCollision::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ARampCollision::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
