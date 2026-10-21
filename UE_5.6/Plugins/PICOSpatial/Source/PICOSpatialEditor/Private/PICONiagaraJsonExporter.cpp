// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICONiagaraJsonExporter.h"
#include "PICOSpatialNiagaraAsset.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "HAL/IConsoleManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "PICONiagaraJsonExporter"

DEFINE_LOG_CATEGORY_STATIC(LogPICONiagaraJsonExporter, Log, All);

// Console variable to control whether we export the JSON sidecar.
static TAutoConsoleVariable<int32> CVarPICONiagaraExportJson(
	TEXT("PICO.Niagara.ExportJson"),
	0, // 0: disabled (default), 1: enabled
	TEXT("If 1, exports a JSON sidecar (.json) when saving PICOSpatialNiagaraAsset."),
	ECVF_Default);

bool PICONiagaraJsonExporter::IsExportEnabled()
{
	return CVarPICONiagaraExportJson.GetValueOnGameThread() != 0;
}

bool PICONiagaraJsonExporter::ExportIfEnabled(const UPICOSpatialNiagaraAsset* Asset, const FString& UAssetPath)
{
	if (!Asset)
	{
		UE_LOG(LogPICONiagaraJsonExporter, Warning, TEXT("Cannot export JSON for null asset"));
		return false;
	}

	if (!IsExportEnabled())
	{
		return true;
	}

	FString JsonPath = FPaths::ChangeExtension(UAssetPath, TEXT(".json"));
	return ExportToFile(Asset, JsonPath);
}

bool PICONiagaraJsonExporter::ExportToFile(const UPICOSpatialNiagaraAsset* Asset, const FString& JsonPath)
{
	if (!Asset)
	{
		UE_LOG(LogPICONiagaraJsonExporter, Warning, TEXT("Cannot export JSON for null asset"));
		return false;
	}

	TSharedRef<FJsonObject> JsonObject = BuildJsonObject(Asset);
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	if (!FJsonSerializer::Serialize(JsonObject, Writer))
	{
		UE_LOG(LogPICONiagaraJsonExporter, Error, TEXT("Failed to serialize JSON for asset: %s"), *Asset->GetName());
		return false;
	}

	if (!FFileHelper::SaveStringToFile(JsonString, *JsonPath))
	{
		UE_LOG(LogPICONiagaraJsonExporter, Error, TEXT("Failed to write JSON to file: %s"), *JsonPath);
		return false;
	}

	UE_LOG(LogPICONiagaraJsonExporter, Log, TEXT("Successfully exported JSON to: %s"), *JsonPath);
	return true;
}

static FString LifeCycleModeToString(ENiagaraPICO_LifeCycleMode Mode)
{
	switch (Mode)
	{
	case ENiagaraPICO_LifeCycleMode::System: return TEXT("System");
	case ENiagaraPICO_LifeCycleMode::Self: return TEXT("Self");
	default: return TEXT("Unknown");
	}
}

static FString LoopBehaviorToString(ENiagaraPICO_LoopBehavior LoopBehavior)
{
	switch (LoopBehavior)
	{
	case ENiagaraPICO_LoopBehavior::Infinite: return TEXT("Infinite");
	case ENiagaraPICO_LoopBehavior::Once: return TEXT("Once");
	case ENiagaraPICO_LoopBehavior::Multiple: return TEXT("Multiple");
	default: return TEXT("Unknown");
	}
}

static FString SizeModeToString(ENiagaraPICO_SizeMode Mode)
{
	switch (Mode)
	{
	case ENiagaraPICO_SizeMode::Uniform: return TEXT("Uniform");
	case ENiagaraPICO_SizeMode::RandomUniform: return TEXT("RandomUniform");
	case ENiagaraPICO_SizeMode::NonUniform: return TEXT("NonUniform");
	case ENiagaraPICO_SizeMode::RandomNonUniform: return TEXT("RandomNonUniform");
	default: return TEXT("Unknown");
	}
}

static FString ScaleSizeModeToString(ENiagaraPICO_ScaleSizeMode Mode)
{
	switch (Mode)
	{
	case ENiagaraPICO_ScaleSizeMode::Uniform: return TEXT("Uniform");
	case ENiagaraPICO_ScaleSizeMode::UniformCurve: return TEXT("UniformCurve");
	case ENiagaraPICO_ScaleSizeMode::NonUniform: return TEXT("NonUniform");
	case ENiagaraPICO_ScaleSizeMode::NonUniformCurve: return TEXT("NonUniformCurve");
	default: return TEXT("Unknown");
	}
}

