// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialWidgetEntity.h"
#include "PICOSpatial.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TextureResource.h"

#if PLATFORM_ANDROID
#include "IVulkanDynamicRHI.h"
#endif

void FWidgetEntityProxy::ConstructWidgetEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (UWidgetComponent* WidgetComponent = Cast<UWidgetComponent>(SceneComponent))
	{
		if (bInitialize)
		{
			WidgetComponent->SetTickWhenOffscreen(true);
			Size = WidgetComponent->GetDrawSize();
		}
		else
		{
			if (bTransformDirty)
			{
				RotateTransform(Transform, FRotator(0, -90, 0));
				Transform.SetScale3D(FVector(Transform.GetScale3D().Y, Transform.GetScale3D().X, Transform.GetScale3D().Z));
			}
		}

		WidgetRenderTarget = WidgetComponent->GetRenderTarget();
		bManuallyUpdateWidget = WidgetComponent->GetManuallyRedraw();
	}
}

FWidgetEntityProxy::FWidgetEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	:FMeshEntityProxy(InParentEntityProxy, SceneComponent)
	, Size(FVector2D(100, 100))
{
	ConstructWidgetEntityData(SceneComponent, true);
}

void FWidgetEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FMeshEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstructWidgetEntityData(SceneComponent, false);
	bMaterialChanged = false;
}

bool FWidgetEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FMeshEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.Get()->IsValid())
		{
#if PLATFORM_ANDROID
			WidgetMesh = spatialpack::MeshResource::createPlane(Size.X / 100.0f, Size.Y / 100.0f);
			NativeEntity.Get()->entity.getOrAddModelComponent().setMesh(WidgetMesh);

			if (WidgetRenderTarget)
			{
				FPICOSpatial::Get().InitRT(WidgetRenderTarget);

				spatialpack::RenderTextureDescriptor rtDesc;
				rtDesc.setWidth(WidgetRenderTarget->SizeX).setHeight(WidgetRenderTarget->SizeY).setTexturePixelFormat(spatialpack::TexturePixelFormat::RGBA8_SRGB);
				mRenderTextureBufferQueue = spatialpack::RenderTextureBufferQueue::create(rtDesc);
				renderTexture = mRenderTextureBufferQueue.asTextureResource();
				if (!renderTexture.isValid())
				{
					UE_LOG(LogPICOSpatial, Warning, TEXT("FWidgetEntityProxy:%s failed to create TextureResource!"), *Name);
				}
				else
				{
					auto WidgetUnlitMaterial = spatialpack::UnlitMaterial::create(spatialpack::MaterialBlendingMode::TRANSPARENT);
					WidgetUnlitMaterial.setBaseColorTextureParameter(spatialpack::MaterialTextureParameter{ renderTexture });
					WidgetMaterial = WidgetUnlitMaterial.toMaterial();
					NativeEntity.Get()->entity.getOrAddModelComponent().setMaterial(WidgetMaterial);

					const ERHIInterfaceType RHIType = RHIGetInterfaceType();

					ENQUEUE_RENDER_COMMAND(CreateTexture)(
						[this, RHIType](FRHICommandListImmediate& RHICmdList)
						{
							if (RHIType == ERHIInterfaceType::Vulkan)
							{
								IVulkanDynamicRHI* VulkanRHI = GetIVulkanDynamicRHI();
								if (VulkanRHI)
								{
									ETextureCreateFlags UnifiedCreateFlags = {};
									UnifiedCreateFlags |= TexCreate_Dynamic;
									UnifiedCreateFlags |= TexCreate_ShaderResource;
									UnifiedCreateFlags |= TexCreate_ResolveTargetable;
									UnifiedCreateFlags |= TexCreate_RenderTargetable;
									UnifiedCreateFlags |= TexCreate_SRGB;
									UnifiedCreateFlags |= TexCreate_External;

									RenderTexturesRHI.Reset();
									for (int32 i = 0; i < mRenderTextureBufferQueue.getBufferCount(); i++)
									{
										spatialpack::TextureResource textureResource = mRenderTextureBufferQueue.getBufferByIndex(i);
										if (textureResource.isValid())
										{
											FTextureRHIRef RenderTextureRHI = VulkanRHI->RHICreateTexture2DFromResource(
												GPixelFormats[PF_R8G8B8A8].UnrealFormat,
												WidgetRenderTarget->SizeX,
												WidgetRenderTarget->SizeY,
												1, 1,
												(VkImage)textureResource.getRawTextureHandle(),
												UnifiedCreateFlags,
												FClearValueBinding::Black);
											if (RenderTextureRHI.IsValid())
											{
												RenderTexturesRHI.Add(textureResource.getResourceId(), RenderTextureRHI);
											}
										}
									}
								}
							}
							else if (RHIType == ERHIInterfaceType::OpenGL && false)// TODO: Currently, do not support gl
							{
								IOpenGLDynamicRHI* OpenGLRHI = GetIOpenGLDynamicRHI();
								if (OpenGLRHI)
								{
									ETextureCreateFlags GLCreateFlags = {};
									GLCreateFlags |= TexCreate_Dynamic;
									GLCreateFlags |= TexCreate_ShaderResource;
									GLCreateFlags |= TexCreate_RenderTargetable;
									GLCreateFlags |= TexCreate_SRGB;

									RenderTexturesRHI.Reset();
									for (int32 i = 0; i < mRenderTextureBufferQueue.getBufferCount(); i++)
									{
										spatialpack::TextureResource textureResource = mRenderTextureBufferQueue.getBufferByIndex(i);
										if (textureResource.isValid())
										{
											FTextureRHIRef RenderTextureRHI = OpenGLRHI->RHICreateTexture2DFromResource(
												GPixelFormats[PF_R8G8B8A8].UnrealFormat,
												WidgetRenderTarget->SizeX,
												WidgetRenderTarget->SizeY,
												1,  // NumMips
												1,  // NumSamples
												1,  // NumSamplesTileMem
												FClearValueBinding::Black,
												(uint32)(uintptr_t)textureResource.getRawTextureHandle(),
												GLCreateFlags);
											if (RenderTextureRHI.IsValid())
											{
												RenderTexturesRHI.Add(textureResource.getResourceId(), RenderTextureRHI);
											}
										}
									}
								}
							}
							else
							{
								FString RHIString = FApp::GetGraphicsRHI();
								UE_LOG(LogPICOSpatial, Warning, TEXT("FWidgetEntityProxy: %s is not supported for widget rendering"), *RHIString);
							}
						});
					FlushRenderingCommands();
				}
			}
#endif // PLATFORM_ANDROID

			if (bTransformDirty)
			{
				RotateTransform(Transform, FRotator(0, -90, 0));
				Transform.SetScale3D(FVector(Transform.GetScale3D().Y, Transform.GetScale3D().X, Transform.GetScale3D().Z));
			}
			bMaterialChanged = false;
			return true;
		}
	}
	return false;
}

