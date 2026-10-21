// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/RichCurve.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "PICOSpatialNiagaraAsset.generated.h"

#define MAX_RENDER_NUMS 10

/** Enumeration for Niagara renderer types supported in PICO Spatial */
UENUM(BlueprintType)
enum class ENiagaraPICO_RendererType : uint8
{
	/** Sprite-based particle rendering */
	Sprite,
	/** Mesh-based particle rendering */
	Mesh,
	/** Ribbon/trail particle rendering */
	Ribbon
};

/** Simplified particle facing modes supported by the native runtime. */
UENUM(BlueprintType)
enum class ENiagaraPICO_FacingMode : uint8
{
	/** Face towards the camera position. */
	CameraPosition,
	/** Keep the billboard parallel to the camera plane. */
	CameraPlane,
	/** Keep the particle orientation free from camera-facing constraints. */
	Free
};

/** Enumeration for particle velocity modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_VelocityMode : uint8
{
	/** Linear velocity direction */
	Linear,
	/** Velocity directed from a specific point */
	FromPoint,
	/** Velocity within a cone shape */
	InCone
};

/** Enumeration for box or plane emission modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_BoxOrPlaneMode : uint8
{
	/** Unset mode */
	Unset,
	/** Box emission shape */
	Box,
	/** Plane emission shape */
	Plane
};

/** Enumeration for mesh orientation modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_MeshOrientationMode : uint8
{
	/** Unset orientation */
	Unset,
	/** Random orientation */
	Random,
	/** System-defined orientation */
	System,
	/** Orient to a specific vector */
	OrientToVector,
	/** Orient to a matrix transformation */
	OrientToMatrix,
	/** Orient to a quaternion rotation */
	OrientToQuaternion,
};

/** Enumeration for mesh orientation update methods */
UENUM(BlueprintType)
enum class ENiagaraPICO_UpdateMeshOrientationMethod : uint8
{
	/** Unset method */
	Unset,
	/** Update using rotation rate */
	RotationRate,
	/** Orient to vector with smoothing */
	OrientToVector_S,
	/** Orient towards a position */
	OrientToPosition,
	/** Flight-based orientation */
	FlightOrientation,
	/** Rolling motion orientation */
	RollingOrientation,
};

/** Enumeration for particle lifecycle modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_LifeCycleMode : uint8
{
	/** Follows system lifecycle */
	System,
	/** Independent lifecycle */
	Self,
};

/** Enumeration for cone angle measurement modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_ConeAngleMode : uint8
{
	/** Angle in degrees */
	Degrees,
	/** Normalized angle (0-1) */
	NormalizedAngle,
	/** Angle in radians */
	Radians
};

/** Enumeration for particle emission shape types */
UENUM(BlueprintType)
enum class ENiagaraPICO_ShapeType : uint8
{
	/** Unset shape */
	Unset,
	/** Spherical emission shape */
	Sphere,
	/** Cylindrical emission shape */
	Cylinder,
	/** Box or plane emission shape */
	BoxOrPlane,
	/** Torus emission shape */
	Torus,
	/** Ring emission shape */
	Ring,
	/** Cone emission shape */
	Cone,
};

/** Enumeration for particle size modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_SizeMode : uint8
{
	/** Unset size mode */
	Unset,
	/** Uniform size for all particles */
	Uniform,
	/** Random uniform size within range */
	RandomUniform,
	/** Non-uniform size (X,Y,Z) */
	NonUniform,
	/** Random non-uniform size */
	RandomNonUniform,
};

/** Enumeration for scale size modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_ScaleSizeMode : uint8
{
	/** Unset scale mode */
	Unset,
	/** Uniform scaling */
	Uniform,
	/** Uniform scaling with curve */
	UniformCurve,
	/** Non-uniform scaling */
	NonUniform,
	/** Non-uniform scaling with curve */
	NonUniformCurve,
};

/** Enumeration for particle color modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_ColorMode : uint8
{
	/** Unset color mode */
	Unset,
	/** Directly set color value */
	DirectSet,
	/** Random color within range */
	RandomRange,
	/** Random hue with fixed saturation/value */
	RandomHue,
};

