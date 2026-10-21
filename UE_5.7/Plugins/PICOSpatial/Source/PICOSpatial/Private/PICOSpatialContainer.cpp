// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialContainer.h"
#include "PICOSpatial.h"
#include "Components/BoxComponent.h"
#include "PICOSpatialSceneEntity.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidPlatformApplicationMisc.h"
#endif

int32 APICOSpatialContainer::bNeedSharedRendering = 0;
TMap<uint64, TWeakObjectPtr<APICOSpatialContainer>> APICOSpatialContainer::OpenedContainersByID;

APICOSpatialContainer::APICOSpatialContainer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUseManualActorList(false)
	, ManualActorList()
	, bIsSpatialContainerOpening(false)
	, bDirty(false)
	, bTransformDirtyThisFrame(false)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	// Create directional light socket component
	DirectionalLightSocket = CreateDefaultSubobject<USceneComponent>(TEXT("DirectionalLightSocket"));
	check(DirectionalLightSocket != nullptr);
	DirectionalLightSocket->SetupAttachment(RootComponent);

	// Create root 3D component for spatial positioning
	Root3dComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Root3dComponent"));
	check(Root3dComponent != nullptr);

	// Configure box component for bounded containers and cache reference
	CachedBoxComponent = Cast<UBoxComponent>(GetCollisionComponent());
	if (CachedBoxComponent)
	{
		UBoxComponent* Box = CachedBoxComponent.Get();
		Root3dComponent->SetIsScreenSizeScaled(true);
		Root3dComponent->SetupAttachment(Box);
		Box->InitBoxExtent(FVector(100.0f, 100.0f, 100.0f));
		Box->SetLineThickness(10.0f);
		Box->ShapeColor = FColor::MakeRandomColor();
		Box->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
		Box->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		Box->SetGenerateOverlapEvents(false);
	}
	
	// Initialize spatial container name from actor label
	FString ActorName;
#if WITH_EDITOR
	ActorName = GetActorLabel();
#else
	ActorName = GetActorNameOrLabel();
#endif
	SpatialContainerName = ActorName;
}

void APICOSpatialContainer::RegisterOpenedContainer(uint64 Id, APICOSpatialContainer* Container)
{
	if (Id == static_cast<uint64>(-1) || !IsValid(Container))
	{
		return;
	}

	OpenedContainersByID.Add(Id, Container);
}

void APICOSpatialContainer::UnregisterOpenedContainer(uint64 Id, APICOSpatialContainer* Container)
{
	if (Id == static_cast<uint64>(-1))
	{
		return;
	}

	if (TWeakObjectPtr<APICOSpatialContainer>* RegisteredContainer = OpenedContainersByID.Find(Id))
	{
		if (!RegisteredContainer->IsValid() || RegisteredContainer->Get() == Container)
		{
			OpenedContainersByID.Remove(Id);
		}
	}
}

APICOSpatialContainer* APICOSpatialContainer::FindOpenedContainer(uint64 Id)
{
	if (TWeakObjectPtr<APICOSpatialContainer>* RegisteredContainer = OpenedContainersByID.Find(Id))
	{
		if (APICOSpatialContainer* Container = RegisteredContainer->Get())
		{
			return Container;
		}

		OpenedContainersByID.Remove(Id);
	}

	return nullptr;
}

void APICOSpatialContainer::ResetBackgroundTimeoutState()
{
	BackgroundElapsedSeconds = 0.0f;
	bBackgroundTimeout = false;
}

void APICOSpatialContainer::ReleaseAndroidGlobalRefs()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		if (PICOSpatialActivityInstance != nullptr)
		{
			Env->DeleteGlobalRef(static_cast<jobject>(PICOSpatialActivityInstance));
			PICOSpatialActivityInstance = nullptr;
		}
		if (GameActivityInstance != nullptr)
		{
			Env->DeleteGlobalRef(static_cast<jobject>(GameActivityInstance));
			GameActivityInstance = nullptr;
		}
		if (PICOSpatialActivityClass != nullptr)
		{
			Env->DeleteGlobalRef(static_cast<jclass>(PICOSpatialActivityClass));
			PICOSpatialActivityClass = nullptr;
		}
		if (GameActivityClass != nullptr)
		{
			Env->DeleteGlobalRef(static_cast<jclass>(GameActivityClass));
			GameActivityClass = nullptr;
		}
		if (GameApplicationContext != nullptr)
		{
			Env->DeleteGlobalRef(static_cast<jobject>(GameApplicationContext));
			GameApplicationContext = nullptr;
		}
	}
