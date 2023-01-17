// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAdvancedSteamUtils, Log, All);

class FAdvancedSteamUtilsModule : public IModuleInterface
{
public:

#pragma region IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
#pragma endregion IModuleInterface
};