static FString VelocityModeToString(ENiagaraPICO_VelocityMode Mode)
{
	switch (Mode)
	{
	case ENiagaraPICO_VelocityMode::Linear: return TEXT("Linear");
	case ENiagaraPICO_VelocityMode::FromPoint: return TEXT("FromPoint");
	case ENiagaraPICO_VelocityMode::InCone: return TEXT("InCone");
	default: return TEXT("Unknown");
	}
}

static FString BoxOrPlaneModeToString(ENiagaraPICO_BoxOrPlaneMode Mode)
{
	switch (Mode)
	{
	case ENiagaraPICO_BoxOrPlaneMode::Box: return TEXT("Box");
	case ENiagaraPICO_BoxOrPlaneMode::Plane: return TEXT("Plane");
	default: return TEXT("Unknown");
	}
}

static FString ConeAngleModeToString(ENiagaraPICO_ConeAngleMode Mode)
{
	switch (Mode)
	{
	case ENiagaraPICO_ConeAngleMode::Degrees: return TEXT("Degrees");
	case ENiagaraPICO_ConeAngleMode::NormalizedAngle: return TEXT("NormalizedAngle");
	case ENiagaraPICO_ConeAngleMode::Radians: return TEXT("Radians");
	default: return TEXT("Unknown");
	}
}

static FString SpriteRotationModeToString(ENiagaraPICO_SpriteRotationMode Mode)
{
	switch (Mode)
	{
	case ENiagaraPICO_SpriteRotationMode::Random: return TEXT("Random");
	case ENiagaraPICO_SpriteRotationMode::DirectAngle: return TEXT("DirectAngle");
	case ENiagaraPICO_SpriteRotationMode::DirectNormalizedAngle: return TEXT("DirectNormalizedAngle");
	default: return TEXT("Unknown");
	}
}

static FString OrientationMethodToString(ENiagaraPICO_OrientationMethod Method)
{
	switch (Method)
	{
	case ENiagaraPICO_OrientationMethod::RotationRate: return TEXT("RotationRate");
	case ENiagaraPICO_OrientationMethod::OrientToVector_S: return TEXT("OrientToVector_S");
	case ENiagaraPICO_OrientationMethod::OrientToPosition: return TEXT("OrientToPosition");
	case ENiagaraPICO_OrientationMethod::FlightOrientation: return TEXT("FlightOrientation");
	case ENiagaraPICO_OrientationMethod::RollingOrientation: return TEXT("RollingOrientation");
	default: return TEXT("Unknown");
	}
}

static FString MeshOrientationModeToString(ENiagaraPICO_MeshOrientationMode Mode)
{
	switch (Mode)
	{
	case ENiagaraPICO_MeshOrientationMode::Random: return TEXT("Random");
	case ENiagaraPICO_MeshOrientationMode::System: return TEXT("System");
	case ENiagaraPICO_MeshOrientationMode::OrientToVector: return TEXT("OrientToVector");
	case ENiagaraPICO_MeshOrientationMode::OrientToMatrix: return TEXT("OrientToMatrix");
	case ENiagaraPICO_MeshOrientationMode::OrientToQuaternion: return TEXT("OrientToQuaternion");
	default: return TEXT("Unknown");
	}
}

static FString UpdateMeshOrientationMethodToString(ENiagaraPICO_UpdateMeshOrientationMethod Method)
{
	switch (Method)
	{
	case ENiagaraPICO_UpdateMeshOrientationMethod::RotationRate: return TEXT("RotationRate");
	case ENiagaraPICO_UpdateMeshOrientationMethod::OrientToVector_S: return TEXT("OrientToVector_S");
	case ENiagaraPICO_UpdateMeshOrientationMethod::OrientToPosition: return TEXT("OrientToPosition");
	case ENiagaraPICO_UpdateMeshOrientationMethod::FlightOrientation: return TEXT("FlightOrientation");
	case ENiagaraPICO_UpdateMeshOrientationMethod::RollingOrientation: return TEXT("RollingOrientation");
	default: return TEXT("Unknown");
	}
}

