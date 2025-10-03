// Fill out your copyright notice in the Description page of Project Settings.


#include "SkatePawn.h"


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

}

// Called to bind functionality to input
void ASkatePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