#else
	GameApplicationContext = nullptr;
	GameActivityInstance = nullptr;
	GameActivityClass = nullptr;
	PICOSpatialActivityInstance = nullptr;
	PICOSpatialActivityClass = nullptr;
#endif
}

bool APICOSpatialContainer::CloseSpatialContainerInternal(bool bDestroyNativeContainer)
{
	const uint64 ClosedContainerId = ContainerID;
	const bool bHadOpenState =
		bIsSpatialContainerOpening ||
		SpatialContainer != nullptr ||
		ClosedContainerId != static_cast<uint64>(-1) ||
		GameApplicationContext != nullptr ||
		GameActivityInstance != nullptr ||
		GameActivityClass != nullptr ||
		PICOSpatialActivityInstance != nullptr ||
		PICOSpatialActivityClass != nullptr;

	if (!bHadOpenState)
	{
		return false;
	}

	if (bHasEnterForeground)
	{
		PauseSpatialContainerPICO();
	}

	FPICOSpatial::Get().Containers.Remove(this);
	bIsSpatialContainerOpening = false;
	SetActorTickEnabled(false);
	ResetBackgroundTimeoutState();

#if PLATFORM_ANDROID
	UnregisterOpenedContainer(ClosedContainerId, this);

	if (SpatialContainer != nullptr)
	{
		void* ActivityInstance = bDefaultSpatialContainer ? GameActivityInstance : PICOSpatialActivityInstance;
		if (ActivityInstance != nullptr)
		{
			SpatialContainer->unbindEntity(ActivityInstance);
		}
	}

	if (bDestroyNativeContainer && !bDefaultSpatialContainer && FPICOSpatial::Get().CurrentScene && SpatialContainer != nullptr)
	{
		switch (SpatialContainerTypePICO)
		{
		case ESpatialContainerTypePICO::Bounded:
			FPICOSpatial::Get().CurrentScene->destroy(static_cast<spatialpack::WindowContainer*>(SpatialContainer));
			break;
		case ESpatialContainerTypePICO::Unbounded:
			FPICOSpatial::Get().CurrentScene->destroy(static_cast<spatialpack::Stage*>(SpatialContainer));
			break;
		}
	}
#endif

	bBindedToSpatialContainer = false;
	SpatialContainer = nullptr;
	ReleaseAndroidGlobalRefs();
	ContainerID = static_cast<uint64>(-1);
	bIsOnSight = false;
	bHasFocus = false;

	if (ClosedContainerId != static_cast<uint64>(-1))
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("Window container closed with ID: %llu"), ClosedContainerId);
	}

	return true;
}

