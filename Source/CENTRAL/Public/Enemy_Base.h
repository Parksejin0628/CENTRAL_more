// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy_Base.generated.h"

UCLASS()
class CENTRAL_API AEnemy_Base : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy_Base();
	bool CanDetectPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	ACharacter *PlayerCharacter;
	FColor DebugColor_Range;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};