void FWidgetEntityProxy::UpdateNativeEntity()
{
	FMeshEntityProxy::UpdateNativeEntity();
#if PLATFORM_ANDROID
	if (!bManuallyUpdateWidget && WidgetRenderTarget && WidgetRenderTarget->GetResource() && renderTexture.isValid())
	{
		if (bCopyInProgress && CopyCompleteFence.IsValid())
		{
			if (CopyCompleteFence->Poll())
			{
				bCopyInProgress = false;
				CopyCompleteFence->Clear();
				if (CurrentRenderingTexture.isValid())
				{
					mRenderTextureBufferQueue.getProducer().enqueueBuffer(CurrentRenderingTexture, true);
				}
			}
			else
			{
				return;
			}
		}

		CurrentRenderingTexture = mRenderTextureBufferQueue.getProducer().dequeueBuffer();
		if (CurrentRenderingTexture.isValid())
		{
			ENQUEUE_RENDER_COMMAND(CopyTexture)(
				[this](FRHICommandListImmediate& RHICmdList)
				{
					FRHITexture* SrcTexture = WidgetRenderTarget->GetResource()->GetTextureRHI()->GetTexture2D();
					FTextureRHIRef* TextureRHIRefFound = RenderTexturesRHI.Find(CurrentRenderingTexture.getResourceId());
					if (TextureRHIRefFound)
					{
						FTextureRHIRef DstRenderTextureRHI = RenderTexturesRHI[CurrentRenderingTexture.getResourceId()];
						if (DstRenderTextureRHI.IsValid())
						{
							FRHITexture* DstTexture = DstRenderTextureRHI->GetTexture2D();
							if (SrcTexture && DstTexture)
							{
								const FIntRect Rect(FIntPoint(0, 0), DstTexture->GetSizeXY());
								ETextureCopyBlendModifier SrcTextureCopyModifier = ETextureCopyBlendModifier::TransparentAlphaPassthrough;
								FPICOSpatial::Get().CopyTexture_RenderThread(RHICmdList, SrcTexture, FIntRect(), DstTexture, Rect, false, ERenderTargetActions::Clear_Store, ERHIAccess::SRVMask, SrcTextureCopyModifier);

								if (!CopyCompleteFence.IsValid())
								{
									CopyCompleteFence = RHICreateGPUFence(TEXT("WidgetTextureCopyFence"));
								}

								RHICmdList.WriteGPUFence(CopyCompleteFence);
								bCopyInProgress = true;
							}
						}
					}
				});
			FlushRenderingCommands();
		}
	}
#endif // PLATFORM_ANDROID
}
