// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#pragma push_macro("INVALID_OBJECT")
#pragma push_macro("IN")
#pragma push_macro("OUT")
#undef INVALID_OBJECT
#undef IN
#undef OUT
#include "SpatialPack.h"
#pragma pop_macro("OUT")
#pragma pop_macro("IN")
#pragma pop_macro("INVALID_OBJECT")

#include "PICOSpatialFunctionLibrary.h"
#include "Components/ShapeComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "ECS/PICOSpatialECSCustomSceneEntityComponent.h"

FORCEINLINE FQuat ToFQuat(const spatialpack::Quaternionf& Quat)
{
	return FQuat(-Quat.x, -Quat.z, -Quat.y, Quat.w);
}

FORCEINLINE spatialpack::Quaternionf ToSpQuat(const FQuat& Quat)
{
    const FQuat Normalized = Quat.GetNormalized();
	return spatialpack::Quaternionf{ (float)Normalized.W, -(float)Normalized.X, -(float)Normalized.Z, -(float)Normalized.Y };
}

FORCEINLINE FVector ToFVector(const spatialpack::Vector3f& Vector, float Scale = 1.0f)
{
	return FVector(Vector.x * Scale, Vector.z * Scale, Vector.y * Scale);
}

FORCEINLINE FVector3f ToFVector3f(const spatialpack::Vector3f& Vector, float Scale = 1.0f)
{
	return FVector3f(Vector.x * Scale, Vector.z * Scale, Vector.y * Scale);
}

FORCEINLINE spatialpack::Vector3f ToSpVector(const FVector& Vector, float Scale = 1.0f)
{
	if (Vector.IsZero())
		return spatialpack::Vector3f{ 0.0f, 0.0f, 0.0f };

	if (FMath::IsNearlyZero(Scale))
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("ToSpVector called with near-zero scale; using scale 1.0 to avoid invalid values."));
		Scale = 1.0f;
	}

	return spatialpack::Vector3f{ (float)Vector.X / Scale, (float)Vector.Z / Scale, (float)Vector.Y / Scale };
}

FORCEINLINE spatialpack::Vector3f ToSpVector(const FVector3f& Vector, float Scale = 1.0f)
{
	if (Vector.IsZero())
		return spatialpack::Vector3f{ 0.0f, 0.0f, 0.0f };

	if (FMath::IsNearlyZero(Scale))
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("ToSpVector called with near-zero scale; using scale 1.0 to avoid invalid values."));
		Scale = 1.0f;
	}

	return spatialpack::Vector3f{ (float)Vector.X / Scale, (float)Vector.Z / Scale, (float)Vector.Y / Scale };
}

FORCEINLINE spatialpack::Matrix4x4 transformToMatrix(const spatialpack::Transform& transform) {
	const spatialpack::Vector3f& position = transform.translation;
	const spatialpack::Quaternionf& rotation = transform.rotation;
	const spatialpack::Vector3f& scale = transform.scale;

	float qw = rotation.w;
	float qx = rotation.x;
	float qy = rotation.y;
	float qz = rotation.z;

	float rot3x3[3][3];
	rot3x3[0][0] = 1.0f - 2.0f * (qy * qy + qz * qz);
	rot3x3[0][1] = 2.0f * (qx * qy + qw * qz);
	rot3x3[0][2] = 2.0f * (qx * qz - qw * qy);

	rot3x3[1][0] = 2.0f * (qx * qy - qw * qz);
	rot3x3[1][1] = 1.0f - 2.0f * (qx * qx + qz * qz);
	rot3x3[1][2] = 2.0f * (qy * qz + qw * qx);

	rot3x3[2][0] = 2.0f * (qx * qz + qw * qy);
	rot3x3[2][1] = 2.0f * (qy * qz - qw * qx);
	rot3x3[2][2] = 1.0f - 2.0f * (qx * qx + qy * qy);

	spatialpack::Matrix4x4 result;

	result.values[0][0] = scale.x * rot3x3[0][0];
	result.values[0][1] = scale.x * rot3x3[0][1];
	result.values[0][2] = scale.x * rot3x3[0][2];
	result.values[0][3] = 0.0f;

	result.values[1][0] = scale.y * rot3x3[1][0];
	result.values[1][1] = scale.y * rot3x3[1][1];
	result.values[1][2] = scale.y * rot3x3[1][2];
	result.values[1][3] = 0.0f;

	result.values[2][0] = scale.z * rot3x3[2][0];
	result.values[2][1] = scale.z * rot3x3[2][1];
	result.values[2][2] = scale.z * rot3x3[2][2];
	result.values[2][3] = 0.0f;

	result.values[3][0] = position.x;
	result.values[3][1] = position.y;
	result.values[3][2] = position.z;
	result.values[3][3] = 1.0f;

	return result;
}

