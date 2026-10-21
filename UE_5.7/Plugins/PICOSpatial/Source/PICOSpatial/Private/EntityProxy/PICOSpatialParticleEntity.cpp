// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialParticleEntity.h"

#include "PICOSpatial.h"
#include "PICOSpatialNiagaraAsset.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraSystemInstanceController.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "UObject/Package.h"

namespace
{
// Maps the simplified PICO facing mode onto Copenhagen's split orientation/alignment API.
// CameraPosition: set DEFAULT orientation and FACING alignment.
// CameraPlane: set VIEW alignment only, keep the current orientation untouched.
// Free: set FREE orientation only, keep the current alignment untouched.
void ApplyFacingModeToCopenhagenParticle(
	spatialpack::CopenhagenParticle& CopenhagenParticle,
	ENiagaraPICO_FacingMode FacingMode,
	spatialpack::CopenhagenParticleOrientationMode DefaultOrientationMode)
{
	switch (FacingMode)
	{
	case ENiagaraPICO_FacingMode::CameraPosition:
		CopenhagenParticle.setOrientationMode(spatialpack::CopenhagenParticleOrientationMode::DEFAULT);
		CopenhagenParticle.setRenderAlignment(spatialpack::CopenhagenParticleRenderAlignment::FACING);
		break;
	case ENiagaraPICO_FacingMode::CameraPlane:
		CopenhagenParticle.setRenderAlignment(spatialpack::CopenhagenParticleRenderAlignment::VIEW);
		break;
	case ENiagaraPICO_FacingMode::Free:
		CopenhagenParticle.setOrientationMode(spatialpack::CopenhagenParticleOrientationMode::FREE);
		break;
	default:
		CopenhagenParticle.setOrientationMode(DefaultOrientationMode);
		break;
	}
}
}

void FParticleEntityProxy::ConstructParticleEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (UNiagaraComponent* NiagaraComponent = Cast<UNiagaraComponent>(SceneComponent))
	{
		Name = NiagaraComponent->GetName();
		UNiagaraSystem* NiagaraSystem = NiagaraComponent->GetAsset();
		if (!NiagaraSystem)
		{
			LoadedAssetObjectPtr = nullptr;
			UE_LOG(LogPICOSpatial, Warning, TEXT("ParticleEntity '%s' has no Niagara system asset."), *Name);
			return;
		}

		FString AssetName = TEXT("/Game/AssetsPICO") + NiagaraSystem->GetPathName();
		UE_LOG(LogPICOSpatial, Verbose, TEXT("ParticleEntity AssetName:%s"), *AssetName);
		UE_LOG(LogPICOSpatial, Verbose, TEXT("LoadNiagaraPropertiesFromAsset Start"));
		LoadNiagaraPropertiesFromAsset(AssetName);
		UE_LOG(LogPICOSpatial, Verbose, TEXT("LoadNiagaraPropertiesFromAsset End"));
		ParticlePosition = ToSpVector(NiagaraComponent->GetComponentLocation(),100.f);
	}
}

FParticleEntityProxy::FParticleEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	: FPrimitiveEntityProxy(InParentEntityProxy, SceneComponent)
{
	ConstructParticleEntityData(SceneComponent, true);
}

void FParticleEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FPrimitiveEntityProxy::UpdateEntityProxy(SceneComponent);
}

