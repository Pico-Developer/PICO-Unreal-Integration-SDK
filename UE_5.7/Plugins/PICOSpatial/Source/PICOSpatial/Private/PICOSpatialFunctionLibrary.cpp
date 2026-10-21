// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialFunctionLibrary.h"
#include "PICOSpatialModule.h"
#include "PICOSpatial.h"

DEFINE_LOG_CATEGORY(LogPICOSpatial);

UPICOSpatialFunctionLibrary::UPICOSpatialFunctionLibrary()
{
}

UEventManagerPICO* UPICOSpatialFunctionLibrary::EventManagerPICO = nullptr;

UEventManagerPICO* UPICOSpatialFunctionLibrary::GetEventManagerPICO()
{
	if (EventManagerPICO == nullptr)
	{
		EventManagerPICO = NewObject<UEventManagerPICO>();
		EventManagerPICO->AddToRoot();
	}
	return EventManagerPICO;
}

bool UPICOSpatialFunctionLibrary::EnablePICOSpatialPICO(bool bEnable, bool bClearAllNativeCachedResources)
{
	return FPICOSpatial::Get().EnablePICOSpatial(bEnable, bClearAllNativeCachedResources);
}

bool UPICOSpatialFunctionLibrary::IsPICOSpatialEnabledPICO()
{
	return FPICOSpatial::Get().IsPICOSpatialEnabled();
}

bool UPICOSpatialFunctionLibrary::EnableDefaultWindowInputPICO(bool bEnable)
{
	return FPICOSpatial::Get().EnableDefaultWindowInput(bEnable);
}

bool UPICOSpatialFunctionLibrary::ModifyDefaultWindowPICO(int32 Alpha, bool bCustomSize, int32 Width, int32 Height, int32 OffsetX, int32 OffsetY)
{
	const int32 ClampedAlpha = FMath::Clamp(Alpha, 0, 100);
	if (bCustomSize && (Width <= 0 || Height <= 0))
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("ModifyDefaultWindow failed: custom size requires positive Width and Height. Width=%d Height=%d"), Width, Height);
		return false;
	}

	return FPICOSpatial::Get().ModifyDefaultWindow(ClampedAlpha, bCustomSize, Width, Height, OffsetX, OffsetY);
}