void APICOSpatialContainer::BeginPlay()
{
	Super::BeginPlay();

	// Register callbacks for window container events
	FPICOSpatial& PICOSpatialInstance = FPICOSpatial::Get();
	OnWindowContainerCreatedDelegateHandle = PICOSpatialInstance.OnWindowContainerCreatedDelegate.AddUObject(this, &APICOSpatialContainer::WindowContainerOnCreatedCallback);
	OnWindowContainerDestroyedDelegateHandle = PICOSpatialInstance.OnWindowContainerDestroyedDelegate.AddUObject(this, &APICOSpatialContainer::WindowContainerOnDestroyedCallback);
	OnWindowContainerForegroundDelegateHandle = PICOSpatialInstance.OnWindowContainerForegroundDelegate.AddUObject(this, &APICOSpatialContainer::WindowContainerOnForegroundCallback);
	OnWindowContainerBackgroundDelegateHandle = PICOSpatialInstance.OnWindowContainerBackgroundDelegate.AddUObject(this, &APICOSpatialContainer::WindowContainerOnBackgroundCallback);
	OnWindowContainerFocusDelegateHandle = PICOSpatialInstance.OnWindowContainerFocusDelegate.AddUObject(this, &APICOSpatialContainer::WindowContainerOnFocusCallback);
	OnWindowContainerSightDelegateHandle = PICOSpatialInstance.OnWindowContainerSightDelegate.AddUObject(this, &APICOSpatialContainer::WindowContainerOnSightCallback);

	// Load settings for default spatial container
	if (bDefaultSpatialContainer)
	{
		if (const UPICOSpatialRuntimeSettings* PICOSpatialRuntimeSettings = GetDefault<UPICOSpatialRuntimeSettings>())
		{
			SpatialContainerTypePICO = PICOSpatialRuntimeSettings->SpatialContainerTypePICO;
			SpatialContainerSize = PICOSpatialRuntimeSettings->SpatialContainerSize;
			SpatialContainerName = PICOSpatialRuntimeSettings->SpatialContainerName;
			bScaleContentToSpatialContainer = PICOSpatialRuntimeSettings->bScaleContentToSpatialContainer;
			UnboundedStylePICO = PICOSpatialRuntimeSettings->UnboundedStylePICO;
			bOpenSpatialContainerAtBegin = true;
		}
	}

	// Position components for bounded containers
	if (SpatialContainerTypePICO == ESpatialContainerTypePICO::Bounded)
	{
		// Use cached box component if available
		if (!CachedBoxComponent)
		{
			CachedBoxComponent = Cast<UBoxComponent>(GetCollisionComponent());
		}
		
		if (CachedBoxComponent)
		{
			const FVector BoxExtent = CachedBoxComponent.Get()->GetUnscaledBoxExtent();
			Root3dComponent->SetRelativeLocation(FVector(BoxExtent.X, 0.f, 0.f));
			DirectionalLightSocket->SetRelativeLocation(FVector(0.f, 0.f, BoxExtent.Z));
		}
	}

	// Open spatial container if configured to do so
	if (bOpenSpatialContainerAtBegin)
	{
		OpenSpatialContainerPICO();
	}

	bDirty = true;
	
	// Initialize root entity proxy
	if (!SpatialContainerRootEntityProxy.IsValid())
	{
		SpatialContainerRootEntityProxy = MakeShareable(new FSceneEntityProxy(nullptr, Root3dComponent));
		SpatialContainerRootEntityProxy->bIsSpatialContainerRootEntity = true;
	}
}

void APICOSpatialContainer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Clean up manual actor list when container is being destroyed
	ClearManualActorList();

	// Clean up root entity proxy
	if (SpatialContainerRootEntityProxy.IsValid())
	{
		SpatialContainerRootEntityProxy.Reset();
		SpatialContainerRootEntityProxy = nullptr;
	}

	// Close spatial container
	CloseSpatialContainerPICO();
	
	// Unregister all delegate handles
	FPICOSpatial& PICOSpatialInstance = FPICOSpatial::Get();
	PICOSpatialInstance.OnWindowContainerCreatedDelegate.Remove(OnWindowContainerCreatedDelegateHandle);
	PICOSpatialInstance.OnWindowContainerDestroyedDelegate.Remove(OnWindowContainerDestroyedDelegateHandle);
	PICOSpatialInstance.OnWindowContainerForegroundDelegate.Remove(OnWindowContainerForegroundDelegateHandle);
	PICOSpatialInstance.OnWindowContainerBackgroundDelegate.Remove(OnWindowContainerBackgroundDelegateHandle);
	PICOSpatialInstance.OnWindowContainerFocusDelegate.Remove(OnWindowContainerFocusDelegateHandle);
	PICOSpatialInstance.OnWindowContainerSightDelegate.Remove(OnWindowContainerSightDelegateHandle);
	
	// Reset delegate handles
	OnWindowContainerCreatedDelegateHandle.Reset();
	OnWindowContainerDestroyedDelegateHandle.Reset();
	OnWindowContainerForegroundDelegateHandle.Reset();
	OnWindowContainerBackgroundDelegateHandle.Reset();
	OnWindowContainerFocusDelegateHandle.Reset();
	OnWindowContainerSightDelegateHandle.Reset();
}