/** Enumeration for sprite rotation modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_SpriteRotationMode : uint8
{
	/** Unset rotation mode */
	Unset,
	/** Random rotation */
	Random,
	/** Direct angle specification */
	DirectAngle,
	/** Direct normalized angle (0-1) */
	DirectNormalizedAngle,
};

/** Enumeration for orientation update methods */
UENUM(BlueprintType)
enum class ENiagaraPICO_OrientationMethod : uint8
{
	/** Update using rotation rate */
	RotationRate,
	/** Orient to vector with smoothing */
	OrientToVector_S,
	/** Orient towards a position */
	OrientToPosition,
	/** Flight-based orientation */
	FlightOrientation,
	/** Rolling motion orientation */
	RollingOrientation,
};

/** Enumeration for particle lifetime modes */
UENUM(BlueprintType)
enum class ENiagaraPICO_LifeTimeMode : uint8
{
	/** Directly set lifetime */
	DirectSet,
	/** Random lifetime within range */
	Random,
};

/** Enumeration for particle types */
UENUM(BlueprintType)
enum class ENiagaraPICO_ParticleType : uint8
{
	/** Main particle system */
	MainParticle,
	/** Sub-particle system */
	SubParticle,
};

/** Enumeration for loop behaviors */
UENUM(BlueprintType)
enum class ENiagaraPICO_LoopBehavior : uint8
{
	/** Loop infinitely */
	Infinite,
	/** Play once */
	Once,
	/** Play multiple times */
	Multiple
};

/** Base structure for Niagara renderers */
USTRUCT(BlueprintType)
struct FNiagaraRendererBasePICO
{
	GENERATED_BODY()
	
	/** Type of renderer (Sprite, Mesh, Ribbon) */
	UPROPERTY(VisibleAnywhere, Category = "Renderer", meta = (DisplayName = "Render Type"))
	ENiagaraPICO_RendererType RenderType;
	
	/** Name identifier for this renderer */
	UPROPERTY(VisibleAnywhere, Category = "Renderer", meta = (DisplayName = "Render Name"))
	FString RenderName;

	/** Facing mode used by the native particle runtime. */
	UPROPERTY(VisibleAnywhere, Category = "Renderer", meta = (DisplayName = "Facing Mode"))
	ENiagaraPICO_FacingMode FacingMode;

	FNiagaraRendererBasePICO(): RenderType(), FacingMode(ENiagaraPICO_FacingMode::CameraPosition)
	{
	}
};

/** Structure for sprite-based Niagara renderers */
USTRUCT(BlueprintType)
struct FNiagaraSpriteRendererPICO:public FNiagaraRendererBasePICO
{
	GENERATED_BODY()
	
	/** Material used for sprite rendering */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Renderer", meta = (DisplayName = "Render Material"))
	TObjectPtr<UMaterialInterface> RenderMaterial;

	/** Size of sub-images for sprite sheet animations */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Renderer", meta = (DisplayName = "Sub Image Size"))
	FVector2D SubImageSize = FVector2D(1.0f, 1.0f);
};

/** Structure for mesh-based Niagara renderers */
USTRUCT(BlueprintType)
struct FNiagaraMeshRendererPICO:public FNiagaraRendererBasePICO
{
	GENERATED_BODY()
	
	/** Static mesh used for particle rendering */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Renderer", meta = (DisplayName = "Mesh"))
	TObjectPtr<UStaticMesh> Mesh;

	/** Whether to override the mesh's default material */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Renderer", meta = (DisplayName = "Enable Override Mesh Material"))
	bool bEnableOverrideMeshMaterial = false;
	
	/** Material to use when overriding mesh material */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Renderer", meta = (DisplayName = "Override Mesh Material"))
	TObjectPtr<UMaterialInterface> OverrideMeshMaterial;
};

/** Structure representing a curve for Niagara parameters */
USTRUCT(BlueprintType)
struct FNiagaraCurvePICO
{
	GENERATED_BODY()
	
	/** Array of curve keys defining the curve shape */
	UPROPERTY(VisibleAnywhere, Category = "Curve", meta = (DisplayName = "Curve Keys"))
	TArray<FRichCurveKey> Keys;

