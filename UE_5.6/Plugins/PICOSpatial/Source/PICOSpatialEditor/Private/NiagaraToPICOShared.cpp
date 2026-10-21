// Copyright Epic Games, Inc. All Rights Reserved.

#include "NiagaraToPICOShared.h"
#include "PICOSpatialEditor.h"
#include "PICOSpatialNiagaraAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraGraph.h"
#include "NiagaraScript.h"
#include "NiagaraScriptSource.h"
#include "NiagaraDataInterfaceCurve.h"
#include "NiagaraDataInterfaceColorCurve.h"
#include "NiagaraDataInterfaceVectorCurve.h"
#include "NiagaraNodeOutput.h"
#include "NiagaraScriptVariable.h"
#include "NiagaraSystemEditorData.h"
#include "FileHelpers.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Editor.h"

#if WITH_EDITOR
#include "PackageTools.h"
#endif // WITH_EDITOR

/**
 * Namespace containing utilities for converting Niagara systems to PICO Spatial format.
 */
namespace NiagaraToPICO
{
	static TAutoConsoleVariable<int32> CVarPICOSpatialNiagaraSilentSaveBeforeConvert(
		TEXT("PICO.Niagara.SilentSaveBeforeConvert"),
		0,
		TEXT("Whether Niagara-to-PICO conversion silently saves the source Niagara asset before conversion.\n")
		TEXT("0: Open/close editor only (default).\n")
		TEXT("1: Open editor and silently save the source Niagara asset before conversion."));

	// PICO Custom Module Tags
	const FString CurveModuleTag = TEXT("CurveModule");
	static const FString NewEnumerator0Tag = TEXT("NewEnumerator0");
	static const FString NewEnumerator1Tag = TEXT("NewEnumerator1");
	static const FString NewEnumerator2Tag = TEXT("NewEnumerator2");
	static const FString NewEnumerator3Tag = TEXT("NewEnumerator3");
	static const FString NewEnumerator4Tag = TEXT("NewEnumerator4");
	static const FString NewEnumerator5Tag = TEXT("NewEnumerator5");

	// Emitter State Tags
	const FString LifeCycleModeTag = TEXT("Life Cycle Mode");
	const FString LoopDurationTag = TEXT("Loop Duration");
	const FString LoopCountTag = TEXT("Loop Count");
	const FString LoopDelayTag = TEXT("Loop Delay");
	const FString EmitterStateTag = TEXT("EmitterState");
	const FString LoopBehaviorTag = TEXT("Loop Behavior");
	const FString UseLoopDelayTag = TEXT("UseLoopDelay");

	// Sub UV Animation Tags
	static const FString StartFrameRangeOverrideTag = TEXT("Start Frame Range Override");
	static const FString EndFrameRangeOverrideTag = TEXT("End Frame Range Override");
	const FString SubUVAnimationTag = TEXT("SubUVAnimation");
	static const FString UseStartFrameTag = TEXT("UseStartFrame");
	static const FString UseEndFrameTag = TEXT("UseEndFrame");

	// Initialize Particle Tags
	static const FString LifetimeTag = TEXT("Lifetime");
	static const FString LifetimeMaxTag = TEXT("Lifetime Max");
	static const FString LifetimeMinTag = TEXT("Lifetime Min");
	static const FString PositionOffsetTag = TEXT("Position Offset");
	static const FString PositionTag = TEXT("Position");
	static const FString UniformSpriteSizeTag = TEXT("Uniform Sprite Size");
	static const FString UniformSpriteSizeMaxTag = TEXT("Uniform Sprite Size Max");
	static const FString UniformSpriteSizeMinTag = TEXT("Uniform Sprite Size Min");
	static const FString SpriteSizeTag = TEXT("Sprite Size");
	static const FString SpriteSizeMaxTag = TEXT("Sprite Size Max");
	static const FString SpriteSizeMinTag = TEXT("Sprite Size Min");
	static const FString LifetimeModeTag = TEXT("Lifetime Mode");
	static const FString ScaleSpriteSizeModeTag = TEXT("Scale Sprite Size Mode");

	// Scale Size Tags
	static const FString UniformCurveSpriteScaleTag = TEXT("Uniform Curve Sprite Scale");
	static const FString UniformScaleFactorTag = TEXT("Uniform Scale Factor");
	static const FString NonUniformScaleTag = TEXT("Non Uniform Scale");
	static const FString MeshScaleTag = TEXT("Mesh Scale");
	static const FString MeshScaleMaxTag = TEXT("Mesh Scale Max");
	static const FString MeshScaleMinTag = TEXT("Mesh Scale Min");
	static const FString MeshUniformScaleTag = TEXT("Mesh Uniform Scale");
	static const FString MeshUniformScaleMaxTag = TEXT("Mesh Uniform Scale Max");
	static const FString MeshUniformScaleMinTag = TEXT("Mesh Uniform Scale Min");
	static const FString ScaleFactorTag = TEXT("Scale Factor");
	static const FString SpriteSizeModeTag = TEXT("Sprite Size Mode");
	static const FString MeshScaleModeTag = TEXT("Mesh Scale Mode");
	static const FString ScaleMeshSizeTag = TEXT("ScaleMeshSize");

	// Spawning Tags
	static const FString SpawnScriptTag = TEXT("SpawnScript");
	static const FString UpdateScriptTag = TEXT("UpdateScript");
	static const FString EventScriptTag = TEXT("EventScript");
	static const FString EmitterUpdateScriptTag = TEXT("EmitterUpdateScript");
	static const FString EmitterSpawnScriptTag = TEXT("EmitterSpawnScript");
	static const FString SpawnRateTag = TEXT("SpawnRate");
	static const FString SpawnCountTag = TEXT("Spawn Count");

	// Color Tags
	static const FString ColorCurveTag = TEXT("ColorCurve");
	static const FString ColorTag = TEXT("Color");
	static const FString ColorMaximumTag = TEXT("Color Maximum");
	static const FString ColorMinimumTag = TEXT("Color Minimum");
	static const FString ScaleAlphaTag = TEXT("Scale Alpha");
	static const FString ScaleColorTag = TEXT("Scale Color");
	static const FString ColorModeTag = TEXT("Color Mode");
	static const FString VectorCurveTag = TEXT("VectorCurve");
	static const FString MakeLinearColorFromVectorAndFloatTag = TEXT("MakeLinearColorFromVectorAndFloat");
	static const FString VectorRGBTag = TEXT("Vector (RGB)");

	// Mesh Tags
	static const FString MeshOrientationModeTag = TEXT("Mesh Orientation Mode");
	static const FString MeshOrientationMethodTag = TEXT("Orientation Method");

	// Force Tags
	static const FString AccelerationForceTag = TEXT("AccelerationForce");
	static const FString LinearForceTag = TEXT("LinearForce");
	static const FString AccelerationTag = TEXT("Acceleration");
	static const FString ForceTag = TEXT("Force");
	static const FString GravityTag = TEXT("Gravity");
	static const FString VectorNoiseForceTag = TEXT("VectorNoiseForce");
	static const FString NoiseForceAmountTag = TEXT("Force Amount");
	static const FString DragTag = TEXT("Drag");
	static const FString RotationalDragTag = TEXT("RotationalDrag");

	// Speed Tags
	static const FString VelocityModeTag = TEXT("Velocity Mode");
	static const FString VelocitySpeedTag = TEXT("Velocity Speed");
	static const FString VelocitySpeedScaleTag = TEXT("Velocity Speed Scale");
	static const FString VelocityTag = TEXT("Velocity");
	static const FString ConeAngleTag = TEXT("Cone Angle");
	static const FString VelocityScaleTag = TEXT("Velocity Scale");
	static const FString ScaleVelocityTag = TEXT("ScaleVelocity");

