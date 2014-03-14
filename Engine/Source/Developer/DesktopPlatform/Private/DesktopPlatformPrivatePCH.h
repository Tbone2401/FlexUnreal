// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Core.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDesktopPlatform, Log, All);

#include "IDesktopPlatform.h"

#if PLATFORM_WINDOWS
#include "Windows/DesktopPlatformWindows.h"
#elif PLATFORM_MAC
#include "Mac/DesktopPlatformMac.h"
#endif