	bool IsValid() const
	{
		return Keys.Num() > 0;
	}
};

/** Structure representing a 3D vector curve */
USTRUCT(BlueprintType)
struct FNiagaraVectorCurvePICO
{
	GENERATED_BODY()
	
	/** X-component curve */
	UPROPERTY(VisibleAnywhere, Category = "Vector Curve", meta = (DisplayName = "X Curve"))
	FNiagaraCurvePICO XCurve;
	
	/** Y-component curve */
	UPROPERTY(VisibleAnywhere, Category = "Vector Curve", meta = (DisplayName = "Y Curve"))
	FNiagaraCurvePICO YCurve;
	
	/** Z-component curve */
	UPROPERTY(VisibleAnywhere, Category = "Vector Curve", meta = (DisplayName = "Z Curve"))
	FNiagaraCurvePICO ZCurve;
	
	bool IsValid() const
	{
		return XCurve.Keys.Num() > 0;
	}
};

/** Structure representing a color curve with RGBA components */
USTRUCT(BlueprintType)
struct FNiagaraColorCurvePICO
{
	GENERATED_BODY()
	
	/** Red color component curve */
	UPROPERTY(VisibleAnywhere, Category = "Color Curve", meta = (DisplayName = "Red Curve"))
	FNiagaraCurvePICO RedCurve;
	
	/** Green color component curve */
	UPROPERTY(VisibleAnywhere, Category = "Color Curve", meta = (DisplayName = "Green Curve"))
	FNiagaraCurvePICO GreenCurve;
	
	/** Blue color component curve */
	UPROPERTY(VisibleAnywhere, Category = "Color Curve", meta = (DisplayName = "Blue Curve"))
	FNiagaraCurvePICO BlueCurve;
	
	/** Alpha transparency component curve */
	UPROPERTY(VisibleAnywhere, Category = "Color Curve", meta = (DisplayName = "Alpha Curve"))
	FNiagaraCurvePICO AlphaCurve;
	
	bool IsValid() const
	{
		return RedCurve.Keys.Num() > 0;
	}
};


/** Comprehensive structure defining a Niagara emitter for PICO Spatial */
USTRUCT(BlueprintType)
struct FNiagaraEmitterPICO
{
	GENERATED_BODY()

	/** Lifecycle mode determining how this emitter behaves */
	UPROPERTY(VisibleAnywhere, Category = "Emitter Settings", meta = (DisplayName = "Life Cycle Mode"))
	ENiagaraPICO_LifeCycleMode LifeCycleMode;
	
	/** Display name for this emitter */
	UPROPERTY(VisibleAnywhere, Category = "Emitter Settings", meta = (DisplayName = "Emitter Name"))
	FString EmitterName;

	/** Unique identifier for this emitter */
	UPROPERTY(VisibleAnywhere, Category = "Emitter Settings", meta = (DisplayName = "Emitter ID"))
	FGuid EmitterID;

	/** Whether particles are simulated in local space relative to the emitter */
	UPROPERTY(VisibleAnywhere, Category = "Emitter Settings", meta = (DisplayName = "Local Space"))
	bool bLocalSpace;

	/** Duration for one complete loop cycle */
	UPROPERTY(VisibleAnywhere, Category = "Loop Settings", meta = (DisplayName = "Loop Duration"))
	float LoopDuration;

	/** Whether to add a delay before starting each loop */
	UPROPERTY(VisibleAnywhere, Category = "Loop Settings", meta = (DisplayName = "Use Loop Delay"))
	bool bUseLoopDelay;
	
	/** Delay time before each loop starts */
	UPROPERTY(VisibleAnywhere, Category = "Loop Settings", meta = (DisplayName = "Loop Delay"))
	float LoopDelay;

	/** Number of times the emitter should loop */
	UPROPERTY(VisibleAnywhere, Category = "Loop Settings", meta = (DisplayName = "Loop Count"))
	int32 LoopCount;

	/** Behavior defining how the emitter loops */
	UPROPERTY(VisibleAnywhere, Category = "Loop Settings", meta = (DisplayName = "Loop Behavior"))
	ENiagaraPICO_LoopBehavior LoopBehavior;

