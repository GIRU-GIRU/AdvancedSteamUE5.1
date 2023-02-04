#pragma once

#include "Structs.generated.h"

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

	FSessionInfo() : Address(""), Ping(9999), SessionName(""), PlayerCount(0), MaxPlayers(0), GameMode(""), Map(""), Favorited(false), VACEnabled(false), Private(false){}

	UPROPERTY(BlueprintReadWrite)
	FString Address;
    
	UPROPERTY(BlueprintReadWrite)
	int Ping;

	UPROPERTY(BlueprintReadWrite)
	FString SessionName;

	UPROPERTY(BlueprintReadWrite)
	int PlayerCount;

	UPROPERTY(BlueprintReadWrite)
	int MaxPlayers;

	UPROPERTY(BlueprintReadWrite)
	FString GameMode;

	UPROPERTY(BlueprintReadWrite)
	FString Map;

	UPROPERTY(BlueprintReadWrite)
	bool Favorited;

	UPROPERTY(BlueprintReadWrite)
	bool VACEnabled;

	UPROPERTY(BlueprintReadWrite)
	bool Private;
};

USTRUCT(BlueprintType)
struct FAuthRequestData
{
	GENERATED_BODY()

	FString ID;
	FString AuthTicket;

	bool IsValid() const
	{
		return !ID.IsEmpty() && !AuthTicket.IsEmpty();
	}
};