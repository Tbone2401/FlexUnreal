// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#if PLATFORM_WINDOWS

#include "Windows/WindowsErrorReport.h"

typedef FWindowsErrorReport FPlatformErrorReport;

#elif PLATFORM_LINUX

#include "Linux/LinuxErrorReport.h"

typedef FLinuxErrorReport FPlatformErrorReport;

#elif PLATFORM_MAC

#include "Mac/MacErrorReport.h"

typedef FMacErrorReport FPlatformErrorReport;

#endif // PLATFORM_LINUX