static FString RendererTypeToString(ENiagaraPICO_RendererType Type)
{
	switch (Type)
	{
	case ENiagaraPICO_RendererType::Sprite: return TEXT("Sprite");
	case ENiagaraPICO_RendererType::Mesh: return TEXT("Mesh");
	case ENiagaraPICO_RendererType::Ribbon: return TEXT("Ribbon");
	default: return TEXT("Unknown");
	}
}

static FString FacingModeToString(ENiagaraPICO_FacingMode Mode)
{
	switch (Mode)
	{
	case ENiagaraPICO_FacingMode::CameraPosition: return TEXT("CameraPosition");
	case ENiagaraPICO_FacingMode::CameraPlane: return TEXT("CameraPlane");
	case ENiagaraPICO_FacingMode::Free: return TEXT("Free");
	default: return TEXT("Unknown");
	}
}

static FString RichCurveInterpModeToString(ERichCurveInterpMode InterpMode)
{
	switch (InterpMode)
	{
	case RCIM_Constant: return TEXT("Constant");
	case RCIM_Linear: return TEXT("Linear");
	case RCIM_Cubic: return TEXT("Cubic");
	case RCIM_None: return TEXT("None");
	default: return TEXT("Unknown");
	}
}

static FString RichCurveTangentModeToString(ERichCurveTangentMode TangentMode)
{
	switch (TangentMode)
	{
	case RCTM_Auto: return TEXT("Auto");
	case RCTM_User: return TEXT("User");
	case RCTM_Break: return TEXT("Break");
	case RCTM_None: return TEXT("None");
	case RCTM_SmartAuto: return TEXT("SmartAuto");
	default: return TEXT("Unknown");
	}
}

static FString RichCurveTangentWeightModeToString(ERichCurveTangentWeightMode TangentWeightMode)
{
	switch (TangentWeightMode)
	{
	case RCTWM_WeightedNone: return TEXT("None");
	case RCTWM_WeightedArrive: return TEXT("Arrive");
	case RCTWM_WeightedLeave: return TEXT("Leave");
	case RCTWM_WeightedBoth: return TEXT("Both");
	default: return TEXT("Unknown");
	}
}

static TSharedPtr<FJsonObject> CurveToJson(const FNiagaraCurvePICO& Curve)
{
	if (Curve.Keys.IsEmpty())
	{
		return nullptr;
	}

	TSharedRef<FJsonObject> CurveObj = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> KeysArray;

	for (const FRichCurveKey& Key : Curve.Keys)
	{
		TSharedRef<FJsonObject> KeyObj = MakeShared<FJsonObject>();
		KeyObj->SetNumberField(TEXT("time"), Key.Time);
		KeyObj->SetNumberField(TEXT("value"), Key.Value);
		KeyObj->SetStringField(TEXT("interpMode"), RichCurveInterpModeToString(Key.InterpMode));
		KeyObj->SetStringField(TEXT("tangentMode"), RichCurveTangentModeToString(Key.TangentMode));
		KeyObj->SetStringField(TEXT("tangentWeightMode"), RichCurveTangentWeightModeToString(Key.TangentWeightMode));
		KeyObj->SetNumberField(TEXT("arriveTangent"), Key.ArriveTangent);
		KeyObj->SetNumberField(TEXT("arriveTangentWeight"), Key.ArriveTangentWeight);
		KeyObj->SetNumberField(TEXT("leaveTangent"), Key.LeaveTangent);
		KeyObj->SetNumberField(TEXT("leaveTangentWeight"), Key.LeaveTangentWeight);
		KeysArray.Add(MakeShared<FJsonValueObject>(KeyObj));
	}

	CurveObj->SetArrayField(TEXT("keys"), KeysArray);
	return CurveObj;
}

static TSharedPtr<FJsonObject> VectorCurveToJson(const FNiagaraVectorCurvePICO& Curve)
{
	TSharedRef<FJsonObject> CurveObj = MakeShared<FJsonObject>();
	bool bHasAnyCurve = false;

	if (auto XCurve = CurveToJson(Curve.XCurve))
	{
		CurveObj->SetObjectField(TEXT("x"), XCurve);
		bHasAnyCurve = true;
	}
	if (auto YCurve = CurveToJson(Curve.YCurve))
	{
		CurveObj->SetObjectField(TEXT("y"), YCurve);
		bHasAnyCurve = true;
	}
	if (auto ZCurve = CurveToJson(Curve.ZCurve))
	{
		CurveObj->SetObjectField(TEXT("z"), ZCurve);
		bHasAnyCurve = true;
	}

	if (!bHasAnyCurve)
	{
		return nullptr;
	}

	return CurveObj;
}

