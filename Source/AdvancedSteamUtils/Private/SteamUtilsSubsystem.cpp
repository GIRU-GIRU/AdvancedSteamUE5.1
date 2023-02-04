#include "SteamUtilsSubsystem.h"

#include <steam/isteamfriends.h>
#include <steam/isteamuser.h>

#include "OnlineSessionSettings.h"
#include "OnlineSubsystemSteam.h"
#include "AdvancedSteamUtils.h"
#include "Enums.h"
#include "HttpClientSubsystem.h"
#include "Structs.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"

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
		SteamSessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USteamUtilitiesSubsystem::CallbackSessionCreated);
		SteamSessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USteamUtilitiesSubsystem::CallbackJoinSession);
		//SteamSessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &USteamUtilitiesSubsystem::CallbackSessionStarted);

		SteamSessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &USteamUtilitiesSubsystem::CallbackAcceptedUserInvite);
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

void USteamUtilitiesSubsystem::CallbackSessionCreated(FName ServerName, bool Success)
{
	if (IOnlineSession* SteamSessionInterface = Steam->GetSessionInterface().Get())
	{
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Session Created Callback success for %s"), *ServerName.ToString());

		if (Steam->IsSteamClientAvailable())
		{
			if (GetWorld())
			{
				UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Client session started"));
			}
		}
		else if (Steam->IsSteamServerAvailable())
		{
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Dedicated Session successfully created"));
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Dedicated Session status is %s"), EOnlineSessionState::ToString(SteamSessionInterface->GetSessionState("DedicatedServer")));

			SteamSessionInterface->StartSession("DedicatedServer");
		}
	}
}

void USteamUtilitiesSubsystem::RequestCreateServerSession(FName ServerName)
{
	FOnlineSessionSettings SessionSettings;

	if (IOnlineSession* SteamSessionInterface = Steam->GetSessionInterface().Get())
	{
		if (Steam->IsSteamClientAvailable())
		{
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Creating client session %s"), *ServerName.ToString());

			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bAllowInvites = true;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bUsesPresence = true;
			SessionSettings.bIsDedicated = false;
			SessionSettings.bIsLANMatch = false;
			SessionSettings.NumPublicConnections = 5;
			SessionSettings.bAllowJoinViaPresence = true;
			SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;

			SteamSessionInterface->CreateSession(LOCAL_USER_NUMBER, ServerName, SessionSettings);
		}
		else if (Steam->IsSteamServerAvailable())
		{
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Creating dedicated server session %s"), *ServerName.ToString());

			SessionSettings.bAllowJoinInProgress = true;
			SessionSettings.bAllowInvites = true;
			SessionSettings.bShouldAdvertise = true;
			SessionSettings.bUsesPresence = false;
			SessionSettings.bIsDedicated = true;
			SessionSettings.bIsLANMatch = false;
			SessionSettings.NumPublicConnections = 20;
			SessionSettings.BuildUniqueId = 1;
			SessionSettings.bAllowJoinViaPresence = true;

			//Custom Settings
			SessionSettings.Set(SETTING_MAPNAME, GetWorld()->GetMapName(),
			                    EOnlineDataAdvertisementType::ViaOnlineService);
			SessionSettings.Set(SETTING_MATCHING_HOPPER, FString("Deathmatch"),
			                    EOnlineDataAdvertisementType::DontAdvertise);
			SessionSettings.Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
			SessionSettings.Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"),
			                    EOnlineDataAdvertisementType::DontAdvertise);
			SessionSettings.Set("SETTING_SERVER_NAME", ServerName.ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
			SessionSettings.Set(SETTING_GAMEMODE, FString(TEXT("GameModeName")), EOnlineDataAdvertisementType::ViaOnlineService);
			SessionSettings.Set(SETTING_MAPNAME, FString(TEXT("AssaultTest")), EOnlineDataAdvertisementType::ViaOnlineService);

			SteamSessionInterface->CreateSession(LOCAL_USER_NUMBER, ServerName, SessionSettings);
		}
	}
}

void USteamUtilitiesSubsystem::SearchForSessions()
{
	SessionSearchResults.Empty();

	if (IOnlineSession* SteamSessionInterface = Steam->GetSessionInterface().Get())
	{
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Searching for sessions"));

		SessionSearchSettings = MakeShareable(new FOnlineSessionSearch);
		SessionSearchSettings->bIsLanQuery = false;
		SessionSearchSettings->MaxSearchResults = 1000;
		SessionSearchSettings->QuerySettings.Set(SEARCH_PRESENCE, false, EOnlineComparisonOp::Equals);
		SessionSearchSettings->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
		SessionSearchSettings->QuerySettings.Set(SEARCH_EMPTY_SERVERS_ONLY, false, EOnlineComparisonOp::Equals);
		SessionSearchSettings->QuerySettings.Set(SEARCH_NONEMPTY_SERVERS_ONLY, false, EOnlineComparisonOp::Equals);

		SteamSessionInterface->FindSessions(LOCAL_USER_NUMBER, SessionSearchSettings.ToSharedRef());
	}
}