FORCEINLINE spatialpack::Matrix4x4 ToSpMatrix(const FMatrix& UEMatrix, float Scale = 1.0f)
{
	FTransform UETransform(UEMatrix);

	spatialpack::Vector3f spTranslation = ToSpVector(UETransform.GetLocation(), Scale);
	spatialpack::Quaternionf spRotation = ToSpQuat(UETransform.GetRotation());
	spatialpack::Vector3f spScale = ToSpVector(UETransform.GetScale3D(), 1.0f);

	spatialpack::Transform spTransform;
	spTransform.translation = spTranslation;
	spTransform.rotation = spRotation;
	spTransform.scale = spScale;

	return transformToMatrix(spTransform);
}

/**
 * Normalize degrees to the range [0, 1].
 * @param Degrees The degrees to be normalized
 * @return A normalized floating-point number in the range [0, 1]
 */
FORCEINLINE float NormalizeDegrees(float Degrees)
{
	const float NormalizedDegrees = FMath::Fmod(FMath::Fmod(Degrees, 360.0f) + 360.0f, 360.0f);
	return NormalizedDegrees / 360.0f;
}

/**
 * Calculate the radius of a cone based on its height and cone angle.
 * @param Height The height of the cone
 * @param ConeAngle The apex angle of the cone, which can be in degrees or radians depending on the bIsDegree parameter
 * @param bIsDegree A boolean value indicating whether ConeAngle is in degrees. Defaults to true
 * @return The calculated radius of the cone
 */
// Function to calculate the radius of a cone
FORCEINLINE float CalculateConeRadius(float Height, float ConeAngle, bool bIsDegree = true)
{
	if (bIsDegree) 
	{
		// Convert the angle from degrees to radians
		ConeAngle = ConeAngle * PI / 180.0f;
	}
	// Calculate the half-cone angle
	float HalfConeAngle = ConeAngle / 2.0f;
	// Use the tangent function to calculate the radius
	return FMath::Abs(Height * FMath::Tan(HalfConeAngle));
}

/**
 * Normalize radians to the range [0, 1].
 * @param Radians The radians to be normalized
 * @return A normalized floating-point number in the range [0, 1]
 */
FORCEINLINE float NormalizeRadians(float Radians)
{
	const float TwoPi = 2.0f * PI;
	const float NormalizedRadians = FMath::Fmod(FMath::Fmod(Radians, TwoPi) + TwoPi, TwoPi);
	return NormalizedRadians / TwoPi;
}

FORCEINLINE spatialpack::Color4f ToSpColor(const FColor& Color)
{
	return spatialpack::Color4f{ Color.ReinterpretAsLinear().R, Color.ReinterpretAsLinear().G, Color.ReinterpretAsLinear().B, Color.ReinterpretAsLinear().A };
}

FORCEINLINE spatialpack::Color4f ToSpColor(const FLinearColor& Color)
{
	return spatialpack::Color4f{ Color.R, Color.G, Color.B, Color.A };
}

FORCEINLINE void RotateTransform(FTransform& InTransform, const FRotator& Rotator)
{
	InTransform.SetRotation(Rotator.Quaternion() * InTransform.GetRotation());
}


FORCEINLINE spatialpack::ParticleBlendMode MaterialBlendingModeToParticleBlendMode(spatialpack::MaterialBlendingMode blendingMode)
{
	using namespace spatialpack;
	switch (blendingMode)
	{
	case MaterialBlendingMode::OPAQUE:
		return ParticleBlendMode::OPAQUE;
	case MaterialBlendingMode::TRANSPARENT:
	case MaterialBlendingMode::FADE:
		return ParticleBlendMode::ALPHA;
	case MaterialBlendingMode::ADD:
		return ParticleBlendMode::ADDITIVE;
	default:
		UE_LOG(LogPICOSpatial, Warning, TEXT("Unsupported MaterialBlendingMode, defaulting to ALPHA"));
		return ParticleBlendMode::ALPHA;
	}
}

