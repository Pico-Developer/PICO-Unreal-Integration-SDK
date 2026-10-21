#include "ECS/PICOSpatialECSModelComponent.h"

#include "ECS/PICOSpatialECSMaterialResourceAsset.h"
#include "ECS/PICOSpatialECSMeshResourceAsset.h"
#include "Engine/Texture2D.h"
#include "PICOSpatial.h"
#include "UObject/Package.h"

#if PLATFORM_ANDROID
#include "PICOSpatial.h"
#include "PICOSpatialUtility.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"

TMap<FString, TWeakObjectPtr<UMaterialInterface>> UPICOSpatialECSModelComponent::WeakDynamicMaterialInstances;
#endif

void UPICOSpatialECSModelComponent::BeginDestroy()
{
#if PLATFORM_ANDROID
	for (const FString& DynamicMaterialInstance : DynamicMaterialInstances)
	{
		if (!DynamicMaterialInstance.IsEmpty() && FPICOSpatial::Get().RemoveDynamicMaterialInstance(DynamicMaterialInstance))
		{
			WeakDynamicMaterialInstances.Remove(DynamicMaterialInstance);
		}
	}
#endif
	Super::BeginDestroy();
}

void UPICOSpatialECSModelComponent::setMeshResource(UPICOSpatialECSMeshResourceAsset* InMeshResource)
{
	MeshResource = InMeshResource;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		spatialpack::ModelComponent ModelComponent = GetCachedNativeEntity().getOrAddModelComponent();
		if (UPICOSpatialECSMeshResourceAsset* ResourcePtr = MeshResource.Get())
		{
			const spatialpack::MeshResource& NativeMesh = ResourcePtr->GetOrCreateNativeMeshResource(false);
			if (NativeMesh.isValid())
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS][Model] Set MeshResource NativeMesh: %llu"), NativeMesh.getResourceId());
				ModelComponent.setMesh(NativeMesh);
				return;
			}
		}
		ModelComponent.setMesh(spatialpack::MeshResource());
	}
#endif
}

void UPICOSpatialECSModelComponent::setMaterialResource(UPICOSpatialECSMaterialResourceAsset* InMaterial, int32 MaterialIndex)
{
        if (MaterialIndex < 0)
        {
                UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Model] Ignore setMaterialResource with invalid MaterialIndex=%d"), MaterialIndex);
                return;
        }

        if (MaterialIndex >= Materials.Num())
        {
                Materials.SetNum(MaterialIndex + 1);
#if PLATFORM_ANDROID
                MaterialPaths.SetNum(MaterialIndex + 1);
                DynamicMaterialInstances.SetNum(MaterialIndex + 1);
                ShaderGraphFlags.SetNum(MaterialIndex + 1);
                MaterialParameters.SetNum(MaterialIndex + 1);
#endif
        }
        Materials[MaterialIndex] = InMaterial;
#if PLATFORM_ANDROID
        auto RemoveCachedDynamicMaterialInstance = [&]()
        {
                if (MaterialIndex < DynamicMaterialInstances.Num() && !DynamicMaterialInstances[MaterialIndex].IsEmpty())
		{
                        if (FPICOSpatial::Get().RemoveDynamicMaterialInstance(DynamicMaterialInstances[MaterialIndex]))
			{
                                WeakDynamicMaterialInstances.Remove(DynamicMaterialInstances[MaterialIndex]);
			}
                        DynamicMaterialInstances[MaterialIndex] = FString();
                }
        };

        auto ResetMaterialSlotState = [&]()
        {
                RemoveCachedDynamicMaterialInstance();
                if (MaterialIndex < MaterialPaths.Num())
		{
                        MaterialPaths[MaterialIndex] = FString();
		}
                if (MaterialIndex < DynamicMaterialInstances.Num())
		{
                        DynamicMaterialInstances[MaterialIndex] = FString();
		}
                if (MaterialIndex < ShaderGraphFlags.Num())
                {
                        ShaderGraphFlags[MaterialIndex] = false;
                }
                if (MaterialIndex < MaterialParameters.Num())
                {
                        MaterialParameters[MaterialIndex] = FMaterialInputParameters();
                }
        };

        if (UPICOSpatialECSMaterialResourceAsset* MaterialAsset = InMaterial)
        {
                if (MaterialAsset->SourceType == EPICOSpatialECSMaterialSourceType::FromUEMaterial)
                {
                        if (UMaterialInterface* UEMaterial = MaterialAsset->getUEMaterial())
                        {
                                ProcessMaterialAtIndex(MaterialIndex, UEMaterial, true);
                        }
                        else
                        {
                                ResetMaterialSlotState();
                        }
                }
                else
                {
                        ResetMaterialSlotState();
                }
        }
        else
        {
                ResetMaterialSlotState();
        }
