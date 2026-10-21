// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialUSDModule.h"
#include "PICOSpatialShaderGraphUsdConvertor.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformAtomics.h"
#include "PICOSpatialUsdHeaders.h"
#include "UObject/GCObjectScopeGuard.h"

DEFINE_LOG_CATEGORY(LogPICOSpatialUSD);

void FPICOSpatialUSDModule::StartupModule()
{
	// Module startup logic can be added here if needed
}

void FPICOSpatialUSDModule::ShutdownModule()
{
	// Module shutdown logic can be added here if needed
}

bool FPICOSpatialUSDModule::ConvertMaterialToUsd(const UMaterialInterface* Material, const FString& OutputUSDPath)
{
#if USE_USD_SDK
    static int32 GUsdMtlxMaterialSequence = 0;
    const int32 MaterialSequence = FPlatformAtomics::InterlockedIncrement(&GUsdMtlxMaterialSequence);

    FGCObjectScopeGuard MaterialGuard(Material);
    pxr::UsdStageRefPtr Stage = pxr::UsdStage::CreateNew(TCHAR_TO_UTF8(*OutputUSDPath));
    if (!Stage)
    {
        const FString MaterialPath = Material ? Material->GetPathName() : FString(TEXT("<null>"));
        UE_LOG(LogPICOSpatialUSD, Warning, TEXT("[USD_MTLX][#%04d][MAT=%s][FAIL][CreateStage] Failed to create USD stage: %s"),
            MaterialSequence, *MaterialPath, *OutputUSDPath);
        return false;
    }

    FShaderGraphUsdConvertorPICO Convertor(Material, Stage);
    Convertor.SetLogContext(MaterialSequence, OutputUSDPath);
    bool bConvertSuccess = Convertor.Convert();
    
    if (!bConvertSuccess)
    {
        Stage = nullptr;

        IFileManager& FileManager = IFileManager::Get();
        if (FileManager.FileExists(*OutputUSDPath))
        {
            FileManager.Delete(*OutputUSDPath, false, true);
        }

        return false;
    }

    if (!Stage->GetRootLayer()->Save())
    {
        const FString MaterialPath = Material ? Material->GetPathName() : FString(TEXT("<null>"));
        UE_LOG(LogPICOSpatialUSD, Warning, TEXT("[USD_MTLX][#%04d][MAT=%s][FAIL][Save] Failed to save USD: %s"),
            MaterialSequence, *MaterialPath, *OutputUSDPath);
        Stage = nullptr;

        IFileManager& FileManager = IFileManager::Get();
        if (FileManager.FileExists(*OutputUSDPath))
        {
            FileManager.Delete(*OutputUSDPath, false, true);
        }

        return false;
    }
    return true;
#else
    return false;
#endif
}

bool FPICOSpatialUSDModule::CopyAndModifyUsdMaterial(const FString& SourceParentUSDPath, const FString& TargetUSDPath, const UMaterialInterface* TargetMaterial)
{
#if USE_USD_SDK
    IFileManager& FileManager = IFileManager::Get();
    if (!FileManager.FileExists(*SourceParentUSDPath))
    {
        UE_LOG(LogPICOSpatialUSD, Error, TEXT("Source USD file does not exist: %s"), *SourceParentUSDPath);
        return false;
    }

    pxr::UsdStageRefPtr SourceStage = pxr::UsdStage::Open(TCHAR_TO_UTF8(*SourceParentUSDPath));
    if (!SourceStage)
    {
        UE_LOG(LogPICOSpatialUSD, Error, TEXT("Failed to open source USD file: %s"), *SourceParentUSDPath);
        return false;
    }

    if (!SourceStage->Export(TCHAR_TO_UTF8(*TargetUSDPath)))
    {
        UE_LOG(LogPICOSpatialUSD, Error, TEXT("Failed to export USD to: %s"), *TargetUSDPath);
        return false;
    }

    pxr::UsdStageRefPtr TargetStage = pxr::UsdStage::Open(TCHAR_TO_UTF8(*TargetUSDPath));
    if (!TargetStage)
    {
        UE_LOG(LogPICOSpatialUSD, Error, TEXT("Failed to open target USD file: %s"), *TargetUSDPath);
        return false;
    }

    TargetStage->GetRootLayer()->Save();
    return true;
#else
    return false;
#endif
}