enum class EEntityProxyType : uint8
{
    Unknown,
    SceneEntity,
	CustomSceneEntity,
    StaticMeshEntity,
    InstancedStaticMeshEntity,
    SkeletalMeshEntity,
    DynamicMeshEntity,
	WidgetEntity,
    ShapeEntity,
    TextEntity,
	SceneCapture2dEntity,
	ParticleEntity,
	DirectionalLightEntity
};

FORCEINLINE EEntityProxyType GetEntityProxyType(USceneComponent* SceneComponent)
{
    if (!SceneComponent)
    {
        return EEntityProxyType::Unknown;
    }

	if (SceneComponent->IsA(UCustomSceneEntityComponent::StaticClass()))
	{
		return EEntityProxyType::CustomSceneEntity;
	}

    if (SceneComponent->IsA(UPrimitiveComponent::StaticClass()))
    {
        if (SceneComponent->IsA(UMeshComponent::StaticClass()))
        {
            if (SceneComponent->IsA(UStaticMeshComponent::StaticClass()))
            {
                if (SceneComponent->IsA(UInstancedStaticMeshComponent::StaticClass()))
                {
                    return EEntityProxyType::InstancedStaticMeshEntity;
                }
                return EEntityProxyType::StaticMeshEntity;
            }
            else if (SceneComponent->IsA(USkeletalMeshComponent::StaticClass()))
            {
                return EEntityProxyType::SkeletalMeshEntity;
            }
            else if (SceneComponent->IsA(UDynamicMeshComponent::StaticClass()))
            {
                return EEntityProxyType::DynamicMeshEntity;
			}
			else if (SceneComponent->IsA(UWidgetComponent::StaticClass()))
			{
				return EEntityProxyType::WidgetEntity;
			}
        }
        else if (SceneComponent->IsA(UTextRenderComponent::StaticClass()))
        {
            return EEntityProxyType::TextEntity;
        }
        else if (SceneComponent->IsA(UShapeComponent::StaticClass()))
        {
            return EEntityProxyType::ShapeEntity;
        }
        else if (SceneComponent->IsA(UFXSystemComponent::StaticClass()))
        {
        	return EEntityProxyType::ParticleEntity;
        }
    }
	else if (SceneComponent->IsA(USceneCaptureComponent2D::StaticClass()))
	{
		return EEntityProxyType::SceneCapture2dEntity;
	}
	else if (SceneComponent->IsA(UDirectionalLightComponent::StaticClass()))
	{
		return EEntityProxyType::DirectionalLightEntity;
	}
    return EEntityProxyType::SceneEntity;
}

FORCEINLINE EInputActionPICO ConvertHoverAction(const spatialpack::HoverAction& Action)
{
	switch (Action)
	{
	case spatialpack::HoverAction::ENTERED:
		return EInputActionPICO::Started;
	case spatialpack::HoverAction::HOVERING:
		return EInputActionPICO::Ongoing;
	case spatialpack::HoverAction::EXITED:
		return EInputActionPICO::Completed;
	default:
		UE_LOG(LogPICOSpatial, Warning, TEXT("Unsupported HoverAction, defaulting to Canceled"));
		return EInputActionPICO::Canceled;
	}
}

FORCEINLINE EInputActionPICO ConvertDragAction(const spatialpack::DragAction& Action)
{
	switch (Action)
	{
	case spatialpack::DragAction::DRAG_BEGIN:
		return EInputActionPICO::Started;
	case spatialpack::DragAction::DRAGGING:
		return EInputActionPICO::Ongoing;
	case spatialpack::DragAction::DRAG_CANCEL:
		return EInputActionPICO::Canceled;
	case spatialpack::DragAction::DRAG_END:
		return EInputActionPICO::Completed;
	default:
		UE_LOG(LogPICOSpatial, Warning, TEXT("Unsupported DragAction, defaulting to Canceled"));
		return EInputActionPICO::Canceled;
	}
}

FORCEINLINE EInputActionPICO ConvertLongPressAction(const spatialpack::LongPressAction& Action)
{
	switch (Action)
	{
	case spatialpack::LongPressAction::BEGIN:
		return EInputActionPICO::Started;
	case spatialpack::LongPressAction::END:
		return EInputActionPICO::Completed;
	default:
		UE_LOG(LogPICOSpatial, Warning, TEXT("Unsupported LongPressAction, defaulting to Canceled"));
		return EInputActionPICO::Canceled;
	}
}

enum ETextureCopyBlendModifier : uint8
{
	Opaque,
	TransparentAlphaPassthrough,
	PremultipliedAlphaBlend,
};
