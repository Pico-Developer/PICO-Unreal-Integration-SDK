// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "PICO_VerificationObject.h"
#include "PICO_PortalSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class PICOOPENXRPORTAL_API UPICO_PortalSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()


public:
	/**
	 * Initialize the subsystem. USubsystem override
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
	
	UPROPERTY()
	TArray<UPICOVerificationObject*> FixedVerificationObjects;
	UPROPERTY()
	TArray<UPICOVerificationObject*> RequiredVerificationObjects;
	
	bool Refresh(bool& bOutNeedRestartEditor);

	void HandleOnActorSpawned(AActor* Actor);

private:
	template<typename VerificationObject>
	void RegisterObject();
	FDelegateHandle OnActorSpawnedHandle;

};
