// Fill out your copyright notice in the Description page of Project Settings.

#include "PICO_LightEstimation.h"

#include "ClearQuad.h"
#include "CommonRenderResources.h"
#include "ExternalTexture.h"
#include "IOpenXRHMDModule.h"
#include "OculusShaders.h"
#include "OpenXRCore.h"
#include "PICO_MRModule.h"
#include "ScreenRendering.h"
#include "openxr/pico_light_estimation.h"
#include "TextureResource.h"


#define VULKAN_CUBEMAP_POSITIVE_Y 2
#define VULKAN_CUBEMAP_NEGATIVE_Y 3

FLightEstimationExtensionPICO::FLightEstimationExtensionPICO()
	: CurrentResolution(ESpatialLightEstimationResolutionPICO::TextureResolution_128x128)
	, LightEstimationSize(128)
{
}


void FLightEstimationExtensionPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_LIGHT_ESTIMATION_EXTENSION_NAME);
}

void FLightEstimationExtensionPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	if (FSpatialSensingExtensionPICO::GetInstance()->IsSupportsSpatialSensing())
	{
		bSupportLightEstimateEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_LIGHT_ESTIMATION_EXTENSION_NAME);
		UE_LOG(LogMRPICO, Verbose, TEXT("bSupportLightEstimateEXT:%d"), bSupportLightEstimateEXT)

		if (bSupportLightEstimateEXT)
		{
			XrSystemLightEstimationPropertiesPICO LightEstimationPropertiesPICO = {
				(XrStructureType)XR_TYPE_SYSTEM_LIGHT_ESTIMATION_PROPERTIES_PICO
			};
			XrSystemProperties SPLightEstimation{XR_TYPE_SYSTEM_PROPERTIES, &LightEstimationPropertiesPICO};
			XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &SPLightEstimation));
			bSupportLightEstimation = LightEstimationPropertiesPICO.supportsEnvironmentTexture == XR_TRUE;
			UE_LOG(LogMRPICO, Log, TEXT("bSupportLightEstimation:%d"), bSupportLightEstimation)
		}
	}
}

void FLightEstimationExtensionPICO::PostCreateSession(XrSession InSession)
{
	FMixedRealityPICO::PostCreateSession(InSession);

	const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();

	UE_LOG(LogMRPICO, Log, TEXT("Settings->bEnableLightEstimation:%d"), Settings->bEnableLightEstimation)

	if (Settings->bEnableLightEstimation)
	{
		FSenseDataProviderCreateInfoLightEstimationPICO CreateInfoLightEstimation = {};
		EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;
		CreateInfoLightEstimation.Resolution=Settings->LightEstimationResolution;

		
		if (CreateProvider(CreateInfoLightEstimation, OutResult))
		{
			UE_LOG(LogMRPICO, Log, TEXT("OnBeginSession CreateLightEstimationProvider Success!!"));
		}
		else
		{
			UE_LOG(LogMRPICO, Error, TEXT("OnBeginSession CreateLightEstimationProvider failed OutResult:%d"),
			       OutResult);
		}
	}
}

bool FLightEstimationExtensionPICO::RequestLightEstimation(const FPICOPollFutureDelegate& Delegate,EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportLightEstimation)
	{
		XrFutureEXT Handle;
		XrSenseDataQueryInfoPICO cSenseDataQueryInfoBD = {};
		cSenseDataQueryInfoBD.type = XR_TYPE_SENSE_DATA_QUERY_INFO_PICO;

		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrQuerySenseDataAsyncPICO(
			ProviderHandle, &cSenseDataQueryInfoBD, &Handle);
		bResult = XR_SUCCEEDED(xrResult);

		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);

		if (bResult)
		{
			bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(Handle, Delegate);
		}
	}

	return bResult;
}

bool FLightEstimationExtensionPICO::GetLightEstimationInfo(const XrFutureEXT& FutureHandle,ULightEstimationTexturePICO* EnvironmentCaptureProbeTexture,float& SourceCubemapAngle,EResultPICO& OutResult)
{
	if (bSupportLightEstimation)
	{
		FSenseDataQueryCompletionPICO SenseDataQueryCompletion;
		if (!FSpatialSensingExtensionPICO::GetInstance()->QuerySenseDataComplete(
			ProviderHandle, FutureHandle, SenseDataQueryCompletion, OutResult))
		{
			return false;
		}

		if (PXR_FAILURE(SenseDataQueryCompletion.FutureResult))
		{
			OutResult = SenseDataQueryCompletion.FutureResult;
			return false;
		}

		if (SenseDataQueryCompletion.SnapShotHandle == XR_NULL_HANDLE)
		{
			OutResult = EResultPICO::XR_Error_HandleInvalid;
			return false;
		}

		FQueriedSenseDataPICO QueriedSenseData;
		if (!FSpatialSensingExtensionPICO::GetInstance()->GetQueriedSenseData(
			ProviderHandle, SenseDataQueryCompletion.SnapShotHandle, QueriedSenseData, OutResult))
		{
			return false;
		}

		UE_LOG(LogMRPICO, Error, TEXT("QueriedSenseData.QueriedSpatialEntityInfos:%d"), QueriedSenseData.QueriedSpatialEntityInfos.Num());


		
		for (auto EntityInfo : QueriedSenseData.QueriedSpatialEntityInfos)
		{
			TArray<ESpatialEntityComponentTypePICO> ComponentTypes;

			if (!FSpatialSensingExtensionPICO::GetInstance()->EnumerateSpatialEntityComponentTypes(
				SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, ComponentTypes, OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("EnumerateSpatialEntityComponentTypes Failed OutResult:%d"), OutResult);
				continue;
			}

			UE_LOG(LogMRPICO, Error, TEXT("ComponentTypes:%d"), ComponentTypes.Num());
			
			for (ESpatialEntityComponentTypePICO ComponentType : ComponentTypes)
			{
				switch (ComponentType)
				{
				case ESpatialEntityComponentTypePICO::Location:
					{
						FTransform cFTransform;
						if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityLocation(
							SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFTransform, OutResult))
						{
							UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityLocation Failed OutResult:%d"), OutResult);
							continue;
						}
						//Todo:Opposite to the direction of the UE, I don't know what the reason is, add a symbol first
						SourceCubemapAngle= -cFTransform.GetRotation().Rotator().Yaw;
						UE_LOG(LogMRPICO, Verbose, TEXT("GetSpatialEntityLocation:%s"), *cFTransform.ToString());
						UE_LOG(LogMRPICO, Verbose, TEXT("SourceCubemapAngle:%f"), SourceCubemapAngle);
					}
					break;
				case ESpatialEntityComponentTypePICO::LightEstimation:
					{
						if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityLightEstimateData(
							SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity,LightEstimationSize,EnvironmentCaptureProbeTexture, OutResult))
						{
							UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityLightEstimateData Failed OutResult:%d"), OutResult);
							continue;
						}

						if (EnvironmentCaptureProbeTexture)
						{
							UE_LOG(LogTemp, Warning, TEXT("EnvironmentCaptureProbeTexture"));
						}
						
						UE_LOG(LogMRPICO, Verbose, TEXT("GetSpatialEntityLightEstimateData Success OutResult:%d"), OutResult);
					}
					break;
				default: ;
				}
			}
		}

		if (!FSpatialSensingExtensionPICO::GetInstance()->DestroySenseDataQueryResult(
			SenseDataQueryCompletion.SnapShotHandle, OutResult))
		{
			UE_LOG(LogMRPICO, Error, TEXT("DestroySenseDataQueryResult Failed!"));
			return false;
		}
		return true;
	}
	return false;
}

