// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICOOpenXRRuntimeSettings.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/FileManager.h"
#include "Engine/RendererSettings.h"
#include "Misc/ConfigUtilities.h"
#include "Engine/RendererSettings.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY(LogPICOOpenXRSettings);

namespace
{
	const TCHAR* PICOOpenXRRuntimeSettingsSection = TEXT("/Script/PICOOpenXRRuntimeSettings.PICOOpenXRRuntimeSettings");

#if WITH_EDITOR
	void ApplyChangedPropertyToConsoleVariable(UPICOOpenXRRuntimeSettings* Settings, FProperty* PropertyThatChanged)
	{
		if (!Settings || !PropertyThatChanged)
		{
			return;
		}

		const FString& CVarName = PropertyThatChanged->GetMetaData(TEXT("ConsoleVariable"));
		if (CVarName.IsEmpty())
		{
			return;
		}

		IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
		if (!CVar || (CVar->GetFlags() & ECVF_ReadOnly) != 0)
		{
			return;
		}

		if (FByteProperty* ByteProperty = CastField<FByteProperty>(PropertyThatChanged))
		{
			if (ByteProperty->Enum != nullptr)
			{
				CVar->Set(ByteProperty->GetPropertyValue_InContainer(Settings), ECVF_SetByPluginHighPriority);
			}
		}
		else if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(PropertyThatChanged))
		{
			FNumericProperty* UnderlyingProp = EnumProperty->GetUnderlyingProperty();
			void* PropertyAddress = EnumProperty->ContainerPtrToValuePtr<void>(Settings);
			CVar->Set((int32)UnderlyingProp->GetSignedIntPropertyValue(PropertyAddress), ECVF_SetByPluginHighPriority);
		}
		else if (FBoolProperty* BoolProperty = CastField<FBoolProperty>(PropertyThatChanged))
		{
			CVar->Set((int32)BoolProperty->GetPropertyValue_InContainer(Settings), ECVF_SetByPluginHighPriority);
		}
		else if (FIntProperty* IntProperty = CastField<FIntProperty>(PropertyThatChanged))
		{
			CVar->Set(IntProperty->GetPropertyValue_InContainer(Settings), ECVF_SetByPluginHighPriority);
		}
		else if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(PropertyThatChanged))
		{
			CVar->Set(FloatProperty->GetPropertyValue_InContainer(Settings), ECVF_SetByPluginHighPriority);
		}
		else if (FStrProperty* StringProperty = CastField<FStrProperty>(PropertyThatChanged))
		{
			CVar->Set(*StringProperty->GetPropertyValue_InContainer(Settings), ECVF_SetByPluginHighPriority);
		}
		else if (FNameProperty* NameProperty = CastField<FNameProperty>(PropertyThatChanged))
		{
			CVar->Set(*NameProperty->GetPropertyValue_InContainer(Settings).ToString(), ECVF_SetByPluginHighPriority);
		}
	}
#endif
}

IMPLEMENT_MODULE(FDefaultModuleImpl, PICOOpenXRRuntimeSettings);

UPICOOpenXRRuntimeSettings::UPICOOpenXRRuntimeSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPICOOpenXRRuntimeSettings::ApplyCVarSettingsFromProjectSettings()
{
	UE::ConfigUtilities::ApplyCVarSettingsFromIni(PICOOpenXRRuntimeSettingsSection, *GEngineIni, ECVF_SetByPluginHighPriority);
}

