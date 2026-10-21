// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialMeshEntity.h"
#include "PICOSpatial.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/Package.h"

TMap<FString, TWeakObjectPtr<UMaterialInterface>> FMeshEntityProxy::WeakDynamicMaterialInstances;

void FMeshEntityProxy::ConstructMeshEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (!bVisible)
	{
		return;
	}

	const UMeshComponent* MeshComponent = Cast<UMeshComponent>(SceneComponent);
	if (!MeshComponent)
	{
		return;
	}

	const TArray<UMaterialInterface*>& NewMaterials = MeshComponent->GetMaterials();
	const int32 MaterialNum = NewMaterials.Num();

	bMaterialChanged = false;
	MaterialParameters.Reset(MaterialNum);

	if (NewMaterials.Num() != Materials.Num())
	{
		Materials.Reset(MaterialNum);
		DynamicMaterialInstances.Reset(MaterialNum);
		ShaderGraphFlags.Reset(MaterialNum);

		for (int32 i = 0; i < MaterialNum; i++)
		{
			ProcessMaterialAtIndex(i, NewMaterials[i], true);
		}
		bMaterialChanged = true;
	}
	else
	{
		for (int32 i = 0; i < MaterialNum; i++)
		{
			ProcessMaterialAtIndex(i, NewMaterials[i], false);
		}
	}
}

FMeshEntityProxy::FMeshEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	:FPrimitiveEntityProxy(InParentEntityProxy, SceneComponent)
	, Materials({})
{
	ConstructMeshEntityData(SceneComponent, true);
}

FMeshEntityProxy::~FMeshEntityProxy()
{
	for (const FString& DynamicMaterialInstance : DynamicMaterialInstances)
	{
		if (!DynamicMaterialInstance.IsEmpty() && FPICOSpatial::Get().RemoveDynamicMaterialInstance(DynamicMaterialInstance))
		{
			WeakDynamicMaterialInstances.Remove(DynamicMaterialInstance);
		}
	}

	for (auto It = WeakDynamicMaterialInstances.CreateIterator(); It; ++It)
	{
		if (!It.Value().IsValid())
		{
			const FString& MaterialPath = It.Key();
			UE_LOG(LogPICOSpatial, Log, TEXT("Removing invalid dynamic material instance: %s"), *MaterialPath);

			FPICOSpatial::Get().RemoveDynamicMaterialInstance(MaterialPath, true);

			It.RemoveCurrent();
		}
	}
}

void FMeshEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FPrimitiveEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstructMeshEntityData(SceneComponent, false);
}

bool FMeshEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FPrimitiveEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.Get()->IsValid())
		{
			return true;
		}
	}
	return false;
}

void FMeshEntityProxy::UpdateNativeEntity()
{
	FPrimitiveEntityProxy::UpdateNativeEntity();
#if PLATFORM_ANDROID
	spatialpack::SceneEntity entity = NativeEntity.Get()->entity;
	if (!entity.isValid())
	{
		return;
	}

	if (bMaterialChanged)
	{
		for (int32 i = 0; i < Materials.Num(); i++)
		{
			if (Materials[i] != "")
			{
				spatialpack::Material* NativeMaterial = FPICOSpatial::Get().CacheMaterialResource(Materials[i], DynamicMaterialInstances[i]);
				if (NativeMaterial && NativeMaterial->isValid())
				{
					UE_LOG(LogPICOSpatial, Verbose, TEXT("Set MeshComponent:%s, Material[%d]:%s, NativeMaterial:%llu"), *Name, i, *Materials[i], NativeMaterial->getResourceId());
					entity.getOrAddModelComponent().setMaterial(*NativeMaterial, i);
				}
				else
				{
					entity.getOrAddModelComponent().setMaterial(spatialpack::Material(), i);
				}
			}
			else if (entity.hasComponent(spatialpack::ModelComponent::getType()))
			{
				entity.getOrAddModelComponent().setMaterial(spatialpack::Material(), i);
			}
		}
		bMaterialChanged = false;
	}

	if (bVisible != entity.getOrAddModelComponent().isVisible())
	{
		entity.getOrAddModelComponent().setVisible(bVisible);
	}


	if (bVisible)
	{
		for (int32 i = 0; i < Materials.Num(); i++)
		{
			if (ShaderGraphFlags[i])
			{
				UpdateNativeMaterialParameters(i, entity);
			}
		}
	}
#endif // PLATFORM_ANDROID
}

