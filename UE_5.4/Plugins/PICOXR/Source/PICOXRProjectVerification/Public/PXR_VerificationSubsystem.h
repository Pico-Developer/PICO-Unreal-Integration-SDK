// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "PXR_VerificationObject.h"

#include "PXR_VerificationSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PICOXRPROJECTVERIFICATION_API UPXR_VerificationSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()


public:
	/**
	 * Initialize the subsystem. USubsystem override
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY()
	TArray<UVerificationObject*> FixedVerificationObjects;
	UPROPERTY()
	TArray<UVerificationObject*> RequiredVerificationObjects;
	
	bool Refresh(bool& bOutNeedRestartEditor);
private:
	template<typename VerificationObject>
	void RegisterObject();

};
