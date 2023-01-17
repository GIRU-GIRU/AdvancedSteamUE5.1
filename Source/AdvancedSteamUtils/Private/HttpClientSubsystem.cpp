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

template<typename UserClass, typename... VarTypes>
TSharedRef<IHttpRequest> UHttpClientSubsystem::CreateRequest(FString Endpoint, ERestVerbType Verb, UserClass* WorldContextObject, typename TMemFunPtrType<false, UserClass, void(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)>::Type InFunc)
{
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();

	Request->OnProcessRequestComplete().BindUObject(WorldContextObject, InFunc);
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