void APICOSpatialContainer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Cache singleton reference to avoid repeated calls
	FPICOSpatial& PICOSpatialInstance = FPICOSpatial::Get();

	// Background timeout: accumulate time while the container is open but in the background.
	// Tick is always active so this timer works even when foreground is false.
	if (bIsSpatialContainerOpening && !bHasEnterForeground && BackgroundAutoCloseSeconds > 0.0f)
	{
		BackgroundElapsedSeconds += DeltaSeconds;
		if (BackgroundElapsedSeconds >= BackgroundAutoCloseSeconds && !bBackgroundTimeout)
		{
			bBackgroundTimeout = true;
			UE_LOG(LogPICOSpatial, Log,
				TEXT("Container '%s' background timeout after %.1fs (threshold %.1fs), proxies will be destroyed"),
				*GetName(), BackgroundElapsedSeconds, BackgroundAutoCloseSeconds);
		}
	}
	else if (bIsSpatialContainerOpening && !bHasEnterForeground && bBackgroundTimeout)
	{
		ResetBackgroundTimeoutState();
	}

	// Skip all expensive work while in background - only the timer above needs to run.
	if (!bHasEnterForeground)
	{
		return;
	}

	// Early out if PICO Spatial is not running or proxy is invalid
	if (!PICOSpatialInstance.bPICOSpatialRunning || !SpatialContainerRootEntityProxy.IsValid())
	{
		return;
	}

	// Cache component transform reference
	const FTransform& CurrentTransform = Root3dComponent->GetComponentTransform();

	// Reset frame dirty flag
	bTransformDirtyThisFrame = false;

	// Fast path: Quick dirty check using position/rotation/scale separately
	// This avoids the full Equals() call in most cases
	if (!bDirty)
	{
		// Use component-wise comparison with reasonable tolerance for better performance
		// Increased from 0.01f to 0.1f (1mm) - still very precise but reduces unnecessary updates
		constexpr float TRANSFORM_TOLERANCE = 0.1f;
		
		const FVector& LastPos = LastTranslation.GetLocation();
		const FVector& CurrPos = CurrentTransform.GetLocation();
		const FQuat& LastRot = LastTranslation.GetRotation();
		const FQuat& CurrRot = CurrentTransform.GetRotation();
		const FVector& LastScale = LastTranslation.GetScale3D();
		const FVector& CurrScale = CurrentTransform.GetScale3D();
		
		// Fast component-wise comparison with early exit
		bDirty = !LastPos.Equals(CurrPos, TRANSFORM_TOLERANCE) ||
		         !LastRot.Equals(CurrRot, TRANSFORM_TOLERANCE) ||
		         !LastScale.Equals(CurrScale, TRANSFORM_TOLERANCE);
	}

	// Update transform dirty flag
	SpatialContainerRootEntityProxy->bTransformDirty |= bDirty;
	bTransformDirtyThisFrame = bDirty;
	
	if (bDirty)
	{
		FVector Scale = FVector::OneVector;
		
		// Calculate scale for bounded containers using cached box component
		if (SpatialContainerTypePICO == ESpatialContainerTypePICO::Bounded)
		{
			const FVector CurrentLocation = CurrentTransform.GetLocation();
			
			if (bScaleContentToSpatialContainer)
			{
				// Use cached box component to avoid repeated Cast
				if (!CachedBoxComponent)
				{
					CachedBoxComponent = Cast<UBoxComponent>(GetCollisionComponent());
				}
				
				if (CachedBoxComponent)
				{
					const FVector VolumeSize = (CachedBoxComponent.Get()->GetScaledBoxExtent() * 2.0f).GetAbs();
					// Avoid division by zero - use fast path when possible
					Scale.X = (VolumeSize.X > KINDA_SMALL_NUMBER) ? (SpatialContainerSize.X / VolumeSize.X) : 1.0f;
					Scale.Y = (VolumeSize.Y > KINDA_SMALL_NUMBER) ? (SpatialContainerSize.Y / VolumeSize.Y) : 1.0f;
					Scale.Z = (VolumeSize.Z > KINDA_SMALL_NUMBER) ? (SpatialContainerSize.Z / VolumeSize.Z) : 1.0f;
				}
			}
		}

		// Calculate final transform for spatial container
		// Use const static to avoid recreating this quaternion every frame
		static const FQuat NegativeYaw90(FRotator(0.f, -90.f, 0.f));

		// Combine quaternion operations
		const FQuat FinalRotation = NegativeYaw90 * CurrentTransform.GetRotation().Inverse();
		// Combine vector operations to reduce intermediate calculations
		const FVector Position = FinalRotation.RotateVector(CurrentTransform.GetLocation() * Scale * -1.0f);

		SpatialContainerRootEntityProxy->Transform = FTransform(FinalRotation, Position, Scale);

		LastTranslation = CurrentTransform;
		bDirty = false;
	}

	// Cache native entity pointer to reduce dereferencing
	auto* NativeEntityPtr = SpatialContainerRootEntityProxy->NativeEntity.Get();
	
	// Initialize native entity if not yet created
	if (!NativeEntityPtr->IsValid() && SpatialContainer != nullptr)
	{
		SpatialContainerRootEntityProxy->InitializeNativeEntity(PICOSpatialInstance.CurrentScene, SpatialContainer);
		// Re-cache pointer after initialization
		NativeEntityPtr = SpatialContainerRootEntityProxy->NativeEntity.Get();
	}

	// Update native entity and bind to container
	if (NativeEntityPtr->IsValid())
	{
		SpatialContainerRootEntityProxy->UpdateNativeEntity();
		
		if (!bBindedToSpatialContainer && SpatialContainer != nullptr)
		{
#if PLATFORM_ANDROID
			void* ActivityInstance = bDefaultSpatialContainer ? GameActivityInstance : PICOSpatialActivityInstance;
			
			if ((bDefaultSpatialContainer || PICOSpatialActivityInstance != nullptr) && ActivityInstance != nullptr)
			{
				SpatialContainer->bindEntity(ActivityInstance, NativeEntityPtr->entity.getEntityUUID());
				bBindedToSpatialContainer = true;
				
				// Set IBL scale after successful binding
				SetGlobalIBLScalePICO(IBLScale);
			}
#endif
		}
	}
}

