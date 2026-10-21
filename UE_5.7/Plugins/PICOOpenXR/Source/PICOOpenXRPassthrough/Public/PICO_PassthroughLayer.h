// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/ObjectMacros.h"
#include "Components/StereoLayerComponent.h"
#include "PICO_PassthroughLayerShape.h"
#include "PICO_PassthroughLayer.generated.h"

UCLASS(Abstract, meta = (DisplayName = "Passthrough Layer Base PICO"))
class PICOOPENXRPASSTHROUGH_API UPassthroughLayerBasePICO : public UStereoLayerShape
{
	GENERATED_BODY()

public:

	/** Ordering of passthrough layer in relation to scene rendering */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", DisplayName = "Layer Placement")
		TEnumAsByte<enum EPassthroughLayerOrderPICO> LayerOrder;

	/** Opacity of the (main) passthrough texture.  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (UIMin = 0.0, UIMax = 1.0, ClampMin = 0.0, ClampMax = 1.0))
		float TextureOpacityFactor = 1.0f;

	/** Enable edge color */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough")
		bool bEnableEdgeColor = false;

	/** Color of the passthrough edge rendering effect. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough")
		FLinearColor EdgeColor;

	/** Enable color mapping */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough")
		bool bEnableColorMap = false;

	/** Type of colormapping to perform */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (EditCondition = "bEnableColorMap", EditConditionHides))
		EColorMapTypePICO ColorMapType;

	/** Whether to use color map curve or gradient*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (EditCondition = "bEnableColorMap && ColorMapType == 1", EditConditionHides))
		bool bUseColorMapCurve = false;

	/** Passthrough color mapping gradient converts grayscale to color*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (EditCondition = "bEnableColorMap && bUseColorMapCurve && ColorMapType == 1", EditConditionHides))
		UCurveLinearColor* ColorMapCurve;

	/** Contrast setting for color mapping*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (ClampMin = "-1", ClampMax = "1", EditCondition = "bEnableColorMap && ColorMapType > 0", EditConditionHides))
		float Contrast = 0.0f;

	/** Brightness setting for color mapping*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (ClampMin = "-1", ClampMax = "1", EditCondition = "bEnableColorMap && ColorMapType > 0", EditConditionHides))
		float Brightness = 0.0f;

	/** Posterize setting for grayscale and grayscale to color mapping*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (ClampMin = "0", ClampMax = "1", EditCondition = "bEnableColorMap && ColorMapType > 0 && ColorMapType < 3", EditConditionHides))
		float Posterize = 0.0f;

	/** Saturation setting for color adjustment mapping*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (ClampMin = "-1", ClampMax = "1", EditCondition = "bEnableColorMap && ColorMapType == 3", EditConditionHides))
		float Saturation = 0.0f;

	/** Color value that will be multiplied to the current color map*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (EditCondition = "bEnableColorMap", EditConditionHides))
		FLinearColor ColorScale = FLinearColor::White;

	/** Color value that will be added to the current color map*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO|Passthrough", meta = (EditCondition = "bEnableColorMap", EditConditionHides))
		FLinearColor ColorOffset = FLinearColor::Black;

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void SetTextureOpacity(float InOpacity);

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void EnableEdgeColor(bool bInEnableEdgeColor);

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void EnableColorMap(bool bInEnableColorMap);

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void EnableColorMapCurve(bool bInEnableColorMapCurve);

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void SetEdgeRenderingColor(FLinearColor InEdgeColor);

	/** Set color map controls for grayscale and grayscale to rgb color mapping*/
	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void SetColorMapControls(float InContrast = 0, float InBrightness = 0, float InPosterize = 0);

	/** Set color map controls for color adjustment color mapping*/
	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void SetBrightnessContrastSaturation(float InContrast = 0, float InBrightness = 0, float InSaturation = 0);

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void SetColorScaleAndOffset(FLinearColor InColorScale = FLinearColor::White, FLinearColor InColorOffset = FLinearColor::Black);

	/** Set color curve that will be added to the color map in grayscale modes --> will be converted into a gradient*/
	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void SetColorMapCurve(UCurveLinearColor* InColorMapCurve);

	/** Set color map gradient directly instead through a color curve*/
	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void SetColorMapGradient(const TArray<FLinearColor>& InColorMapGradient);

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void ClearColorMap();

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void SetColorMapType(EColorMapTypePICO InColorMapType);

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void SetLayerPlacement(EPassthroughLayerOrderPICO InLayerOrder);

	void MarkStereoLayerDirty();