	/** Type of particle system (main or sub-particle) */
	UPROPERTY(VisibleAnywhere, Category = "Emitter Settings", meta = (DisplayName = "Particle Type"))
	ENiagaraPICO_ParticleType ParticleType;

	//Spawning
	/** Rate at which particles are spawned per second */
	UPROPERTY(VisibleAnywhere, Category = "Spawning", meta = (DisplayName = "Spawn Rate"))
	float SpawnRate;
	
	/** Number of particles to spawn in a single burst */
	UPROPERTY(VisibleAnywhere, Category = "Spawning", meta = (DisplayName = "Burst Spawn Count"))
	int32 BurstSpawnCount;

	/** Number of particles to spawn when triggered by an event */
	UPROPERTY(VisibleAnywhere, Category = "Spawning", meta = (DisplayName = "Spawn Nums By Event"))
	int32 SpawnNumsByEvent;

	//Life Time
	/** Mode determining how particle lifetime is calculated */
	UPROPERTY(VisibleAnywhere, Category = "Lifetime", meta = (DisplayName = "Life Time Mode"))
	ENiagaraPICO_LifeTimeMode LifeTimeMode;
	
	/** Fixed lifetime value for particles */
	UPROPERTY(VisibleAnywhere, Category = "Lifetime", meta = (DisplayName = "Life Time"))
	float LifeTime;

	/** Minimum lifetime for random lifetime mode */
	UPROPERTY(VisibleAnywhere, Category = "Lifetime", meta = (DisplayName = "Life Time Min"))
	float LifeTimeMin;

	/** Maximum lifetime for random lifetime mode */
	UPROPERTY(VisibleAnywhere, Category = "Lifetime", meta = (DisplayName = "Life Time Max"))
	float LifeTimeMax;

	//Size
	/** Mode for initializing mesh scale */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Scale Initialize Mode"))
	ENiagaraPICO_SizeMode MeshScaleInitializeMode;
	
	/** Mode for updating mesh scale over time */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Scale Update Mode"))
	ENiagaraPICO_ScaleSizeMode MeshScaleUpdateMode;

	/** Uniform scale factor for mesh particles */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Uniform Scale"))
	float MeshUniformScale;

	/** Minimum uniform scale for random scaling */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Uniform Scale Min"))
	float MeshUniformScaleMin;

	/** Maximum uniform scale for random scaling */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Uniform Scale Max"))
	float MeshUniformScaleMax;

	/** Non-uniform scale values (X,Y,Z) for mesh particles */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Non Uniform Scale"))
	FVector3f MeshNonUniformScale;
	
	/** Maximum non-uniform scale values */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Non Uniform Scale Max"))
	FVector3f MeshNonUniformScaleMax;
	
	/** Minimum non-uniform scale values */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Non Uniform Scale Min"))
	FVector3f MeshNonUniformScaleMin;

	/** Scale factor for non-uniform scaling */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Non Uniform Scale Factor"))
	FVector3f MeshNonUniformScaleFactor;

	/** Curve defining mesh size changes over particle lifetime */
	UPROPERTY(VisibleAnywhere, Category = "Mesh Scale", meta = (DisplayName = "Mesh Size Over Life Curve"))
	FNiagaraVectorCurvePICO MeshSizeOverLifeCurve;

	// Sprite Size
	/** Mode for determining sprite particle size */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Size Mode"))
	ENiagaraPICO_SizeMode SizeMode;

	/** Uniform size for sprite particles */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Uniform Sprite Size"))
	float UniformSpriteSize;

	/** Minimum uniform size for random sprite sizing */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Uniform Sprite Size Min"))
	float UniformSpriteSizeMin;

	/** Maximum uniform size for random sprite sizing */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Uniform Sprite Size Max"))
	float UniformSpriteSizeMax;

	/** Maximum sprite size for non-uniform scaling */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Sprite Size Max"))
	FVector2f SpriteSizeMax;

	/** Minimum sprite size for non-uniform scaling */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Sprite Size Min"))
	FVector2f SpriteSizeMin;

