// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "ZipUtilityPlugin.h"

#include "CoreMinimal.h"
#include "CoreUObject.h"
#include "EngineMinimal.h"

// Windows platform includes
#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.

#include "ZipFileFunctionLibrary.h"