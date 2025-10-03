#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RampCollision.generated.h"

UCLASS()
class SKATEBOARD_API ARampCollision : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ARampCollision();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Ramp")
	void SetPawnOffRamp(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Ramp")
	void SetPawnOnRamp(AActor* TargetActor);

};
