#include "SteamUtilsSubsystem.h"

#include <steam/isteamfriends.h>
#include <steam/isteamuser.h>

#include "OnlineSessionSettings.h"
#include "OnlineSubsystemSteam.h"
#include "AdvancedSteamUtils.h"
#include "Enums.h"
#include "HttpClientSubsystem.h"
#include "JsonObjectConverter.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/OnlineSessionInterface.h"

USteamUtilitiesSubsystem::USteamUtilitiesSubsystem()
{
}

void USteamUtilitiesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	UGameInstanceSubsystem::Initialize(Collection);

	Steam = static_cast<FOnlineSubsystemSteam*>(IOnlineSubsystem::Get());

	check(Steam)

	if (IOnlineSession* SteamSessionInterface = Steam->GetSessionInterface().Get())
	{
		SteamSessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &USteamUtilitiesSubsystem::CallbackFindSession);
		//SteamSessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USteamUtilitiesSubsystem::CallbackSessionCreated);
		//SteamSessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USteamUtilitiesSubsystem::CallbackJoinSession);
		//SteamSessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &USteamUtilitiesSubsystem::CallbackSessionStarted);

		//SteamSessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &USteamUtilitiesSubsystem::CallbackAcceptedUserInvite);
	}
}

void USteamUtilitiesSubsystem::Deinitialize()
{
	UGameInstanceSubsystem::Deinitialize();
}

void USteamUtilitiesSubsystem::SetSteamRichPresence(const FString& Key, const FString& Value)
{
	//https://trdwll.com/blog/ue4-and-steam-rich-presence/
	//https://partner.steamgames.com/doc/features/enhancedrichpresence

	//TL:DR You need to do some stuff on your steam partner portal if you want rich presence to work
	SteamFriends()->SetRichPresence(TCHAR_TO_UTF8(*Key), TCHAR_TO_UTF8(*Value));
}

void USteamUtilitiesSubsystem::ClearSteamRichPresence()
{
	SteamFriends()->ClearRichPresence();
}

void USteamUtilitiesSubsystem::SearchForSessions()
{
	if (IOnlineSession* SteamSessionInterface = Steam->GetSessionInterface().Get())
	{
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Searching for sessions"));

		SessionSearchSettings = MakeShareable(new FOnlineSessionSearch);
		SessionSearchSettings->bIsLanQuery = false;
		SessionSearchSettings->MaxSearchResults = 10000;
		SessionSearchSettings->QuerySettings.Set(SEARCH_PRESENCE, false, EOnlineComparisonOp::Equals);
		SessionSearchSettings->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
		SessionSearchSettings->QuerySettings.Set(SEARCH_EMPTY_SERVERS_ONLY, false, EOnlineComparisonOp::Equals);
		SessionSearchSettings->QuerySettings.Set(SEARCH_NONEMPTY_SERVERS_ONLY, false, EOnlineComparisonOp::Equals);

		SteamSessionInterface->FindSessions(LOCAL_USER_NUMBER, SessionSearchSettings.ToSharedRef());
	}
}

void USteamUtilitiesSubsystem::CallbackFindSession(bool bSuccess)
{
	if (bSuccess) //TODO: replace with callback in SessionService
	{
		if (SessionSearchSettings)
		{
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Session Find Callback success"));
			const TArray<FOnlineSessionSearchResult> FoundSessions = SessionSearchSettings->SearchResults;
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Found %d servers"), FoundSessions.Num());
		}
	}
	else
	{
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Session Find Callback failure - unable to find any sessions"));
	}
}

void USteamUtilitiesSubsystem::RequestLogin()
{
	if (UHttpClientSubsystem* HttpClient = GetGameInstance()->GetSubsystem<UHttpClientSubsystem>())
	{
		const FAuthRequestData ReqData = GetAuthRequestData();

		if (ReqData.IsValid() == false)
		{
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Unable to procure correct request data for backend logic"));
		}
		else
		{
			//The endpoint parameter is just to simulate what the backend controllers might look like
			const TSharedRef<IHttpRequest> Req = HttpClient->CreateRequest("Login/Steam", ERestVerbType::Post, this, &USteamUtilitiesSubsystem::CallbackLogin);

			//https://partner.steamgames.com/doc/webapi_overview/oauth
			Req->SetHeader("SteamID", ReqData.ID);
			Req->SetHeader("SteamAuthTicket", ReqData.AuthTicket);

			Req->ProcessRequest();
		}
	}
}

void USteamUtilitiesSubsystem::CallbackLogin(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		FString ResponseContent = Response->GetContentAsString();

		//This is where you would attempt to deserialize your response from the backend etc.

		// if (FJsonObjectConverter::JsonObjectStringToUStruct(ResponseContent, &BackendPlayer, 0, 0, false))
		// {
		// 	SessionTicket = Response->GetHeader("session-ticket");
		//
		// 	if (!SessionTicket.IsEmpty())
		// 	{
		// 		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Successfully logged in as %s"), *BackendPlayer.Id.ToString()));
		//
		// 	}
		// }
	}
}

FAuthRequestData USteamUtilitiesSubsystem::GetAuthRequestData()
{
	FAuthRequestData Result;

	if (SteamUser())
	{
		uint32 Size;
		TArray<uint8> Buffer;
		Buffer.SetNumUninitialized(1024);

		const HAuthTicket TicketHandle = SteamUser()->GetAuthSessionTicket(Buffer.GetData(), 1024, &Size);
		if (TicketHandle == k_HAuthTicketInvalid)
		{
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Get Auth Session ticket failure - to retrieve session ticket"));
		}
		else
		{
			FString AuthTicket = FString::FromHexBlob(Buffer.GetData(), Size);

			const CSteamID CSteamID = SteamUser()->GetSteamID();
			FString SteamID = FString::Printf(TEXT("%llu"), CSteamID.ConvertToUint64());

			Result.ID = SteamID;
			Result.AuthTicket = AuthTicket;
		}
	}

	return Result;
}