	// Rotation Tags
	static const FString SpriteRotationRateTag = TEXT("SpriteRotationRate");
	static const FString UpdateMeshOrientationTag = TEXT("UpdateMeshOrientation");
	static const FString SpriteRotationModeTag = TEXT("Sprite Rotation Mode");
	static const FString SpriteRotationAngleTag = TEXT("Sprite Rotation Angle");
	static const FString SpriteRotationAngleMaxTag = TEXT("Sprite Rotation Angle Max");
	static const FString SpriteRotationAngleMinTag = TEXT("Sprite Rotation Angle Min");
	static const FString OrientationMethodTag = TEXT("Orientation Method");
	static const FString RotationRateTag = TEXT("Rotation Rate");

	// Shape Location Tags
	static const FString ShapePrimitiveTag = TEXT("Shape Primitive");
	static const FString ShapeLocationTag = TEXT("ShapeLocation");
	static const FString PlaneSizeTag = TEXT("Plane Size");
	static const FString BoxSizeTag = TEXT("Box Size");
	static const FString BoxPlaneModeTag = TEXT("Box / Plane Mode");
	static const FString ConeAngleModeTag = TEXT("Cone Angle Mode");
	static const FString ConeLengthTag = TEXT("Cone Length");
	static const FString SphereSurfaceDistributionTag = TEXT("Sphere Surface Distribution");
	static const FString SphereRadiusTag = TEXT("Sphere Radius");
	static const FString CylinderRadiusTag = TEXT("Cylinder Radius");
	static const FString CylinderHeightTag = TEXT("Cylinder Height");
	static const FString LargeRadiusTag = TEXT("Large Radius");
	static const FString HandleRadiusTag = TEXT("Handle Radius");
	static const FString ConeRadialAngleTag = TEXT("Cone Radial Angle");

	// Collision Tags
	static const FString CPUCollisionTypeTag = TEXT("CPU Collision Type");
	static const FString AnalyticalCollisionPlanePosition1Tag = TEXT("Analytical Collision Plane Position 1");
	static const FString AnalyticalCollisionNormal1Tag = TEXT("Analytical Collision Normal 1");
	static const FString AnalyticalCollisionPlanePosition2Tag = TEXT("Analytical Collision Plane Position 2");
	static const FString AnalyticalCollisionNormal2Tag = TEXT("Analytical Collision Normal 2");
	static const FString BounceRestitutionTag = TEXT("Restitution");

	/**
	 * Enumeration of variable types for Niagara to PICO conversion
	 */
	enum class EVariableType : uint8
	{
		None,
		Color,
		ColorMaximum,
		ColorMinimum,
		SpawnRate,
		LifeTime,
		LifeTimeMin,
		LifeTimeMax,
		SpawnCount,
		UniformSpriteSize,
		UniformSpriteSizeMax,
		UniformSpriteSizeMin,
		SpriteSize,
		SpriteSizeMax,
		SpriteSizeMin,
		UniformScaleFactor,
		NonUniformScale,
		MeshScale,
		MeshScaleMax,
		MeshScaleMin,
		MeshUniformScale,
		MeshUniformScaleMax,
		MeshUniformScaleMin,
		ScaleFactor,
		PositionOffset,
		Force,
		Acceleration,
		Gravity,
		NoiseForceAmount,
		UniformCurveSpriteScale,
		ColorCurve,
		VectorFromCurve,
		ScaleVelocity,
		ScaleMeshSize,
		Drag,
		SpriteRotationAngle,
		SpriteRotationAngleMax,
		SpriteRotationAngleMin,
		SpriteRotationRate,
		RotationRate,
		VelocitySpeed,
		VelocitySpeedScale,
		Velocity,
		ConeAngle,
		StartFrameRangeOverride,
		EndFrameRangeOverride,
		LoopDuration,
		LoopCount,
		LoopDelay,
		AnalyticalCollisionPlanePosition1,
		AnalyticalCollisionNormal1,
		AnalyticalCollisionPlanePosition2,
		AnalyticalCollisionNormal2,
		BounceRestitution,
		SphereRadius,
		PlaneSize,
		BoxSize,
		CylinderHeight,
		CylinderRadius,
		ConeLength,
		LargeRadiusTag,
		HandleRadiusTag,
		ConeRadialAngleTag,
	};

	/** Mapping from tag strings to variable types */
	static const TMap<FString, EVariableType> TagToVariableTypeMap = []()
		{
			TMap<FString, EVariableType> Map;
			Map.Add(ColorTag, EVariableType::Color);
			Map.Add(ColorMaximumTag, EVariableType::ColorMaximum);
			Map.Add(ColorMinimumTag, EVariableType::ColorMinimum);
			Map.Add(SpawnRateTag, EVariableType::SpawnRate);
			Map.Add(LifetimeTag, EVariableType::LifeTime);
			Map.Add(LifetimeMinTag, EVariableType::LifeTimeMin);
			Map.Add(LifetimeMaxTag, EVariableType::LifeTimeMax);
			Map.Add(SpawnCountTag, EVariableType::SpawnCount);
			Map.Add(UniformSpriteSizeTag, EVariableType::UniformSpriteSize);
			Map.Add(UniformSpriteSizeMaxTag, EVariableType::UniformSpriteSizeMax);
			Map.Add(UniformSpriteSizeMinTag, EVariableType::UniformSpriteSizeMin);
			Map.Add(SpriteSizeTag, EVariableType::SpriteSize);
			Map.Add(SpriteSizeMaxTag, EVariableType::SpriteSizeMax);
			Map.Add(SpriteSizeMinTag, EVariableType::SpriteSizeMin);
			Map.Add(UniformScaleFactorTag, EVariableType::UniformScaleFactor);
			Map.Add(NonUniformScaleTag, EVariableType::NonUniformScale);
			Map.Add(MeshScaleTag, EVariableType::MeshScale);
			Map.Add(MeshScaleMaxTag, EVariableType::MeshScaleMax);
			Map.Add(MeshScaleMinTag, EVariableType::MeshScaleMin);
			Map.Add(MeshUniformScaleTag, EVariableType::MeshUniformScale);
			Map.Add(MeshUniformScaleMaxTag, EVariableType::MeshUniformScaleMax);
			Map.Add(MeshUniformScaleMinTag, EVariableType::MeshUniformScaleMin);
			Map.Add(ScaleFactorTag, EVariableType::ScaleFactor);
			Map.Add(PositionOffsetTag, EVariableType::PositionOffset);
			Map.Add(AccelerationTag, EVariableType::Acceleration);
			Map.Add(ForceTag, EVariableType::Force);
			Map.Add(GravityTag, EVariableType::Gravity);
			Map.Add(NoiseForceAmountTag, EVariableType::NoiseForceAmount);
			Map.Add(UniformCurveSpriteScaleTag, EVariableType::UniformCurveSpriteScale);
			Map.Add(ColorCurveTag, EVariableType::ColorCurve);
			Map.Add(ScaleVelocityTag, EVariableType::ScaleVelocity);
			Map.Add(ScaleMeshSizeTag, EVariableType::ScaleMeshSize);
			Map.Add(DragTag, EVariableType::Drag);
			Map.Add(SpriteRotationAngleTag, EVariableType::SpriteRotationAngle);
			Map.Add(SpriteRotationAngleMaxTag, EVariableType::SpriteRotationAngleMax);
			Map.Add(SpriteRotationAngleMinTag, EVariableType::SpriteRotationAngleMin);
			Map.Add(RotationRateTag, EVariableType::RotationRate);
			Map.Add(VelocitySpeedTag, EVariableType::VelocitySpeed);
			Map.Add(VelocitySpeedScaleTag, EVariableType::VelocitySpeedScale);
			Map.Add(VelocityTag, EVariableType::Velocity);
			Map.Add(ConeAngleTag, EVariableType::ConeAngle);
			Map.Add(StartFrameRangeOverrideTag, EVariableType::StartFrameRangeOverride);
			Map.Add(EndFrameRangeOverrideTag, EVariableType::EndFrameRangeOverride);
			Map.Add(LoopDurationTag, EVariableType::LoopDuration);
			Map.Add(LoopCountTag, EVariableType::LoopCount);
			Map.Add(LoopDelayTag, EVariableType::LoopDelay);
			Map.Add(AnalyticalCollisionPlanePosition1Tag, EVariableType::AnalyticalCollisionPlanePosition1);
			Map.Add(AnalyticalCollisionNormal1Tag, EVariableType::AnalyticalCollisionNormal1);
			Map.Add(AnalyticalCollisionPlanePosition2Tag, EVariableType::AnalyticalCollisionPlanePosition2);
			Map.Add(AnalyticalCollisionNormal2Tag, EVariableType::AnalyticalCollisionNormal2);
			Map.Add(BounceRestitutionTag, EVariableType::BounceRestitution);
			Map.Add(SphereRadiusTag, EVariableType::SphereRadius);
			Map.Add(PlaneSizeTag, EVariableType::PlaneSize);
			Map.Add(BoxSizeTag, EVariableType::BoxSize);
			Map.Add(CylinderHeightTag, EVariableType::CylinderHeight);
			Map.Add(CylinderRadiusTag, EVariableType::CylinderRadius);
			Map.Add(ConeLengthTag, EVariableType::ConeLength);
			Map.Add(LargeRadiusTag, EVariableType::LargeRadiusTag);
			Map.Add(HandleRadiusTag, EVariableType::HandleRadiusTag);
			Map.Add(ConeRadialAngleTag, EVariableType::ConeRadialAngleTag);
			return Map;
		}();