#endif
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		spatialpack::ModelComponent ModelComponent = GetCachedNativeEntity().getOrAddModelComponent();
                if (UPICOSpatialECSMaterialResourceAsset* MaterialAsset = InMaterial)
		{
			if (MaterialAsset->SourceType == EPICOSpatialECSMaterialSourceType::FromUEMaterial)
			{
				if (MaterialIndex >= 0 && MaterialIndex < MaterialPaths.Num())
				{
					spatialpack::Material* NativeMaterial = FPICOSpatial::Get().CacheMaterialResource(MaterialPaths[MaterialIndex], DynamicMaterialInstances[MaterialIndex]);
					if (NativeMaterial && NativeMaterial->isValid())
					{
						UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS][Model] Set Material[%d]: %s, NativeMaterial: %llu"), MaterialIndex, *MaterialPaths[MaterialIndex], NativeMaterial->getResourceId());
						ModelComponent.setMaterial(*NativeMaterial, MaterialIndex);
					}
					else
					{
						ModelComponent.setMaterial(spatialpack::Material(), MaterialIndex);
					}
				}
                                else
                                {
                                        ModelComponent.setMaterial(spatialpack::Material(), MaterialIndex);
                                }
			}
			else
			{
				const spatialpack::Material& NativeMaterial = MaterialAsset->GetOrCreateNativeMaterial(false);
				if (NativeMaterial.isValid())
				{
					UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS][Model] Set Material[%d] NativeMaterial: %llu"), MaterialIndex, NativeMaterial.getResourceId());
					ModelComponent.setMaterial(NativeMaterial, MaterialIndex);
				}
				else
				{
					ModelComponent.setMaterial(spatialpack::Material(), MaterialIndex);
				}
			}
		}
		else
		{
			ModelComponent.setMaterial(spatialpack::Material(), MaterialIndex);
		}
	}
#endif
}

void UPICOSpatialECSModelComponent::clearMaterials()
{
#if PLATFORM_ANDROID
	for (const FString& DynamicMaterialInstance : DynamicMaterialInstances)
	{
		if (!DynamicMaterialInstance.IsEmpty())
		{
			if (FPICOSpatial::Get().RemoveDynamicMaterialInstance(DynamicMaterialInstance))
			{
				WeakDynamicMaterialInstances.Remove(DynamicMaterialInstance);
			}
		}
	}
#endif
	Materials.Empty();
#if PLATFORM_ANDROID
	MaterialPaths.Empty();
	DynamicMaterialInstances.Empty();
	ShaderGraphFlags.Empty();
	MaterialParameters.Empty();
	if (HasNativeEntity())
	{
		spatialpack::ModelComponent ModelComponent = GetCachedNativeEntity().getOrAddModelComponent();
		int32 Count = ModelComponent.getMaterialCount();
		for (int32 i = 0; i < Count; i++)
		{
			ModelComponent.setMaterial(spatialpack::Material(), i);
		}
	}
#endif
}

int32 UPICOSpatialECSModelComponent::getMaterialCount() const
{
	return Materials.Num();
}