void FMeshEntityProxy::UpdateMaterialStoredParameters(uint32 Index, UMaterialInterface* Material, bool bIsStaticMaterialInstance, bool bIsDynamicMaterialInstance, bool UpdateStaticOnce)
{
	if (ShaderGraphFlags[Index] && ((bIsStaticMaterialInstance && UpdateStaticOnce) || bIsDynamicMaterialInstance))
	{
		UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material);
		FMaterialInputParameters& MaterialInputParameters = MaterialParameters.Add_GetRef(FMaterialInputParameters());
		MaterialInstance->GetAllParametersOfType(EMaterialParameterType::Scalar, MaterialInputParameters.ScalarParameters);
		MaterialInstance->GetAllParametersOfType(EMaterialParameterType::Vector, MaterialInputParameters.ColorParameters);
		MaterialInstance->GetAllParametersOfType(EMaterialParameterType::Texture, MaterialInputParameters.TextureParameters);
	}
	else
	{
		MaterialParameters.Add(FMaterialInputParameters());
	}
}

void FMeshEntityProxy::UpdateNativeMaterialParameters(int MaterialIndex, spatialpack::SceneEntity& entity)
{
#if PLATFORM_ANDROID
	spatialpack::Material NativeMaterial = entity.getOrAddModelComponent().getMaterial(MaterialIndex);

	if (!NativeMaterial.isValid())
	{
		return;
	}

	spatialpack::ShaderGraphMaterial ShaderGraphMaterial = spatialpack::ShaderGraphMaterial::castFromMaterial(NativeMaterial);
	if (!ShaderGraphMaterial.isValid())
	{
		ShaderGraphFlags[MaterialIndex] = false;
		return;
	}

	for (const auto& Pair : MaterialParameters[MaterialIndex].ScalarParameters)
	{
		const FMaterialParameterInfo& ParameterInfo = Pair.Key;
		const FMaterialParameterMetadata& ParameterMetadata = Pair.Value;
		// Add _S suffix to distinguish from Vector and Texture parameters
		FString ParameterName = ParameterInfo.Name.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_S");
		float ParameterValue = ParameterMetadata.Value.AsScalar();
		ShaderGraphMaterial.setParameter(TCHAR_TO_UTF8(*ParameterName), ParameterValue);
	}

	for (const auto& Pair : MaterialParameters[MaterialIndex].ColorParameters)
	{
		const FMaterialParameterInfo& ParameterInfo = Pair.Key;
		const FMaterialParameterMetadata& ParameterMetadata = Pair.Value;
		// Add _V suffix to distinguish from Scalar and Texture parameters
		FString ParameterName = ParameterInfo.Name.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_V");
		FLinearColor ParameterValue = ParameterMetadata.Value.AsLinearColor();
		ShaderGraphMaterial.setParameter(TCHAR_TO_UTF8(*ParameterName), ToSpColor(ParameterValue));
	}

	for (const auto& Pair : MaterialParameters[MaterialIndex].TextureParameters)
	{
		const FMaterialParameterInfo& ParameterInfo = Pair.Key;
		const FMaterialParameterMetadata& ParameterMetadata = Pair.Value;
		// Add _T suffix to distinguish from Scalar and Vector parameters
		FString ParameterName = ParameterInfo.Name.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_T");
		UObject* TextureObject = ParameterMetadata.Value.AsTextureObject();
		if (TextureObject)
		{
			UTexture2D* Texture2D = Cast<UTexture2D>(TextureObject);
			if (Texture2D)
			{
				spatialpack::TextureResource* TextureResource = FPICOSpatial::Get().CacheTextureResource(Texture2D->GetPackage()->GetPathName());
				if (TextureResource && TextureResource->isValid())
				{
					ShaderGraphMaterial.setParameter(TCHAR_TO_UTF8(*ParameterName), spatialpack::MaterialTextureParameter{ *TextureResource });
				}
			}
		}
	}
#endif // PLATFORM_ANDROID
}

