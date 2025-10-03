
// Fill out your copyright notice in the Description page of Project Settings.

#include "SkatePawn.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"

ASkatePawn::ASkatePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Skateboard mesh as root
    Skateboard = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Skateboard"));
    RootComponent = Skateboard;

    // Arrow component
    ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
    ArrowComp->SetupAttachment(Skateboard);

    // Spring arm
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(Skateboard);
    SpringArm->TargetArmLength = 400.f;
    SpringArm->bUsePawnControlRotation = true;

    // Camera
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;
}

void ASkatePawn::BeginPlay()
{
    Super::BeginPlay();
}

void ASkatePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!ArrowComp || !GetWorld()) return;

    // -------------------- Ground Detection --------------------
    FVector TraceStart = ArrowComp->GetComponentLocation();
    FVector TraceEnd = TraceStart + ArrowComp->GetForwardVector() * 60.0f;
    float Radius = bIsOnGround ? 12.0f : 4.0f;

    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore{ this };

    bIsOnGround = UKismetSystemLibrary::SphereTraceMulti(
        GetWorld(), TraceStart, TraceEnd, Radius,
        UEngineTypes::ConvertToTraceType(ECC_Visibility),
        true, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true
    ) && HitResults.Num() > 0;

    // -------------------- Controller Input Snapshot --------------------
    bool bIsAPressed = false, bIsDPressed = false, bIsSPressed = false;
    float VelocityLength = 0.f;
    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    if (PC && Skateboard)
    {
        bIsAPressed = PC->IsInputKeyDown(EKeys::A);
        bIsDPressed = PC->IsInputKeyDown(EKeys::D);
        bIsSPressed = PC->IsInputKeyDown(EKeys::S);
        VelocityLength = Skateboard->GetPhysicsLinearVelocity().Size();
    }

    // -------------------- Upside-down correction --------------------
    FVector CurrentVelocity = GetVelocity();
    if (!bIsOnGround && !bIsOnRamp && CurrentVelocity.Size() <= 2.0f && Skateboard)
    {
        FRotator CurrentRotation = Skateboard->GetComponentRotation();
        Skateboard->SetWorldRotation(FRotator(0.0f, CurrentRotation.Yaw, 0.0f));
    }

    // -------------------- Stop on S key --------------------
    if (bIsOnGround && bIsSPressed && Skateboard)
    {
        Skateboard->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
    }

    // -------------------- Ramp flight --------------------
    if (!bIsOnGround && bIsOnRamp && Skateboard)
    {
        Skateboard->SetPhysicsLinearVelocity(RampInitialVelocity, false);
        RampInitialVelocity -= (UpVector * 5.0f);
        Skateboard->SetWorldRotation(RampInAirRotation);
    }

    // -------------------- Ramp landing flip --------------------
    if (bIsOnGround && bIsOnRamp && !bIsSPressed && Skateboard)
    {
        FVector Forward = Skateboard->GetForwardVector();
        FVector Velocity = Skateboard->GetPhysicsLinearVelocity();
        if (!Velocity.IsNearlyZero())
        {
            if (FVector::DotProduct(Forward, Velocity.GetSafeNormal()) <= 0.f)
            {
                Skateboard->AddLocalRotation(FRotator(0.f, 180.f, 0.f));
            }
        }
    }

    // -------------------- Main ground velocity --------------------
    if (bIsOnGround && !bIsSPressed && Skateboard)
    {
        FVector Forward = Skateboard->GetForwardVector();
        FVector DesiredVelocity = Forward * SkateSpeed;

        float ZValue = bIsJumping ? 500.f :
            bIsOnGround ? DesiredVelocity.Z :
            Skateboard->GetPhysicsLinearVelocity().Z;

        Skateboard->SetPhysicsLinearVelocity(
            FVector(DesiredVelocity.X, DesiredVelocity.Y, ZValue), false
        );
    }

    // -------------------- Camera follow --------------------
    if (Skateboard && SpringArm)
    {
        FVector BoardLocation = Skateboard->GetComponentLocation();
        SpringArm->SetWorldLocation(BoardLocation + FVector(0.f, 0.f, 100.f));
    }

    // -------------------- Angular velocity correction --------------------
    if (Skateboard && !bIsAPressed && !bIsDPressed)
    {
        FVector AngularVel = Skateboard->GetPhysicsAngularVelocityInDegrees();
        AngularVel.Z = 0.f;
        Skateboard->SetPhysicsAngularVelocityInDegrees(AngularVel, false);
    }

    // -------------------- Turning --------------------
    if (Skateboard)
    {
        // Turn in place
        if ((bIsAPressed || bIsDPressed) && VelocityLength <= 10.f)
        {
            float YawStep = bIsDPressed ? 1.f : -1.f;
            Skateboard->AddLocalRotation(FRotator(0.f, YawStep, 0.f));
        }

        // Turn while moving on ground
        if ((bIsAPressed || bIsDPressed) && VelocityLength > 10.f && bIsOnGround)
        {
            FVector AngularVel = Skateboard->GetPhysicsAngularVelocityInDegrees();
            AngularVel.Z = bIsDPressed ? 120.f : -120.f;
            Skateboard->SetPhysicsAngularVelocityInDegrees(AngularVel, false);
        }

        // Turn in air after ramp
        if ((bIsAPressed || bIsDPressed) && VelocityLength > 10.f && !bIsOnGround && bIsOnRamp)
        {
            FVector Axis = RampInitialRotation.RotateVector(FVector::UpVector).GetSafeNormal();
            float AngleDeg = bIsDPressed ? 3.f : -3.f;

            FQuat DeltaQuat = FQuat(Axis, FMath::DegreesToRadians(AngleDeg));
            FQuat CurrentQuat = RampInAirRotation.Quaternion();
            RampInAirRotation = (DeltaQuat * CurrentQuat).Rotator();
        }

        // General turn in air
        if ((bIsAPressed || bIsDPressed) && VelocityLength > 10.f && !bIsOnGround)
        {
            float YawStep = bIsDPressed ? 2.f : -2.f;
            Skateboard->AddLocalRotation(FRotator(0.f, YawStep, 0.f));
            MidAirRotation += YawStep;
        }
    }

    // -------------------- Ground State Event --------------------
    if (bIsOnGround != bPrevIsOnGround)
    {
        OnGroundStateChanged.Broadcast(bIsOnGround);
        bPrevIsOnGround = bIsOnGround;
    }
}

void ASkatePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

