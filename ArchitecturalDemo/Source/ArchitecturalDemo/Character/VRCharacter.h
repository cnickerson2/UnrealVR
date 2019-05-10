// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

UCLASS()
class ARCHITECTURALDEMO_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

    void MoveForward(float throttle);
    void MoveRight(float throttle);

    void UpdateDestinationMarker();
    void BeginTeleport();
    void FinishTeleport();
    bool FindTeleportDestination(FVector & OutLocation);
    void UpdateBlinders();

    void StartFade(float FromAlpha, float ToAlpha);

protected:
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class UCameraComponent* CameraComponent;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class USceneComponent* VRRoot;

    UPROPERTY(VisibleAnywhere, Category = "Movement")
    class UStaticMeshComponent* DestinationMarker;

    UPROPERTY(VisibleAnywhere, Category = "Camera")
    class UPostProcessComponent * PostProcessComponent;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float TeleportDistance = 1000; // 10 metres?

    // How long the teleport fade should last in seconds.
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float FadeDuration = 1;

    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    FVector TeleportProjectionExtent = FVector(100, 100, 100);

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    class UMaterialInterface * BlinderMaterialBase;

    UPROPERTY()
    class UMaterialInstanceDynamic * DynamicMaterialInstance;

    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    class UCurveFloat * RadiusVsVelocity;
};