bool FParticleEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FPrimitiveEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.IsValid() && LoadedAssetObjectPtr)
		{
#if PLATFORM_ANDROID
			for (auto& Pair : LoadedAssetObjectPtr->GuidToEmittersMap)
			{
				const FNiagaraEmitterPICO& NiagaraEmitterPICO = Pair.Value;
				if (NiagaraEmitterPICO.ParticleType != ENiagaraPICO_ParticleType::MainParticle)
				{
					continue;
				}

				TSharedPtr<FSubParticleEntityProxy, ESPMode::ThreadSafe> SubEntity = CreateSubEntity(InCurrentScene,SpatialContainer);
				FNativeEntity* Entity = SubEntity->SubNativeEntity.Get();
				if (Entity && Entity->entity.isValid())
				{
					UE_LOG(LogPICOSpatial, Verbose, TEXT("Created SubEntity Successfully!"));
				}

				UE_LOG(LogPICOSpatial, Verbose, TEXT("EmitterGuid: %s"), *NiagaraEmitterPICO.EmitterID.ToString());

				auto CopenhagenParticleComponent = Entity->entity.getOrAddCopenhagenParticleComponent();
				auto TransformComponent = Entity->entity.getOrAddTransformComponent();
				
				//Direction
				if (NiagaraEmitterPICO.Velocity.Size())
				{
					FVector3f Direction = NiagaraEmitterPICO.Velocity / NiagaraEmitterPICO.Velocity.Size();
					UE_LOG(LogPICOSpatial, Verbose, TEXT("EmitDirection:%s"), *Direction.ToString());
					CopenhagenParticleComponent.setBirthDirection(spatialpack::CopenhagenParticleBirthDirection::LOCAL);
					CopenhagenParticleComponent.setEmitDirection(ToSpVector(Direction));
				}

				//Shape
				CopenhagenParticleComponent.setShapeModuleEnabled(true);

				switch (NiagaraEmitterPICO.ShapeType)
				{
				case ENiagaraPICO_ShapeType::Unset:
					{
						CopenhagenParticleComponent.setShapeModuleEnabled(false);

						auto CurrentRotation = TransformComponent.getRotation();
						FQuat NewCurrentRotation = FQuat(FVector(0, 0, 1), PI / 2) * ToFQuat(CurrentRotation);
						TransformComponent.setRotation(ToSpQuat(NewCurrentRotation));
					}
					break;
				case ENiagaraPICO_ShapeType::Sphere:
					{
						UE_LOG(LogPICOSpatial, Verbose, TEXT("Sphere Shape"));

						CopenhagenParticleComponent.setShapeType(spatialpack::CopenhagenParticleEmitterShapeType::SPHERE);
						//Sphere: A sphere with a radius of 1 m and the origin as its center.
						CopenhagenParticleComponent.setShapeScale(ToSpVector(NiagaraEmitterPICO.NonUniformScale * NiagaraEmitterPICO.SphereRadius,100.0f));

						auto CurrentRotation = TransformComponent.getRotation();
						FQuat NewCurrentRotation = FQuat(FVector(0, 0, 1), PI / 2) * ToFQuat(CurrentRotation);
						TransformComponent.setRotation(ToSpQuat(NewCurrentRotation));
					}
					break;
				case ENiagaraPICO_ShapeType::Cylinder:
					{
						UE_LOG(LogPICOSpatial, Verbose, TEXT("Cylinder Shape"));

						//Cylinder: A cone with a base radius of 1 m and a height of 2 m, and the origin is the midpoint of the axis line.
						CopenhagenParticleComponent.setShapeType(spatialpack::CopenhagenParticleEmitterShapeType::CYLINDER);
						FVector3f Size = FVector3f(NiagaraEmitterPICO.NonUniformScale.X * NiagaraEmitterPICO.CylinderRadius,NiagaraEmitterPICO.NonUniformScale.Y * NiagaraEmitterPICO.CylinderRadius,NiagaraEmitterPICO.NonUniformScale.Z * NiagaraEmitterPICO.CylinderHeight / 2);
						CopenhagenParticleComponent.setShapeScale(ToSpVector(Size,100.0f));

						auto CurrentRotation = TransformComponent.getRotation();
						FQuat NewCurrentRotation = FQuat(FVector(0, 0, 1), PI / 2) * ToFQuat(CurrentRotation);
						TransformComponent.setRotation(ToSpQuat(NewCurrentRotation));
					}
					break;
				case ENiagaraPICO_ShapeType::BoxOrPlane:
					{
						UE_LOG(LogPICOSpatial, Verbose, TEXT("%s Shape"),NiagaraEmitterPICO.BoxOrPlaneMode == ENiagaraPICO_BoxOrPlaneMode::Box ? TEXT("Box") : TEXT("Plane"));

						float UniformSize = NiagaraEmitterPICO.BoxOrPlaneMode == ENiagaraPICO_BoxOrPlaneMode::Box ? NiagaraEmitterPICO.BoxSize.X : NiagaraEmitterPICO.PlaneSize.X;
						//Plane: A plane with a side length of 2m, and the origin is the center point of the plane.
						CopenhagenParticleComponent.setShapeType(NiagaraEmitterPICO.BoxOrPlaneMode == ENiagaraPICO_BoxOrPlaneMode::Box ? spatialpack::CopenhagenParticleEmitterShapeType::BOX : spatialpack::CopenhagenParticleEmitterShapeType::PLANE);
						CopenhagenParticleComponent.setShapeScale(ToSpVector(NiagaraEmitterPICO.NonUniformScale * UniformSize / 2,100.0f));

						auto CurrentRotation = TransformComponent.getRotation();
						FQuat NewCurrentRotation = FQuat(FVector(0, 0, 1), PI / 2) * ToFQuat(CurrentRotation);
						TransformComponent.setRotation(ToSpQuat(NewCurrentRotation));
					}
					break;
				case ENiagaraPICO_ShapeType::Torus:
					{
						UE_LOG(LogPICOSpatial, Verbose, TEXT("Torus Shape"));

						CopenhagenParticleComponent.setShapeType(spatialpack::CopenhagenParticleEmitterShapeType::TORUS);
						CopenhagenParticleComponent.setShapeTorusRadius(NiagaraEmitterPICO.TorusHandleRadius / 100.0f);
						CopenhagenParticleComponent.setShapeScale(ToSpVector(NiagaraEmitterPICO.NonUniformScale * NiagaraEmitterPICO.TorusLargeRadius,100.0f));

						auto Rotation = TransformComponent.getRotation();
						FQuat NewRotation = FQuat(FVector(1, 0, 0), PI / 2) * ToFQuat(Rotation);
						TransformComponent.setRotation(ToSpQuat(NewRotation));
					}
					break;
				case ENiagaraPICO_ShapeType::Ring:
					{
						UE_LOG(LogPICOSpatial, Error, TEXT("Ring shape type is not supported"));
					}
					break;
				case ENiagaraPICO_ShapeType::Cone:
					{
						UE_LOG(LogPICOSpatial, Verbose, TEXT("Cone Shape"));

						float ConeRadius = CalculateConeRadius(NiagaraEmitterPICO.ShapeConeLength,NiagaraEmitterPICO.ShapeConeAngle);

						UE_LOG(LogPICOSpatial, Verbose, TEXT("ConeRadius:%f"),ConeRadius);
						
						CopenhagenParticleComponent.setShapeType(spatialpack::CopenhagenParticleEmitterShapeType::CONE);
						FVector3f Size = FVector3f(NiagaraEmitterPICO.NonUniformScale.X * ConeRadius,NiagaraEmitterPICO.NonUniformScale.Y * ConeRadius,NiagaraEmitterPICO.NonUniformScale.Z * NiagaraEmitterPICO.ShapeConeLength / 2);

						UE_LOG(LogPICOSpatial, Verbose, TEXT("ConeSize:%s"), * Size.ToString());

						CopenhagenParticleComponent.setShapeScale(ToSpVector(Size,100.0f));
						CopenhagenParticleComponent.setShapeArc(NiagaraEmitterPICO.ConeRadialAngle);
						
						auto Rotation = TransformComponent.getRotation();
						FQuat NewRotation = FQuat(FVector(1, 0, 0), PI) * ToFQuat(Rotation);
						NewRotation*=FQuat(FVector(0, 0, 1), PI);
						TransformComponent.setRotation(ToSpQuat(NewRotation));

						auto Position = TransformComponent.getPosition();
						Position.y+=NiagaraEmitterPICO.ShapeConeLength / 2/100.0f;
						TransformComponent.setPosition(Position);
					}
					break;
				}

				if (NiagaraEmitterPICO.VelocityMode == ENiagaraPICO_VelocityMode::FromPoint)
				{
					CopenhagenParticleComponent.setShapeModuleEnabled(true);
					CopenhagenParticleComponent.setShapeType(spatialpack::CopenhagenParticleEmitterShapeType::POINT);
				}

				auto EmissionBurst = CopenhagenParticleComponent.getEmissionBurstCount();
				EmissionBurst.setCurveType(spatialpack::CopenhagenCurveType::CONSTANT);
				EmissionBurst.setConstant(NiagaraEmitterPICO.BurstSpawnCount);
				CopenhagenParticleComponent.setEmissionBurstCount(EmissionBurst);


				switch (NiagaraEmitterPICO.LifeCycleMode) {
				case ENiagaraPICO_LifeCycleMode::System:
					{
						if (LoadedAssetObjectPtr->bUseLoopDelay)
						{
							CopenhagenParticleComponent.setEmissionBurstInterval(LoadedAssetObjectPtr->LoopDelay);
						}
						else
						{
							CopenhagenParticleComponent.setEmissionBurstInterval(0);
						}
						CopenhagenParticleComponent.setDuration(LoadedAssetObjectPtr->LoopDuration);
				
						switch (LoadedAssetObjectPtr->LoopBehavior)
						{
						case ENiagaraPICO_LoopBehavior::Infinite:
							CopenhagenParticleComponent.setEmissionBurstCycles(-1);
							UE_LOG(LogPICOSpatial, Verbose, TEXT("ENiagaraPICO_LoopBehavior::Infinite"));
							CopenhagenParticleComponent.setLooping(true);
							
							break;
						case ENiagaraPICO_LoopBehavior::Once:
							CopenhagenParticleComponent.setEmissionBurstCycles(1);
							UE_LOG(LogPICOSpatial, Verbose, TEXT("ENiagaraPICO_LoopBehavior::Once"));
							CopenhagenParticleComponent.setLooping(false);

							break;
						case ENiagaraPICO_LoopBehavior::Multiple:
							CopenhagenParticleComponent.setEmissionBurstCycles(NiagaraEmitterPICO.LoopCount);
							UE_LOG(LogPICOSpatial, Verbose, TEXT("ENiagaraPICO_LoopBehavior::Multiple"));
							CopenhagenParticleComponent.setLooping(false);
							
							break;
						}
					}
					break;
				case ENiagaraPICO_LifeCycleMode::Self:
					{
						if (NiagaraEmitterPICO.bUseLoopDelay)
						{
							CopenhagenParticleComponent.setEmissionBurstInterval(NiagaraEmitterPICO.LoopDelay);
						}
						else
						{
							CopenhagenParticleComponent.setEmissionBurstInterval(0);
						}
						CopenhagenParticleComponent.setDuration(NiagaraEmitterPICO.LoopDuration);
						UE_LOG(LogPICOSpatial, Verbose, TEXT("setEmissionBurstCount:%d"),NiagaraEmitterPICO.BurstSpawnCount);

				
						switch (NiagaraEmitterPICO.LoopBehavior)
						{
						case ENiagaraPICO_LoopBehavior::Infinite:
							CopenhagenParticleComponent.setEmissionBurstCycles(-1);
							UE_LOG(LogPICOSpatial, Verbose, TEXT("ENiagaraPICO_LoopBehavior::Infinite"));
							CopenhagenParticleComponent.setLooping(true);

							break;
						case ENiagaraPICO_LoopBehavior::Once:
							CopenhagenParticleComponent.setEmissionBurstCycles(1);
							UE_LOG(LogPICOSpatial, Verbose, TEXT("ENiagaraPICO_LoopBehavior::Once"));
							CopenhagenParticleComponent.setLooping(false);
							break;
						case ENiagaraPICO_LoopBehavior::Multiple:
							CopenhagenParticleComponent.setEmissionBurstCycles(LoadedAssetObjectPtr->LoopCount);
							UE_LOG(LogPICOSpatial, Verbose, TEXT("ENiagaraPICO_LoopBehavior::Multiple"));
							CopenhagenParticleComponent.setLooping(false);
							
							break;
						}
					}
					break;
				}
				
				

				//Spawning
				auto EmissionRate = CopenhagenParticleComponent.getEmissionRateOverTime();
				EmissionRate.setCurveType(spatialpack::CopenhagenCurveType::CONSTANT);
				EmissionRate.setConstant(NiagaraEmitterPICO.SpawnRate);
				CopenhagenParticleComponent.setEmissionRateOverTime(EmissionRate);

				//SubparticleSpawnMode
				CopenhagenParticleComponent.setSubparticleSpawnMode(spatialpack::CopenhagenSubparticleSpawnMode::ON_DEATH);

				//Main Emitter
				auto CopenhagenParticle = CopenhagenParticleComponent.getMainParticle();

				uint32 MaxParticles = 0;
				const float MaxParticleLifeTime = NiagaraEmitterPICO.LifeTimeMode == ENiagaraPICO_LifeTimeMode::Random
					? NiagaraEmitterPICO.LifeTimeMax
					: NiagaraEmitterPICO.LifeTime;
				float BurstParticleCount = 0.0f;
				const float EffectiveLoopDuration = NiagaraEmitterPICO.LifeCycleMode == ENiagaraPICO_LifeCycleMode::System
					? LoadedAssetObjectPtr->LoopDuration
					: NiagaraEmitterPICO.LoopDuration;
				if (NiagaraEmitterPICO.BurstSpawnCount > 0)
				{
					int32 BurstOverlapCount = 1;
					if (EffectiveLoopDuration > SMALL_NUMBER && MaxParticleLifeTime > SMALL_NUMBER)
					{
						BurstOverlapCount = FMath::Max(1, FMath::CeilToInt32(MaxParticleLifeTime / EffectiveLoopDuration));
					}
					else if (EffectiveLoopDuration <= SMALL_NUMBER)
					{
						UE_LOG(LogPICOSpatial, Warning,
							TEXT("Emitter '%s' has invalid LoopDuration %f, use one burst contribution for MaxParticles."),
							*NiagaraEmitterPICO.EmitterName, EffectiveLoopDuration);
					}
					BurstParticleCount = NiagaraEmitterPICO.BurstSpawnCount * BurstOverlapCount;
				}

				switch (NiagaraEmitterPICO.LifeTimeMode) {
				case ENiagaraPICO_LifeTimeMode::DirectSet:
					{
						MaxParticles = static_cast<uint32>(FMath::Max(0, FMath::CeilToInt32(NiagaraEmitterPICO.SpawnRate * NiagaraEmitterPICO.LifeTime + BurstParticleCount)));
					}
					break;
				case ENiagaraPICO_LifeTimeMode::Random:
					{
						
						MaxParticles = static_cast<uint32>(FMath::Max(0, FMath::CeilToInt32(NiagaraEmitterPICO.SpawnRate * NiagaraEmitterPICO.LifeTimeMax + BurstParticleCount)));
					}
					break;
				}
					
				CopenhagenParticle.setMaxParticles(MaxParticles);

				UE_LOG(LogPICOSpatial, Verbose, TEXT("Main MaxParticles:%d"),MaxParticles);
				
				NiagaraEmitterPICOToCopenhagenParticle(NiagaraEmitterPICO, CopenhagenParticle);
				
				//Sub Emitter
				if (LoadedAssetObjectPtr->ParentToSubGuidMap.Contains(NiagaraEmitterPICO.EmitterID))
				{
					const FGuid SubGuid = LoadedAssetObjectPtr->ParentToSubGuidMap[NiagaraEmitterPICO.EmitterID];
					if (!LoadedAssetObjectPtr->GuidToEmittersMap.Contains(SubGuid))
					{
						UE_LOG(LogPICOSpatial, Warning, TEXT("Sub emitter guid '%s' is missing from emitters map."), *SubGuid.ToString());
						continue;
					}

					const FNiagaraEmitterPICO& SubNiagaraEmitterPICO = LoadedAssetObjectPtr->GuidToEmittersMap[SubGuid];
					auto SubCopenhagenParticle = CopenhagenParticleComponent.getSubParticle();

					uint32 SubMaxParticles = SubNiagaraEmitterPICO.SpawnNumsByEvent * MaxParticles;
					
					SubCopenhagenParticle.setMaxParticles(SubMaxParticles);
					UE_LOG(LogPICOSpatial, Verbose, TEXT("Sub MaxParticles:%d"),SubMaxParticles);

					auto SubEmitterEmissionBurstCount = CopenhagenParticleComponent.getSubEmitterEmissionBurstCount();
					SubEmitterEmissionBurstCount.setCurveType(spatialpack::CopenhagenCurveType::CONSTANT);
					SubEmitterEmissionBurstCount.setConstant(SubNiagaraEmitterPICO.SpawnNumsByEvent);
					CopenhagenParticleComponent.setSubEmitterEmissionBurstCount(SubEmitterEmissionBurstCount);
					UE_LOG(LogPICOSpatial, Verbose, TEXT("SubEmitterEmissionBurstCount:%d"),SubNiagaraEmitterPICO.SpawnNumsByEvent);
					
					SubCopenhagenParticle.setEnabled(true);
					UE_LOG(LogPICOSpatial, Verbose, TEXT("SubEmitterGuid: %s"), *SubNiagaraEmitterPICO.EmitterID.ToString());
					
					NiagaraEmitterPICOToCopenhagenParticle(SubNiagaraEmitterPICO, SubCopenhagenParticle);
				}
			}
#endif // PLATFORM_ANDROID
		}
		return true;
	}
	return false;
}

