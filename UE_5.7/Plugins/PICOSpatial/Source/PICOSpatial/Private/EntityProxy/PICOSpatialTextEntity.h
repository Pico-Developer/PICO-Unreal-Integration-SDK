// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialPrimitiveEntity.h"

/**
 * Text entity proxy.
 * Represents a text render component in the spatial scene.
 * Handles text rendering with customizable alignment, color, and sizing.
 */
class FTextEntityProxy : public FPrimitiveEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The text render component this proxy represents
	 */
	FTextEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FTextEntityProxy() {}

	/** Updates proxy data from the text render component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with text render data */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current text proxy data */
	virtual void UpdateNativeEntity() override;

protected:
	/**
	 * Constructs text-specific entity data.
	 * @param SceneComponent The scene component to extract text render data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructTextEntityData(USceneComponent* SceneComponent, bool bInitialize);

	/** Converts UE horizontal alignment to spatial pack alignment */
	FORCEINLINE spatialpack::HorizontalAlignment HorizontalAlignmentConvert(const EHorizTextAligment& HAligment);
	
	/** Converts UE vertical alignment to spatial pack alignment */
	FORCEINLINE spatialpack::VerticalAlignment VerticalAlignmentConvert(const EVerticalTextAligment& VAligment);
	
	/** Checks if two colors are different enough to be considered dirty */
	FORCEINLINE bool IsColorDirty(const spatialpack::Color4f& ColorA, const spatialpack::Color4f& ColorB);

public:
	// Static Data
	
	/** Font size scaling factor */
	float FontSizeScale = 8.0f;
	
	/** Vertical to horizontal scale ratio */
	float VHScale = 1.2f;
	
	/** Stored transform for text positioning */
	FTransform StoredTransform;

	// Dynamic Data
	
	/** Text content to render */
	FText Text;
	
	/** Horizontal alignment of the text */
	spatialpack::HorizontalAlignment HorizontalAlignment;
	
	/** Vertical alignment of the text */
	spatialpack::VerticalAlignment VerticalAlignment;
	
	/** Text render color */
	spatialpack::Color4f TextRenderColor;
	
	/** Text height */
	float Height;
	
	/** Text width */
	float Width;
	
	/** Font size */
	uint32 FontSize;
	
	/** Bit array tracking which properties are dirty */
	TBitArray<> DirtyStates;
	
	/** Last known visible state */
	bool bLastVisibleState;
};