void UPICOSpatialECSModelComponent::setAutoInstancingEnabled(bool bEnabled)
{
	bAutoInstancingEnabled = bEnabled;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddModelComponent().setAutoInstancingEnabled(bEnabled);
	}
#endif
}

bool UPICOSpatialECSModelComponent::getAutoInstancingEnabled() const
{
	return bAutoInstancingEnabled;
}

void UPICOSpatialECSModelComponent::setVisible(bool bInVisible)
{
	bVisible = bInVisible;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddModelComponent().setVisible(bVisible);
	}
#endif
}

bool UPICOSpatialECSModelComponent::isVisible() const
{
	return bVisible;
}

void UPICOSpatialECSModelComponent::setVisualBoundsMargin(FVector InBoundsMargin)
{
	VisualBoundsMargin = InBoundsMargin;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		spatialpack::Vector3f SpMargin = ToSpVector(VisualBoundsMargin, 100.0f);
		GetCachedNativeEntity().getOrAddModelComponent().setVisualBoundsMargin(SpMargin);
	}
#endif
}

FVector UPICOSpatialECSModelComponent::getVisualBoundsMargin() const
{
	return VisualBoundsMargin;
}

UPICOSpatialECSMeshResourceAsset* UPICOSpatialECSModelComponent::getMeshResource() const
{
	return MeshResource.Get();
}

UPICOSpatialECSMaterialResourceAsset* UPICOSpatialECSModelComponent::getMaterialResource(int32 MaterialIndex) const
{
	if (MaterialIndex >= 0 && MaterialIndex < Materials.Num())
	{
		return Materials[MaterialIndex].Get();
	}
	return nullptr;
}

void UPICOSpatialECSModelComponent::UpdateMaterialParameters()
{
#if PLATFORM_ANDROID
	if (!HasNativeEntity())
	{
		return;
	}

	spatialpack::SceneEntity entity = GetCachedNativeEntity();
	if (!entity.isValid())
	{
		return;
	}

	for (int32 i = 0; i < Materials.Num(); i++)
	{
		UpdateMaterialParameter(i);
	}
#endif
}

void UPICOSpatialECSModelComponent::UpdateMaterialParameter(int32 MaterialIndex)
{
#if PLATFORM_ANDROID
	if (!HasNativeEntity())
	{
		return;
	}

	spatialpack::SceneEntity entity = GetCachedNativeEntity();
	if (!entity.isValid())
	{
		return;
	}

	if (MaterialIndex < 0 || MaterialIndex >= Materials.Num())
	{
		return;
	}

        if (UPICOSpatialECSMaterialResourceAsset* MaterialAsset = Materials[MaterialIndex].Get())
	{
		if (MaterialAsset->SourceType == EPICOSpatialECSMaterialSourceType::FromUEMaterial)
		{
			if (UMaterialInterface* MaterialPtr = MaterialAsset->getUEMaterial())
			{
				ProcessMaterialAtIndex(MaterialIndex, MaterialPtr, false);
			}
		}
	}

	if (MaterialIndex < ShaderGraphFlags.Num() && ShaderGraphFlags[MaterialIndex])
	{
		UpdateNativeMaterialParameters(MaterialIndex, entity);
	}
#endif
}

#if PLATFORM_ANDROID
void UPICOSpatialECSModelComponent::OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity)
{
	UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS][Model] OnInitializeNativeComponent UUID=%llu"), (uint64)InNativeEntity.getEntityUUID());
	InNativeEntity.getOrAddModelComponent();
}

