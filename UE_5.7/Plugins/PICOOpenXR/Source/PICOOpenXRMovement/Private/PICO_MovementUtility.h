// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFrameWork/Actor.h"
#include "GameFramework/WorldSettings.h"

template <typename T>
T* FindComponentByNamePICO(AActor* Actor, const FName& ComponentName)
{
	if (IsValid(Actor) && (ComponentName != NAME_None))
	{
		TArray<T*> ComponentsOfType;
		Actor->GetComponents<T>(ComponentsOfType);
		T** FoundComponent = ComponentsOfType.FindByPredicate([Name = ComponentName.ToString()](T* Component) { return Component->GetName().Equals(Name); });

		if (FoundComponent != nullptr)
		{
			return *FoundComponent;
		}
	}

	return nullptr;
}

FORCEINLINE bool GetUnitScaleFactorFromSettingsPICO(UWorld* World, float& outWorldToMeters)
{
	if (IsValid(World))
	{
		const auto* WorldSettings = World->GetWorldSettings();
		if (IsValid(WorldSettings))
		{
			outWorldToMeters = WorldSettings->WorldToMeters;
			return true;
		}
	}
	return false;
}