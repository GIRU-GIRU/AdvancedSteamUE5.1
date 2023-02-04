// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpClientSubsystem.h"
#include "Enums.h"

#include "HttpModule.h"

void UHttpClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHttpClientSubsystem::Deinitialize()
{
	Super::Deinitialize();
}


TSharedRef<IHttpRequest> UHttpClientSubsystem::CreateRequest(FString Endpoint, ERestVerbType Verb, FHttpRequestCompleteDelegate InFunc)
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();

	Request->OnProcessRequestComplete() = InFunc;
	Request->SetVerb(EnumAsString(Verb));
	Request->SetHeader(TEXT("Host"), "HostHeader");
	Request->SetHeader(TEXT("User-Agent"), "X-UnrealEngine-Agent");
	Request->SetHeader("Accept", TEXT("*/*"));
	Request->SetHeader("SessionTicket", SessionTicket);
	
	Request->SetURL(BaseEndpoint + "/" + Endpoint);
	
	return Request;
}




template <typename T>
FString UHttpClientSubsystem::EnumAsString(T EnumValue)
{
	static_assert(TIsEnumClass<T>::Value, "Invalid type provided");
	return StaticEnum<T>()->GetValueAsString(EnumValue);
}
