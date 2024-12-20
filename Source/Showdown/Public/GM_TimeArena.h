// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GM_TimeArena.generated.h"

class AGS_TimeArena;
class APC_Player;
class ACHAR_Player;

UCLASS()
class SHOWDOWN_API AGM_TimeArena : public AGameMode
{
	GENERATED_BODY()
	
public:

UFUNCTION(BlueprintNativeEvent, Category="Server")
void SpawnCharacterForPlayer(APC_Player* PlayerController);

void SpawnCharacterForPlayer_Implementation(APC_Player* PlayerController);

protected:


UFUNCTION()
virtual void PostLogin(APlayerController* NewPlayer) override;

UFUNCTION()
virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;

UFUNCTION()
virtual void Logout(AController* ExitingController) override;

UFUNCTION(Exec)
void StartGame();

UFUNCTION(BlueprintCallable, Category = "Game Setup")
void AssignTeam();

#pragma region References
TArray<APlayerController*> PlayerControllerList;
TArray<APC_Player*> PC_PlayerList;
AGS_TimeArena* GameStateTimeArena;


#pragma endregion


#pragma region Teams
protected:
UPROPERTY()
TArray<APC_Player*> TeamAlphaPlayerControllers;
UPROPERTY()
TArray<APC_Player*> TeamBravoPlayerControllers;
#pragma endregion


#pragma region Spawning
protected:

	//Snapshot Class to Spawn
	UPROPERTY(EditDefaultsOnly, Category = "Custom Spawn")
	TSubclassOf<ACHAR_Player> SnapshotSpawnBPClass;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Custom Spawn")
	float SpawnZoneTimeInterval;

public:

	UFUNCTION(BlueprintCallable, Category = "Custom Spawn")
	void CreateSpawnPoint(APC_Player* PlayerReference);

	


#pragma endregion


};