static TSharedPtr<FJsonObject> ColorCurveToJson(const FNiagaraColorCurvePICO& Curve)
{
	TSharedRef<FJsonObject> CurveObj = MakeShared<FJsonObject>();
	bool bHasAnyCurve = false;

	if (auto RedCurve = CurveToJson(Curve.RedCurve))
	{
		CurveObj->SetObjectField(TEXT("r"), RedCurve);
		bHasAnyCurve = true;
	}
	if (auto GreenCurve = CurveToJson(Curve.GreenCurve))
	{
		CurveObj->SetObjectField(TEXT("g"), GreenCurve);
		bHasAnyCurve = true;
	}
	if (auto BlueCurve = CurveToJson(Curve.BlueCurve))
	{
		CurveObj->SetObjectField(TEXT("b"), BlueCurve);
		bHasAnyCurve = true;
	}
	if (auto AlphaCurve = CurveToJson(Curve.AlphaCurve))
	{
		CurveObj->SetObjectField(TEXT("a"), AlphaCurve);
		bHasAnyCurve = true;
	}

	if (!bHasAnyCurve)
	{
		return nullptr;
	}

	return CurveObj;
}

static TSharedRef<FJsonObject> Vector3fToJson(const FVector3f& Vec)
{
	TSharedRef<FJsonObject> VecObj = MakeShared<FJsonObject>();
	VecObj->SetNumberField(TEXT("x"), Vec.X);
	VecObj->SetNumberField(TEXT("y"), Vec.Y);
	VecObj->SetNumberField(TEXT("z"), Vec.Z);
	return VecObj;
}

static TSharedRef<FJsonObject> Vector2fToJson(const FVector2f& Vec)
{
	TSharedRef<FJsonObject> VecObj = MakeShared<FJsonObject>();
	VecObj->SetNumberField(TEXT("x"), Vec.X);
	VecObj->SetNumberField(TEXT("y"), Vec.Y);
	return VecObj;
}

static TSharedRef<FJsonObject> Vector2DToJson(const FVector2D& Vec)
{
	TSharedRef<FJsonObject> VecObj = MakeShared<FJsonObject>();
	VecObj->SetNumberField(TEXT("x"), Vec.X);
	VecObj->SetNumberField(TEXT("y"), Vec.Y);
	return VecObj;
}

static TSharedRef<FJsonObject> LinearColorToJson(const FLinearColor& Color)
{
	TSharedRef<FJsonObject> ColorObj = MakeShared<FJsonObject>();
	ColorObj->SetNumberField(TEXT("r"), Color.R);
	ColorObj->SetNumberField(TEXT("g"), Color.G);
	ColorObj->SetNumberField(TEXT("b"), Color.B);
	ColorObj->SetNumberField(TEXT("a"), Color.A);
	return ColorObj;
}