ESpatialLightEstimationResolutionPICO FLightEstimationExtensionPICO::GetCurrentLightEstimationResolution()
{
	return CurrentResolution;
}

FLightEstimationExtensionPICO* FLightEstimationExtensionPICO::GetInstance()
{
	return &FPICOOpenXRMRModule::Get().GetMRModuleExtension().GetLightEstimationPICO();
}

bool FLightEstimationExtensionPICO::CreateProvider(const FSenseDataProviderCreateInfoBasePICO& createInfo,EResultPICO& OutResult)
{
	check(createInfo.Type == EProviderTypePICO::Pico_Provider_Light_Estimate);
	bool bResult = false;
	if (bSupportLightEstimation)
	{
		const FSenseDataProviderCreateInfoLightEstimationPICO& LightEstimationInfo = static_cast<const FSenseDataProviderCreateInfoLightEstimationPICO&>(createInfo);
		
		Type = EProviderTypePICO::Pico_Provider_Light_Estimate;
		if (IsHandleValid())
		{
			UE_LOG(LogMRPICO, Error, TEXT("CreateProvider failed already create"));
			OutResult = EResultPICO::XR_Error_LimitReached;
			return false;
		}
		
		XrEnvironmentTextureCreateConfigInfoPICO xrEnvironmentTextureCreateConfigInfoPico = {
			.type = XR_TYPE_ENVIRONMENT_TEXTURE_CREATE_CONFIG_INFO_PICO,
			.next = nullptr, .pixelFormat = XR_ENVIRONMENT_TEXTURE_PIXEL_FORMAT_RGBA_16FLOAT_PICO,
			.resolution = static_cast<XrEnvironmentTextureResolutionPICO>(LightEstimationInfo.Resolution),
			.transferType = XR_ENVIRONMENT_TEXTURE_TRANSFER_TYPE_VULKAN_PICO
		};

		CurrentResolution = LightEstimationInfo.Resolution;

		switch (LightEstimationInfo.Resolution) {
		case ESpatialLightEstimationResolutionPICO::TextureResolution_8x8:
			LightEstimationSize=FIntPoint(8,8);
			break;
		case ESpatialLightEstimationResolutionPICO::TextureResolution_16x16:
			LightEstimationSize=FIntPoint(16,16);
			break;
		case ESpatialLightEstimationResolutionPICO::TextureResolution_32x32:
			LightEstimationSize=FIntPoint(32,32);
			break;
		case ESpatialLightEstimationResolutionPICO::TextureResolution_64x64:
			LightEstimationSize=FIntPoint(64,64);
			break;
		case ESpatialLightEstimationResolutionPICO::TextureResolution_128x128:
			LightEstimationSize=FIntPoint(128,128);
			break;
		}

		UE_LOG(LogMRPICO, Log, TEXT("CreateProvider with resolution:%s"), *LightEstimationSize.ToString());

		XrSenseDataProviderCreateInfoLightEstimationPICO LightEstimationProviderCreateInfoPico = {};
		LightEstimationProviderCreateInfoPico.type = XR_TYPE_SENSE_DATA_PROVIDER_CREATE_INFO_LIGHT_ESTIMATION_PICO;
		LightEstimationProviderCreateInfoPico.createFlags = XR_LIGHT_ESTIMATION_CREATE_ENVIRONMENT_TEXTURE_BIT_PICO;
		LightEstimationProviderCreateInfoPico.next = &xrEnvironmentTextureCreateConfigInfoPico;

		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrCreateSenseDataProviderPICO(
			Session,
			reinterpret_cast<XrSenseDataProviderCreateInfoBaseHeaderPICO*>(&LightEstimationProviderCreateInfoPico),
			&ProviderHandle);
		bResult = XR_SUCCEEDED(xrResult);
		ProviderHandle = bResult ? ProviderHandle : XR_NULL_HANDLE;

		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}

	return bResult;
}