bool APICOSpatialContainer::OpenSpatialContainerPICO()
{
	if (!bIsSpatialContainerOpening && FPICOSpatial::Get().CurrentScene && FPICOSpatial::Get().bPICOSpatialEnabled)
	{
#if PLATFORM_ANDROID
		if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
		{
			static jmethodID GetAppContextMethod = FJavaWrapper::FindMethod(
				Env,
				FJavaWrapper::GameActivityClassID,
				"getApplicationContext",
				"()Landroid/content/Context;",
				false
			);

			if (GetAppContextMethod != nullptr)
			{
				jobject LocalAppContext = FJavaWrapper::CallObjectMethod(
					Env,
					FJavaWrapper::GameActivityThis,
					GetAppContextMethod
				);

				if (LocalAppContext != nullptr)
				{
					GameApplicationContext = static_cast<void*>(Env->NewGlobalRef(LocalAppContext));
				}
			}
			check(GameApplicationContext != nullptr);

			GameActivityInstance = static_cast<void*>(Env->NewGlobalRef(FJavaWrapper::GameActivityThis));
			check(GameActivityInstance != nullptr);

			GameActivityClass = static_cast<void*>(Env->NewGlobalRef(FJavaWrapper::GameActivityClassID));
			check(GameActivityClass != nullptr);

			if (!bDefaultSpatialContainer)
			{
				jclass PICOSpatialActivity = AndroidJavaEnv::FindJavaClass("com/epicgames/unreal/PICOSpatialActivity");
				check(PICOSpatialActivity != nullptr);

				PICOSpatialActivityClass = static_cast<void*>(Env->NewGlobalRef(PICOSpatialActivity));
				check(PICOSpatialActivityClass != nullptr);

				Env->DeleteLocalRef(PICOSpatialActivity);
			}
		}

		switch (SpatialContainerTypePICO)
		{
		case ESpatialContainerTypePICO::Bounded:
		{
			if (bDefaultSpatialContainer)
			{
				spatialpack::WindowContainerBindArgs BindArgs{};
				ContainerID = spatialpack::AppForm::getIdByActivityContext(GameActivityInstance);
				BindArgs.setAppFormId(ContainerID);
				SpatialContainer = FPICOSpatial::Get().CurrentScene->bind(BindArgs);
			}
			else
			{
				const float Centimeter2PixelFactor = 0.01f / spatialpack::MeasureUtility::getPixelToMeterFactor();
				UE_LOG(LogPICOSpatial, Log, TEXT("Centimeter2PixelFactor: %f"), Centimeter2PixelFactor);

				const int32 ContainerWidth = static_cast<int32>(SpatialContainerSize.Y * Centimeter2PixelFactor);
				const int32 ContainerHeight = static_cast<int32>(SpatialContainerSize.Z * Centimeter2PixelFactor);
				const int32 ContainerDepth = static_cast<int32>(SpatialContainerSize.X * Centimeter2PixelFactor);

				UE_LOG(LogPICOSpatial, Log, TEXT("SpatialContainerSize: %s, Width: %d, Height: %d, Depth: %d"), *SpatialContainerSize.ToString(), ContainerWidth, ContainerHeight, ContainerDepth);
				
				spatialpack::WindowContainerCreateArgs CreateArgs{};
				CreateArgs.setAndroidApplicationContext(GameApplicationContext)
					.setActivityClass(PICOSpatialActivityClass)
					.setName(TCHAR_TO_UTF8(*SpatialContainerName))
					.setSize(spatialpack::Int3(ContainerWidth, ContainerHeight, ContainerDepth))
					.setMinSize(spatialpack::Int3(ContainerWidth, ContainerHeight, ContainerDepth))
					.setMaxSize(spatialpack::Int3(ContainerWidth, ContainerHeight, ContainerDepth))
					.setStyle(spatialpack::WindowContainerStyle::VOLUMETRIC)
					.setScaleType(static_cast<spatialpack::WindowContainerScaleType>(WindowContainerScaleType))
					.setCaptionBarStyle(static_cast<spatialpack::WindowContainerCaptionBarStyle>(WindowContainerCaptionBarStyle));
				SpatialContainer = FPICOSpatial::Get().CurrentScene->create(CreateArgs);
			}
			break;
		}
		case ESpatialContainerTypePICO::Unbounded:
		{
			if (bDefaultSpatialContainer)
			{
				spatialpack::StageBindArgs BindArgs{};
				ContainerID = spatialpack::AppForm::getIdByActivityContext(GameActivityInstance);
				BindArgs.setAppFormId(ContainerID);
				SpatialContainer = FPICOSpatial::Get().CurrentScene->bind(BindArgs);
			}
			else
			{
				const spatialpack::StageStyle StageStyle = (SpatialContainerTypePICO == ESpatialContainerTypePICO::Unbounded)
					? static_cast<spatialpack::StageStyle>(UnboundedStylePICO)
					: spatialpack::StageStyle::FULL_IMMERSION;

				spatialpack::StageCreateArgs CreateArgs = {};
				CreateArgs.setAndroidApplicationContext(GameApplicationContext)
					.setActivityClass(PICOSpatialActivityClass)
					.setName(TCHAR_TO_UTF8(*SpatialContainerName))
					.setStyle(StageStyle);
				SpatialContainer = FPICOSpatial::Get().CurrentScene->create(CreateArgs);
			}
			break;
		}
		}
#endif // PLATFORM_ANDROID
	}

	if (SpatialContainer)
	{
		bIsSpatialContainerOpening = true;
		SetActorTickEnabled(true);
#if PLATFORM_ANDROID
		RegisterOpenedContainer(ContainerID, this);
#endif
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Container] Open success actor=%s containerId=%llu spatialContainer=%p default=%d"),
			*GetNameSafe(this), ContainerID, SpatialContainer, bDefaultSpatialContainer);
		if (bDefaultSpatialContainer)
		{
			ResumeSpatialContainerPICO();
		}
		return true;
	}

	ReleaseAndroidGlobalRefs();
	ContainerID = static_cast<uint64>(-1);
	ResetBackgroundTimeoutState();
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Container] Open failed actor=%s enabled=%d scene=%p"), *GetNameSafe(this), FPICOSpatial::Get().bPICOSpatialEnabled, FPICOSpatial::Get().CurrentScene);
	return false;
}

