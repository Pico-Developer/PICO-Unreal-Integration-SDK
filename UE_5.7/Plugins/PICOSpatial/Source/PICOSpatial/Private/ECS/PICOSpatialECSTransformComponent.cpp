#include "ECS/PICOSpatialECSTransformComponent.h"

#if PLATFORM_ANDROID
#include "PICOSpatialUtility.h"
#endif

FVector UPICOSpatialECSTransformComponent::getPosition()
{
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		return ToFVector(GetCachedNativeEntity().getOrAddTransformComponent().getPosition(), 100.0f);
	}
#endif
	return Position;
}

void UPICOSpatialECSTransformComponent::setPosition(const FVector& InPosition)
{
	Position = InPosition;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTransformComponent().setPosition(ToSpVector(Position, 100.0f));
	}
#endif
}

FVector UPICOSpatialECSTransformComponent::getScale()
{
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		return ToFVector(GetCachedNativeEntity().getOrAddTransformComponent().getScale(), 1.0f);
	}
#endif
	return Scale;
}

void UPICOSpatialECSTransformComponent::setScale(const FVector& InScale)
{
	Scale = InScale;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTransformComponent().setScale(ToSpVector(Scale, 1.0f));
	}
#endif
}

FQuat UPICOSpatialECSTransformComponent::getRotation()
{
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		return ToFQuat(GetCachedNativeEntity().getOrAddTransformComponent().getRotation());
	}
#endif
	return Rotation;
}

void UPICOSpatialECSTransformComponent::setRotation(const FQuat& InRotation)
{
	Rotation = InRotation;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTransformComponent().setRotation(ToSpQuat(Rotation));
	}
#endif
}

FTransform UPICOSpatialECSTransformComponent::getLocalTransform()
{
	return FTransform(getRotation(), getPosition(), getScale());
}

void UPICOSpatialECSTransformComponent::setLocalTransform(const FTransform& InTransform)
{
	setPosition(InTransform.GetLocation());
	setRotation(InTransform.GetRotation());
	setScale(InTransform.GetScale3D());
}

#if PLATFORM_ANDROID
void UPICOSpatialECSTransformComponent::OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity)
{
	InNativeEntity.getOrAddTransformComponent();
}

void UPICOSpatialECSTransformComponent::ApplyAllPropertiesToNative()
{
	if (!HasNativeEntity())
	{
		return;
	}

	spatialpack::TransformComponent TransformComponent = GetCachedNativeEntity().getOrAddTransformComponent();
	TransformComponent.setPosition(ToSpVector(Position, 100.0f));
	TransformComponent.setRotation(ToSpQuat(Rotation));
	TransformComponent.setScale(ToSpVector(Scale, 1.0f));
}
#endif