	/**
	 * Extracts the initialize particle module name from a stat ID string
	 */
	FString ExtractInitializeParticle(const FString& Input)
	{
		const int32 FirstDoubleSlashIndex = Input.Find(TEXT("//"));
		if (FirstDoubleSlashIndex == INDEX_NONE)
		{
			return FString();
		}

		const int32 SecondDoubleSlashIndex = Input.Find(TEXT("//"), ESearchCase::IgnoreCase, ESearchDir::FromStart, FirstDoubleSlashIndex + 2);
		if (SecondDoubleSlashIndex == INDEX_NONE)
		{
			return FString();
		}

		const int32 StartIndex = SecondDoubleSlashIndex + 2;
		const FString MiddlePart = Input.Mid(StartIndex);

		int32 DelimiterIndex = MiddlePart.Find(TEXT("####"));
		if (DelimiterIndex == INDEX_NONE)
		{
			DelimiterIndex = MiddlePart.Find(TEXT("_"));
		}

		FString Result;
		if (DelimiterIndex != INDEX_NONE)
		{
			Result = MiddlePart.Left(DelimiterIndex);
		}
		else
		{
			Result = MiddlePart;
		}

		const FString EmitterSuffix = TEXT("_Emitter");
		if (Result.EndsWith(EmitterSuffix))
		{
			Result = Result.LeftChop(EmitterSuffix.Len());
		}

		return Result;
	}

	/**
	 * Gets the item name from a long path name
	 */
	FString GetItemName(const FString& LongName)
	{
		int32 IndexOfLastSlash = INDEX_NONE;
		LongName.FindLastChar(TEXT('.'), IndexOfLastSlash);
		return LongName.Mid(IndexOfLastSlash + 1);
	}

	/**
	 * Gets the module name from a long path name
	 */
	FString GetModuleName(const FString& LongName)
	{
		int32 LastDotIndex = INDEX_NONE;
		LongName.FindLastChar(TEXT('.'), LastDotIndex);
		if (LastDotIndex == INDEX_NONE)
		{
			return LongName;
		}

		const FString SubStrBeforeLastDot = LongName.Left(LastDotIndex);

		int32 SecondLastDotIndex = INDEX_NONE;
		SubStrBeforeLastDot.FindLastChar(TEXT('.'), SecondLastDotIndex);
		if (SecondLastDotIndex == INDEX_NONE)
		{
			return SubStrBeforeLastDot;
		}

		return SubStrBeforeLastDot.Mid(SecondLastDotIndex + 1);
	}

	/**
	 * Splits a string by dot character
	 */
	bool SplitStringByDot(const FString& InputString, FString& OutFirstPart, FString& OutSecondPart)
	{
		const int32 DotIndex = InputString.Find(TEXT("."));
		if (DotIndex != INDEX_NONE)
		{
			OutFirstPart = InputString.Left(DotIndex);
			OutSecondPart = InputString.Mid(DotIndex + 1);
			return true;
		}
		return false;
	}

	/** Last node type name encountered during processing */
	FString LastNodeTypeName;

	/** Last node name encountered during processing */
	FString LastNodeName;

	/**
	 * Filters trailing numeric characters from a string
	 */
	FString FilterTrailingNumbers(const FString& Input)
	{
		int32 LastNonDigitIndex = Input.Len() - 1;
		while (LastNonDigitIndex >= 0 && FChar::IsDigit(Input[LastNonDigitIndex]))
		{
			--LastNonDigitIndex;
		}

		if (LastNonDigitIndex >= 0)
		{
			return Input.Left(LastNonDigitIndex + 1);
		}
		return FString();
	}

	/**
	 * Converts a Niagara graph to a Niagara asset
	 */
	bool NiagaraGraphToNiagaraAsset(const UNiagaraGraph* NodeGraph, UPICOSpatialNiagaraAsset* NiagaraAsset)
	{
		if (!NodeGraph || !NiagaraAsset)
		{
			return false;
		}

		UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("NiagaraSystem Name:%s"), *NiagaraAsset->NiagaraSystemName);

		for (const UEdGraphNode* Node : NodeGraph->Nodes)
		{
			if (!Node)
			{
				continue;
			}

			for (const UEdGraphPin* Pin : Node->Pins)
			{
				if (!Pin)
				{
					continue;
				}

				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("NiagaraSystem Node Name:%s"), *Pin->GetName());
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("NiagaraSystem Node DefaultAsString:%s"), *Pin->GetDefaultAsString());