void FParticleEntityProxy::UpdateNativeEntity()
{
	FPrimitiveEntityProxy::UpdateNativeEntity();
}

void FParticleEntityProxy::LoadNiagaraPropertiesFromAsset(const FString& PackagePath)
{
	LoadedAssetObjectPtr = Cast<UPICOSpatialNiagaraAsset>(StaticLoadObject(UPICOSpatialNiagaraAsset::StaticClass(), nullptr, *PackagePath));
	if (LoadedAssetObjectPtr)
	{
		if (!LoadedAssetObjectPtr->GuidToEmittersMap.Num())
		{
			UE_LOG(LogPICOSpatial, Error, TEXT("LoadedAsset->GuidToEmittersMap.Num ==0"));
		}
	}
	else
	{
		UE_LOG(LogPICOSpatial, Error, TEXT("Failed to load Custom Niagara Asset."));
	}
}

void FParticleEntityProxy::HandleSizeOverLifetime(const FNiagaraEmitterPICO& NiagaraEmitterPICO, spatialpack::CopenhagenParticle& CopenhagenParticle, spatialpack::CopenhagenCurveType CurveType)
{
#if PLATFORM_ANDROID
	CopenhagenParticle.setSizeOverLifetimeModuleEnabled(true);
	auto sizeOverLife = CopenhagenParticle.getSizeOverLifetime();
	sizeOverLife.setCurveType(CurveType);
	sizeOverLife.setCurvePreWrapMode(spatialpack::CopenhagenCurveWrapMode::LOOP);
	sizeOverLife.setCurvePostWrapMode(spatialpack::CopenhagenCurveWrapMode::PINGPONG);
	sizeOverLife.clearCurveKeys();

	FNiagaraCurvePICO NiagaraCurvePICO;
	float UpdateScaleFactor = 1.0f;
	GetUpdateScaleFactorOfEmitter(NiagaraEmitterPICO, UpdateScaleFactor);
	
	switch (NiagaraEmitterPICO.MainRendererType)
	{
	case ENiagaraPICO_RendererType::Sprite:
		{
			NiagaraCurvePICO = NiagaraEmitterPICO.SizeOverLifeCurve;
		}
		break;
	case ENiagaraPICO_RendererType::Mesh:
		{
			NiagaraCurvePICO = NiagaraEmitterPICO.MeshSizeOverLifeCurve.XCurve;
			UE_LOG(LogPICOSpatial, Verbose, TEXT("Non-Uniform mesh curves not supported"));
		}
		break;
	case ENiagaraPICO_RendererType::Ribbon:
		break;
	}

	switch (CurveType)
	{
	case spatialpack::CopenhagenCurveType::CURVE:
		{
			float InitializeSize = 0.0f;
			GetInitializeSizeOfEmitter(NiagaraEmitterPICO, InitializeSize);
			for (auto RichKey : NiagaraCurvePICO.Keys)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("SizeOverLifeCurve Time:%f"), RichKey.Time);
				float KeyValue = RichKey.Value * InitializeSize * UpdateScaleFactor;
				UE_LOG(LogPICOSpatial, Verbose, TEXT("SizeOverLifeCurve KeyValue:%f"), KeyValue);
				spatialpack::Vector2f Key = {RichKey.Time, KeyValue};
				sizeOverLife.addCurveKey(
					Key, Key, Key,
					spatialpack::CopenhagenCurveInterpType::BEZIER, spatialpack::CopenhagenCurveInterpType::BEZIER);
			}
		}
		break;
	case spatialpack::CopenhagenCurveType::STRIP:
		{
			float InitializeMaxSize = 0.0f;
			float InitializeMinSize = 0.0f;
			GetInitializeRandomSizeOfEmitter(NiagaraEmitterPICO, InitializeMaxSize, InitializeMinSize);
			for (auto RichKey : NiagaraCurvePICO.Keys)
			{
				spatialpack::Vector2f Key = {RichKey.Time, RichKey.Value * InitializeMinSize * UpdateScaleFactor};
				sizeOverLife.addMinCurveKey(
					Key, Key, Key,
					spatialpack::CopenhagenCurveInterpType::BEZIER, spatialpack::CopenhagenCurveInterpType::BEZIER);
			}
			for (auto RichKey : NiagaraCurvePICO.Keys)
			{
				spatialpack::Vector2f Key = {RichKey.Time, RichKey.Value * InitializeMaxSize * UpdateScaleFactor};
				sizeOverLife.addMaxCurveKey(
					Key, Key, Key,
					spatialpack::CopenhagenCurveInterpType::BEZIER, spatialpack::CopenhagenCurveInterpType::BEZIER);
			}
		}
		break;
	default:
		break;
	}

	//sizeOverLife.setCurveMultiplier(NiagaraEmitterPICO.MeshNonUniformScaleFactor.X);
	CopenhagenParticle.setSizeOverLifetime(sizeOverLife);