void UPICOSpatialECSModelComponent::ApplyAllPropertiesToNative()
{
	if (!HasNativeEntity())
	{
		return;
	}

	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS][Model] ApplyAllPropertiesToNative UUID=%llu"),
		(uint64)GetCachedNativeEntity().getEntityUUID());

	spatialpack::ModelComponent ModelComponent = GetCachedNativeEntity().getOrAddModelComponent();

	if (UPICOSpatialECSMeshResourceAsset* ResourcePtr = MeshResource.Get())
	{
		const spatialpack::MeshResource& NativeMesh = ResourcePtr->GetOrCreateNativeMeshResource(false);
		if (NativeMesh.isValid())
		{
			UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS][Model] Apply MeshResource NativeMesh: %llu"), NativeMesh.getResourceId());
			ModelComponent.setMesh(NativeMesh);
		}
		else
		{
			ModelComponent.setMesh(spatialpack::MeshResource());
		}
	}
	else
	{
		ModelComponent.setMesh(spatialpack::MeshResource());
	}

	MaterialPaths.SetNum(Materials.Num());
	DynamicMaterialInstances.SetNum(Materials.Num());
	ShaderGraphFlags.SetNum(Materials.Num());
	MaterialParameters.SetNum(Materials.Num());

	for (int32 i = 0; i < Materials.Num(); i++)
	{
                if (UPICOSpatialECSMaterialResourceAsset* MaterialAsset = Materials[i].Get())
		{
			if (MaterialAsset->SourceType == EPICOSpatialECSMaterialSourceType::FromUEMaterial)
			{
				if (UMaterialInterface* MaterialPtr = MaterialAsset->getUEMaterial())
				{
					ProcessMaterialAtIndex(i, MaterialPtr, true);
					spatialpack::Material* NativeMaterial = FPICOSpatial::Get().CacheMaterialResource(MaterialPaths[i], DynamicMaterialInstances[i]);
					if (NativeMaterial && NativeMaterial->isValid())
					{
						UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS][Model] Apply Material[%d]: %s, NativeMaterial: %llu"), i, *MaterialPaths[i], NativeMaterial->getResourceId());
						ModelComponent.setMaterial(*NativeMaterial, i);
					}
					else
					{
						ModelComponent.setMaterial(spatialpack::Material(), i);
					}
				}
				else
				{
					MaterialPaths[i] = FString();
					DynamicMaterialInstances[i] = FString();
					ShaderGraphFlags[i] = false;
					MaterialParameters[i] = FMaterialInputParameters();
					ModelComponent.setMaterial(spatialpack::Material(), i);
				}
			}
			else
			{
				MaterialPaths[i] = FString();
				DynamicMaterialInstances[i] = FString();
				ShaderGraphFlags[i] = false;
				MaterialParameters[i] = FMaterialInputParameters();

				const spatialpack::Material& NativeMaterial = MaterialAsset->GetOrCreateNativeMaterial(false);
				if (NativeMaterial.isValid())
				{
					UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS][Model] Apply Material[%d] NativeMaterial: %llu"), i, NativeMaterial.getResourceId());
					ModelComponent.setMaterial(NativeMaterial, i);
				}
				else
				{
					ModelComponent.setMaterial(spatialpack::Material(), i);
				}
			}
		}
		else
		{
			MaterialPaths[i] = FString();
			DynamicMaterialInstances[i] = FString();
			ShaderGraphFlags[i] = false;
			MaterialParameters[i] = FMaterialInputParameters();
			ModelComponent.setMaterial(spatialpack::Material(), i);
		}
	}

	ModelComponent.setAutoInstancingEnabled(bAutoInstancingEnabled);
	ModelComponent.setVisible(bVisible);

	spatialpack::Vector3f SpMargin = ToSpVector(VisualBoundsMargin, 100.0f);
	ModelComponent.setVisualBoundsMargin(SpMargin);
}

