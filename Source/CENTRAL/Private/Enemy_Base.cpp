// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Base.h"
#include "Engine/HitResult.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#define DETECT_RANGE 500.f

// Sets default values
AEnemy_Base::AEnemy_Base()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	DebugColor_Range = FColor::White;
}

// Called when the game starts or when spawned
void AEnemy_Base::BeginPlay()
{
	Super::BeginPlay();
	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
}

// Called every frame
void AEnemy_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//DrawDebugSphere(GetWorld(), GetActorLocation(), DETECT_RANGE, 20, DebugColor_Range, true, 0.2f);
	//CanDetectPlayer();
}

// Called to bind functionality to input
void AEnemy_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool AEnemy_Base::CanDetectPlayer()
{
	FVector ThisLocation = GetActorLocation();
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	float PlayerDistance = UKismetMathLibrary::VSize(ThisLocation - PlayerLocation);
	FHitResult Hit;
	FCollisionQueryParams QueryParams;

	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(PlayerCharacter);
	GetWorld()->LineTraceSingleByChannel(Hit, ThisLocation, PlayerLocation, ECollisionChannel::ECC_Visibility, QueryParams);
	UE_LOG(LogTemp, Warning, TEXT("현재 거리 %f"), PlayerDistance);
	if (PlayerDistance < DETECT_RANGE && !Hit.bBlockingHit)
	{
		DebugColor_Range = FColor::Red;
		return true;
	}
	DebugColor_Range = FColor::White;
	return false;
}