TSharedRef<FJsonObject> PICONiagaraJsonExporter::BuildJsonObject(const UPICOSpatialNiagaraAsset* Asset)
{
	TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();

	Root->SetStringField(TEXT("systemName"), Asset->NiagaraSystemName);
	Root->SetStringField(TEXT("loopBehavior"), LoopBehaviorToString(Asset->LoopBehavior));

	Root->SetNumberField(TEXT("loopDuration"), Asset->LoopDuration);
	Root->SetNumberField(TEXT("loopCount"), Asset->LoopCount);
	Root->SetBoolField(TEXT("useLoopDelay"), Asset->bUseLoopDelay);
	Root->SetNumberField(TEXT("loopDelay"), Asset->LoopDelay);

	TArray<TSharedPtr<FJsonValue>> EmittersArray;
	for (const TPair<FGuid, FNiagaraEmitterPICO>& EmitterPair : Asset->GuidToEmittersMap)
	{
		const FGuid& EmitterGuid = EmitterPair.Key;
		const FNiagaraEmitterPICO& Emitter = EmitterPair.Value;

		TSharedRef<FJsonObject> EmitterObj = MakeShared<FJsonObject>();
		EmitterObj->SetStringField(TEXT("emitterId"), EmitterGuid.ToString(EGuidFormats::DigitsWithHyphens));
		EmitterObj->SetStringField(TEXT("emitterName"), Emitter.EmitterName);

		FString ParticleTypeString;
		switch (Emitter.ParticleType)
		{
		case ENiagaraPICO_ParticleType::MainParticle: ParticleTypeString = TEXT("MainParticle"); break;
		case ENiagaraPICO_ParticleType::SubParticle: ParticleTypeString = TEXT("SubParticle"); break;
		default: ParticleTypeString = TEXT("Unknown"); break;
		}
		EmitterObj->SetStringField(TEXT("particleType"), ParticleTypeString);

		EmitterObj->SetStringField(TEXT("lifeCycleMode"), LifeCycleModeToString(Emitter.LifeCycleMode));
		EmitterObj->SetStringField(TEXT("loopBehavior"), LoopBehaviorToString(Emitter.LoopBehavior));
		EmitterObj->SetNumberField(TEXT("loopDuration"), Emitter.LoopDuration);
		EmitterObj->SetNumberField(TEXT("loopCount"), Emitter.LoopCount);
		EmitterObj->SetBoolField(TEXT("useLoopDelay"), Emitter.bUseLoopDelay);
		EmitterObj->SetNumberField(TEXT("loopDelay"), Emitter.LoopDelay);
		EmitterObj->SetBoolField(TEXT("localSpace"), Emitter.bLocalSpace);
		EmitterObj->SetNumberField(TEXT("spawnRate"), Emitter.SpawnRate);
		EmitterObj->SetNumberField(TEXT("burstSpawnCount"), Emitter.BurstSpawnCount);
		EmitterObj->SetNumberField(TEXT("spawnNumsByEvent"), Emitter.SpawnNumsByEvent);

		FString LifeTimeModeString;
		switch (Emitter.LifeTimeMode)
		{
		case ENiagaraPICO_LifeTimeMode::DirectSet: LifeTimeModeString = TEXT("DirectSet"); break;
		case ENiagaraPICO_LifeTimeMode::Random: LifeTimeModeString = TEXT("Random"); break;
		default: LifeTimeModeString = TEXT("Unknown"); break;
		}
		EmitterObj->SetStringField(TEXT("lifeTimeMode"), LifeTimeModeString);
		EmitterObj->SetNumberField(TEXT("lifeTime"), Emitter.LifeTime);
		EmitterObj->SetNumberField(TEXT("lifeTimeMin"), Emitter.LifeTimeMin);
		EmitterObj->SetNumberField(TEXT("lifeTimeMax"), Emitter.LifeTimeMax);

		EmitterObj->SetStringField(TEXT("meshScaleInitializeMode"), SizeModeToString(Emitter.MeshScaleInitializeMode));
		EmitterObj->SetStringField(TEXT("meshScaleUpdateMode"), ScaleSizeModeToString(Emitter.MeshScaleUpdateMode));
		EmitterObj->SetNumberField(TEXT("meshUniformScale"), Emitter.MeshUniformScale);
		EmitterObj->SetNumberField(TEXT("meshUniformScaleMin"), Emitter.MeshUniformScaleMin);
		EmitterObj->SetNumberField(TEXT("meshUniformScaleMax"), Emitter.MeshUniformScaleMax);
		EmitterObj->SetObjectField(TEXT("meshNonUniformScale"), Vector3fToJson(Emitter.MeshNonUniformScale));
		EmitterObj->SetObjectField(TEXT("meshNonUniformScaleMin"), Vector3fToJson(Emitter.MeshNonUniformScaleMin));
		EmitterObj->SetObjectField(TEXT("meshNonUniformScaleMax"), Vector3fToJson(Emitter.MeshNonUniformScaleMax));
		EmitterObj->SetObjectField(TEXT("meshNonUniformScaleFactor"), Vector3fToJson(Emitter.MeshNonUniformScaleFactor));
		if (auto SizeCurve = VectorCurveToJson(Emitter.MeshSizeOverLifeCurve))
		{
			EmitterObj->SetObjectField(TEXT("meshSizeOverLifeCurve"), SizeCurve);
		}

		EmitterObj->SetStringField(TEXT("sizeMode"), SizeModeToString(Emitter.SizeMode));
		EmitterObj->SetNumberField(TEXT("uniformSpriteSize"), Emitter.UniformSpriteSize);
		EmitterObj->SetNumberField(TEXT("uniformSpriteSizeMin"), Emitter.UniformSpriteSizeMin);
		EmitterObj->SetNumberField(TEXT("uniformSpriteSizeMax"), Emitter.UniformSpriteSizeMax);
		EmitterObj->SetObjectField(TEXT("spriteSize"), Vector2fToJson(Emitter.SpriteSize));
		EmitterObj->SetObjectField(TEXT("spriteSizeMin"), Vector2fToJson(Emitter.SpriteSizeMin));
		EmitterObj->SetObjectField(TEXT("spriteSizeMax"), Vector2fToJson(Emitter.SpriteSizeMax));
		EmitterObj->SetObjectField(TEXT("nonUniformScaleFactor"), Vector2fToJson(Emitter.NonUniformScaleFactor));
		EmitterObj->SetStringField(TEXT("scaleSizeMode"), ScaleSizeModeToString(Emitter.ScaleSizeMode));
		EmitterObj->SetNumberField(TEXT("uniformScaleFactor"), Emitter.UniformScaleFactor);
		if (auto SizeOverLifeCurve = CurveToJson(Emitter.SizeOverLifeCurve))
		{
			EmitterObj->SetObjectField(TEXT("sizeOverLifeCurve"), SizeOverLifeCurve);
		}

		FString ColorModeString;
		switch (Emitter.ColorMode)
		{
		case ENiagaraPICO_ColorMode::DirectSet: ColorModeString = TEXT("DirectSet"); break;
		case ENiagaraPICO_ColorMode::RandomRange: ColorModeString = TEXT("RandomRange"); break;
		case ENiagaraPICO_ColorMode::RandomHue: ColorModeString = TEXT("RandomHue"); break;
		default: ColorModeString = TEXT("Unknown"); break;
		}
		EmitterObj->SetStringField(TEXT("colorMode"), ColorModeString);
		EmitterObj->SetObjectField(TEXT("color"), LinearColorToJson(Emitter.Color));
		EmitterObj->SetObjectField(TEXT("colorMinimum"), LinearColorToJson(Emitter.ColorMinimum));
		EmitterObj->SetObjectField(TEXT("colorMaximum"), LinearColorToJson(Emitter.ColorMaximum));
		if (auto ColorCurve = ColorCurveToJson(Emitter.ColorOverLifeCurve))
		{
			EmitterObj->SetObjectField(TEXT("colorOverLifeCurve"), ColorCurve);
		}

		FString ShapeTypeString;
		switch (Emitter.ShapeType)
		{
		case ENiagaraPICO_ShapeType::Sphere: ShapeTypeString = TEXT("Sphere"); break;
		case ENiagaraPICO_ShapeType::Cylinder: ShapeTypeString = TEXT("Cylinder"); break;
		case ENiagaraPICO_ShapeType::BoxOrPlane: ShapeTypeString = TEXT("BoxOrPlane"); break;
		case ENiagaraPICO_ShapeType::Torus: ShapeTypeString = TEXT("Torus"); break;
		case ENiagaraPICO_ShapeType::Ring: ShapeTypeString = TEXT("Ring"); break;
		case ENiagaraPICO_ShapeType::Cone: ShapeTypeString = TEXT("Cone"); break;
		default: ShapeTypeString = TEXT("Unknown"); break;
		}
		EmitterObj->SetStringField(TEXT("shapeType"), ShapeTypeString);
		EmitterObj->SetStringField(TEXT("boxOrPlaneMode"), BoxOrPlaneModeToString(Emitter.BoxOrPlaneMode));
		EmitterObj->SetStringField(TEXT("coneAngleMode"), ConeAngleModeToString(Emitter.ConeAngleMode));
		EmitterObj->SetNumberField(TEXT("coneAngle"), Emitter.ConeAngle);
		EmitterObj->SetNumberField(TEXT("shapeConeLength"), Emitter.ShapeConeLength);
		EmitterObj->SetNumberField(TEXT("shapeConeAngle"), Emitter.ShapeConeAngle);
		EmitterObj->SetNumberField(TEXT("sphereRadius"), Emitter.SphereRadius);
		EmitterObj->SetNumberField(TEXT("cylinderRadius"), Emitter.CylinderRadius);
		EmitterObj->SetNumberField(TEXT("cylinderHeight"), Emitter.CylinderHeight);
		EmitterObj->SetNumberField(TEXT("torusLargeRadius"), Emitter.TorusLargeRadius);
		EmitterObj->SetNumberField(TEXT("torusHandleRadius"), Emitter.TorusHandleRadius);
		EmitterObj->SetNumberField(TEXT("coneRadialAngle"), Emitter.ConeRadialAngle);
		EmitterObj->SetObjectField(TEXT("planeSize"), Vector2fToJson(Emitter.PlaneSize));
		EmitterObj->SetObjectField(TEXT("boxSize"), Vector3fToJson(Emitter.BoxSize));
		EmitterObj->SetObjectField(TEXT("shapeNonUniformScale"), Vector3fToJson(Emitter.NonUniformScale));

		EmitterObj->SetObjectField(TEXT("totalForce"), Vector3fToJson(Emitter.TotalForce));
		EmitterObj->SetObjectField(TEXT("velocity"), Vector3fToJson(Emitter.Velocity));
		EmitterObj->SetNumberField(TEXT("drag"), Emitter.Drag);
		EmitterObj->SetNumberField(TEXT("noiseForceAmount"), Emitter.NoiseForceAmount);

		EmitterObj->SetStringField(TEXT("velocityMode"), VelocityModeToString(Emitter.VelocityMode));
		EmitterObj->SetNumberField(TEXT("velocitySpeed"), Emitter.VelocitySpeed);
		EmitterObj->SetNumberField(TEXT("velocitySpeedScale"), Emitter.VelocitySpeedScale);
		if (auto VelocityCurve = VectorCurveToJson(Emitter.VelocityScaleCurve))
		{
			EmitterObj->SetObjectField(TEXT("velocityScaleCurve"), VelocityCurve);
		}

		EmitterObj->SetStringField(TEXT("spriteRotationMode"), SpriteRotationModeToString(Emitter.SpriteRotationMode));
		EmitterObj->SetStringField(TEXT("orientationMethod"), OrientationMethodToString(Emitter.OrientationMethod));
		EmitterObj->SetNumberField(TEXT("spriteRotationAngle"), Emitter.SpriteRotationAngle);
		EmitterObj->SetNumberField(TEXT("spriteRotationAngleMin"), Emitter.SpriteRotationAngleMin);
		EmitterObj->SetNumberField(TEXT("spriteRotationAngleMax"), Emitter.SpriteRotationAngleMax);
		EmitterObj->SetNumberField(TEXT("spriteRotationRate"), Emitter.SpriteRotationRate);
		EmitterObj->SetNumberField(TEXT("meshRotationRate"), Emitter.MeshRotationRate);
		EmitterObj->SetStringField(TEXT("meshOrientationMode"), MeshOrientationModeToString(Emitter.MeshOrientationMode));
		EmitterObj->SetStringField(TEXT("updateMeshOrientationMethod"), UpdateMeshOrientationMethodToString(Emitter.UpdateMeshOrientationMethod));

		EmitterObj->SetBoolField(TEXT("subUVAnimation"), Emitter.bSubUVAnimation);
		EmitterObj->SetBoolField(TEXT("useStartFrame"), Emitter.bUseStartFrame);
		EmitterObj->SetNumberField(TEXT("startFrameRangeOverride"), Emitter.StartFrameRangeOverride);
		EmitterObj->SetBoolField(TEXT("useEndFrame"), Emitter.bUseEndFrame);
		EmitterObj->SetNumberField(TEXT("endFrameRangeOverride"), Emitter.EndFrameRangeOverride);

		EmitterObj->SetStringField(TEXT("mainRendererType"), RendererTypeToString(Emitter.MainRendererType));

		TArray<TSharedPtr<FJsonValue>> SpriteRenderersArray;
		for (const FNiagaraSpriteRendererPICO& Renderer : Emitter.SpriteRenderers)
		{
			TSharedRef<FJsonObject> RendererObj = MakeShared<FJsonObject>();
			RendererObj->SetStringField(TEXT("renderName"), Renderer.RenderName);
			RendererObj->SetStringField(TEXT("renderType"), RendererTypeToString(Renderer.RenderType));
			RendererObj->SetStringField(TEXT("facingMode"), FacingModeToString(Renderer.FacingMode));
			if (Renderer.RenderMaterial)
			{
				RendererObj->SetStringField(TEXT("materialPath"), Renderer.RenderMaterial->GetPathName());
			}
			RendererObj->SetObjectField(TEXT("subImageSize"), Vector2DToJson(Renderer.SubImageSize));
			SpriteRenderersArray.Add(MakeShared<FJsonValueObject>(RendererObj));
		}
		EmitterObj->SetArrayField(TEXT("spriteRenderers"), SpriteRenderersArray);

		TArray<TSharedPtr<FJsonValue>> MeshRenderersArray;
		for (const FNiagaraMeshRendererPICO& Renderer : Emitter.MeshRenders)
		{
			TSharedRef<FJsonObject> RendererObj = MakeShared<FJsonObject>();
			RendererObj->SetStringField(TEXT("renderName"), Renderer.RenderName);
			RendererObj->SetStringField(TEXT("renderType"), RendererTypeToString(Renderer.RenderType));
			RendererObj->SetStringField(TEXT("facingMode"), FacingModeToString(Renderer.FacingMode));
			if (Renderer.Mesh)
			{
				RendererObj->SetStringField(TEXT("meshPath"), Renderer.Mesh->GetPathName());
			}
			RendererObj->SetBoolField(TEXT("enableOverrideMeshMaterial"), Renderer.bEnableOverrideMeshMaterial);
			if (Renderer.bEnableOverrideMeshMaterial && Renderer.OverrideMeshMaterial)
			{
				RendererObj->SetStringField(TEXT("overrideMaterialPath"), Renderer.OverrideMeshMaterial->GetPathName());
			}
			MeshRenderersArray.Add(MakeShared<FJsonValueObject>(RendererObj));
		}
		EmitterObj->SetArrayField(TEXT("meshRenderers"), MeshRenderersArray);

		if (Emitter.SubNodeToNodeMap.Num() > 0)
		{
			TArray<TSharedPtr<FJsonValue>> SubNodeMapArray;
			for (const TPair<FString, FString>& Pair : Emitter.SubNodeToNodeMap)
			{
				TSharedRef<FJsonObject> PairObj = MakeShared<FJsonObject>();
				PairObj->SetStringField(TEXT("subNode"), Pair.Key);
				PairObj->SetStringField(TEXT("node"), Pair.Value);
				SubNodeMapArray.Add(MakeShared<FJsonValueObject>(PairObj));
			}
			EmitterObj->SetArrayField(TEXT("subNodeToNodeMap"), SubNodeMapArray);
		}

		EmitterObj->SetBoolField(TEXT("enableCollision"), Emitter.bEnableCollision);
		if (Emitter.bEnableCollision)
		{
			EmitterObj->SetObjectField(TEXT("analyticalCollisionNormal1"), Vector3fToJson(Emitter.AnalyticalCollisionNormal1));
			EmitterObj->SetObjectField(TEXT("analyticalCollisionNormal2"), Vector3fToJson(Emitter.AnalyticalCollisionNormal2));
			EmitterObj->SetObjectField(TEXT("analyticalCollisionPlanePosition1"), Vector3fToJson(Emitter.AnalyticalCollisionPlanePosition1));
			EmitterObj->SetObjectField(TEXT("analyticalCollisionPlanePosition2"), Vector3fToJson(Emitter.AnalyticalCollisionPlanePosition2));
			EmitterObj->SetNumberField(TEXT("bounceRestitution"), Emitter.BounceRestitution);
		}

		EmittersArray.Add(MakeShared<FJsonValueObject>(EmitterObj));
	}
	Root->SetArrayField(TEXT("emitters"), EmittersArray);

	TArray<TSharedPtr<FJsonValue>> ParentToSubArray;
	for (const TPair<FGuid, FGuid>& Pair : Asset->ParentToSubGuidMap)
	{
		TSharedRef<FJsonObject> PairObj = MakeShared<FJsonObject>();
		PairObj->SetStringField(TEXT("parentGuid"), Pair.Key.ToString(EGuidFormats::DigitsWithHyphens));
		PairObj->SetStringField(TEXT("subGuid"), Pair.Value.ToString(EGuidFormats::DigitsWithHyphens));
		ParentToSubArray.Add(MakeShared<FJsonValueObject>(PairObj));
	}
	Root->SetArrayField(TEXT("parentToSubEmitters"), ParentToSubArray);

	return Root;
}

#undef LOCTEXT_NAMESPACE