bool FPICOSpatialUSDModule::CreateMaterialInstance(const FString& ParentUSDPath, const FString& InstanceUSDPath, const UMaterialInterface* Material)
{
#if USE_USD_SDK
    FGCObjectScopeGuard MaterialGuard(Material);
    pxr::UsdStageRefPtr parentStage = pxr::UsdStage::Open(TCHAR_TO_UTF8(*ParentUSDPath));
    if (!parentStage)
    {
        UE_LOG(LogPICOSpatialUSD, Error, TEXT("Failed to open parent USD: %s"), *ParentUSDPath);
        return false;
    }

    pxr::UsdStageRefPtr instanceStage = pxr::UsdStage::CreateNew(TCHAR_TO_UTF8(*InstanceUSDPath));
    if (!instanceStage)
    {
        return false;
    }

    const FString ParentFullPath = FPaths::ConvertRelativePathToFull(ParentUSDPath);
    const FString InstanceFullPath = FPaths::ConvertRelativePathToFull(InstanceUSDPath);

    FString ParentDir = FPaths::CreateStandardFilename(FPaths::GetPath(ParentFullPath));
    FString InstanceDir = FPaths::CreateStandardFilename(FPaths::GetPath(InstanceFullPath));

    FString RelativePath;
    if (ParentDir.Equals(InstanceDir, ESearchCase::IgnoreCase))
    {
        RelativePath = FPaths::GetCleanFilename(ParentFullPath);
    }
    else
    {
        RelativePath = FPaths::CreateStandardFilename(ParentFullPath);
        FString InstanceBaseDir = InstanceDir;
        if (!InstanceBaseDir.EndsWith(TEXT("/")))
        {
            InstanceBaseDir += TEXT("/");
        }
        if (!FPaths::MakePathRelativeTo(RelativePath, *InstanceBaseDir))
        {
            RelativePath = FPaths::CreateStandardFilename(ParentFullPath);
        }
    }

    instanceStage->GetRootLayer()->GetSubLayerPaths().push_back(TCHAR_TO_UTF8(*RelativePath));

    // Extract material parameters if material is provided
    TMap<FMaterialParameterInfo, FMaterialParameterMetadata> ScalarParameters;
    TMap<FMaterialParameterInfo, FMaterialParameterMetadata> VectorParameters;
    TMap<FMaterialParameterInfo, FMaterialParameterMetadata> TextureParameters;

    if (Material)
    {
        if (const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material))
        {
            MaterialInstance->GetAllParametersOfType(EMaterialParameterType::Scalar, ScalarParameters);
            MaterialInstance->GetAllParametersOfType(EMaterialParameterType::Vector, VectorParameters);
            MaterialInstance->GetAllParametersOfType(EMaterialParameterType::Texture, TextureParameters);
        }
    }

    pxr::UsdPrimRange primRange = parentStage->Traverse();
    for (const pxr::UsdPrim& prim : primRange)
    {
        if (prim.IsA<pxr::UsdShadeMaterial>())
        {
            pxr::UsdPrim overridePrim = instanceStage->OverridePrim(prim.GetPath());
            pxr::UsdShadeMaterial overrideMaterial(overridePrim);

            // Override scalar parameters
            for (const auto& Pair : ScalarParameters)
            {
                const FMaterialParameterInfo& ParameterInfo = Pair.Key;
                const FMaterialParameterMetadata& ParameterMetadata = Pair.Value;
                FString ParameterName = ParameterInfo.Name.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_S");
                float ParameterValue = ParameterMetadata.Value.AsScalar();

                pxr::UsdShadeInput scalarInput = overrideMaterial.GetInput(pxr::TfToken(TCHAR_TO_UTF8(*ParameterName)));
                if (!scalarInput)
                {
                    scalarInput = overrideMaterial.CreateInput(pxr::TfToken(TCHAR_TO_UTF8(*ParameterName)), pxr::SdfValueTypeNames->Float);
                }
                scalarInput.Set(ParameterValue);
            }

            // Override vector parameters
            for (const auto& Pair : VectorParameters)
            {
                const FMaterialParameterInfo& ParameterInfo = Pair.Key;
                const FMaterialParameterMetadata& ParameterMetadata = Pair.Value;
                FString ParameterName = ParameterInfo.Name.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_V");
                FLinearColor ParameterValue = ParameterMetadata.Value.AsLinearColor();

                pxr::UsdShadeInput vectorInput = overrideMaterial.GetInput(pxr::TfToken(TCHAR_TO_UTF8(*ParameterName)));
                if (!vectorInput)
                {
                    vectorInput = overrideMaterial.CreateInput(pxr::TfToken(TCHAR_TO_UTF8(*ParameterName)), pxr::SdfValueTypeNames->Color4f);
                }
                vectorInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
                vectorInput.Set(pxr::GfVec4f(ParameterValue.R, ParameterValue.G, ParameterValue.B, ParameterValue.A));
            }

            // Override texture parameters
            for (const auto& Pair : TextureParameters)
            {
                const FMaterialParameterInfo& ParameterInfo = Pair.Key;
                const FMaterialParameterMetadata& ParameterMetadata = Pair.Value;
                FString ParameterName = ParameterInfo.Name.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_T");
                UObject* TextureObject = ParameterMetadata.Value.AsTextureObject();

                if (TextureObject)
                {
                    UTexture* Texture = Cast<UTexture>(TextureObject);
                    if (Texture)
                    {
                        FString TexturePath = FShaderGraphUsdConvertorPICO::GetTexturePathFromTexture(Texture, instanceStage);
                        if (!TexturePath.IsEmpty())
                        {
                            pxr::UsdShadeInput textureInput = overrideMaterial.GetInput(pxr::TfToken(TCHAR_TO_UTF8(*ParameterName)));
                            if (!textureInput)
                            {
                                textureInput = overrideMaterial.CreateInput(pxr::TfToken(TCHAR_TO_UTF8(*ParameterName)), pxr::SdfValueTypeNames->Asset);
                            }
                            textureInput.GetAttr().SetColorSpace(pxr::TfToken("srgb_texture"));
                            textureInput.Set(pxr::SdfAssetPath(TCHAR_TO_UTF8(*TexturePath)));
                        }
                    }
                }
            }
        }
    }

    instanceStage->GetRootLayer()->Save();
    return true;
#else
    return false;
#endif
}

IMPLEMENT_MODULE(FPICOSpatialUSDModule, PICOSpatialUSD)