void UPICOSpatialECSModelComponent::UpdateMaterialStoredParameters(uint32 Index, UMaterialInterface* Material, bool bIsStaticMaterialInstance, bool bIsDynamicMaterialInstance, bool UpdateStaticOnce)
{
	if (MaterialParameters.Num() <= (int32)Index)
	{
		MaterialParameters.SetNum((int32)Index + 1);
	}

	if (Index < (uint32)ShaderGraphFlags.Num() && ShaderGraphFlags[Index] && ((bIsStaticMaterialInstance && UpdateStaticOnce) || bIsDynamicMaterialInstance))
	{
		UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material);
		if (MaterialInstance)
		{
			FMaterialInputParameters& MaterialInputParameters = MaterialParameters[(int32)Index];
			MaterialInputParameters = FMaterialInputParameters();
			MaterialInstance->GetAllParametersOfType(EMaterialParameterType::Scalar, MaterialInputParameters.ScalarParameters);
			MaterialInstance->GetAllParametersOfType(EMaterialParameterType::Vector, MaterialInputParameters.ColorParameters);
			MaterialInstance->GetAllParametersOfType(EMaterialParameterType::Texture, MaterialInputParameters.TextureParameters);
		}
	}
	else
	{
		MaterialParameters[(int32)Index] = FMaterialInputParameters();
	}
}

void UPICOSpatialECSModelComponent::UpdateNativeMaterialParameters(int MaterialIndex, spatialpack::SceneEntity& entity)
{
	spatialpack::Material NativeMaterial = entity.getOrAddModelComponent().getMaterial(MaterialIndex);

	if (!NativeMaterial.isValid())
	{
		return;
	}

	spatialpack::ShaderGraphMaterial ShaderGraphMaterial = spatialpack::ShaderGraphMaterial::castFromMaterial(NativeMaterial);
	if (!ShaderGraphMaterial.isValid())
	{
		if (MaterialIndex < ShaderGraphFlags.Num())
		{
			ShaderGraphFlags[MaterialIndex] = false;
		}
		return;
	}

	if (MaterialIndex >= MaterialParameters.Num())
	{
		return;
	}

	for (const auto& Pair : MaterialParameters[MaterialIndex].ScalarParameters)
	{
		const FMaterialParameterInfo& ParameterInfo = Pair.Key;
		const FMaterialParameterMetadata& ParameterMetadata = Pair.Value;
		FString ParameterName = ParameterInfo.Name.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_S");
		float ParameterValue = ParameterMetadata.Value.AsScalar();
		ShaderGraphMaterial.setParameter(TCHAR_TO_UTF8(*ParameterName), ParameterValue);
	}

	for (const auto& Pair : MaterialParameters[MaterialIndex].ColorParameters)
	{
		const FMaterialParameterInfo& ParameterInfo = Pair.Key;
		const FMaterialParameterMetadata& ParameterMetadata = Pair.Value;
		FString ParameterName = ParameterInfo.Name.ToString().Replace(TEXT(" "), TEXT("")) + TEXT("_V");
		FLinearColor ParameterValue = ParameterMetadata.Value.AsLinearColor();
		ShaderGraphMaterial.setParameter(TCHAR_TO_UTF8(*ParameterName), ToSpColor(ParameterValue));
	}

	for (const auto& Pair : MaterialParameters[MaterialIndex].TextureParameters)
	{
		const FMaterialParameterInfo& ParameterInfo = Pair.Key;
		const FMaterialParameterMetadata& ParameterMetadata = Pair.Value;
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
}

void UPICOSpatialECSModelComponent::ProcessMaterialAtIndex(int32 Index, UMaterialInterface* SourceMaterial, bool bForceUpdate)
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

	if (MaterialPaths.Num() <= Index)
	{
		MaterialPaths.SetNum(Index + 1);
		DynamicMaterialInstances.SetNum(Index + 1);
		ShaderGraphFlags.SetNum(Index + 1);
	}

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

	MaterialPaths[Index] = MaterialNamePath;
	DynamicMaterialInstances[Index] = DynamicMaterialInstanceNamePath;
	ShaderGraphFlags[Index] = true;

	if (MaterialParameters.Num() <= Index)
	{
		MaterialParameters.SetNum(Index + 1);
	}

	UpdateMaterialStoredParameters(Index, SourceMaterial, bIsStaticMaterialInstance, bIsDynamicMaterialInstance, bForceUpdate);
}
#endif