	/** Base sprite size */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Sprite Size"))
	FVector2f SpriteSize;

	/** Non-uniform scale factor for sprites */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Non Uniform Scale Factor"))
	FVector2f NonUniformScaleFactor;

	/** Mode for scaling sprite size over time */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Scale Size Mode"))
	ENiagaraPICO_ScaleSizeMode ScaleSizeMode;
	
	/** Uniform scale factor applied over time */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Uniform Scale Factor"))
	float UniformScaleFactor;

	/** Curve defining size changes over particle lifetime */
	UPROPERTY(VisibleAnywhere, Category = "Sprite Size", meta = (DisplayName = "Size Over Life Curve"))
	FNiagaraCurvePICO SizeOverLifeCurve;

	//Velocity
	/** Curve defining velocity scaling over particle lifetime */
	UPROPERTY(VisibleAnywhere, Category = "Velocity", meta = (DisplayName = "Velocity Scale Curve"))
	FNiagaraVectorCurvePICO VelocityScaleCurve;

	//Color
	/** Mode determining how particle color is set */
	UPROPERTY(VisibleAnywhere, Category = "Color", meta = (DisplayName = "Color Mode"))
	ENiagaraPICO_ColorMode ColorMode;
	
	/** Curve defining color changes over particle lifetime */
	UPROPERTY(VisibleAnywhere, Category = "Color", meta = (DisplayName = "Color Over Life Curve"))
	FNiagaraColorCurvePICO ColorOverLifeCurve;

	/** Base color for particles */
	UPROPERTY(VisibleAnywhere, Category = "Color", meta = (DisplayName = "Color"))
	FLinearColor Color;

	/** Minimum color for random color mode */
	UPROPERTY(VisibleAnywhere, Category = "Color", meta = (DisplayName = "Color Minimum"))
	FLinearColor ColorMinimum;

	/** Maximum color for random color mode */
	UPROPERTY(VisibleAnywhere, Category = "Color", meta = (DisplayName = "Color Maximum"))
	FLinearColor ColorMaximum;

	//Shape
	/** Type of emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Shape Type"))
	ENiagaraPICO_ShapeType ShapeType;

	/** Mode for box or plane shapes */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Box Or Plane Mode"))
	ENiagaraPICO_BoxOrPlaneMode BoxOrPlaneMode;

	/** Mode for measuring cone angles */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Cone Angle Mode"))
	ENiagaraPICO_ConeAngleMode ConeAngleMode;

	/** Angle of cone emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Cone Angle"))
	float ConeAngle;

	/** Length of cone emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Shape Cone Length"))
	float ShapeConeLength;

	/** Angle of cone emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Shape Cone Angle"))
	float ShapeConeAngle;
	
	/** Radius of spherical emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Sphere Radius"))
	float SphereRadius;

	/** Radius of cylindrical emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Cylinder Radius"))
	float CylinderRadius;

	/** Height of cylindrical emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Cylinder Height"))
	float CylinderHeight;

	/** Large radius of torus emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Torus Large Radius"))
	float TorusLargeRadius;

	/** Handle radius of torus emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Torus Handle Radius"))
	float TorusHandleRadius;

	/** Radial angle for cone emission */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Cone Radial Angle"))
	float ConeRadialAngle;

	/** Size of plane emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Plane Size"))
	FVector2f PlaneSize;

	/** Size of box emission shape */
	UPROPERTY(VisibleAnywhere, Category = "Shape", meta = (DisplayName = "Box Size"))
	FVector3f BoxSize;

	//ShapeScale
	/** Non-uniform scale applied to emission shapes */
	UPROPERTY(VisibleAnywhere, Category = "Shape Scale", meta = (DisplayName = "Non Uniform Scale"))
	FVector3f NonUniformScale;

	//Force
	/** Total force applied to particles */
	UPROPERTY(VisibleAnywhere, Category = "Forces", meta = (DisplayName = "Total Force"))
	FVector3f TotalForce;

	/** Initial velocity for particles */
	UPROPERTY(VisibleAnywhere, Category = "Forces", meta = (DisplayName = "Velocity"))
	FVector3f Velocity;

