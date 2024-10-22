// Fill out your copyright notice in the Description page of Project Settings.


#include "SMC_BaseWeapon.h"
#include "ENUM_TimeArena.h"
#include "PC_Player.h"
#include "CHAR_Player.h"
#include "ACTOR_BaseWeaponProjectile.h"

#pragma region Defaults

USMC_BaseWeapon::USMC_BaseWeapon()
{
	Damage = 10.0f;
	MuzzleOffset = FVector(400.0f, 0.0f, 10.0f);
}

#pragma endregion

#pragma region Projectile Functions

void USMC_BaseWeapon::FireWeapon(ACHAR_Player* RequestingPlayer)
{
	FString fireWeapon = FString::Printf(TEXT("SMC_BaseWeapon::FireWeapon() - Weapon Component Fired Weapon!"));
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, fireWeapon);

	if (RequestingPlayer == nullptr)
	{
		FString invalidCharRef = FString::Printf(TEXT("SMC_BaseWeapon::FireWeapon() - invalid Character Reference"));
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, invalidCharRef);
		return;
	}

	if (BulletProjectileClass != nullptr)
	{
		//Try to fire a projectile
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			//Get Player Controller to reference a projectile spawn point.
			CharacterSelf = RequestingPlayer;
			ControllerSelf = Cast<APC_Player>(CharacterSelf->GetController());
			const FRotator BulletProjectileSpawnRotation = ControllerSelf->PlayerCameraManager->GetCameraRotation();
			const FVector BulletProjectileSpawnLocation = CharacterSelf->GetActorLocation() + BulletProjectileSpawnRotation.RotateVector(MuzzleOffset);
			FVector TestProjectileSpawnLocation = FVector(100.0f, 100.0f, 100.0f);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters BulletProjectileSpawnParameters;
			BulletProjectileSpawnParameters.Owner = GetOwner();
			//BulletProjectileSpawnParameters.Instigator = GetInstigator();
			BulletProjectileSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			//Spawn the Bullet Projectile
			World->SpawnActor<AACTOR_BaseWeaponProjectile>(BulletProjectileClass, BulletProjectileSpawnLocation, BulletProjectileSpawnRotation, BulletProjectileSpawnParameters);
			UE_LOG(LogTemp, Warning, TEXT("Actor location is: X= %f, Y=%f, Z=%f"),BulletProjectileSpawnLocation.X, BulletProjectileSpawnLocation.Y, BulletProjectileSpawnLocation.Z)

		}
	}
}

#pragma endregion