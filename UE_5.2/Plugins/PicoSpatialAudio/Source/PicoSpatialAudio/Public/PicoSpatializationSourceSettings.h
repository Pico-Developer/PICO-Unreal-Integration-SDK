//  Copyright © 2015-2023 Pico Technology Co., Ltd. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#if WITH_EDITOR
#include "Editor.h"
#endif
#include "IAudioExtensionPlugin.h"
#include "PicoSpatialAudioEnums.h"
#include "pxr_audio_spatializer_types.h"
#include "PxrAudioSpatializerCommonUtils.h"
#include "PicoSpatializationSourceSettings.generated.h"

/**
 * 
 */
UCLASS()
class PICOSPATIALAUDIO_API UPicoSpatializationSourceSettings : public USpatializationPluginSourceSettingsBase
{
	GENERATED_BODY()

public:
	UPicoSpatializationSourceSettings();
#if WITH_EDITOR
	// See if Audio Component references this settings instance:
	bool DoesAudioComponentReferenceThis(class UAudioComponent* InAudioComponent);

	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// Extra gain of the sound source (in dBFS).
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "SpatializationSettings",
		meta = (ClampMin = "0.0", ClampMax = "24.0", UIMin = "0.0", UIMax = "24.0"))
	float SourceGainDb;

	// Reflection gain of the sound source (in dBFS).
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "SpatializationSettings",
		meta = (ClampMin = "-120.0", ClampMax = "48.0", UIMin = "-120.0", UIMax = "48.0"))
	float ReflectionGainDb;
	
	// Volumetric Size (Radius) of the sound source (in meters).
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "SpatializationSettings",
		meta = (ClampMin = "0.0", ClampMax = "100000.0", UIMin = "0.0", UIMax = "100000.0"))
	float VolumetricSize;

	// Volumetric Size (Radius) of the sound source (in meters).
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "SpatializationSettings")
	bool EnableDoppler;

	// Roll-off model to use for sound source distance attenuation. Select 'None' (default) to use Unreal attenuation settings
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "SpatializationSettings|Attenuation")
	EPxrAudioSpatializer_SourceAttenuationMode AttenuationMode;

	// Minimum attenuation distance when chosen "Inverse Squared" attenuation mode
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "SpatializationSettings|Attenuation",
		meta = (ClampMin = "0.0", ClampMax = "1000000.0", UIMin = "0.0", UIMax = "1000000.0"))
	float MinAttenuationDistance;

	// Maximum attenuation distance when chosen "Inverse Squared" attenuation mode
	UPROPERTY(GlobalConfig, EditAnywhere, Category = "SpatializationSettings|Attenuation",
		meta = (ClampMin = "0.0", ClampMax = "1000000.0", UIMin = "0.0", UIMax = "1000000.0"))
	float MaxAttenuationDistance;

	DistanceAttenuationCallback DirectSoundDistanceAttenuationCallback;
	DistanceAttenuationCallback IndirectSoundDistanceAttenuationCallback;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "SpatializationSettings|Directivity",
		meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float DirectivityAlpha;

	UPROPERTY(GlobalConfig, EditAnywhere, Category = "SpatializationSettings|Directivity",
		meta = (ClampMin = "0.0", UIMin = "0.0"))
	float DirectivityOrder;

	UPROPERTY(EditAnywhere, Category = "SpatializationSettings|Directivity")
	bool bVisualizePolarPattern;

	UPROPERTY(EditAnywhere, Category = "SpatializationSettings|Directivity")
	UMaterial* PolarPatternMaterial;

	// Sets the sound source gain, applies, and updates
	UFUNCTION(BlueprintCallable, Category = "PicoSpatialAudio|SoundSourceSpatializationSettings")
	void SetSoundSourceGain(float GainDb);

	// Sets the sound source reflection gain, applies, and updates
	UFUNCTION(BlueprintCallable, Category = "PicoSpatialAudio|SoundSourceSpatializationSettings")
	void SetSoundSourceReflectionGain(float GainDb);

	// Sets the sound source volumetric size (radius), applies, and updates
	UFUNCTION(BlueprintCallable, Category = "PicoSpatialAudio|SoundSourceSpatializationSettings")
	void SetSoundSourceSize(float Radius);

	// Sets the sound source attenuation distance (meters), applies, and updates
	UFUNCTION(BlueprintCallable, Category = "PicoSpatialAudio|SoundSourceSpatializationSettings")
	void SetSoundSourceAttenuationDistance(float Min, float Max);

	// Sets on/off status of source doppler effect, applies, and updates
	UFUNCTION(BlueprintCallable, Category = "PicoSpatialAudio|SoundSourceSpatializationSettings")
	void SetSoundSourceDoppler(bool On);

	// Sets source directivity, applies, and updates
	UFUNCTION(BlueprintCallable, Category = "PicoSpatialAudio|SoundSourceSpatializationSettings")
	void SetSoundSourceDirectivity(float Alpha, float Order);

	UFUNCTION(BlueprintCallable, Category = "PicoSpatialAudio|SoundSourceSpatializationSettings")
	void SetSoundSourcePolarPatternVisualization(bool OnOff);

	UFUNCTION(BlueprintCallable, Category = "PicoSpatialAudio|SoundSourceSpatializationSettings")
	void SetSoundSourcePolarPatternMaterial(UMaterial* Material);
};

