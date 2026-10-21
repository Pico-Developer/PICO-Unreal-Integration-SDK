// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "PICO_IMovementModule.h"
#include "ILiveLinkSource.h"
#include "PICO_LiveLinkSource.h"
#include "PICO_MovementExtension.h"

class FPICOOpenXRMovementModule : public IPICOOpenXRMovementModule
{
public:
	static inline FPICOOpenXRMovementModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPICOOpenXRMovementModule>("PICOOpenXRMovement");
	}

	FFaceTrackingPICO& GetFaceTrackingPICOExtension() { return MovementExtension.GetFaceTrackingPICO(); }
	FBodyTrackingPICO& GetBodyTrackingPICOExtension() { return MovementExtension.GetBodyTrackingPICO(); }
	FMotionTrackingPICO& GetMotionTrackingPICOExtension() { return MovementExtension.GetMotionTrackingPICO(); }
	FEyeTrackingPICO& GetEyeTrackingPICOExtension() { return MovementExtension.GetEyeTrackingPICO(); }
	FExpandDevicePICO& GetFExpandDevicePICOExtension() { return MovementExtension.GetExpandDevicePICO(); }

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	/* Live link */
	virtual TSharedPtr<ILiveLinkSource> GetLiveLinkSource();
	virtual bool IsLiveLinkSourceValid() const;
	virtual void AddLiveLinkSource();
	virtual void RemoveLiveLinkSource();

private:
	TSharedPtr<PICOLiveLink::LiveLinkSource> MovementSource{ nullptr };

public:
	FMovementExtensionPICO MovementExtension;
};