				if (Pin->PinName.ToString() == LoopBehaviorTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();

					if (DefaultValue == NewEnumerator0Tag)
					{
						NiagaraAsset->LoopBehavior = ENiagaraPICO_LoopBehavior::Infinite;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						NiagaraAsset->LoopBehavior = ENiagaraPICO_LoopBehavior::Once;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						NiagaraAsset->LoopBehavior = ENiagaraPICO_LoopBehavior::Multiple;
					}
				}
				else if (Pin->PinName.ToString() == UseLoopDelayTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					NiagaraAsset->bUseLoopDelay = DefaultValue == TEXT("true");
				}
			}
		}

		return true;
	}

	/**
	 * Converts a Niagara graph to a Niagara emitter
	 */
	bool NiagaraGraphToNiagaraEmitter(const UNiagaraGraph* NodeGraph, FNiagaraEmitterPICO& Emitter)
	{
		if (!NodeGraph)
		{
			return false;
		}

		UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Node Emitter Name:%s"), *Emitter.EmitterName);

		for (const UEdGraphNode* Node : NodeGraph->Nodes)
		{
			if (!Node)
			{
				continue;
			}

			for (const UEdGraphPin* Pin : Node->Pins)
			{
				if (!Pin)
				{
					continue;
				}

				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Node Name:%s"), *Pin->GetName());
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Node DefaultAsString:%s"), *Pin->GetDefaultAsString());

				FString NodeName;
				FString NodeTypeName;
				if (SplitStringByDot(Pin->GetName(), NodeName, NodeTypeName))
				{
					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("NodeTypeName:%s"), *NodeTypeName);
					if (LastNodeTypeName == VelocityScaleTag && NodeTypeName == VectorCurveTag)
					{
						Emitter.SubNodeToNodeMap.Add(NodeName, LastNodeName);
					}
					else if (LastNodeTypeName == VectorRGBTag && NodeTypeName == VectorCurveTag)
					{
						Emitter.SubNodeToNodeMap.Add(NodeName, LastNodeName);
					}
					else if (LastNodeTypeName == ScaleFactorTag && NodeTypeName == VectorCurveTag)
					{
						Emitter.SubNodeToNodeMap.Add(NodeName, LastNodeName);
					}

					LastNodeTypeName = NodeTypeName;
					LastNodeName = NodeName;
				}

				if (Pin && Pin->PinName.ToString() == ShapePrimitiveTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();

					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.ShapeType = ENiagaraPICO_ShapeType::Sphere;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.ShapeType = ENiagaraPICO_ShapeType::Cylinder;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.ShapeType = ENiagaraPICO_ShapeType::BoxOrPlane;
					}
					else if (DefaultValue == NewEnumerator3Tag)
					{
						Emitter.ShapeType = ENiagaraPICO_ShapeType::Torus;
					}
					else if (DefaultValue == NewEnumerator4Tag)
					{
						Emitter.ShapeType = ENiagaraPICO_ShapeType::Ring;
					}
					else if (DefaultValue == NewEnumerator5Tag)
					{
						Emitter.ShapeType = ENiagaraPICO_ShapeType::Cone;
					}
				}
				else if (Pin && Pin->PinName.ToString() == SpriteSizeModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();

					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.SizeMode = ENiagaraPICO_SizeMode::Unset;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.SizeMode = ENiagaraPICO_SizeMode::RandomUniform;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.SizeMode = ENiagaraPICO_SizeMode::RandomNonUniform;
					}
					else if (DefaultValue == NewEnumerator3Tag)
					{
						Emitter.SizeMode = ENiagaraPICO_SizeMode::Uniform;
					}
					else if (DefaultValue == NewEnumerator4Tag)
					{
						Emitter.SizeMode = ENiagaraPICO_SizeMode::NonUniform;
					}
				}
				else if (Pin && Pin->PinName.ToString() == MeshScaleModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();

					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.MeshScaleInitializeMode = ENiagaraPICO_SizeMode::Unset;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.MeshScaleInitializeMode = ENiagaraPICO_SizeMode::RandomUniform;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.MeshScaleInitializeMode = ENiagaraPICO_SizeMode::RandomNonUniform;
					}
					else if (DefaultValue == NewEnumerator3Tag)
					{
						Emitter.MeshScaleInitializeMode = ENiagaraPICO_SizeMode::Uniform;
					}
					else if (DefaultValue == NewEnumerator4Tag)
					{
						Emitter.MeshScaleInitializeMode = ENiagaraPICO_SizeMode::NonUniform;
					}
				}
				else if (Pin && Pin->PinName.ToString() == LifetimeModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();

					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.LifeTimeMode = ENiagaraPICO_LifeTimeMode::DirectSet;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.LifeTimeMode = ENiagaraPICO_LifeTimeMode::Random;
					}
				}
				// else if (Pin && Pin->PinName.ToString() == ScaleSpriteSizeModeTag)
				// {
				// 	FString DefaultValue = Pin->GetDefaultAsString();
				// 	if (DefaultValue == NewEnumerator0Tag)
				// 	{
				// 		Emitter.ScaleSizeMode = Emitter.ScaleSizeMode == ENiagaraPICO_ScaleSizeMode::UniformCurve?ENiagaraPICO_ScaleSizeMode::UniformCurve:ENiagaraPICO_ScaleSizeMode::Uniform;
				// 	}
				// 	else if (DefaultValue == NewEnumerator1Tag)
				// 	{
				// 		Emitter.ScaleSizeMode = ENiagaraPICO_ScaleSizeMode::UniformCurve;
				// 	}
				// 	else if (DefaultValue == NewEnumerator2Tag)
				// 	{
				// 		Emitter.ScaleSizeMode = Emitter.ScaleSizeMode == ENiagaraPICO_ScaleSizeMode::UniformCurve?ENiagaraPICO_ScaleSizeMode::UniformCurve:ENiagaraPICO_ScaleSizeMode::NonUniform;
				// 	}
				// }
				else if (Pin && Pin->PinName.ToString() == ColorModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.ColorMode = ENiagaraPICO_ColorMode::Unset;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.ColorMode = ENiagaraPICO_ColorMode::DirectSet;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.ColorMode = ENiagaraPICO_ColorMode::RandomRange;
					}
					else if (DefaultValue == NewEnumerator3Tag)
					{
						Emitter.ColorMode = ENiagaraPICO_ColorMode::RandomHue;
					}
				}
				else if (Pin && Pin->PinName.ToString() == SpriteRotationModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.SpriteRotationMode = ENiagaraPICO_SpriteRotationMode::DirectAngle;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.SpriteRotationMode = ENiagaraPICO_SpriteRotationMode::DirectNormalizedAngle;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.SpriteRotationMode = ENiagaraPICO_SpriteRotationMode::Random;
					}
					else if (DefaultValue == NewEnumerator3Tag)
					{
						Emitter.SpriteRotationMode = ENiagaraPICO_SpriteRotationMode::Unset;
					}
				}
				else if (Pin && Pin->PinName.ToString() == OrientationMethodTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.OrientationMethod = ENiagaraPICO_OrientationMethod::RotationRate;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.OrientationMethod = ENiagaraPICO_OrientationMethod::OrientToVector_S;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.OrientationMethod = ENiagaraPICO_OrientationMethod::OrientToPosition;
					}
					else if (DefaultValue == NewEnumerator3Tag)
					{
						Emitter.OrientationMethod = ENiagaraPICO_OrientationMethod::FlightOrientation;
					}
					else if (DefaultValue == NewEnumerator4Tag)
					{
						Emitter.OrientationMethod = ENiagaraPICO_OrientationMethod::RollingOrientation;
					}
				}
				else if (Pin && Pin->PinName.ToString() == VelocityModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.VelocityMode = ENiagaraPICO_VelocityMode::Linear;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.VelocityMode = ENiagaraPICO_VelocityMode::FromPoint;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.VelocityMode = ENiagaraPICO_VelocityMode::InCone;
					}
				}
				else if (Pin && Pin->PinName.ToString() == BoxPlaneModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.BoxOrPlaneMode = ENiagaraPICO_BoxOrPlaneMode::Box;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.BoxOrPlaneMode = ENiagaraPICO_BoxOrPlaneMode::Plane;
					}
				}
				else if (Pin && Pin->PinName.ToString() == ConeAngleModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.ConeAngleMode = ENiagaraPICO_ConeAngleMode::Degrees;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.ConeAngleMode = ENiagaraPICO_ConeAngleMode::NormalizedAngle;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.ConeAngleMode = ENiagaraPICO_ConeAngleMode::Radians;
					}
				}
				else if (Pin && Pin->PinName.ToString() == UseStartFrameTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					Emitter.bUseStartFrame = DefaultValue == TEXT("true");
				}
				else if (Pin && Pin->PinName.ToString() == UseEndFrameTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					Emitter.bUseEndFrame = DefaultValue == TEXT("true");
				}
				else if (Pin && Pin->PinName.ToString() == UseLoopDelayTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					Emitter.bUseLoopDelay = DefaultValue == TEXT("true");
				}
				else if (Pin && Pin->PinName.ToString() == LoopBehaviorTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.LoopBehavior = ENiagaraPICO_LoopBehavior::Infinite;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.LoopBehavior = ENiagaraPICO_LoopBehavior::Once;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.LoopBehavior = ENiagaraPICO_LoopBehavior::Multiple;
					}
				}
				else if (Pin && Pin->PinName.ToString() == CPUCollisionTypeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.bEnableCollision = false;
					}
					else if (DefaultValue == NewEnumerator3Tag)
					{
						Emitter.bEnableCollision = true;
					}
				}
				else if (Pin && Pin->PinName.ToString() == MeshOrientationModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.MeshOrientationMode = ENiagaraPICO_MeshOrientationMode::Random;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.MeshOrientationMode = ENiagaraPICO_MeshOrientationMode::System;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.MeshOrientationMode = ENiagaraPICO_MeshOrientationMode::OrientToVector;
					}
					else if (DefaultValue == NewEnumerator3Tag)
					{
						Emitter.MeshOrientationMode = ENiagaraPICO_MeshOrientationMode::OrientToMatrix;
					}
					else if (DefaultValue == NewEnumerator4Tag)
					{
						Emitter.MeshOrientationMode = ENiagaraPICO_MeshOrientationMode::OrientToQuaternion;
					}
				}
				else if (Pin && Pin->PinName.ToString() == MeshOrientationMethodTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.UpdateMeshOrientationMethod = ENiagaraPICO_UpdateMeshOrientationMethod::RotationRate;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.UpdateMeshOrientationMethod = ENiagaraPICO_UpdateMeshOrientationMethod::OrientToVector_S;
					}
					else if (DefaultValue == NewEnumerator2Tag)
					{
						Emitter.UpdateMeshOrientationMethod = ENiagaraPICO_UpdateMeshOrientationMethod::OrientToPosition;
					}
					else if (DefaultValue == NewEnumerator3Tag)
					{
						Emitter.UpdateMeshOrientationMethod = ENiagaraPICO_UpdateMeshOrientationMethod::FlightOrientation;
					}
					else if (DefaultValue == NewEnumerator4Tag)
					{
						Emitter.UpdateMeshOrientationMethod = ENiagaraPICO_UpdateMeshOrientationMethod::RollingOrientation;
					}
				}
				else if (Pin && Pin->PinName.ToString() == LifeCycleModeTag)
				{
					const FString DefaultValue = Pin->GetDefaultAsString();
					if (DefaultValue == NewEnumerator0Tag)
					{
						Emitter.LifeCycleMode = ENiagaraPICO_LifeCycleMode::System;
					}
					else if (DefaultValue == NewEnumerator1Tag)
					{
						Emitter.LifeCycleMode = ENiagaraPICO_LifeCycleMode::Self;
					}
				}
			}
		}
		return true;
	}

	/**
	 * Converts an array of Niagara scripts to a Niagara asset
	 */
	TArray<FString> ModuleNames;
	bool NiagaraScriptsToNiagaraAsset(const TArray<UNiagaraScript*>& Scripts, FNiagaraEmitterPICO& Emitter)
	{
		ModuleNames.Empty();
		//Stored all structured Niagara data
		struct FNiagara2PICOVariable
		{
			TMap<EVariableType, const FNiagaraVariableWithOffset*> ItemVarMap;
			TMap<EVariableType, FNiagaraCurvePICO > ItemCurveMap;
			TMap<EVariableType, FNiagaraVectorCurvePICO > ItemVectorCurveMap;
			TMap<EVariableType, FNiagaraColorCurvePICO > ItemColorCurveMap;

			FNiagara2PICOVariable(const TMap<EVariableType, const FNiagaraVariableWithOffset*>& InItemVarMap)
				: ItemVarMap(InItemVarMap)
			{
			}

			FNiagara2PICOVariable(const TMap<EVariableType, FNiagaraCurvePICO>& InItemCurveMap)
				: ItemCurveMap(InItemCurveMap)
			{
			}

			FNiagara2PICOVariable(const TMap<EVariableType, FNiagaraVectorCurvePICO>& InItemVectorCurveMap)
				: ItemVectorCurveMap(InItemVectorCurveMap)
			{
			}

			FNiagara2PICOVariable(const TMap<EVariableType, FNiagaraColorCurvePICO>& InItemColorCurveMap)
				: ItemColorCurveMap(InItemColorCurveMap)
			{
			}

			FNiagara2PICOVariable() = default;
		};

		TMap<FString, FNiagara2PICOVariable> Niagara2PICOVariables;

		for (UNiagaraScript* Script : Scripts)
		{
			//TODO for now only support spawn script and EmitterUpdateScript
			if (Script->GetName() == UpdateScriptTag
				|| Script->GetName() == EmitterSpawnScriptTag)
			{
				continue;
			}

			for (const FNiagaraVariableWithOffset& Var : Script->RapidIterationParameters.ReadParameterVariables())
			{
				const int32 Offset = Var.Offset;
				const int32 SizeInBytes = Var.GetSizeInBytes();

				FString ItemName = GetItemName(Var.GetName().ToString());
				FString ModuleName = GetModuleName(Var.GetName().ToString());

				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable GetName:%s"), *Var.GetName().ToString());
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable TypeName:%s"), *ItemName);
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Module:%s"), *ModuleName);

				//TODO:this part will be deleted on future,for now only use for test
				if (Var.GetType() == FNiagaraTypeDefinition::GetFloatDef())
				{
					float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Offset, SizeInBytes);

					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Val:%f"), Val);
					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Type FloatDef"));
				}
				else if (Var.GetType() == FNiagaraTypeDefinition::GetIntDef())
				{
					int32 Val = *(int32*)Script->RapidIterationParameters.GetParameterData(Offset, SizeInBytes);

					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Val:%d"), Val);
					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Type IntDef"));
				}
				else if (Var.GetType() == FNiagaraTypeDefinition::GetVec2Def())
				{
					FVector2f Val = *(FVector2f*)Script->RapidIterationParameters.GetParameterData(Offset, SizeInBytes);

					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Val:%s"), *Val.ToString());
					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Type Vec2Def"));
				}
				else if (Var.GetType() == FNiagaraTypeDefinition::GetVec3Def())
				{
					FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Offset, SizeInBytes);

					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Val:%s"), *Val.ToString());
					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Type Vec3Def"));
				}
				else if (Var.GetType() == FNiagaraTypeDefinition::GetVec4Def())
				{
					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Type Vec4Def"));
				}
				else if (Var.GetType() == FNiagaraTypeDefinition::GetColorDef())
				{
					FLinearColor Val = *(FLinearColor*)Script->RapidIterationParameters.GetParameterData(Offset, SizeInBytes);

					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Val:%s"), *Val.ToString());
					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Type ColorDef"));
				}
				else if (Var.GetType() == FNiagaraTypeDefinition::GetParameterMapDef())
				{
					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Type ParameterMapDef"));
				}
				else if (Var.GetType() == FNiagaraTypeDefinition::GetUObjectDef())
				{
					UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("Variable Type ParameterMapDef"));
				}

				//Structured as enumerated values through each VariableName
				EVariableType VariableType = EVariableType::None;

				if (TagToVariableTypeMap.Contains(ItemName))
				{
					if (ItemName == SpawnRateTag)
					{
						ModuleName = SpawnScriptTag;
					}
					else if (ItemName == SpawnCountTag)
					{
						ModuleName = SpawnScriptTag;
					}
					if (ItemName == LoopDurationTag)
					{
						ModuleName = EmitterStateTag;
					}
					else if (ItemName == LoopCountTag)
					{
						ModuleName = EmitterStateTag;
					}
					else if (ItemName == LoopDelayTag)
					{
						ModuleName = EmitterStateTag;
					}

					VariableType = TagToVariableTypeMap[ItemName];
				}

				if (Niagara2PICOVariables.Contains(ModuleName))
				{
					Niagara2PICOVariables[ModuleName].ItemVarMap.Add(VariableType, &Var);
				}
				else
				{
					TMap<EVariableType, const FNiagaraVariableWithOffset*> Vars;
					Vars.Add(VariableType, &Var);
					Niagara2PICOVariables.Add(ModuleName, FNiagara2PICOVariable{ Vars });
				}
			}

			for (const FNiagaraScriptResolvedDataInterfaceInfo& DataInterfaceInfo : Script->GetResolvedDataInterfaces())
			{
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("DataInterfaceInfo:%s"), *DataInterfaceInfo.Name.ToString());
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("ResolvedVariable:%s"), *DataInterfaceInfo.ResolvedVariable.GetName().ToString());
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("ParameterStoreVariable:%s"), *DataInterfaceInfo.ParameterStoreVariable.GetName().ToString());

				FString ModuleName = GetModuleName(DataInterfaceInfo.Name.ToString());
				FString ItemName = GetItemName(DataInterfaceInfo.Name.ToString());
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("DataInterfaceInfo ItemName:%s"), *ItemName);
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("DataInterfaceInfo ModuleName:%s"), *ModuleName);

				if (Emitter.SubNodeToNodeMap.Contains(ModuleName))
				{
					FString NiagaraModuleName = Emitter.SubNodeToNodeMap[ModuleName];
					FString NiagaraModuleType = FilterTrailingNumbers(NiagaraModuleName);

					if (NiagaraModuleType == ScaleVelocityTag)
					{
						if (UNiagaraDataInterfaceVectorCurve* CurveDI = Cast<UNiagaraDataInterfaceVectorCurve>(DataInterfaceInfo.ResolvedDataInterface))
						{
							if (Niagara2PICOVariables.Contains(CurveModuleTag))
							{
								FNiagaraVectorCurvePICO NiagaraVectorCurvePICO = {};
								NiagaraVectorCurvePICO.XCurve.Keys = CurveDI->XCurve.Keys;
								NiagaraVectorCurvePICO.YCurve.Keys = CurveDI->YCurve.Keys;
								NiagaraVectorCurvePICO.ZCurve.Keys = CurveDI->ZCurve.Keys;
								Niagara2PICOVariables[CurveModuleTag].ItemVectorCurveMap.Add(EVariableType::ScaleVelocity, NiagaraVectorCurvePICO);
							}
							else
							{
								TMap<EVariableType, FNiagaraVectorCurvePICO > ItemVectorCurveMap;
								FNiagaraVectorCurvePICO NiagaraVectorCurvePICO = {};
								NiagaraVectorCurvePICO.XCurve.Keys = CurveDI->XCurve.Keys;
								NiagaraVectorCurvePICO.YCurve.Keys = CurveDI->YCurve.Keys;
								NiagaraVectorCurvePICO.ZCurve.Keys = CurveDI->ZCurve.Keys;
								ItemVectorCurveMap.Add(EVariableType::ScaleVelocity, NiagaraVectorCurvePICO);
								Niagara2PICOVariables.Add(CurveModuleTag, FNiagara2PICOVariable{ ItemVectorCurveMap });
							}
						}
					}
					else if (NiagaraModuleType == MakeLinearColorFromVectorAndFloatTag)
					{
						if (UNiagaraDataInterfaceVectorCurve* CurveDI = Cast<UNiagaraDataInterfaceVectorCurve>(DataInterfaceInfo.ResolvedDataInterface))
						{
							if (Niagara2PICOVariables.Contains(CurveModuleTag))
							{
								FNiagaraColorCurvePICO NiagaraColorCurvePICO = {};
								NiagaraColorCurvePICO.RedCurve.Keys = CurveDI->XCurve.Keys;
								NiagaraColorCurvePICO.GreenCurve.Keys = CurveDI->YCurve.Keys;
								NiagaraColorCurvePICO.BlueCurve.Keys = CurveDI->ZCurve.Keys;
								//NiagaraColorCurvePICO.AlphaCurve.Keys=ColorCurve->AlphaCurve.Keys;
								Niagara2PICOVariables[CurveModuleTag].ItemColorCurveMap.Add(EVariableType::ColorCurve, NiagaraColorCurvePICO);
							}
							else
							{
								TMap<EVariableType, FNiagaraColorCurvePICO > ItemColorCurveMap;
								FNiagaraColorCurvePICO NiagaraColorCurvePICO = {};
								NiagaraColorCurvePICO.RedCurve.Keys = CurveDI->XCurve.Keys;
								NiagaraColorCurvePICO.GreenCurve.Keys = CurveDI->YCurve.Keys;
								NiagaraColorCurvePICO.BlueCurve.Keys = CurveDI->ZCurve.Keys;
								//NiagaraColorCurvePICO.AlphaCurve.Keys=ColorCurve->AlphaCurve.Keys;
								ItemColorCurveMap.Add(EVariableType::ColorCurve, NiagaraColorCurvePICO);
								Niagara2PICOVariables.Add(CurveModuleTag, FNiagara2PICOVariable{ ItemColorCurveMap });
							}
						}
					}
					else if (NiagaraModuleType == ScaleMeshSizeTag)
					{
						if (UNiagaraDataInterfaceVectorCurve* CurveDI = Cast<UNiagaraDataInterfaceVectorCurve>(DataInterfaceInfo.ResolvedDataInterface))
						{
							if (Niagara2PICOVariables.Contains(CurveModuleTag))
							{
								FNiagaraVectorCurvePICO NiagaraVectorCurvePICO = {};
								NiagaraVectorCurvePICO.XCurve.Keys = CurveDI->XCurve.Keys;
								NiagaraVectorCurvePICO.YCurve.Keys = CurveDI->YCurve.Keys;
								NiagaraVectorCurvePICO.ZCurve.Keys = CurveDI->ZCurve.Keys;
								Niagara2PICOVariables[CurveModuleTag].ItemVectorCurveMap.Add(EVariableType::ScaleMeshSize, NiagaraVectorCurvePICO);
							}
							else
							{
								TMap<EVariableType, FNiagaraVectorCurvePICO > ItemVectorCurveMap;
								FNiagaraVectorCurvePICO NiagaraVectorCurvePICO = {};
								NiagaraVectorCurvePICO.XCurve.Keys = CurveDI->XCurve.Keys;
								NiagaraVectorCurvePICO.YCurve.Keys = CurveDI->YCurve.Keys;
								NiagaraVectorCurvePICO.ZCurve.Keys = CurveDI->ZCurve.Keys;
								ItemVectorCurveMap.Add(EVariableType::ScaleMeshSize, NiagaraVectorCurvePICO);
								Niagara2PICOVariables.Add(CurveModuleTag, FNiagara2PICOVariable{ ItemVectorCurveMap });
							}
						}
					}
				}

				if (ItemName == UniformCurveSpriteScaleTag)
				{
					if (UNiagaraDataInterfaceCurve* CurveDI = Cast<UNiagaraDataInterfaceCurve>(DataInterfaceInfo.ResolvedDataInterface))
					{
						if (Niagara2PICOVariables.Contains(CurveModuleTag))
						{
							FNiagaraCurvePICO NiagaraCurvePICO = {};
							NiagaraCurvePICO.Keys = CurveDI->Curve.Keys;
							Niagara2PICOVariables[CurveModuleTag].ItemCurveMap.Add(EVariableType::UniformCurveSpriteScale, NiagaraCurvePICO);
						}
						else
						{
							TMap<EVariableType, FNiagaraCurvePICO > ItemCurveMap;
							FNiagaraCurvePICO NiagaraCurvePICO = {};
							NiagaraCurvePICO.Keys = CurveDI->Curve.Keys;
							ItemCurveMap.Add(EVariableType::UniformCurveSpriteScale, NiagaraCurvePICO);
							Niagara2PICOVariables.Add(CurveModuleTag, FNiagara2PICOVariable{ ItemCurveMap });
						}
					}
				}
				else if (ItemName == ColorCurveTag)
				{
					UNiagaraDataInterfaceColorCurve* ColorCurve = Cast<UNiagaraDataInterfaceColorCurve>(DataInterfaceInfo.ResolvedDataInterface);
					if (ColorCurve)
					{
						UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("DataInterfaceInfo Keys:%d"), ColorCurve->RedCurve.Keys.Num());
						if (Niagara2PICOVariables.Contains(CurveModuleTag))
						{
							FNiagaraColorCurvePICO NiagaraColorCurvePICO = {};
							NiagaraColorCurvePICO.RedCurve.Keys = ColorCurve->RedCurve.Keys;
							NiagaraColorCurvePICO.GreenCurve.Keys = ColorCurve->GreenCurve.Keys;
							NiagaraColorCurvePICO.BlueCurve.Keys = ColorCurve->BlueCurve.Keys;
							NiagaraColorCurvePICO.AlphaCurve.Keys = ColorCurve->AlphaCurve.Keys;
							Niagara2PICOVariables[CurveModuleTag].ItemColorCurveMap.Add(EVariableType::ColorCurve, NiagaraColorCurvePICO);
						}
						else
						{
							TMap<EVariableType, FNiagaraColorCurvePICO > ItemColorCurveMap;
							FNiagaraColorCurvePICO NiagaraColorCurvePICO = {};
							NiagaraColorCurvePICO.RedCurve.Keys = ColorCurve->RedCurve.Keys;
							NiagaraColorCurvePICO.GreenCurve.Keys = ColorCurve->GreenCurve.Keys;
							NiagaraColorCurvePICO.BlueCurve.Keys = ColorCurve->BlueCurve.Keys;
							NiagaraColorCurvePICO.AlphaCurve.Keys = ColorCurve->AlphaCurve.Keys;
							ItemColorCurveMap.Add(EVariableType::ColorCurve, NiagaraColorCurvePICO);
							Niagara2PICOVariables.Add(CurveModuleTag, FNiagara2PICOVariable{ ItemColorCurveMap });
						}
					}
				}
			}

			if (Script->GetName() == EmitterUpdateScriptTag)
			{
				if (Niagara2PICOVariables.Contains(SpawnScriptTag))
				{
					for (auto Pair : Niagara2PICOVariables[SpawnScriptTag].ItemVarMap)
					{
						switch (Pair.Key)
						{
						case EVariableType::SpawnCount:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetIntDef())
							{
								int32 Val = *(int32*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.BurstSpawnCount = Val;
							}
						}
						break;
						case EVariableType::SpawnRate:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.SpawnRate = Val;
							}
						}
						break;
						default:;
						}

					}

				}

				if (Niagara2PICOVariables.Contains(EmitterStateTag))
				{
					for (auto Pair : Niagara2PICOVariables[EmitterStateTag].ItemVarMap)
					{
						switch (Pair.Key)
						{
						case EVariableType::LoopCount:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetIntDef())
							{
								int32 Val = *(int32*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.LoopCount = Val;
							}
						}
						break;
						case EVariableType::LoopDuration:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.LoopDuration = Val;
							}
						}
						break;
						case EVariableType::LoopDelay:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.LoopDelay = Val;
							}
						}
						break;
						default:;
						}

					}

				}
			}


			for (const TStatId StatId : Script->GetStatScopeIDs())
			{
				FString ModuleName = ExtractInitializeParticle(StatId.GetName().ToString());

				ModuleNames.Add(ModuleName);
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("StatId ModuleName:%s"), *ModuleName);
				UE_LOG(LogPICOSpatialEditor, VeryVerbose, TEXT("StatId:%s"), *StatId.GetName().ToString());

				if (ModuleName == SubUVAnimationTag)
				{
					Emitter.bSubUVAnimation = true;
				}

				if (Niagara2PICOVariables.Contains(ModuleName))
				{
					for (auto Pair : Niagara2PICOVariables[ModuleName].ItemVarMap)
					{
						switch (Pair.Key) {
						case EVariableType::None:
							break;
						case EVariableType::LifeTime:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.LifeTime = Val;
							}
						}
						break;
						case EVariableType::LifeTimeMin:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.LifeTimeMin = Val;
							}
						}
						break;
						case EVariableType::LifeTimeMax:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.LifeTimeMax = Val;
							}
						}
						break;
						case EVariableType::Color:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetColorDef())
							{
								FLinearColor Val = *(FLinearColor*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.Color = Val;
							}
						}
						break;
						case EVariableType::ColorMaximum:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetColorDef())
							{
								FLinearColor Val = *(FLinearColor*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.ColorMaximum = Val;
							}
						}
						break;
						case EVariableType::ColorMinimum:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetColorDef())
							{
								FLinearColor Val = *(FLinearColor*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.ColorMinimum = Val;
							}
						}
						break;
						case EVariableType::UniformSpriteSize:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.UniformSpriteSize = Val;
							}
						}
						break;
						case EVariableType::UniformSpriteSizeMin:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.UniformSpriteSizeMin = Val;
							}
						}
						break;
						case EVariableType::UniformSpriteSizeMax:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.UniformSpriteSizeMax = Val;
							}
						}
						break;
						case EVariableType::SpriteSize:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec2Def())
							{
								FVector2f Val = *(FVector2f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.SpriteSize = Val;
							}
						}
						break;
						case EVariableType::SpriteSizeMax:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec2Def())
							{
								FVector2f Val = *(FVector2f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.SpriteSizeMax = Val;
							}
						}
						break;
						case EVariableType::SpriteSizeMin:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec2Def())
							{
								FVector2f Val = *(FVector2f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.SpriteSizeMin = Val;
							}
						}
						break;
						case EVariableType::UniformScaleFactor:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.UniformScaleFactor *= Val;
								Emitter.ScaleSizeMode = ENiagaraPICO_ScaleSizeMode::Uniform;
							}
						}
						break;
						case EVariableType::MeshScale:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (!Val.ContainsNaN())
								{
									Emitter.MeshNonUniformScale *= Val;
								}
							}
						}
						break;
						case EVariableType::MeshScaleMax:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (!Val.ContainsNaN())
								{
									Emitter.MeshNonUniformScaleMax *= Val;
								}
							}
						}
						break;
						case EVariableType::MeshScaleMin:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (!Val.ContainsNaN())
								{
									Emitter.MeshNonUniformScaleMin *= Val;
								}
							}
						}
						break;
						case EVariableType::MeshUniformScale:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.MeshUniformScale = Val;
							}
						}
						break;
						case EVariableType::MeshUniformScaleMax:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.MeshUniformScaleMax = Val;
							}
						}
						break;
						case EVariableType::MeshUniformScaleMin:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.MeshUniformScaleMin = Val;
							}
						}
						break;
						case EVariableType::ScaleFactor:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.MeshNonUniformScaleFactor *= Val;
								Emitter.MeshScaleUpdateMode = ENiagaraPICO_ScaleSizeMode::NonUniform;
							}
							else if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec2Def())
							{
								FVector2f Val = *(FVector2f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.NonUniformScaleFactor *= Val;
								Emitter.ScaleSizeMode = ENiagaraPICO_ScaleSizeMode::Uniform;
							}
						}
						break;
						case EVariableType::Acceleration:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.TotalForce += Val;
							}
						}
						break;
						case EVariableType::Force:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.TotalForce += Val;
							}
						}
						break;
						case EVariableType::Gravity:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.TotalForce += Val;
							}
						}
						break;
						case EVariableType::NoiseForceAmount:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.NoiseForceAmount = Val;
							}
						}
						break;
						case EVariableType::Velocity:
						{
							if (Emitter.VelocityMode == ENiagaraPICO_VelocityMode::Linear)
							{
								if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
								{
									FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
									Emitter.Velocity += Val;
								}
							}
						}
						break;
						case EVariableType::ConeAngle:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (ModuleName == ShapeLocationTag)
								{
									Emitter.ShapeConeAngle = Val;
								}
								else
								{
									Emitter.ConeAngle = Val;
								}
							}
						}
						break;
						case EVariableType::ConeLength:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.ShapeConeLength = Val;
							}
						}
						break;
						case EVariableType::Drag:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.Drag = Val;
							}
						}
						break;
						case EVariableType::SpriteRotationAngle:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.SpriteRotationAngle = Val;
							}
						}
						break;
						case EVariableType::SpriteRotationAngleMax:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.SpriteRotationAngleMax = Val;
							}
						}
						break;
						case EVariableType::SpriteRotationAngleMin:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.SpriteRotationAngleMin = Val;
							}
						}
						break;
						case EVariableType::RotationRate:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (ModuleName == SpriteRotationRateTag)
								{
									Emitter.SpriteRotationRate = Val;
								}
								else if (ModuleName == UpdateMeshOrientationTag)
								{
									Emitter.MeshRotationRate = Val;
								}
							}
						}
						break;
						case EVariableType::VelocitySpeed:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.VelocitySpeed = Val;
							}
						}
						break;
						case EVariableType::VelocitySpeedScale:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.VelocitySpeedScale = Val;
							}
						}
						break;
						case EVariableType::NonUniformScale:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (!Val.ContainsNaN())
								{
									Emitter.NonUniformScale = Val;
								}
							}
						}
						break;
						case EVariableType::StartFrameRangeOverride:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetIntDef())
							{
								int32 Val = *(int32*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.StartFrameRangeOverride = Val;
							}
						}
						break;
						case EVariableType::EndFrameRangeOverride:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetIntDef())
							{
								int32 Val = *(int32*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.EndFrameRangeOverride = Val;
							}
						}
						break;
						case EVariableType::AnalyticalCollisionNormal1:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (!Val.ContainsNaN())
								{
									Emitter.AnalyticalCollisionNormal1 = Val;
								}
							}
						}
						break;
						case EVariableType::AnalyticalCollisionNormal2:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (!Val.ContainsNaN())
								{
									Emitter.AnalyticalCollisionNormal2 = Val;
								}
							}
						}
						break;
						case EVariableType::AnalyticalCollisionPlanePosition1:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (!Val.ContainsNaN())
								{
									Emitter.AnalyticalCollisionPlanePosition1 = Val;
								}
							}
						}
						break;
						case EVariableType::AnalyticalCollisionPlanePosition2:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								if (!Val.ContainsNaN())
								{
									Emitter.AnalyticalCollisionPlanePosition2 = Val;
								}
							}
						}
						break;
						case EVariableType::BounceRestitution:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.BounceRestitution = Val;
							}
						}
						break;
						case EVariableType::SphereRadius:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.SphereRadius = Val;
							}
						}
						break;
						case EVariableType::PlaneSize:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec2Def())
							{
								FVector2f Val = *(FVector2f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.PlaneSize = Val;
							}
						}
						break;
						case EVariableType::BoxSize:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetVec3Def())
							{
								FVector3f Val = *(FVector3f*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.BoxSize = Val;
							}
						}
						break;
						case EVariableType::CylinderHeight:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.CylinderHeight = Val;
							}
						}
						break;
						case EVariableType::CylinderRadius:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.CylinderRadius = Val;
							}
						}
						break;
						case EVariableType::LargeRadiusTag:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.TorusLargeRadius = Val;
							}
						}
						break;
						case EVariableType::HandleRadiusTag:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.TorusHandleRadius = Val;
							}
						}
						break;
						case EVariableType::ConeRadialAngleTag:
						{
							if (Pair.Value->GetType() == FNiagaraTypeDefinition::GetFloatDef())
							{
								float Val = *(float*)Script->RapidIterationParameters.GetParameterData(Pair.Value->Offset, Pair.Value->GetSizeInBytes());
								Emitter.ConeRadialAngle = Val;
							}
						}
						break;
						default:;
						}
					}
				}
			}



			if (!ModuleNames.Contains(ShapeLocationTag))
			{
				Emitter.ShapeType = ENiagaraPICO_ShapeType::Unset;
			}

			if (Niagara2PICOVariables.Contains(CurveModuleTag))
			{
				for (auto Pair : Niagara2PICOVariables[CurveModuleTag].ItemCurveMap)
				{
					switch (Pair.Key) {
					case EVariableType::UniformCurveSpriteScale:
					{
						if (Pair.Value.IsValid())
						{
							Emitter.SizeOverLifeCurve = Pair.Value;
							Emitter.ScaleSizeMode = ENiagaraPICO_ScaleSizeMode::UniformCurve;
						}
					}
					break;
					default:;
					}
				}

				for (auto Pair : Niagara2PICOVariables[CurveModuleTag].ItemColorCurveMap)
				{
					switch (Pair.Key) {
					case EVariableType::ColorCurve:
					{
						if (Pair.Value.IsValid())
						{
							Emitter.ColorOverLifeCurve = Pair.Value;
						}
					}
					break;
					default:;
					}

				}

				for (auto Pair : Niagara2PICOVariables[CurveModuleTag].ItemVectorCurveMap)
				{
					switch (Pair.Key) {
					case EVariableType::ScaleVelocity:
					{
						if (Pair.Value.IsValid())
						{
							Emitter.VelocityScaleCurve = Pair.Value;
						}
					}
					break;
					case EVariableType::ScaleMeshSize:
					{
						if (Pair.Value.IsValid())
						{
							Emitter.MeshSizeOverLifeCurve = Pair.Value;
							Emitter.MeshScaleUpdateMode = ENiagaraPICO_ScaleSizeMode::NonUniformCurve;
						}
					}
					break;
					default:;
					}

				}
			}


		}

		return true;
	}

	void OpenAssetEditor(UObject* Asset)
	{
		if (!Asset) return;

		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		if (AssetEditorSubsystem)
		{
			AssetEditorSubsystem->OpenEditorForAsset(Asset);
		}
	}

	void CloseAssetEditor(UObject* Asset)
	{
		if (!Asset) return;

		UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
		if (AssetEditorSubsystem)
		{
			TArray<IAssetEditorInstance*> Editors = AssetEditorSubsystem->FindEditorsForAsset(Asset);
			for (IAssetEditorInstance* Editor : Editors)
			{
				if (Editor)
				{
					Editor->CloseWindow(EAssetEditorCloseReason::CloseAllEditorsForAsset);
				}
			}
		}
	}

	bool PrepareNiagaraSystemForConversion(UNiagaraSystem* NiagaraSystem)
	{
		if (!NiagaraSystem)
		{
			return false;
		}

		UPackage* NiagaraPackage = NiagaraSystem->GetPackage();
		if (!NiagaraPackage)
		{
			UE_LOG(LogPICOSpatialEditor, Warning, TEXT("PrepareNiagaraSystemForConversion: '%s' has no package."),
				*NiagaraSystem->GetPathName());
			return false;
		}

		const bool bWasEditorOpen = GEditor && GEditor->GetEditorSubsystem<UAssetEditorSubsystem>() &&
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorsForAsset(NiagaraSystem).Num() > 0;

		UE_LOG(LogPICOSpatialEditor, Log, TEXT("PrepareNiagaraSystemForConversion: opening Niagara editor for '%s' (AlreadyOpen=%d)"),
			*NiagaraSystem->GetPathName(), bWasEditorOpen ? 1 : 0);
		OpenAssetEditor(NiagaraSystem);

		bool bSaved = true;
		if (CVarPICOSpatialNiagaraSilentSaveBeforeConvert.GetValueOnGameThread() != 0)
		{
			TArray<UPackage*> PackagesToSave;
			PackagesToSave.Add(NiagaraPackage);

			FEditorFileUtils::FPromptForCheckoutAndSaveParams SaveParams;
			SaveParams.bCheckDirty = false;
			SaveParams.bPromptToSave = false;
			SaveParams.bIsExplicitSave = true;

			const FEditorFileUtils::EPromptReturnCode SaveResult = FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, SaveParams);
			bSaved = (SaveResult == FEditorFileUtils::EPromptReturnCode::PR_Success);
			UE_LOG(LogPICOSpatialEditor, Log, TEXT("PrepareNiagaraSystemForConversion: silent save '%s' result=%d"),
				*NiagaraPackage->GetName(), bSaved ? 1 : 0);
		}
		else
		{
			UE_LOG(LogPICOSpatialEditor, Log, TEXT("PrepareNiagaraSystemForConversion: silent save disabled by PICO.Niagara.SilentSaveBeforeConvert for '%s'"),
				*NiagaraSystem->GetPathName());
		}

		if (!bWasEditorOpen)
		{
			CloseAssetEditor(NiagaraSystem);
			UE_LOG(LogPICOSpatialEditor, Log, TEXT("PrepareNiagaraSystemForConversion: closed Niagara editor for '%s'"),
				*NiagaraSystem->GetPathName());
		}

		return bSaved;
	}

}