void UPICOOpenXRRuntimeSettings::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITOR
	Super::TryUpdateDefaultConfigFile();

	if (GConfig)
	{
		const FString Value = FString("<meta-data android:name=\"") + "pvr.app.type" + "\" android:value=\"" + "vr" + "\" />";
		const FString MetaDataName = TEXT("android:name=\"pvr.app.type\"");
		const FString SettingName = TEXT("ExtraActivitySettings");
		const FString SectionName = TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings");
		const FString DefaultEnginePath = FConfigCacheIni::NormalizeConfigIniPath(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("DefaultEngine.ini")));
		FString ExistingValue;
		bool bHasMatchingSetting = false;
		GConfig->LoadFile(DefaultEnginePath);
		GConfig->GetString(*SectionName, *SettingName, ExistingValue, DefaultEnginePath);
		if (!ExistingValue.IsEmpty())
		{
			TArray<FString> ExistingSettings;
			ExistingValue.ParseIntoArray(ExistingSettings, TEXT("\\n"), true);

			for (FString ExistingSetting : ExistingSettings)
			{
				ExistingSetting.TrimStartAndEndInline();
				if (ExistingSetting.Contains(MetaDataName))
				{
					bHasMatchingSetting = true;
					break;
				}
			}
		}

		if (!bHasMatchingSetting)
		{
			if (!ExistingValue.IsEmpty())
			{
				if (!ExistingValue.EndsWith(TEXT("\\n")))
				{
					ExistingValue += TEXT("\\n");
				}
				ExistingValue += Value;
			}
			else
			{
				ExistingValue = Value;
			}

			GConfig->SetString(*SectionName, *SettingName, *ExistingValue, DefaultEnginePath);
		}
		GConfig->Flush(false);
		GConfig->UnloadFile(DefaultEnginePath);
	}

	ToggleOcclusionCulling();
#endif
}

bool UPICOOpenXRRuntimeSettings::GetBoolConfigByKey(const FString& InKeyName)
{
	if(const FConfigSection* Section = GConfig->GetSection(TEXT("/Script/PICOOpenXRRuntimeSettings.PICOOpenXRRuntimeSettings"), false, GEngineIni))
	{
		for(FConfigSectionMap::TConstIterator It(*Section); It; ++It)
		{
			const FString& KeyString = It.Key().GetPlainNameString(); 
			const FString& ValueString = It.Value().GetValue();

			if (KeyString==InKeyName)
			{
				const FString& NewValueString = UE::ConfigUtilities::ConvertValueFromHumanFriendlyValue(*ValueString);

				if (NewValueString == TEXT("1"))
				{
					return true;
				}
			}
		}
	}
	return false;
}

void UPICOOpenXRRuntimeSettings::ToggleOcclusionCulling()
{
	URendererSettings* RendererSettings = GetMutableDefault<URendererSettings>();
	RendererSettings->bOcclusionCulling = !bDisableOcclusionCulling;
	RendererSettings->UpdateSinglePropertyInConfigFile(RendererSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(URendererSettings, bOcclusionCulling)), RendererSettings->GetDefaultConfigFilename());
}

#if WITH_EDITOR
void UPICOOpenXRRuntimeSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	GConfig->Flush(false);
	ApplyChangedPropertyToConsoleVariable(this, PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty : PropertyChangedEvent.Property);

	FString Dst = FPaths::ProjectDir() / TEXT("pico_splash.png");
	if (!(bUsingOSSplash && IFileManager::Get().Copy(*Dst, *OSSplashScreen.FilePath, true) == COPY_OK))
	{
		IFileManager::Get().Delete(*Dst, true);
	}

	static const FName EnableSemanticsAlignWithVertex = GET_MEMBER_NAME_CHECKED(UPICOOpenXRRuntimeSettings, bSemanticsAlignWithVertex);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == EnableSemanticsAlignWithVertex)
	{
		if (bSemanticsAlignWithVertex == true)
		{
			bSemanticsAlignWithTriangle = false;
			UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOOpenXRRuntimeSettings, bSemanticsAlignWithTriangle)), GetDefaultConfigFilename());
		}
	}

	static const FName EnableSemanticsAlignWithTriangle = GET_MEMBER_NAME_CHECKED(UPICOOpenXRRuntimeSettings, bSemanticsAlignWithTriangle);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == EnableSemanticsAlignWithTriangle)
	{
		if (bSemanticsAlignWithTriangle == true)
		{
			bSemanticsAlignWithVertex = false;
			UpdateSinglePropertyInConfigFile(GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOOpenXRRuntimeSettings, bSemanticsAlignWithVertex)), GetDefaultConfigFilename());
		}
	}

	ToggleOcclusionCulling();
}

#endif
