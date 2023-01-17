// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdvancedSteamUtils.h"
#include "Core.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FAdvancedSteamUtilsModule"

DEFINE_LOG_CATEGORY(LogAdvancedSteamUtils);

void FAdvancedSteamUtilsModule::StartupModule()
{
	FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("AdvancedSteamUtils", "Advanced Steam Utils plugin loaded successfully"));
}

void FAdvancedSteamUtilsModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAdvancedSteamUtilsModule, AdvancedSteamUtils)
