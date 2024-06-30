// Copyright Epic Games, Inc. All Rights Reserved.

#include "CENTRALCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACENTRALCharacter

ACENTRALCharacter::ACENTRALCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	CanAttack = true;
	IsAttacking = false;
	IsRolling = false;
	IsEskilling = false;
	ComboCount = 0;
	InputComboAttack = false;
	GoNextCombo = false;
	DoOnce = false;
}

void ACENTRALCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ACENTRALCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACENTRALCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACENTRALCharacter::Look);

		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ACENTRALCharacter::Rolling);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ACENTRALCharacter::InputAttack);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ACENTRALCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACENTRALCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller-
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACENTRALCharacter::Rolling() {

	if (CheckJumping()||IsRolling||IsAttacking||IsEskilling) return;
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance) {
		RollingStamina();
		AnimInstance->Montage_Play(RollingMontage);
	}
	IsRolling = true;
}

void ACENTRALCharacter::StopRolling() {
	IsRolling = false;
}

//좌클릭 입력시 IsAttacking따라 단타 공격인지 콤보공격인지 판별
void ACENTRALCharacter::InputAttack() {

	if (CheckJumping()||IsRolling||IsEskilling) return;

	if (!IsAttacking) {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("First Attack input"));
		IsAttacking = true;
		Attack();
	}
	else {
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Attack Combo input"));
		InputComboAttack = true;
	}
}

//공격 에니메이션을 진행하는 함수
void ACENTRALCharacter::Attack() {
	if (AttackingStamina()) {
		StopCombo();
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance||DoOnce) return;
	DoOnce = true;

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("Attack()"));
	switch (ComboCount) {
	case 0:
		AnimInstance->Montage_Play(AttackMontage1);
		ComboCount = 1;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("1"));
		break;
	case 1:
		AnimInstance->Montage_Play(AttackMontage2);
		ComboCount = 0;
		break;
	}
	DoOnce = false;
}

//다음 콤보로 연결하는 노티파이가 호출하는 함수
void  ACENTRALCharacter::NextComboAttacking() {

	if (!InputComboAttack)
	{
		StopCombo();
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("NotCombo"));
	}
	else {
		InputComboAttack = false;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("NextAttack()"));
		Attack();
	}
}

//콤보를 멈추는 함수
void ACENTRALCharacter::StopCombo() {
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("StopAttack"));
	InputComboAttack = false;
	IsAttacking = false;
	ComboCount = 0;
}

bool ACENTRALCharacter::CheckJumping() {
	if (GetCharacterMovement()->IsFalling()) return true;
	else return false;
}

//void ACENTRALCharacter::Jump() {
//	if (CheckJumping() || IsRolling || IsAttacking) return;
//	bPressedJump = true;
//	JumpKeyHoldTime = 0.0f;
//}