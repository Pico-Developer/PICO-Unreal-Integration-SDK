// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_HandTrackingFunctionLibrary.h"
#include "PICO_IHandTrackingModule.h"
#include "PICO_HandTracking.h"

bool UHandTrackingFunctionLibraryPICO::StartHandTrackingPICO()
{
	FHandTrackingPICO* HandTrackingPICO = static_cast<FHandTrackingPICO*>(IPICOOpenXRHandTrackingModule::Get().GetHandTrackingPICO());
	if (HandTrackingPICO)
	{
		return HandTrackingPICO->StartHandTracking();
	}
	return false;
}

void UHandTrackingFunctionLibraryPICO::StopHandTrackingPICO()
{
	FHandTrackingPICO* HandTrackingPICO = static_cast<FHandTrackingPICO*>(IPICOOpenXRHandTrackingModule::Get().GetHandTrackingPICO());
	if (HandTrackingPICO)
	{
		HandTrackingPICO->StopHandTracking();
	}
}

bool UHandTrackingFunctionLibraryPICO::UpdateHandTrackingDataPICO()
{
	FHandTrackingPICO* HandTrackingPICO = static_cast<FHandTrackingPICO*>(IPICOOpenXRHandTrackingModule::Get().GetHandTrackingPICO());
	if (HandTrackingPICO)
	{
		return HandTrackingPICO->UpdateHandTrackingData();
	}
	return false;
}

bool UHandTrackingFunctionLibraryPICO::IsHandTrackingSupportPICO()
{
	FHandTrackingPICO* HandTrackingPICO = static_cast<FHandTrackingPICO*>(IPICOOpenXRHandTrackingModule::Get().GetHandTrackingPICO());
	if (HandTrackingPICO)
	{
		return HandTrackingPICO->IsHandTrackingSupportedByDevice();
	}
	return false;
}

bool UHandTrackingFunctionLibraryPICO::IsHandTrackingRunningPICO()
{
	FHandTrackingPICO* HandTrackingPICO = static_cast<FHandTrackingPICO*>(IPICOOpenXRHandTrackingModule::Get().GetHandTrackingPICO());
	if (HandTrackingPICO)
	{
		return HandTrackingPICO->IsHandTrackingRunning();
	}
	return false;
}

bool UHandTrackingFunctionLibraryPICO::GetHandTrackingDataPICO(EControllerHand Hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations, TArray<float>& OutRadii, TArray<FVector>& LinearVelocity, TArray<FVector>& AngularVelocity, float& Scale)
{
	FHandTrackingPICO* HandTrackingPICO = static_cast<FHandTrackingPICO*>(IPICOOpenXRHandTrackingModule::Get().GetHandTrackingPICO());
	if (HandTrackingPICO)
	{
		return HandTrackingPICO->GetHandTrackingData(Hand, OutPositions, OutRotations, OutRadii, LinearVelocity, AngularVelocity, Scale);
	}
	return false;
}

bool UHandTrackingFunctionLibraryPICO::GetHandTrackingMeshScalePICO(EControllerHand Hand, float& Scale)
{
	FHandTrackingPICO* HandTrackingPICO = static_cast<FHandTrackingPICO*>(IPICOOpenXRHandTrackingModule::Get().GetHandTrackingPICO());
	if (HandTrackingPICO)
	{
		return HandTrackingPICO->GetHandTrackingMeshScale(Hand, Scale);
	}
	return false;
}
