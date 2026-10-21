#pragma once

#include "CoreMinimal.h"
#include "ECS/PICOSpatialECSComponent.h"
#include "PICOSpatialECSTextComponent.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EPICOSpatialTextHorizontalAlignment : uint8
{
	Left,
	Center,
	Right,
	Justify,
	Flush,
};

UENUM(BlueprintType)
enum class EPICOSpatialTextVerticalAlignment : uint8
{
	Top,
	Center,
	Bottom,
};

UENUM(BlueprintType)
enum class EPICOSpatialTextHorizontalOverflowMode : uint8
{
	Wrap,
	Clip,
	Ellipsis,
	Overflows,
};

UENUM(BlueprintType)
enum class EPICOSpatialTextVerticalOverflowMode : uint8
{
	Clip,
	Overflows,
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class PICOSPATIAL_API UPICOSpatialECSTextComponent : public UPICOSpatialECSComponent
{
	GENERATED_BODY()

public:
	virtual EPICOSpatialECSComponentType GetComponentType() const override
	{
		return EPICOSpatialECSComponentType::Text;
	}

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setTextWidth(float InTextWidth);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setTextHeight(float InTextHeight);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setText(const FString& InText);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setFontSize(int32 InFontSize);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setTextColor(const FLinearColor& InTextColor);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setUnderLine(bool bInUnderLine);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setBackgroundResource(UTexture2D* InBackgroundTexture);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setBackgroundColor(const FLinearColor& InBackgroundColor);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setBackgroundCornerRadius(float InBackgroundCornerRadius);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setHorizontalAlignment(EPICOSpatialTextHorizontalAlignment InHorizontalAlignment);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setVerticalAlignment(EPICOSpatialTextVerticalAlignment InVerticalAlignment);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setHorizontalOverflowMode(EPICOSpatialTextHorizontalOverflowMode InHorizontalOverflowMode);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setVerticalOverflowMode(EPICOSpatialTextVerticalOverflowMode InVerticalOverflowMode);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setEdgeInsets(const FVector4& InEdgeInsets);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Text Component")
	void setRendererEnabled(bool bInRendererEnabled);

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	float getTextWidth() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	float getTextHeight() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	FString getText() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	int32 getFontSize() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	FLinearColor getTextColor() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	bool getUnderLine() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	UTexture2D* getBackgroundResource() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	FLinearColor getBackgroundColor() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	float getBackgroundCornerRadius() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	EPICOSpatialTextHorizontalAlignment getHorizontalAlignment() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	EPICOSpatialTextVerticalAlignment getVerticalAlignment() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	EPICOSpatialTextHorizontalOverflowMode getHorizontalOverflowMode() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	EPICOSpatialTextVerticalOverflowMode getVerticalOverflowMode() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	FVector4 getEdgeInsets() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Text Component")
	bool getRendererEnabled() const;

#if PLATFORM_ANDROID
protected:
	virtual void OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity) override;
	virtual void ApplyAllPropertiesToNative() override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	FText Text = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	float TextWidth = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	float TextHeight = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	int32 FontSize = 32;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	bool bUnderline = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	TObjectPtr<UTexture2D> BackgroundTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	FLinearColor BackgroundColor = FLinearColor(-1.0f, -1.0f, -1.0f, -1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	float BackgroundCornerRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	EPICOSpatialTextHorizontalAlignment HorizontalAlignment = EPICOSpatialTextHorizontalAlignment::Center;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	EPICOSpatialTextVerticalAlignment VerticalAlignment = EPICOSpatialTextVerticalAlignment::Center;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	EPICOSpatialTextHorizontalOverflowMode HorizontalOverflowMode = EPICOSpatialTextHorizontalOverflowMode::Wrap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	EPICOSpatialTextVerticalOverflowMode VerticalOverflowMode = EPICOSpatialTextVerticalOverflowMode::Clip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	FVector4 EdgeInsets = FVector4(0.0f, 0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text Component")
	bool bRendererEnabled = true;
};
