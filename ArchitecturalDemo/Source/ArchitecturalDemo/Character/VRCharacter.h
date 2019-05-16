// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"


#pragma region Forward Declarations
class UMotionControllerComponent;
class UInputComponent;
class UCameraComponent;
class USceneComponent;
class UStaticMeshComponent;
class UPostProcessComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UCurveFloat;
#pragma endregion Forward Declarations

UENUM()
enum class EHandedness
{
    Left,
    Right,
    None
};

UCLASS()
class ARCHITECTURALDEMO_API AVRCharacter : public ACharacter
{
    GENERATED_BODY()

#pragma region Properties
private:

    /** The Parent Scene component in which all VR elements are rooted */
    UPROPERTY(VisibleAnywhere, Category = "VR")
    USceneComponent* VRRoot;

    /** The VR Camera */
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    UCameraComponent* CameraComponent;
    /** The Post Processing that applies the Blinder effect */
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    UPostProcessComponent* PostProcessComponent;
    /** How long the teleport fade should last in seconds. */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    float FadeDuration = 1;
    /** The editable material in which the blinder instance is created */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    UMaterialInterface * BlinderMaterialBase;
    /** The actual material instance of the blinder that is created and updated at runtime */
    UPROPERTY(VisibleAnywhere, Category = "Camera")
    UMaterialInstanceDynamic * DynamicMaterialInstance;
    /** The curve that determines how wide the blinder effect is based off velocity of the character */
    UPROPERTY(EditDefaultsOnly, Category = "Camera")
    UCurveFloat * RadiusVsVelocity;

    /** The marker that appears on the ground */
    UPROPERTY(VisibleAnywhere, Category = "Movement")
    UStaticMeshComponent* DestinationMarker;
    /** How far from the player can the marker appear */
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float TeleportDistance = 1000;
    /** The distance checked by the marker to see if it can hit a nav mesh */
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    FVector TeleportProjectionExtent = FVector(100, 100, 100);

    /** The Motion Controller for the left hand */
    UPROPERTY(VisibleAnywhere, Category = "Controllers")
    UMotionControllerComponent* LeftHand;
    /** The Motion Controller for the right hand */
    UPROPERTY(VisibleAnywhere, Category = "Controllers")
    UMotionControllerComponent* RightHand;
    /** The last hand that tried to teleport */
    UPROPERTY()
    EHandedness LastUsedHandedness = EHandedness::None;
#pragma endregion Properties

#pragma region Methods
public:
    /** Sets default values for this character's properties */
    AVRCharacter();

private:
    /** Called when the game starts or when spawned */
    void BeginPlay() override;
    /** Called to bind functionality to input */
    void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    /** Called every frame */
    void Tick(float DeltaTime) override;

    /** Update the offset of the camera to ensure the Character Capsule is always centered on the player's head */
    void CenterCharacterOnHead();
  
    /**  Move the character forward by the throttle amount. If negative, character moves backwards */
    void MoveForward(float Throttle);
    /**  Move the character right by the throttle amount. If negative, character moves left */
    void MoveRight(float Throttle);


    /** Updates the position of the Marker */
    void UpdateDestinationMarker();
    /** 
     * Try to find a destination on the NavMesh that is within Teleport Distance
     * @param OutLocation - Assigns the Location of the Teleport Destination, if acceptable. Else, nothing is assigned.
     * @return bool - Found an appropriate Teleport Destination on the NavMesh
     */
    bool CanFindTeleportDestination(FVector & OutLocation);
    /** 
    * Return the component that the origin of the teleport should be 
    * @param OutOriginComponent - Assigns the Origin Component depending on the Last Used Handed
    */
    void GetTeleportOrigin(USceneComponent* &OutOriginComponent);
    /** 
     * Find out if there is something that can be hit within the Teleport Distance of the Origin
     * @param OutHitResult - Assigns the results of hit of the line cast
     * @param OriginComponent - The Origin Component depending on the Last Used Handed
     * @return bool - The line cast hit something
     */
    bool CanHitSomethingWithinTeleportDistance(FHitResult& OutHitResult, USceneComponent* const & OriginComponent);
    /** 
     * Find out if the Hit Result is within the extent range of a NavMesh
     * @param OutNavLocation - Assigns the Location on the NavMesh
     * @param HitResult - The results of hit of the line cast
     * @return bool - There was a NavMesh within the extent of the HitResult
     */
    bool IsWhatIHitNearANavMesh(FNavLocation& OutNavLocation, FHitResult const & HitResult);


    /** Called when the right hand trigger is pulled. */
    void TeleportRight();
    /** Called when the left hand trigger is pulled. */
    void TeleportLeft();
    /** Called when teleport button is pressed by anything other than a motion controller */
    void TeleportCenter();
    /** Check if the player was using this hand last time they clicked teleport */
    bool WasThisHandTheSameAsLastTime(EHandedness ThisTimesHandedness)
    /** Start teleporting to the destination */
    void BeginTeleport();
    /** Finish teleporting to the destination */
    void FinishTeleport();
    /** Fade the Camera from one Alpha to another. 1 is fully coloured. 0 is fully clear */
    void StartFade(float FromAlpha, float ToAlpha);


    /** Create a dynamic Blinder material instance that can be changed by code */
    void SetupBlinders();
    /** Change the size of the Blinder post process based on speed */
    void UpdateBlinders();
    /** Return the position of the center of the blinder */
    FVector2D GetBlinderCenter();
#pragma endregion Methods
};
