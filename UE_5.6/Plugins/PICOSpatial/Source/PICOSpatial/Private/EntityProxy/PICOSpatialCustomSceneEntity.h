#pragma once

#include "PICOSpatialSceneEntity.h"

class FCustomSceneEntityProxy : public FSceneEntityProxy
{
public:
	FCustomSceneEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	virtual ~FCustomSceneEntityProxy();

	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	bool RefreshNativeEntityFromComponent();

private:
	TWeakObjectPtr<UCustomSceneEntityComponent> CustomComponent;
	TWeakObjectPtr<UPICOSpatialECSEntity> ECSEntity;
	spatialpack::Scene* LastInitializedScene = nullptr;
	spatialpack::AppForm* LastSpatialContainer = nullptr;
};

