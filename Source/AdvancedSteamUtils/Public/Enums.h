#pragma once

#include "Enums.generated.h"

UENUM(BlueprintType)
enum class ERestVerbType : uint8
{
	Get,
	Post,
	Put,
	Patch,
	Delete,
	Options
};