#endif
	
}

void FParticleEntityProxy::HandleParticleSize(const FNiagaraEmitterPICO& NiagaraEmitterPICO, spatialpack::CopenhagenParticle& CopenhagenParticle)
{
#if PLATFORM_ANDROID
	spatialpack::CopenhagenCurveType CurveType = spatialpack::CopenhagenCurveType::CONSTANT;

	switch (NiagaraEmitterPICO.MainRendererType)
	{
	case ENiagaraPICO_RendererType::Sprite:
		{
			if ((NiagaraEmitterPICO.SizeMode == ENiagaraPICO_SizeMode::Uniform
					|| NiagaraEmitterPICO.SizeMode == ENiagaraPICO_SizeMode::NonUniform)
					&& (NiagaraEmitterPICO.ScaleSizeMode != ENiagaraPICO_ScaleSizeMode::UniformCurve
					&& NiagaraEmitterPICO.ScaleSizeMode != ENiagaraPICO_ScaleSizeMode::NonUniformCurve))
			{
				CurveType = spatialpack::CopenhagenCurveType::CONSTANT;
			}
			else if ((NiagaraEmitterPICO.SizeMode == ENiagaraPICO_SizeMode::RandomUniform
					|| NiagaraEmitterPICO.SizeMode == ENiagaraPICO_SizeMode::RandomNonUniform)
					&& (NiagaraEmitterPICO.ScaleSizeMode != ENiagaraPICO_ScaleSizeMode::UniformCurve
					&& NiagaraEmitterPICO.ScaleSizeMode != ENiagaraPICO_ScaleSizeMode::NonUniformCurve))
			{
				CurveType = spatialpack::CopenhagenCurveType::RANDOM;
			}
			else if ((NiagaraEmitterPICO.SizeMode == ENiagaraPICO_SizeMode::Uniform
					|| NiagaraEmitterPICO.SizeMode == ENiagaraPICO_SizeMode::NonUniform)
					&& (NiagaraEmitterPICO.ScaleSizeMode == ENiagaraPICO_ScaleSizeMode::UniformCurve
					|| NiagaraEmitterPICO.ScaleSizeMode == ENiagaraPICO_ScaleSizeMode::NonUniformCurve))
			{
				CurveType = spatialpack::CopenhagenCurveType::CURVE;
			}
			else if ((NiagaraEmitterPICO.SizeMode == ENiagaraPICO_SizeMode::RandomUniform
					|| NiagaraEmitterPICO.SizeMode == ENiagaraPICO_SizeMode::RandomNonUniform)
				&& (NiagaraEmitterPICO.ScaleSizeMode == ENiagaraPICO_ScaleSizeMode::UniformCurve
					|| NiagaraEmitterPICO.ScaleSizeMode == ENiagaraPICO_ScaleSizeMode::NonUniformCurve))
			{
				CurveType = spatialpack::CopenhagenCurveType::STRIP;
			}
		}
		break;
	case ENiagaraPICO_RendererType::Mesh:
		{
			if ((NiagaraEmitterPICO.MeshScaleInitializeMode == ENiagaraPICO_SizeMode::Uniform
					|| NiagaraEmitterPICO.MeshScaleInitializeMode == ENiagaraPICO_SizeMode::NonUniform)
					&& (NiagaraEmitterPICO.MeshScaleUpdateMode != ENiagaraPICO_ScaleSizeMode::UniformCurve
					&&  NiagaraEmitterPICO.MeshScaleUpdateMode != ENiagaraPICO_ScaleSizeMode::NonUniformCurve))
			{
				CurveType = spatialpack::CopenhagenCurveType::CONSTANT;
			}
			else if ((NiagaraEmitterPICO.MeshScaleInitializeMode == ENiagaraPICO_SizeMode::RandomUniform
					|| NiagaraEmitterPICO.MeshScaleInitializeMode == ENiagaraPICO_SizeMode::RandomNonUniform)
					&& (NiagaraEmitterPICO.MeshScaleUpdateMode != ENiagaraPICO_ScaleSizeMode::UniformCurve
					&& NiagaraEmitterPICO.MeshScaleUpdateMode != ENiagaraPICO_ScaleSizeMode::NonUniformCurve))
			{
				CurveType = spatialpack::CopenhagenCurveType::RANDOM;
			}
			else if ((NiagaraEmitterPICO.MeshScaleInitializeMode == ENiagaraPICO_SizeMode::Uniform
					|| NiagaraEmitterPICO.MeshScaleInitializeMode == ENiagaraPICO_SizeMode::NonUniform)
				&& (NiagaraEmitterPICO.MeshScaleUpdateMode == ENiagaraPICO_ScaleSizeMode::UniformCurve
					|| NiagaraEmitterPICO.MeshScaleUpdateMode == ENiagaraPICO_ScaleSizeMode::NonUniformCurve))
			{
				CurveType = spatialpack::CopenhagenCurveType::CURVE;
			}
			else if ((NiagaraEmitterPICO.MeshScaleInitializeMode == ENiagaraPICO_SizeMode::RandomUniform
					|| NiagaraEmitterPICO.MeshScaleInitializeMode == ENiagaraPICO_SizeMode::RandomNonUniform)
				&& (NiagaraEmitterPICO.MeshScaleUpdateMode == ENiagaraPICO_ScaleSizeMode::UniformCurve
					|| NiagaraEmitterPICO.MeshScaleUpdateMode == ENiagaraPICO_ScaleSizeMode::NonUniformCurve))
			{
				CurveType = spatialpack::CopenhagenCurveType::STRIP;
			}
		}
		break;
	case ENiagaraPICO_RendererType::Ribbon:
		break;
	}

	UE_LOG(LogPICOSpatial, Verbose, TEXT("Final CurveType: %d"), CurveType);

	float UpdateScaleFactor = 1.0f;
	GetUpdateScaleFactorOfEmitter(NiagaraEmitterPICO, UpdateScaleFactor);

	UE_LOG(LogPICOSpatial, Verbose, TEXT("Final UpdateScaleFactor: %f"), UpdateScaleFactor);

	switch (CurveType)
	{
	case spatialpack::CopenhagenCurveType::CONSTANT:
		{
			auto StartSize = CopenhagenParticle.getStartSize();
			StartSize.setCurveType(CurveType);
			float InitializeSize = 0.0f;
			GetInitializeSizeOfEmitter(NiagaraEmitterPICO, InitializeSize);
			InitializeSize *= UpdateScaleFactor;
			UE_LOG(LogPICOSpatial, Verbose, TEXT("CopenhagenCurveType::CONSTANT StartSize:%f"), InitializeSize);

			StartSize.setConstant(InitializeSize);
			CopenhagenParticle.setStartSize(StartSize);
		}
		break;
	case spatialpack::CopenhagenCurveType::RANDOM:
		{
			auto StartSize = CopenhagenParticle.getStartSize();
			StartSize.setCurveType(CurveType);
			float InitializeMaxSize = 0.0f;
			float InitializeMinSize = 0.0f;
			GetInitializeRandomSizeOfEmitter(NiagaraEmitterPICO, InitializeMaxSize, InitializeMinSize);
			InitializeMaxSize *= UpdateScaleFactor;
			InitializeMinSize *= UpdateScaleFactor;
			UE_LOG(LogPICOSpatial, Verbose, TEXT("CopenhagenCurveType::RANDOM InitializeMaxSize:%f"), InitializeMaxSize);
			UE_LOG(LogPICOSpatial, Verbose, TEXT("CopenhagenCurveType::RANDOM InitializeMaxSize:%f"), InitializeMinSize);

			StartSize.setRandomRange({InitializeMinSize, InitializeMaxSize});
			CopenhagenParticle.setStartSize(StartSize);
		}
		break;
	case spatialpack::CopenhagenCurveType::CURVE:
	case spatialpack::CopenhagenCurveType::STRIP:
		{
			HandleSizeOverLifetime(NiagaraEmitterPICO, CopenhagenParticle, CurveType);
		}
		break;
	default:
		break;
	}
#endif
	
}

