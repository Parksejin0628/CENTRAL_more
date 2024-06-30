// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CENTRALCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ACENTRALCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RollAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* RollingMontage;

	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* AttackMontage1;
	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* AttackMontage2;
	UPROPERTY(EditAnywhere, Category = Montages)
	UAnimMontage* AttackMontage3;
public:
	ACENTRALCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void Rolling();

	//공격 관련 함수
	void Attack();
	void InputAttack();
	
	UFUNCTION(BlueprintCallable)
	//점프중인지 확인 하는 함수
	bool CheckJumping();

	UPROPERTY(BlueprintReadWrite)
	bool IsRolling;

	UPROPERTY(BlueprintReadWrite)
	bool IsAttacking;

	UPROPERTY(BlueprintReadWrite)
	bool IsEskilling;


	bool DoOnce;

	//공격 관련 변수
	bool CanAttack;
	bool InputComboAttack;
	bool GoNextCombo;
	int ComboCount;

	//animnotify 함수호출
	UFUNCTION(BlueprintCallable)
	void NextComboAttacking();

	UFUNCTION(BlueprintCallable)
	void StopCombo();
	
	UFUNCTION(BlueprintCallable)
	void StopRolling();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint")
	void RollingStamina();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Blueprint")
	bool AttackingStamina();

	/*ENGINE_API virtual void Jump();*/
};