bool APICOSpatialContainer::CloseSpatialContainerPICO()
{
	return CloseSpatialContainerInternal(true);
}

bool APICOSpatialContainer::PauseSpatialContainerPICO()
{
	if (!bIsSpatialContainerOpening || !bHasEnterForeground)
	{
		return false;
	}

	bNeedSharedRendering = FMath::Max(0, bNeedSharedRendering - 1);

	// Update world rendering state
	if (bAutoManageWorldRendering)
	{
		UGameplayStatics::SetEnableWorldRendering(GetWorld(), bNeedSharedRendering == 0);
	}

	bHasEnterForeground = false;
	
	return true;
}

bool APICOSpatialContainer::ResumeSpatialContainerPICO()
{
	if (!bIsSpatialContainerOpening || bHasEnterForeground)
	{
		return false;
	}

	bNeedSharedRendering++;

	// Update world rendering state
	if (bAutoManageWorldRendering && bNeedSharedRendering > 0)
	{
		UGameplayStatics::SetEnableWorldRendering(GetWorld(), false);
	}

	FPICOSpatial::Get().Containers.Add(this);
	bHasEnterForeground = true;
	ResetBackgroundTimeoutState();

	return true;
}

bool APICOSpatialContainer::SetGlobalIBLScalePICO(float Scale)
{
	if (SpatialContainerRootEntityProxy.IsValid() && SpatialContainerRootEntityProxy->NativeEntity.Get()->IsValid())
	{
		IBLScale = Scale;
#if PLATFORM_ANDROID
		SpatialContainerRootEntityProxy->NativeEntity.Get()->entity.getOrAddEnvironmentLightingSettingsComponent().setScale(Scale);
		return true;
#endif
	}
	return false;
}

