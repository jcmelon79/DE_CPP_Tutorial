// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ForceActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class DE_CPP_TUTORIAL_API AForceActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AForceActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Force Cube")
	UStaticMeshComponent* MeshComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	float ForceStrength;
};