protected:
	TArray<FLinearColor> ColorMapGradient;
	TArray<FLinearColor> NeutralMapGradient;

	TArray<FLinearColor> GenerateColorGradientFromColorCurve(const UCurveLinearColor* InColorMapCurve) const;
	TArray<FLinearColor> GetOrGenerateNeutralColorMapGradient();
	TArray<FLinearColor> GetColorMapGradient(bool bInUseColorMapCurve, const UCurveLinearColor* InColorMapCurve);
	TArray<FLinearColor> GenerateColorMapGradient(bool bInUseColorMapCurve, const UCurveLinearColor* InColorMapCurve);
};

/* FullScreen Passthrough Layer*/
UCLASS(meta = (DisplayName = "Passthrough Full Screen Layer PICO"))
class PICOOPENXRPASSTHROUGH_API UPassthroughFullScreenLayerPICO : public UPassthroughLayerBasePICO
{
	GENERATED_BODY()
public:

	virtual void ApplyShape(IStereoLayers::FLayerDesc& LayerDesc) override;

};

/* User Defined Passthrough Layer*/
UCLASS(meta = (DisplayName = "Passthrough Custom Shape Layer PICO"))
class PICOOPENXRPASSTHROUGH_API UPassthroughCustomShapeLayerPICO : public UPassthroughLayerBasePICO
{
	GENERATED_BODY()
public:
	void AddGeometry(const FString& MeshName, FPICOPassthroughMeshRef PassthroughMesh, FTransform Transform, bool bUpdateTransform);
	void RemoveGeometry(const FString& MeshName);

	virtual void ApplyShape(IStereoLayers::FLayerDesc& LayerDesc) override;
	TArray<FCustomShapeGeometryDescPICO>& GetUserGeometryList() { return UserGeometryList; };

private:
	TArray<FCustomShapeGeometryDescPICO> UserGeometryList;
};

UCLASS(Blueprintable, ClassGroup = (PICO), meta = (BlueprintSpawnableComponent, DisplayName = "Passthrough Layer Component PICO"))
class PICOOPENXRPASSTHROUGH_API UPassthroughLayerComponentPICO : public UStereoLayerComponent
{
	GENERATED_UCLASS_BODY()

public:
	void OnUnregister() override;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdatePassthroughObjects();

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void AddSurfaceGeometryPICO(AStaticMeshActor* StaticMeshActor, bool updateTransform);

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void RemoveSurfaceGeometryPICO(AStaticMeshActor* StaticMeshActor);

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		bool IsSurfaceGeometryPICO(AStaticMeshActor* StaticMeshActor) const;

	// Manually mark the stereo layer passthrough effect for updating
	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		void MarkPassthroughStyleForUpdatePICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		bool PausePassthroughPICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
		bool ResumePassthroughPICO();

	void MarkStereoLayerDirty() { bIsDirty = true; }
protected:
	//virtual bool LayerRequiresTexture() override;

private:

	FPICOPassthroughMeshRef CreatePassthroughMesh(UStaticMesh* StaticMesh);

	UPROPERTY(Transient)
		TMap<FString, AStaticMeshActor*> PassthroughActorMap;

	/** Passthrough style needs to be marked for update **/
	bool bPassthroughStyleNeedsUpdate;

	uint32 passthroughLayerId;

	/** Dirty state determines whether the stereo layer needs updating **/
	bool bIsDirty;

	/** Last transform is cached to determine if the new frames transform has changed **/
	FTransform LastTransform;

	/** Last frames visiblity state **/
	bool bLastVisible;
};