void FParticleEntityProxy::GetInitializeSizeOfEmitter(const FNiagaraEmitterPICO& NiagaraEmitterPICO, float& InitializeSizeOrScale)
{
	switch (NiagaraEmitterPICO.MainRendererType)
	{
	case ENiagaraPICO_RendererType::Sprite:
		{
			switch (NiagaraEmitterPICO.SizeMode)
			{
			case ENiagaraPICO_SizeMode::Uniform:
				{
					InitializeSizeOrScale = NiagaraEmitterPICO.UniformSpriteSize / 100.0f;
				}
				break;
			case ENiagaraPICO_SizeMode::NonUniform:
				{
					InitializeSizeOrScale = NiagaraEmitterPICO.SpriteSize.X / 100.0f;
					UE_LOG(LogPICOSpatial, Warning, TEXT("NonUniform is not currently supported!"));
				}
				break;
			default: ;
			}
		}
		break;
	case ENiagaraPICO_RendererType::Mesh:
		{
			switch (NiagaraEmitterPICO.MeshScaleInitializeMode)
			{
			case ENiagaraPICO_SizeMode::Uniform:
				{
					InitializeSizeOrScale = NiagaraEmitterPICO.MeshUniformScale;
				}
				break;
			case ENiagaraPICO_SizeMode::NonUniform:
				{
					InitializeSizeOrScale = NiagaraEmitterPICO.MeshNonUniformScale.X;
				}
				break;
			default:
				{
					InitializeSizeOrScale = 1.0f;
				}
				break;
			}
		}
		break;
	case ENiagaraPICO_RendererType::Ribbon:
		break;
	}
}

void FParticleEntityProxy::GetUpdateScaleFactorOfEmitter(const FNiagaraEmitterPICO& NiagaraEmitterPICO, float& UpdateScaleFactor)
{
	switch (NiagaraEmitterPICO.MainRendererType)
	{
	case ENiagaraPICO_RendererType::Sprite:
		{
			UpdateScaleFactor = NiagaraEmitterPICO.UniformScaleFactor * NiagaraEmitterPICO.NonUniformScaleFactor.X;
		}
		break;
	case ENiagaraPICO_RendererType::Mesh:
		{
			UpdateScaleFactor = NiagaraEmitterPICO.MeshNonUniformScaleFactor.X;
		}
		break;
	case ENiagaraPICO_RendererType::Ribbon:
		break;
	}
}

void FParticleEntityProxy::GetInitializeRandomSizeOfEmitter(const FNiagaraEmitterPICO& NiagaraEmitterPICO, float& InitializeMaxSizeOrScale, float& InitializeMinSizeOrScale)
{
	switch (NiagaraEmitterPICO.MainRendererType)
	{
	case ENiagaraPICO_RendererType::Sprite:
		{
			switch (NiagaraEmitterPICO.SizeMode)
			{
			case ENiagaraPICO_SizeMode::RandomUniform:
				{
					InitializeMaxSizeOrScale = NiagaraEmitterPICO.UniformSpriteSizeMax / 100.0f;
					InitializeMinSizeOrScale = NiagaraEmitterPICO.UniformSpriteSizeMin / 100.0f;
				}
				break;
			case ENiagaraPICO_SizeMode::RandomNonUniform:
				{
					InitializeMaxSizeOrScale = NiagaraEmitterPICO.SpriteSizeMax.X / 100.0f;
					InitializeMinSizeOrScale = NiagaraEmitterPICO.SpriteSizeMin.X / 100.0f;
				}
				break;
			default: ;
			}
		}
		break;
	case ENiagaraPICO_RendererType::Mesh:
		{
			switch (NiagaraEmitterPICO.MeshScaleInitializeMode)
			{
			case ENiagaraPICO_SizeMode::RandomUniform:
				{
					InitializeMaxSizeOrScale = NiagaraEmitterPICO.MeshUniformScaleMax;
					InitializeMinSizeOrScale = NiagaraEmitterPICO.MeshUniformScaleMin;
				}
				break;
			case ENiagaraPICO_SizeMode::RandomNonUniform:
				{
					InitializeMaxSizeOrScale = NiagaraEmitterPICO.MeshNonUniformScaleMax.X;
					InitializeMinSizeOrScale = NiagaraEmitterPICO.MeshNonUniformScaleMin.X;
				}
				break;
			default:
				{
					InitializeMaxSizeOrScale = 1.0f;
					InitializeMinSizeOrScale = 1.0f;
				}
				break;
			}
		}
		break;
	case ENiagaraPICO_RendererType::Ribbon:
		break;
	}
}

TSharedPtr<FParticleEntityProxy::FSubParticleEntityProxy, ESPMode::ThreadSafe> FParticleEntityProxy::CreateSubEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	UE_LOG(LogPICOSpatial, Verbose, TEXT("CreateSubEntity Start"));

	TSharedPtr<FSubParticleEntityProxy, ESPMode::ThreadSafe> SubEntity = MakeShared<FSubParticleEntityProxy>();
	SubEntity->ParentEntity = NativeEntity;
	SubEntity->Id = SubEntities.Num();
	FString SubEntityName = Name + TEXT("_") + FString::FromInt(SubEntity->Id);
	SubEntity->Name = SubEntityName;
	SubEntities.Add(SubEntity);

	UE_LOG(LogPICOSpatial, Verbose, TEXT("Created SubEntity Name:%s"), *SubEntity->Name);
	UE_LOG(LogPICOSpatial, Verbose, TEXT("Created SubEntity Id:%d"), SubEntity->Id);
	UE_LOG(LogPICOSpatial, Verbose, TEXT("Current SubEntity Num:%d"), SubEntities.Num());