void APICOSpatialContainer::WindowContainerOnCreatedCallback(uint64 Id)
{
	check(IsInGameThread());
	
	if (SpatialContainer == nullptr)
	{
		return;
	}

#if PLATFORM_ANDROID
	spatialpack::AppFormId FormId;
	SpatialContainer->tryGetId(FormId);
	if (Id == FormId)
	{
		ContainerID = Id;
		RegisterOpenedContainer(ContainerID, this);
		UE_LOG(LogPICOSpatial, Log, TEXT("Window container created with ID: %llu"), Id);
	}
#endif
}

void APICOSpatialContainer::WindowContainerOnDestroyedCallback(uint64 Id)
{
	check(IsInGameThread());
	
	if (ContainerID == Id)
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("Window container destroyed with ID: %llu"), Id);
		CloseSpatialContainerInternal(false);
	}
}

void APICOSpatialContainer::WindowContainerOnForegroundCallback(uint64 Id)
{
	check(IsInGameThread());
	
	if (ContainerID == Id)
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("Window container entered foreground with ID: %llu"), Id);
		ResumeSpatialContainerPICO();
	}
}

void APICOSpatialContainer::WindowContainerOnBackgroundCallback(uint64 Id)
{
	check(IsInGameThread());
	
	if (ContainerID == Id)
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("Window container entered background with ID: %llu"), Id);
		PauseSpatialContainerPICO();
	}
}

void APICOSpatialContainer::WindowContainerOnFocusCallback(uint64 Id, bool bInHasFocus)
{
	check(IsInGameThread());
	
	if (ContainerID == Id)
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("Window container focus changed to %s with ID: %llu"), bInHasFocus ? TEXT("focused") : TEXT("unfocused"), Id);
		bHasFocus = bInHasFocus;
	}
}

