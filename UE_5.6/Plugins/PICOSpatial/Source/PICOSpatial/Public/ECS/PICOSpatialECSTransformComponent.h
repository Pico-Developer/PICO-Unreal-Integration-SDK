#pragma once

#include "CoreMinimal.h"
#include "ECS/PICOSpatialECSComponent.h"
#include "PICOSpatialECSTransformComponent.generated.h"

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class PICOSPATIAL_API UPICOSpatialECSTransformComponent : public UPICOSpatialECSComponent
{
	GENERATED_BODY()

public:
	virtual EPICOSpatialECSComponentType GetComponentType() const override
	{
		return EPICOSpatialECSComponentType::Transform;
	}

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	FVector getPosition();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	void setPosition(const FVector& InPosition);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	FVector getScale();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	void setScale(const FVector& InScale);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	FQuat getRotation();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	void setRotation(const FQuat& InRotation);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	FTransform getLocalTransform();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Transform Component")
	void setLocalTransform(const FTransform& InTransform);

#if PLATFORM_ANDROID
protected:
	virtual void OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity) override;
	virtual void ApplyAllPropertiesToNative() override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform Component")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform Component")
	FVector Scale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform Component")
	FQuat Rotation = FQuat::Identity;
};

