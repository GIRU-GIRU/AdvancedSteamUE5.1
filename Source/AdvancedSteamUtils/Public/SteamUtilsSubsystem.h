#pragma once

#include "Interfaces/IHttpRequest.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Structs.h"
#include "SteamUtilsSubsystem.generated.h"

class FOnlineSubsystemSteam;
class FOnlineSessionSearch;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCallbackSearchForSessions, bool, Success);

UCLASS()
class USteamUtilitiesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USteamUtilitiesSubsystem();

#define LOCAL_USER_NUMBER (0)

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	FOnlineSubsystemSteam* Steam;

	//>>>> Sessions
	UFUNCTION(BlueprintCallable)
	void SearchForSessions();

	UFUNCTION(BlueprintCallable)
	void JoinSession(FSessionInfo TargetSession);
	
	UFUNCTION(BlueprintCallable)
	void CallbackSessionCreated(FName ServerName, bool Success);

	UFUNCTION(BlueprintCallable)
	void RequestCreateServerSession(FName ServerName);

	TSharedPtr<FOnlineSessionSearch> SessionSearchSettings;

	/* We have to use callbacks because async/await didnt exist in prehistoric days*/
	void CallbackFindSession(bool bSuccess);

	UPROPERTY(BlueprintAssignable)
	FCallbackSearchForSessions CallbackSearchForSessions;


	void CallbackAcceptedUserInvite(bool bWasSuccessful, int ControllerID, TSharedPtr<const FUniqueNetId, ESPMode::ThreadSafe> UniqueNetId,
	                                const FOnlineSessionSearchResult& OnlineSessionSearchResult);

	bool RequestJoinSession(FOnlineSessionSearchResult Session);
	void CallbackJoinSession(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UPROPERTY(BlueprintReadOnly)
	TArray<FSessionInfo> SessionSearchResults;



	
	//>>>> Rich Presence
	/* This requires specific setup in the steam partner portal, check method body for details*/
	UFUNCTION(BlueprintCallable)
	void SetSteamRichPresence(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable)
	void ClearSteamRichPresence();

	//>>>> Auth
	/* Example of what authenticating with your backend through steam would look like, backend code not included */
	UFUNCTION(BlueprintCallable)
	void RequestLogin();
	void CallbackLogin(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	FAuthRequestData GetAuthRequestData();
};
