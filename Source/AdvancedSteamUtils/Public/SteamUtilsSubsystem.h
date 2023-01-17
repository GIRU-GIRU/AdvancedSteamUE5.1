#pragma once

#include "Interfaces/IHttpRequest.h"
#include "SteamUtilsSubsystem.generated.h"

USTRUCT()
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

class FOnlineSubsystemSteam;
class FOnlineSessionSearch;

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




	//>>>> Rich Presence
	/* This requires specific setup in the steam partner portal, check method body for details*/
	UFUNCTION(BlueprintCallable)
	void SetSteamRichPresence(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable)
	void ClearSteamRichPresence();


	//>>>> Sessions
	TSharedPtr<FOnlineSessionSearch> SessionSearchSettings;

	UFUNCTION(BlueprintCallable)
	void SearchForSessions();

	/* We have to use callbacks because async/await didnt exist in prehistoric days*/
	void CallbackFindSession(bool bSuccess);


	//>>>> Auth
	/* Example of what authenticating with your backend through steam would look like, backend code not included */
	UFUNCTION(BlueprintCallable)
	void RequestLogin();

	void CallbackLogin(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FAuthRequestData GetAuthRequestData();
};