void APICOSpatialContainer::WindowContainerOnSightCallback(uint64 Id, bool bInIsOnSight)
{
	check(IsInGameThread());
	
	if (ContainerID == Id)
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("Window container sight changed to %s with ID: %llu"), bInIsOnSight ? TEXT("on sight") : TEXT("out of sight"), Id);
		bIsOnSight = bInIsOnSight;
	}
}

bool APICOSpatialContainer::AddActorToManualList(AActor* Actor)
{
	if (Actor == nullptr || !Actor->IsValidLowLevel())
	{
		return false;
	}

	if (ManualActorList.Contains(Actor))
	{
		return false;
	}

	ManualActorList.Add(Actor);
	
	// Subscribe to actor's Destroyed event for automatic cleanup
	// OnDestroyed is called earlier than OnEndPlay and is more suitable for cleanup
	if (!Actor->OnDestroyed.Contains(this, FName("OnManagedActorDestroyed")))
	{
		Actor->OnDestroyed.AddDynamic(this, &APICOSpatialContainer::OnManagedActorDestroyed);
	}
	
	return true;
}

bool APICOSpatialContainer::RemoveActorFromManualList(AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	// Unsubscribe from Destroyed event
	if (IsValid(Actor))
	{
		Actor->OnDestroyed.RemoveDynamic(this, &APICOSpatialContainer::OnManagedActorDestroyed);
	}

	return ManualActorList.Remove(Actor) > 0;
}

void APICOSpatialContainer::ClearManualActorList()
{
	// Unsubscribe from all actor Destroyed events before clearing
	for (const TObjectPtr<AActor>& ActorPtr : ManualActorList)
	{
		if (AActor* Actor = ActorPtr.Get())
		{
			if (IsValid(Actor))
			{
				Actor->OnDestroyed.RemoveDynamic(this, &APICOSpatialContainer::OnManagedActorDestroyed);
			}
		}
	}
	
	ManualActorList.Empty();
}

int32 APICOSpatialContainer::CleanInvalidActorsFromList()
{
	if (!bUseManualActorList || ManualActorList.Num() == 0)
	{
		return 0;
	}

	const int32 OriginalCount = ManualActorList.Num();
	
	// Remove null or invalid actors and unsubscribe from their events
	ManualActorList.RemoveAll([this](const TObjectPtr<AActor>& ActorPtr)
	{
		AActor* Actor = ActorPtr.Get();
		const bool bIsInvalid = !IsValid(Actor) || Actor->IsUnreachable() || Actor->IsPendingKillPending();
		
		// Unsubscribe from Destroyed if actor is still valid but being removed
		if (!bIsInvalid && Actor != nullptr)
		{
			Actor->OnDestroyed.RemoveDynamic(this, &APICOSpatialContainer::OnManagedActorDestroyed);
		}
		
		return bIsInvalid;
	});

	const int32 RemovedCount = OriginalCount - ManualActorList.Num();
	
	if (RemovedCount > 0)
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("Cleaned %d invalid actor(s) from manual list in container '%s'"), 
			RemovedCount, *GetName());
	}

	return RemovedCount;
}

void APICOSpatialContainer::OnManagedActorDestroyed(AActor* Actor)
{
	// Proactively remove actor from list when it's being destroyed
	// This happens immediately when actor is destroyed, before the next Tick cleanup
	if (bUseManualActorList && Actor != nullptr)
	{
		const int32 RemovedCount = ManualActorList.Remove(Actor);
		
		if (RemovedCount > 0)
		{
			UE_LOG(LogPICOSpatial, Verbose, 
				TEXT("Container '%s': Proactively removed destroyed actor '%s' from manual list"),
				*GetName(), *Actor->GetName());
		}
		
		// Event is automatically unbound when actor is destroyed, but we can explicitly remove it for safety
		Actor->OnDestroyed.RemoveDynamic(this, &APICOSpatialContainer::OnManagedActorDestroyed);
	}
}
