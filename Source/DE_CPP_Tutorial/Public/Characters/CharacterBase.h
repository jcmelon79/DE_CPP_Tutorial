// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Character.h"
#include "CharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UMatineeCameraShake;
class AInteractableBase;
class UCurveFloat;

UCLASS()
class DE_CPP_TUTORIAL_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACharacterBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	UStaticMeshComponent* MeshComp;

	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TSubclassOf<AInteractableBase> ActorToSpawn;
protected:

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void InteractPressed();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseTurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float BaseLookUpAtRate;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float TraceDistance;

	UFUNCTION(BlueprintNativeEvent)
	void TraceForward();
	void TraceForward_Implementation();

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(Category = "LinearImpulse", EditAnywhere)
	float ImpulseForce;
	void FireForward();

	UPROPERTY(Category = "RadicalImpulse", EditAnywhere)
	bool bApplyRadialForce;
	UPROPERTY(Category = "RadicalImpulse", EditAnywhere)
	float ImpactRadius;
	UPROPERTY(Category = "RadicalImpulse", EditAnywhere)
	float RadialImpactForce;

	TArray<FHitResult> HitActors;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UMatineeCameraShake> CamShake;

	UFUNCTION()
	void SpawnObject(FVector Loc, FRotator Rot);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* CurveFloat;

private:
	AActor* FocusedActor;

	FTimeline SquashTimeline;
	
	UPROPERTY()
	FVector StartScale;
	UPROPERTY()
	FVector TargetScale;

	UFUNCTION()
	void SquashProgress(float Value);
};