	/** Drag force coefficient */
	UPROPERTY(VisibleAnywhere, Category = "Forces", meta = (DisplayName = "Drag"))
	float Drag;

	/** Amount of noise force applied */
	UPROPERTY(VisibleAnywhere, Category = "Forces", meta = (DisplayName = "Noise Force Amount"))
	float NoiseForceAmount;

	//Rotation
	/** Mode for sprite rotation */
	UPROPERTY(VisibleAnywhere, Category = "Rotation", meta = (DisplayName = "Sprite Rotation Mode"))
	ENiagaraPICO_SpriteRotationMode SpriteRotationMode;
	
	/** Method for updating orientation */
	UPROPERTY(VisibleAnywhere, Category = "Rotation", meta = (DisplayName = "Orientation Method"))
	ENiagaraPICO_OrientationMethod OrientationMethod;

	/** Initial rotation angle for sprites */
	UPROPERTY(VisibleAnywhere, Category = "Rotation", meta = (DisplayName = "Sprite Rotation Angle"))
	float SpriteRotationAngle;

	/** Maximum rotation angle for random rotation */
	UPROPERTY(VisibleAnywhere, Category = "Rotation", meta = (DisplayName = "Sprite Rotation Angle Max"))
	float SpriteRotationAngleMax;

	/** Minimum rotation angle for random rotation */
	UPROPERTY(VisibleAnywhere, Category = "Rotation", meta = (DisplayName = "Sprite Rotation Angle Min"))
	float SpriteRotationAngleMin;

	//Rotation Update
	/** Rate of sprite rotation over time */
	UPROPERTY(VisibleAnywhere, Category = "Rotation Update", meta = (DisplayName = "Sprite Rotation Rate"))
	float SpriteRotationRate;

	/** Rate of mesh rotation over time */
	UPROPERTY(VisibleAnywhere, Category = "Rotation Update", meta = (DisplayName = "Mesh Rotation Rate"))
	float MeshRotationRate;

	/** Mode for mesh orientation */
	UPROPERTY(VisibleAnywhere, Category = "Rotation Update", meta = (DisplayName = "Mesh Orientation Mode"))
	ENiagaraPICO_MeshOrientationMode MeshOrientationMode;

	/** Method for updating mesh orientation */
	UPROPERTY(VisibleAnywhere, Category = "Rotation Update", meta = (DisplayName = "Update Mesh Orientation Method"))
	ENiagaraPICO_UpdateMeshOrientationMethod UpdateMeshOrientationMethod;
	
	//Velocity
	/** Mode for particle velocity calculation */
	UPROPERTY(VisibleAnywhere, Category = "Velocity Settings", meta = (DisplayName = "Velocity Mode"))
	ENiagaraPICO_VelocityMode VelocityMode;
	
	/** Speed value for particle velocity */
	UPROPERTY(VisibleAnywhere, Category = "Velocity Settings", meta = (DisplayName = "Velocity Speed"))
	float VelocitySpeed;

	/** Scale factor applied to velocity */
	UPROPERTY(VisibleAnywhere, Category = "Velocity Settings", meta = (DisplayName = "Velocity Speed Scale"))
	float VelocitySpeedScale;

	//Animation
	/** Whether to use sub-UV animation for sprites */
	UPROPERTY(VisibleAnywhere, Category = "Animation", meta = (DisplayName = "Sub UV Animation"))
	bool bSubUVAnimation;

	/** Whether to use a specific start frame */
	UPROPERTY(VisibleAnywhere, Category = "Animation", meta = (DisplayName = "Use Start Frame"))
	bool bUseStartFrame;

	/** Override for the starting frame of animation */
	UPROPERTY(VisibleAnywhere, Category = "Animation", meta = (DisplayName = "Start Frame Range Override"))
	int32 StartFrameRangeOverride;

	/** Whether to use a specific end frame */
	UPROPERTY(VisibleAnywhere, Category = "Animation", meta = (DisplayName = "Use End Frame"))
	bool bUseEndFrame;

	/** Override for the ending frame of animation */
	UPROPERTY(VisibleAnywhere, Category = "Animation", meta = (DisplayName = "End Frame Range Override"))
	int32 EndFrameRangeOverride;