void FMeshEntityProxy::ProcessMaterialAtIndex(int32 Index, UMaterialInterface* SourceMaterial, bool bForceUpdate)
{

	UMaterialInterface* BaseMaterial = nullptr;
	UMaterialInterface* DynamicMaterialInstance = nullptr;
	bool bIsDynamicMaterialInstance = false;

	if (UMaterialInstanceDynamic* MaterialInstanceDynamic = Cast<UMaterialInstanceDynamic>(SourceMaterial))
	{
		BaseMaterial = MaterialInstanceDynamic->Parent;
		DynamicMaterialInstance = MaterialInstanceDynamic;
		bIsDynamicMaterialInstance = true;
	}
	else
	{
		BaseMaterial = SourceMaterial;
	}

	const bool bIsStaticMaterialInstance = BaseMaterial && BaseMaterial->IsA<UMaterialInstanceConstant>();

	FString MaterialNamePath = "";
	if (BaseMaterial)
	{
		UPackage* Package = BaseMaterial->GetPackage();
		if (Package)
		{
			MaterialNamePath = Package->GetPathName();
		}
	}

	FString DynamicMaterialInstanceNamePath = "";
	if (bIsDynamicMaterialInstance && DynamicMaterialInstance)
	{
		DynamicMaterialInstanceNamePath = DynamicMaterialInstance->GetPathName();
		WeakDynamicMaterialInstances.Add(DynamicMaterialInstanceNamePath, DynamicMaterialInstance);
	}

	bool bCurrentMaterialChanged = bForceUpdate;

	if (!bForceUpdate)
	{
		if (MaterialNamePath != Materials[Index] ||
			DynamicMaterialInstanceNamePath != DynamicMaterialInstances[Index])
		{
			if (!DynamicMaterialInstances[Index].IsEmpty() && DynamicMaterialInstances[Index] != DynamicMaterialInstanceNamePath)
			{
				if (FPICOSpatial::Get().RemoveDynamicMaterialInstance(DynamicMaterialInstances[Index]))
				{
					WeakDynamicMaterialInstances.Remove(DynamicMaterialInstances[Index]);
				}
			}
			if (!DynamicMaterialInstanceNamePath.IsEmpty() && DynamicMaterialInstanceNamePath != DynamicMaterialInstances[Index])
			{
				FPICOSpatial::Get().AddDynamicMaterialInstanceReference(DynamicMaterialInstanceNamePath);
			}
			bCurrentMaterialChanged = true;
			bMaterialChanged = true;
		}
	}

	if (bForceUpdate)
	{
		Materials.Add(MaterialNamePath);
		DynamicMaterialInstances.Add(DynamicMaterialInstanceNamePath);
		if (!DynamicMaterialInstanceNamePath.IsEmpty())
		{
			FPICOSpatial::Get().AddDynamicMaterialInstanceReference(DynamicMaterialInstanceNamePath);
		}
		ShaderGraphFlags.Add(true);
	}
	else
	{
		Materials[Index] = MaterialNamePath;
		DynamicMaterialInstances[Index] = DynamicMaterialInstanceNamePath;

		if (bCurrentMaterialChanged)
		{
			ShaderGraphFlags[Index] = true;
		}
	}

	UpdateMaterialStoredParameters(Index, SourceMaterial, bIsStaticMaterialInstance, bIsDynamicMaterialInstance, bCurrentMaterialChanged);
}