#if PLATFORM_ANDROID
	SubEntity->SubNativeEntity = MakeShared<FNativeEntity>();
	SubEntity->SubNativeEntity->entity = InCurrentScene->createEntity();
	SubEntity->SubNativeEntity->entity.setName(TCHAR_TO_UTF8( * SubEntityName));
	SubEntity->SubNativeEntity->entity.setParent(NativeEntity.Get()->entity);

	if (SpatialContainer)
	{
		SpatialContainer->addEntity(SubEntity->SubNativeEntity.Get()->entity);
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Add to SpatialContainer Successfully!"));
	}

	SubEntity->SubNativeEntity->entity.getOrAddTransformComponent().setPosition(ToSpVector(FVector{0,0,0}, 100.0f));
	SubEntity->SubNativeEntity->entity.getOrAddTransformComponent().setRotation(ToSpQuat(FQuat::Identity));
	SubEntity->SubNativeEntity->entity.getOrAddTransformComponent().setScale(ToSpVector(FVector::One()));
#endif

	UE_LOG(LogPICOSpatial, Verbose, TEXT("CreateSubEntity End"));

	return SubEntity;
}

void FParticleEntityProxy::NiagaraEmitterPICOToCopenhagenParticle(const FNiagaraEmitterPICO& NiagaraEmitterPICO, spatialpack::CopenhagenParticle& CopenhagenParticle)
{
#if PLATFORM_ANDROID
	//Prepare Resource
	FString RenderMaterialPath;
	switch (NiagaraEmitterPICO.MainRendererType)
	{
	case ENiagaraPICO_RendererType::Sprite:
		{
			if (!NiagaraEmitterPICO.SpriteRenderers.IsValidIndex(0))
			{
				UE_LOG(LogPICOSpatial, Warning, TEXT("No valid sprite renderer!"));
				return;
			}

			const FNiagaraSpriteRendererPICO& SpriteRenderer = NiagaraEmitterPICO.SpriteRenderers[0];
			//RendererMode
			CopenhagenParticle.setRendererMode(spatialpack::CopenhagenParticleRenderMode::BILLBOARD);
			ApplyFacingModeToCopenhagenParticle(
				CopenhagenParticle,
				SpriteRenderer.FacingMode,
				spatialpack::CopenhagenParticleOrientationMode::DEFAULT);
			// Material is optional, so guard package access.
			if (SpriteRenderer.RenderMaterial && SpriteRenderer.RenderMaterial->GetPackage())
			{
				RenderMaterialPath = SpriteRenderer.RenderMaterial->GetPackage()->GetPathName();
				UE_LOG(LogPICOSpatial, Verbose, TEXT("RenderMaterialPath:%s"), *RenderMaterialPath);
			}
			else
			{
				UE_LOG(LogPICOSpatial, Warning, TEXT("Sprite renderer material is null for emitter '%s'."), *NiagaraEmitterPICO.EmitterName);
			}
		}
		break;
	case ENiagaraPICO_RendererType::Mesh:
		{
			if (!NiagaraEmitterPICO.MeshRenders.IsValidIndex(0))
			{
				UE_LOG(LogPICOSpatial, Error, TEXT("No valid Mesh renderer!"));
				return;
			}

			const FNiagaraMeshRendererPICO& MeshRenderer = NiagaraEmitterPICO.MeshRenders[0];
			ApplyFacingModeToCopenhagenParticle(
				CopenhagenParticle,
				MeshRenderer.FacingMode,
				spatialpack::CopenhagenParticleOrientationMode::FREE);
			// MeshResource
			FString RenderMeshPath;
			if (MeshRenderer.Mesh && MeshRenderer.Mesh->GetPackage())
			{
				RenderMeshPath = MeshRenderer.Mesh->GetPackage()->GetPathName();
				UE_LOG(LogPICOSpatial, Verbose, TEXT("RenderMeshPath:%s"), *RenderMeshPath);

				if (UMaterialInterface* MeshMaterial = MeshRenderer.Mesh->GetMaterial(0))
				{
					if (MeshMaterial->GetPackage())
					{
						RenderMaterialPath = MeshMaterial->GetPackage()->GetPathName();
						UE_LOG(LogPICOSpatial, Verbose, TEXT("RenderMeshMaterialPath:%s"), *RenderMaterialPath);
					}
				}
			}
			else
			{
				UE_LOG(LogPICOSpatial, Error, TEXT("Mesh renderer mesh is null for emitter '%s'."), *NiagaraEmitterPICO.EmitterName);
				return;
			}

			if (MeshRenderer.bEnableOverrideMeshMaterial && MeshRenderer.OverrideMeshMaterial && MeshRenderer.OverrideMeshMaterial->GetPackage())
			{
				RenderMaterialPath = MeshRenderer.OverrideMeshMaterial->GetPackage()->GetPathName();
			}
			
			if (!RenderMeshPath.IsEmpty())
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("RenderMeshPath:%s"), *RenderMeshPath);
				spatialpack::MeshResource* MeshResource = nullptr;
				MeshResource = GetCachedPICOSpatial().CacheMeshResource(RenderMeshPath);
				if (MeshResource)
				{
					CopenhagenParticle.setRendererMesh(*MeshResource);
					UE_LOG(LogPICOSpatial, Verbose, TEXT("MeshResource Name:%hs"), MeshResource->getName());
					CopenhagenParticle.setRendererMode(spatialpack::CopenhagenParticleRenderMode::MESH);
				}
				else
				{
					UE_LOG(LogPICOSpatial, Error, TEXT("Failed to cache mesh resource for path '%s'."), *RenderMeshPath);
					return;
				}
			}
		}
		break;
	case ENiagaraPICO_RendererType::Ribbon:
		{
			//TODO:Ribbon is not supported for now.
			UE_LOG(LogPICOSpatial, Warning, TEXT("Ribbon is not supported!"));
			CopenhagenParticle.setOrientationMode(spatialpack::CopenhagenParticleOrientationMode::DEFAULT);
			CopenhagenParticle.setRenderAlignment(spatialpack::CopenhagenParticleRenderAlignment::FACING);
		}
		break;
	}
	
	
	if (!RenderMaterialPath.IsEmpty())
	{
		spatialpack::Material* MaterialResource = nullptr;
		MaterialResource = GetCachedPICOSpatial().CacheMaterialResource(RenderMaterialPath, "");
				
		if (MaterialResource)
		{
			UE_LOG(LogPICOSpatial, Verbose, TEXT("MaterialResource Name:%hs"), MaterialResource->getName());
			UE_LOG(LogPICOSpatial, Verbose, TEXT("MaterialResource Type is %d"), MaterialResource->getType());

			spatialpack::MaterialType MaterialType = MaterialResource->getType();
			switch (MaterialType)
			{
			case spatialpack::MaterialType::UNLIT:
				{
					spatialpack::UnlitMaterial UnlitMaterial = spatialpack::UnlitMaterial::castFromMaterial( * MaterialResource);
					if (UnlitMaterial.isValid())
					{
						CopenhagenParticle.getRendererMaterial().setBaseColorMap(UnlitMaterial.getBaseColorTextureParameter().texture);
						CopenhagenParticle.getRendererMaterial().setBlendingMode(MaterialBlendingModeToParticleBlendMode(UnlitMaterial.getBlendingMode()));
					}
				}
				break;
			case spatialpack::MaterialType::PHYSICALLY_BASED:
				{
					spatialpack::PhysicallyBasedMaterial PhysicallyBasedMaterial = spatialpack::PhysicallyBasedMaterial::castFromMaterial( * MaterialResource);
					if (PhysicallyBasedMaterial.isValid())
					{
						CopenhagenParticle.getRendererMaterial().setBaseColorMap(PhysicallyBasedMaterial.getBaseColorTextureParameter().texture);
						CopenhagenParticle.getRendererMaterial().setBlendingMode(MaterialBlendingModeToParticleBlendMode(PhysicallyBasedMaterial.getBlendingMode()));
					}
				}
				break;
			default:
				{
					UE_LOG(LogPICOSpatial, Verbose, TEXT("MaterialResource is not Support"));
				}
				break;
			}
		}
	}
	
	//ParticleSize
	HandleParticleSize(NiagaraEmitterPICO, CopenhagenParticle);

	//SimulationSpace
	//TODO:Space is no match for Niagara.
	//CopenhagenParticle.setSimulationSpace(NiagaraEmitterPICO.bLocalSpace ? spatialpack::ParticleSpace::LOCAL : spatialpack::ParticleSpace::WORLD);

	//Speed
	//TODO:Not Support Velocity Curve for Now
	auto speed = CopenhagenParticle.getStartSpeed();
	speed.setCurveType(spatialpack::CopenhagenCurveType::CONSTANT);
	float Speed = 0.f;
	switch (NiagaraEmitterPICO.VelocityMode)
	{
	case ENiagaraPICO_VelocityMode::Linear:
		{
			Speed = NiagaraEmitterPICO.Velocity.Size() * NiagaraEmitterPICO.VelocitySpeedScale / 100.0f;
			CopenhagenParticle.setShapeRandomizeDirection(0.f);
		}
		break;
	case ENiagaraPICO_VelocityMode::FromPoint:
		{
			Speed = NiagaraEmitterPICO.VelocitySpeed * NiagaraEmitterPICO.VelocitySpeedScale / 100.0f;
			CopenhagenParticle.setShapeRandomizeDirection(1.0f);
		}
		break;
	case ENiagaraPICO_VelocityMode::InCone:
		{
			Speed = NiagaraEmitterPICO.VelocitySpeed * NiagaraEmitterPICO.VelocitySpeedScale / 100.0f;
			float ShapeRandomizeDirection = 0.f;
			switch (NiagaraEmitterPICO.ConeAngleMode)
			{
			case ENiagaraPICO_ConeAngleMode::Degrees: ShapeRandomizeDirection = NormalizeDegrees(NiagaraEmitterPICO.ConeAngle);
				break;
			case ENiagaraPICO_ConeAngleMode::Radians: ShapeRandomizeDirection = NormalizeRadians(NiagaraEmitterPICO.ConeAngle);
				break;
			case ENiagaraPICO_ConeAngleMode::NormalizedAngle: ShapeRandomizeDirection = NiagaraEmitterPICO.ConeAngle;
				break;
			}
			UE_LOG(LogPICOSpatial, Verbose, TEXT("InCone ShapeRandomizeDirection:%f"), ShapeRandomizeDirection);

			CopenhagenParticle.setShapeRandomizeDirection(ShapeRandomizeDirection);
		}
		break;
	default: ;
	}

	speed.setConstant(Speed);
	CopenhagenParticle.setStartSpeed(speed);

	//Rotation Speed
	auto AngularVelocity = CopenhagenParticle.getAngularVelocityOverLifetime();
	AngularVelocity.setCurveType(spatialpack::CopenhagenCurveType::CONSTANT);
	float AngularSpeed = NiagaraEmitterPICO.MainRendererType == ENiagaraPICO_RendererType::Mesh ? NiagaraEmitterPICO.MeshRotationRate * 360.f : NiagaraEmitterPICO.SpriteRotationRate;
	AngularVelocity.setConstant(AngularSpeed);
	CopenhagenParticle.setAngularVelocityOverLifetime(AngularVelocity);

	//TODO:wait for Update
	if (NiagaraEmitterPICO.bEnableCollision)
	{
		//Collision
		CopenhagenParticle.setCollisionModuleEnabled(true);
		CopenhagenParticle.clearCollisionPlanes();
		spatialpack::Vector3f AnalyticalCollisionPlanePosition1 = ToSpVector(NiagaraEmitterPICO.AnalyticalCollisionPlanePosition1, 100.0f);
		spatialpack::Vector3f AnalyticalCollisionPlanePosition2 = ToSpVector(NiagaraEmitterPICO.AnalyticalCollisionPlanePosition2, 100.0f);
		AnalyticalCollisionPlanePosition1.x=AnalyticalCollisionPlanePosition1.x-ParticlePosition.x;
		AnalyticalCollisionPlanePosition1.y=AnalyticalCollisionPlanePosition1.y-ParticlePosition.y;
		AnalyticalCollisionPlanePosition1.z=AnalyticalCollisionPlanePosition1.z-ParticlePosition.z;
		AnalyticalCollisionPlanePosition2.x=AnalyticalCollisionPlanePosition2.x-ParticlePosition.x;
		AnalyticalCollisionPlanePosition2.y=AnalyticalCollisionPlanePosition2.y-ParticlePosition.y;
		AnalyticalCollisionPlanePosition2.z=AnalyticalCollisionPlanePosition2.z-ParticlePosition.z;
		CopenhagenParticle.addCollisionPlane(AnalyticalCollisionPlanePosition1, ToSpVector(NiagaraEmitterPICO.AnalyticalCollisionNormal1));
		CopenhagenParticle.addCollisionPlane(AnalyticalCollisionPlanePosition2, ToSpVector(NiagaraEmitterPICO.AnalyticalCollisionNormal2));

		spatialpack::ParticleFloatProperty BounceParticleProperty = {};
		BounceParticleProperty.type = spatialpack::ParticlePropertyType::CONSTANT;
		BounceParticleProperty.value = NiagaraEmitterPICO.BounceRestitution;
		CopenhagenParticle.setCollisionBounce(BounceParticleProperty);
		
		// spatialpack::ParticleFloatProperty DampenParticleProperty = {};
		// DampenParticleProperty.type = spatialpack::ParticlePropertyType::CONSTANT;
		// DampenParticleProperty.value = 0.5f;
		// CopenhagenParticle.setCollisionDampen(DampenParticleProperty);
	}

	//Noise
	if (NiagaraEmitterPICO.NoiseForceAmount)
	{
		CopenhagenParticle.setNoiseModuleEnabled(true);
		CopenhagenParticle.setNoiseStrength(NiagaraEmitterPICO.NoiseForceAmount / 100.0f);
	}

	//Rotation
	CopenhagenParticle.setRotationOverLifetimeModuleEnabled(true);
	auto Rotation = CopenhagenParticle.getStartRotation();


	switch (NiagaraEmitterPICO.MainRendererType)
	{
	case ENiagaraPICO_RendererType::Sprite:
		{
			switch (NiagaraEmitterPICO.SpriteRotationMode)
			{
			case ENiagaraPICO_SpriteRotationMode::Unset:
				break;
			case ENiagaraPICO_SpriteRotationMode::Random:
				{
					Rotation.setCurveType(spatialpack::CopenhagenCurveType::RANDOM);
					Rotation.setRandomRange({NiagaraEmitterPICO.SpriteRotationAngleMin, NiagaraEmitterPICO.SpriteRotationAngleMax});
				}
				break;
			case ENiagaraPICO_SpriteRotationMode::DirectAngle:
				{
					Rotation.setCurveType(spatialpack::CopenhagenCurveType::CONSTANT);
					Rotation.setConstant(NiagaraEmitterPICO.SpriteRotationAngle);
				}
				break;
			case ENiagaraPICO_SpriteRotationMode::DirectNormalizedAngle:
				{
					Rotation.setCurveType(spatialpack::CopenhagenCurveType::CONSTANT);
					Rotation.setConstant(NiagaraEmitterPICO.SpriteRotationAngle * 360.0f);
				}
				break;
			}
		}
		break;
	case ENiagaraPICO_RendererType::Mesh:
		{
			switch (NiagaraEmitterPICO.MeshOrientationMode)
			{
			case ENiagaraPICO_MeshOrientationMode::Random:
				{
					Rotation.setCurveType(spatialpack::CopenhagenCurveType::RANDOM);
					Rotation.setRandomRange({0, 360.0f});
				}
				break;
			case ENiagaraPICO_MeshOrientationMode::System:
				break;
			case ENiagaraPICO_MeshOrientationMode::OrientToVector:
				break;
			case ENiagaraPICO_MeshOrientationMode::OrientToMatrix:
				break;
			case ENiagaraPICO_MeshOrientationMode::OrientToQuaternion:
				break;
			default: ;
			}
		}
		break;
	case ENiagaraPICO_RendererType::Ribbon:
		break;
	}
	CopenhagenParticle.setStartRotation(Rotation);

	//Life Time
	auto Lifetime = CopenhagenParticle.getStartLifetime();
	switch (NiagaraEmitterPICO.LifeTimeMode)
	{
	case ENiagaraPICO_LifeTimeMode::DirectSet:
		{
			Lifetime.setCurveType(spatialpack::CopenhagenCurveType::CONSTANT);
			Lifetime.setConstant(NiagaraEmitterPICO.LifeTime);
			CopenhagenParticle.setStartLifetime(Lifetime);
			UE_LOG(LogPICOSpatial, Verbose, TEXT("DirectSet LifeTime:%f"), NiagaraEmitterPICO.LifeTime);
		}
		break;
	case ENiagaraPICO_LifeTimeMode::Random:
		{
			Lifetime.setCurveType(spatialpack::CopenhagenCurveType::RANDOM);
			Lifetime.setRandomRange({NiagaraEmitterPICO.LifeTimeMin, NiagaraEmitterPICO.LifeTimeMax});
			CopenhagenParticle.setStartLifetime(Lifetime);
			UE_LOG(LogPICOSpatial, Verbose, TEXT("Random LifeTimeMax:%f LifeTimeMin:%f"), NiagaraEmitterPICO.LifeTimeMax, NiagaraEmitterPICO.LifeTimeMin);
		}
		break;
	}

	//Force
	if (!NiagaraEmitterPICO.TotalForce.IsZero())
	{
		CopenhagenParticle.setForceOverLifetimeModuleEnabled(true);

		//TODO Because the current speed does not support vectors, but do not want to lose this part of the details, all temporarily added to the Force
		FVector3f Force = NiagaraEmitterPICO.TotalForce;
		CopenhagenParticle.setForce(ToSpVector(Force, 100.0f));

		UE_LOG(LogPICOSpatial, Verbose, TEXT("AccumulatedForce:%s"), *NiagaraEmitterPICO.TotalForce.ToString());
	}

	//Color
	if (NiagaraEmitterPICO.ColorOverLifeCurve.IsValid())
	{
		CopenhagenParticle.setColorOverLifetimeModuleEnabled(true);
		auto ColorOverLife = CopenhagenParticle.getColorOverLifetime();
		ColorOverLife.setColorType(spatialpack::CopenhagenColorType::GRADIENT);
		ColorOverLife.clearGradientKeys();

		const int32 RedKeyNum = NiagaraEmitterPICO.ColorOverLifeCurve.RedCurve.Keys.Num();
		const int32 GreenKeyNum = NiagaraEmitterPICO.ColorOverLifeCurve.GreenCurve.Keys.Num();
		const int32 BlueKeyNum = NiagaraEmitterPICO.ColorOverLifeCurve.BlueCurve.Keys.Num();
		const int32 ColorKeyNum = FMath::Min3(RedKeyNum, GreenKeyNum, BlueKeyNum);
		if (ColorKeyNum != RedKeyNum || ColorKeyNum != GreenKeyNum || ColorKeyNum != BlueKeyNum)
		{
			UE_LOG(LogPICOSpatial, Warning,
				TEXT("Emitter '%s' color curve key count mismatch: R=%d G=%d B=%d. Truncate to %d keys."),
				*NiagaraEmitterPICO.EmitterName, RedKeyNum, GreenKeyNum, BlueKeyNum, ColorKeyNum);
		}

		for (int32 i = 0; i < ColorKeyNum; i++)
		{
			float Time = NiagaraEmitterPICO.ColorOverLifeCurve.RedCurve.Keys[i].Time;
			float R = NiagaraEmitterPICO.ColorOverLifeCurve.RedCurve.Keys[i].Value;
			float G = NiagaraEmitterPICO.ColorOverLifeCurve.GreenCurve.Keys[i].Value;
			float B = NiagaraEmitterPICO.ColorOverLifeCurve.BlueCurve.Keys[i].Value;
			ColorOverLife.addGradientColorKey({R, G, B}, Time);
		}

		for (int32 i = 0; i < NiagaraEmitterPICO.ColorOverLifeCurve.AlphaCurve.Keys.Num(); i++)
		{
			float Time = NiagaraEmitterPICO.ColorOverLifeCurve.AlphaCurve.Keys[i].Time;
			float A = NiagaraEmitterPICO.ColorOverLifeCurve.AlphaCurve.Keys[i].Value;
			ColorOverLife.addGradientAlphaKey(A, Time);
		}
		CopenhagenParticle.setColorOverLifetime(ColorOverLife);
	}
	else
	{
		auto StartColor = CopenhagenParticle.getStartColor();
		switch (NiagaraEmitterPICO.ColorMode)
		{
		case ENiagaraPICO_ColorMode::Unset:
			break;
		case ENiagaraPICO_ColorMode::DirectSet:
			{
				StartColor.setColorType(spatialpack::CopenhagenColorType::CONSTANT);
				StartColor.setConstant(ToSpColor(NiagaraEmitterPICO.Color));
			}
			break;
		case ENiagaraPICO_ColorMode::RandomRange:
			{
				StartColor.setColorType(spatialpack::CopenhagenColorType::RANDOM);
				StartColor.setRandomRange(ToSpColor(NiagaraEmitterPICO.ColorMinimum), ToSpColor(NiagaraEmitterPICO.ColorMaximum));
			}
			break;
		case ENiagaraPICO_ColorMode::RandomHue:
			break;
		}
		CopenhagenParticle.setStartColor(StartColor);
	}

	//Sub UVAnimation
	if (NiagaraEmitterPICO.bSubUVAnimation && NiagaraEmitterPICO.SpriteRenderers.IsValidIndex(0))
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("NiagaraEmitterPICO.bSubUVAnimation"));
		//TextureSheetAnimation
		CopenhagenParticle.setTextureSheetAnimationModuleEnabled(true);

		const FNiagaraSpriteRendererPICO& SpriteRenderer = NiagaraEmitterPICO.SpriteRenderers[0];
		int32 TileX = SpriteRenderer.SubImageSize.X;
		int32 TileY = SpriteRenderer.SubImageSize.Y;
		if (NiagaraEmitterPICO.bUseStartFrame)
		{
			auto AnimationStartFrame = CopenhagenParticle.getTextureSheetAnimationStartFrame();
			AnimationStartFrame.type = spatialpack::ParticlePropertyType::CONSTANT;
			AnimationStartFrame.value = NiagaraEmitterPICO.StartFrameRangeOverride;
			CopenhagenParticle.setTextureSheetAnimationStartFrame(AnimationStartFrame);
		}
		CopenhagenParticle.setTextureSheetAnimationTileX(TileX);
		CopenhagenParticle.setTextureSheetAnimationTileY(TileY);
		const float LifeTime = NiagaraEmitterPICO.LifeTime?NiagaraEmitterPICO.LifeTime:NiagaraEmitterPICO.LifeTimeMax;
		if (LifeTime > SMALL_NUMBER)
		{
			CopenhagenParticle.setTextureSheetAnimationFPS(TileX * TileY / LifeTime);
		}
		else
		{
			UE_LOG(LogPICOSpatial, Warning,
				TEXT("Emitter '%s' has invalid lifetime %f for sub UV animation. Set animation FPS to 0."),
				*NiagaraEmitterPICO.EmitterName, LifeTime);
			CopenhagenParticle.setTextureSheetAnimationFPS(0.0f);
		}
	}

#endif
}
