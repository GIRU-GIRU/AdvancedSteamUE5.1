// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "UObject/Object.h"
#include "HttpClientSubsystem.generated.h"

/**
 * 
 */


enum class ERestVerbType : uint8;
class FHttpModule;
UCLASS()
class ADVANCEDSTEAMUTILS_API UHttpClientSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	FString SessionTicket;
	FString BaseEndpoint;

	FHttpModule* Http;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

public:
	
	TSharedRef<IHttpRequest> CreateRequest(FString Endpoint, ERestVerbType Verb, FHttpRequestCompleteDelegate InFunc);

protected:
	template <typename T>
	static FString EnumAsString(T EnumValue);
};
