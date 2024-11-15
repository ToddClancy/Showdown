// Fill out your copyright notice in the Description page of Project Settings.


#include "CHAR_Player.h"
#include "PC_Player.h"
#include "CMC_Player.h"
#include "SMC_BaseWeapon.h"
#include "Net/UnrealNetwork.h"
#include "Engine/EngineTypes.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/DamageType.h"
#include "Engine/Engine.h"

// Sets default values
ACHAR_Player::ACHAR_Player(const class FObjectInitializer& ObjectInitializer):
	Super(ObjectInitializer.SetDefaultSubobjectClass<UCMC_Player>(ACharacter::CharacterMovementComponentName))
{
 	//Default Character Properties
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Setup Initial Snapshot State
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));
	GetCMC_Player()->GravityScale = 0.0f;


	//Network Replication
	bReplicates = true;
	SetReplicateMovement(true);

}

void ACHAR_Player::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Replicated Properties
	DOREPLIFETIME(ACHAR_Player, CurrentHealth);

}

// Called when the game starts or when spawned
void ACHAR_Player::BeginPlay()
{
	Super::BeginPlay();
	NetUpdateFrequency = 120.0f;
}

// Called every frame
void ACHAR_Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACHAR_Player::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UCMC_Player* ACHAR_Player::GetCMC_Player() const
{
	return Cast<UCMC_Player>(GetCharacterMovement());
}

#pragma region Health

void ACHAR_Player::OnHealthUpdate()
{
	APC_Player* ReferencePlayer = Cast<APC_Player>(GetController());
	if (ReferencePlayer != nullptr)
	{
		//Host - Everything needs to happen (HUD Update + Character update)
		if (ReferencePlayer->GetLocalRole() == ROLE_Authority)
		{
			//Everything needs to happen. The HUD needs to be updated and the character needs to take damage.
			ReferencePlayer->SetHealthBarPercentage();
			if (CurrentHealth <= 0.0f)
			{
				FString deathMessage = FString::Printf(TEXT("You have perished."));
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
				ReferencePlayer->UpdateHUD = false;
				ReferencePlayer->DestroyPossessedCharacter(this);
				//Destroy();
				//Tell the PC player to destroy the pawn. 
			}
		}
		
		//Character client controls
		else if (GetLocalRole() == ROLE_SimulatedProxy)
		{
			if (CurrentHealth <= 0.0f)
			{
				ReferencePlayer->UpdateHUD = false;
				ServerDestroyCharacter(ReferencePlayer);
			}
			else
			{
				ReferencePlayer->SetHealthBarPercentage();
			}
		}
		
	//	else if (GetLocalRole() == ROLE_AutonomousProxy)
	//	{
	//		if (CurrentHealth <= 0.0f)
	//		{
	//			ReferencePlayer->UpdateHUD = false;
	//			ServerDestroyCharacter(ReferencePlayer);
	//		}
	//		else
	//		{
	//			ReferencePlayer->SetHealthBarPercentage();
	//			FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
	//			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
	//		}
	//	}
		
	}
}

void ACHAR_Player::ServerDestroyCharacter_Implementation(APC_Player* TargetPlayer)
{
	Destroy();
}

float ACHAR_Player::GetCurrentHealthPercentage() const
{
	float CurrentHealthPercentage = CurrentHealth / MaxHealth;
	//UE_LOG(LogTemp, Warning, TEXT("Current Health Percentage = %f"), CurrentHealthPercentage);
	return CurrentHealthPercentage;
}

void ACHAR_Player::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float ACHAR_Player::TakeDamage(float DamageTaken, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// TODO:  The Super implementation is not here.  If i get more sophisticated with the way my damage is applied, then I may have to update this. 
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	return damageApplied;
}

void ACHAR_Player::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

#pragma endregion


#pragma region Weapon Firing Mechanics

void ACHAR_Player::RequestFireWeapon()
{
	//FString requestFireWeapon = FString::Printf(TEXT("CHAR_Player::RequestFireWeapon() - CHARACTER BANG!"));
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, requestFireWeapon);
	CheckIfWeaponEquipped();
}

void ACHAR_Player::CheckIfWeaponEquipped()
{
	USMC_BaseWeapon* EquippedWeaponSMC = FindComponentByClass<USMC_BaseWeapon>();
	if (EquippedWeaponSMC)
	{
		//FString weaponEquipped = FString::Printf(TEXT("CHAR_Player::CheckIfWeaponEquipped() - You have a weapon, BANG!"));
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, weaponEquipped);
		EquippedWeaponSMC->NetworkRequestFireWeapon(this);
	}
	else
	{
		FString noWeaponEquipped = FString::Printf(TEXT("CHAR_Player::CheckIfWeaponEquipped() - STOP - No Weapon Equipped!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, noWeaponEquipped);
	}
}

void ACHAR_Player::MulticastSnapshotPossessionEvent_Implementation()
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetCMC_Player()->GravityScale = 1.0f;
}

void ACHAR_Player::PossessedBy(AController* NewController)
{
	//Server Changes
	Super::PossessedBy(NewController);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetCMC_Player()->GravityScale = 1.0f;

	//RPC to replicate the changes to the clients
	MulticastSnapshotPossessionEvent();

}


#pragma endregion

#pragma region Snapshot Spawn Points

void ACHAR_Player::RequestSetAdditionalSpawnParameters(FVector InputVelocity, float InputHealth, float InputAmmo)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		SetAdditionalSpawnParameters(InputVelocity, InputHealth, InputAmmo);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ACHAR_Player::RequestSetAdditionalSpawnParameters - Lacks authority to make this request"));
	}
}

void ACHAR_Player::SetAdditionalSpawnParameters(FVector InputVelocity, float InputHealth, float InputAmmo)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = InputHealth;
		SnapshotVelocity = InputVelocity;
		SnapshotAmmo = InputAmmo;
		UE_LOG(LogTemp, Warning, TEXT(" ACHAR_Player::SetAdditionalSpawnParameters - Health = %f, Ammo = %f"), CurrentHealth, SnapshotAmmo);

	}
}

#pragma endregion