	//Render
	/** Primary renderer type for this emitter */
	UPROPERTY(VisibleAnywhere, Category = "Rendering", meta = (DisplayName = "Main Renderer Type"))
	ENiagaraPICO_RendererType MainRendererType;
	
	/** Array of sprite renderers */
	UPROPERTY(VisibleAnywhere, Category = "Rendering", meta = (DisplayName = "Sprite Renderers"))
	TArray<FNiagaraSpriteRendererPICO> SpriteRenderers;

	/** Array of mesh renderers */
	UPROPERTY(VisibleAnywhere, Category = "Rendering", meta = (DisplayName = "Mesh Renders"))
	TArray<FNiagaraMeshRendererPICO> MeshRenders;

	/** Mapping from sub-nodes to parent nodes */
	UPROPERTY(VisibleAnywhere, Category = "Rendering", meta = (DisplayName = "Sub Node To Node Map"))
	TMap<FString, FString> SubNodeToNodeMap;

	//Collision
	/** Whether collision is enabled for particles */
	UPROPERTY(VisibleAnywhere, Category = "Collision", meta = (DisplayName = "Enable Collision"))
	bool bEnableCollision;

	/** First analytical collision normal */
	UPROPERTY(VisibleAnywhere, Category = "Collision", meta = (DisplayName = "Analytical Collision Normal 1"))
	FVector3f AnalyticalCollisionNormal1;

	/** Second analytical collision normal */
	UPROPERTY(VisibleAnywhere, Category = "Collision", meta = (DisplayName = "Analytical Collision Normal 2"))
	FVector3f AnalyticalCollisionNormal2;

	/** First collision plane position */
	UPROPERTY(VisibleAnywhere, Category = "Collision", meta = (DisplayName = "Analytical Collision Plane Position 1"))
	FVector3f AnalyticalCollisionPlanePosition1;

	/** Second collision plane position */
	UPROPERTY(VisibleAnywhere, Category = "Collision", meta = (DisplayName = "Analytical Collision Plane Position 2"))
	FVector3f AnalyticalCollisionPlanePosition2;

	/** Bounce restitution coefficient for collision */
	UPROPERTY(VisibleAnywhere, Category = "Collision", meta = (DisplayName = "Bounce Restitution"))
	float BounceRestitution;
	
	FNiagaraEmitterPICO()
		: LifeCycleMode(), bLocalSpace(true)
		  , LoopDuration(0.f)
		  , bUseLoopDelay(false), LoopDelay(0), LoopCount(0)
		  , LoopBehavior(ENiagaraPICO_LoopBehavior::Infinite)
		  , ParticleType(ENiagaraPICO_ParticleType::MainParticle)
		  , SpawnRate(0)
		  , BurstSpawnCount(0)
		  , SpawnNumsByEvent(0)
		  , LifeTimeMode()
		  , LifeTime(0)
		  , LifeTimeMin(0)
		  , LifeTimeMax(0)
		  , MeshScaleInitializeMode(), MeshScaleUpdateMode(), MeshUniformScale(1.0f)
		  , MeshUniformScaleMin(1.0f)
		  , MeshUniformScaleMax(1.0f)
		  , SizeMode(), UniformSpriteSize(0)
		  , UniformSpriteSizeMin(0)
		  , UniformSpriteSizeMax(0)
		  , ScaleSizeMode(), UniformScaleFactor(1.0f)
		  , ShapeType()
		  , ConeAngle(0.f)
		  , ShapeConeLength(0), ShapeConeAngle(0), SphereRadius(0.f), CylinderRadius(0), CylinderHeight(0), TorusLargeRadius(0), TorusHandleRadius(0), ConeRadialAngle(0), PlaneSize(), BoxSize(), Velocity(), Drag(0.f)
		  , NoiseForceAmount(0.f)
		  , OrientationMethod(), SpriteRotationAngle(0.f)
		  , SpriteRotationAngleMax(360.f)
		  , SpriteRotationAngleMin(0.f)
		  , SpriteRotationRate(0.f)
		  , MeshRotationRate(0), MeshOrientationMode(), UpdateMeshOrientationMethod()
		  , VelocitySpeed(0.f)
		  , VelocitySpeedScale(1.0f)
		  , bSubUVAnimation(false)
		  , bUseStartFrame(false)
		  , StartFrameRangeOverride(0)
		  , bUseEndFrame(false)
		  , EndFrameRangeOverride(0), MainRendererType(), bEnableCollision(false)
		  , AnalyticalCollisionNormal1()
		  , AnalyticalCollisionNormal2()
		  , AnalyticalCollisionPlanePosition1()
		  , AnalyticalCollisionPlanePosition2()
		  , BounceRestitution(0.f)
	{
		SpriteSizeMax = FVector2f::Zero();
		SpriteSizeMin = FVector2f::Zero();
		SpriteSize = FVector2f::Zero();
		MeshNonUniformScale = FVector3f::OneVector;
		MeshNonUniformScaleMax = FVector3f::OneVector;
		MeshNonUniformScaleMin = FVector3f::OneVector;
		MeshNonUniformScaleFactor = FVector3f::OneVector;
		NonUniformScaleFactor = FVector2f::One();
		ColorMode = ENiagaraPICO_ColorMode::Unset;
		Color = FLinearColor::White;
		ColorMaximum = FLinearColor::White;
		ColorMinimum = FLinearColor::White;
		NonUniformScale = FVector3f::OneVector;
		TotalForce = FVector3f::Zero();
		SpriteRotationMode = ENiagaraPICO_SpriteRotationMode::Unset;
		VelocityMode = ENiagaraPICO_VelocityMode::Linear;
		BoxOrPlaneMode = ENiagaraPICO_BoxOrPlaneMode::Unset;
		ConeAngleMode = ENiagaraPICO_ConeAngleMode::Degrees;
	}
};