void USteamUtilitiesSubsystem::JoinSession(FSessionInfo TargetSession)
{
	FOnlineSessionSearchResult Target;
	
	if (SessionSearchSettings)
	{
		for (int i = 0; i < SessionSearchSettings->SearchResults.Num(); i++)
		{
			if (SessionSearchSettings->SearchResults[i].Session.OwningUserName == TargetSession.Address)
			{
				Target = SessionSearchSettings->SearchResults[i];
				break;
			}
		}
	}

	if (Target.IsSessionInfoValid())
	{
		RequestJoinSession(Target);
	}
}

void USteamUtilitiesSubsystem::CallbackFindSession(bool bSuccess)
{
	if (bSuccess)
	{
		if (SessionSearchSettings)
		{
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Session Find Callback success"));
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Found %d servers"), SessionSearchSettings->SearchResults.Num());

			for (FOnlineSessionSearchResult Result : SessionSearchSettings->SearchResults)
			{
				FSessionInfo SessionInfo;
				SessionInfo.Ping = Result.PingInMs;
				SessionInfo.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
				SessionInfo.PlayerCount = SessionInfo.MaxPlayers - Result.Session.NumOpenPublicConnections;
				Result.Session.SessionSettings.Get(SETTING_MAPNAME, SessionInfo.Map);
				Result.Session.SessionSettings.Get(SETTING_GAMEMODE, SessionInfo.GameMode);
				Result.Session.SessionSettings.Get("SETTING_SERVER_NAME", SessionInfo.SessionName);
				SessionInfo.Address = Result.Session.OwningUserName;

				SessionSearchResults.Add(SessionInfo);
			}
		}
	}
	else
	{
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Session Find Callback failure - unable to find any sessions"));
	}

	CallbackSearchForSessions.Broadcast(bSuccess);
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
			const FHttpRequestCompleteDelegate OnCompleteDelegate = FHttpRequestCompleteDelegate::CreateUObject(this, &USteamUtilitiesSubsystem::CallbackLogin);
			const TSharedRef<IHttpRequest> Req = HttpClient->CreateRequest("Login/Steam", ERestVerbType::Post, OnCompleteDelegate);

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

void USteamUtilitiesSubsystem::CallbackJoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	switch (Result)
	{
	case EOnJoinSessionCompleteResult::Success:
		if (IOnlineSession* SteamSessionInterface = Steam->GetSessionInterface().Get())
		{
			if (APlayerController* Controller = UGameplayStatics::GetPlayerController(GetWorld(), LOCAL_USER_NUMBER))
			{
				FString TargetAddress;
				SteamSessionInterface->GetResolvedConnectString(SessionName, TargetAddress);

				if (TargetAddress != "")
				{
					UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Traveling to %s"), *SessionName.ToString());
					Controller->ClientTravel(TargetAddress, ETravelType::TRAVEL_Absolute);
				}
				else
				{
					UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Unable to resolve session"));
				}
			}
		}

		break;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Session Full"));
		break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Session does not exist"));
		break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Could not retrieve address"));
		break;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Already in session"));
		break;
	case EOnJoinSessionCompleteResult::UnknownError:
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Unknown error"));
		break;
	default: ;
	}
}

void USteamUtilitiesSubsystem::CallbackAcceptedUserInvite(bool bWasSuccessful, int ControllerID, TSharedPtr<const FUniqueNetId, ESPMode::ThreadSafe> UniqueNetId,
                                                          const FOnlineSessionSearchResult& OnlineSessionSearchResult)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Successfully accepted user invite"));

		if (RequestJoinSession(OnlineSessionSearchResult) == false)
		{
			UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Failed to join session"));
		}
	}
	else
	{
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Failed to accept user invite"));
	}
}

bool USteamUtilitiesSubsystem::RequestJoinSession(FOnlineSessionSearchResult Session)
{
	bool Success = false;

	if (IOnlineSession* SteamSessionInterface = Steam->GetSessionInterface().Get())
	{
		UE_LOG(LogAdvancedSteamUtils, Log, TEXT("Attempting to join %s's steam session"), *Session.Session.OwningUserName);

		Success = SteamSessionInterface->JoinSession(LOCAL_USER_NUMBER, FName(Session.Session.OwningUserName), Session);
	}

	return Success;
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