/**
 * PICO Spatial Niagara Asset
 * Represents a Niagara particle system optimized for PICO Spatial environments
 */
UCLASS(BlueprintType, meta = (DisplayName = "PICO Spatial Niagara Asset"))
class PICOSPATIAL_API UPICOSpatialNiagaraAsset : public UDataAsset
{
	GENERATED_BODY()
public:

	/** Display name for the Niagara system */
	UPROPERTY(VisibleAnywhere, Category = "PICO Spatial|Niagara", meta = (DisplayName = "System Name"))
	FString NiagaraSystemName;
	
	/** Loop behavior for the particle system */
	UPROPERTY(VisibleAnywhere, Category = "PICO Spatial|Niagara", meta = (DisplayName = "Loop Behavior"))
	ENiagaraPICO_LoopBehavior LoopBehavior = ENiagaraPICO_LoopBehavior::Once;

	/** Duration of one complete loop cycle */
	UPROPERTY(VisibleAnywhere, Category = "PICO Spatial|Niagara", meta = (DisplayName = "Loop Duration"))
	float LoopDuration = 0.0f;

	/** Number of times the system should loop */
	UPROPERTY(VisibleAnywhere, Category = "PICO Spatial|Niagara", meta = (DisplayName = "Loop Count"))
	int LoopCount = 1;

	/** Whether to use a delay before each loop */
	UPROPERTY(VisibleAnywhere, Category = "PICO Spatial|Niagara", meta = (DisplayName = "Use Loop Delay"))
	bool bUseLoopDelay=false;

	/** Delay time before each loop starts */
	UPROPERTY(VisibleAnywhere, Category = "PICO Spatial|Niagara", meta = (DisplayName = "Loop Delay"))
	float LoopDelay = 0.f;
	
	/** Mapping of emitter GUIDs to their emitter data */
	UPROPERTY(VisibleAnywhere, Category = "PICO Spatial|Niagara", meta = (DisplayName = "Emitters Map"))
	TMap<FGuid,FNiagaraEmitterPICO> GuidToEmittersMap;
	
	/** Mapping of parent emitter GUIDs to sub-emitter GUIDs */
	UPROPERTY(VisibleAnywhere, Category = "PICO Spatial|Niagara", meta = (DisplayName = "Parent To Sub Map"))
	TMap<FGuid,FGuid> ParentToSubGuidMap;
};
