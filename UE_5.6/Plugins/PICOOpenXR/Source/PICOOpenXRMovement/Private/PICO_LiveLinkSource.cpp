#include "PICO_LiveLinkSource.h"

#include "IHeadMountedDisplayModule.h"

#include "Roles/LiveLinkAnimationTypes.h"
#include "ILiveLinkClient.h"

#define LOCTEXT_NAMESPACE "PICOLiveLink"

namespace PICOLiveLink
{
	template <typename PICODataFields>
	void InitializeRoleStaticData(FLiveLinkBaseStaticData& StaticData)
	{
		constexpr auto FieldsCount = static_cast<uint8>(PICODataFields::COUNT);
		StaticData.PropertyNames.Reserve(FieldsCount);
		for (uint8 XRProperty = 0; XRProperty < FieldsCount; ++XRProperty)
		{
			StaticData.PropertyNames.Add(UEnum::GetValueAsName(static_cast<PICODataFields>(XRProperty)));
		}
	}
	template <typename PICODataFields, typename PICOState, typename RoleTypeStaticData, typename RoleTypeFrameData, typename Role>
	FLiveLinkStaticDataStruct TSubject<PICODataFields, PICOState, RoleTypeStaticData, RoleTypeFrameData, Role>::StaticData() const
	{
		FLiveLinkStaticDataStruct StaticDataStruct(RoleTypeStaticData::StaticStruct());
		RoleTypeStaticData& RoleStaticData(*StaticDataStruct.Cast<RoleTypeStaticData>());
		InitializeRoleStaticData<PICODataFields>(RoleStaticData);
		return StaticDataStruct;
	}
	template <typename PICODataFields, typename PICOState, typename RoleTypeStaticData, typename RoleTypeFrameData, typename Role>
	FLiveLinkFrameDataStruct TSubject<PICODataFields, PICOState, RoleTypeStaticData, RoleTypeFrameData, Role>::FrameData()
	{
		FLiveLinkFrameDataStruct FrameDataStruct(RoleTypeFrameData::StaticStruct());
		RoleTypeFrameData& FrameData(*FrameDataStruct.Cast<RoleTypeFrameData>());
		UpdateFrame(FrameData);
		return FrameDataStruct;
	}

	template <>
	FFaceSubject::TSubject()
		: Name(TEXT("Face"))
		, bLastFrameIsValid(false)
		, bStarted(false)
	{
	}

	template <>
	bool FFaceSubject::Start()
	{
		if (!bStarted)
		{
			bStarted = UMovementFunctionLibraryPICO::StartFaceTrackingPICO(EFaceTrackingModePICO::WithAudioLipsync);
		}
		return bStarted;
	}
	template <>
	bool FFaceSubject::Stop()
	{
		if (bStarted)
		{
			bStarted = !UMovementFunctionLibraryPICO::StopFaceTrackingPICO();
		}
		return !bStarted;
	}

	template <>
	bool FFaceSubject::IsSupported()
	{
		TArray<EFaceTrackingModePICO> SupportedModes;
		bool bSupported = UMovementFunctionLibraryPICO::GetFaceTrackingSupportedPICO(SupportedModes);
		return bSupported;
	}

	template <>
	void FFaceSubject::UpdateFrame(FLiveLinkBaseFrameData& FrameData)
	{
		bLastFrameIsValid = UMovementFunctionLibraryPICO::GetFaceTrackingDataPICO(0, LastState);
		if (bLastFrameIsValid && (LastState.IsUpperFaceDataValid || LastState.IsLowerFaceDataValid))
		{
			constexpr auto FieldsCount = static_cast<uint8>(EFaceBlendShapePICO::COUNT);
			FrameData.PropertyValues.Reserve(FieldsCount);
			for (uint8 i = 0u; i < FieldsCount; ++i)
			{
				FrameData.PropertyValues.Emplace(i >= XR_FACE_EXPRESSION_COUNT_BD ? LastState.LipsyncExpressionWeights[i - XR_FACE_EXPRESSION_COUNT_BD] : LastState.FaceExpressionWeights[i]);
			}
			FrameData.WorldTime = FPlatformTime::Seconds();
		}
	}

	LiveLinkSource::LiveLinkSource()
		: bAnySupported(FFaceSubject::IsSupported())
	{
	}

	void LiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
	{
		Client = InClient;
		SourceGuid = InSourceGuid;

		InitializeMovementSubjects();
		UpdateMovementSubjects();
	}

	bool LiveLinkSource::IsSourceStillValid() const
	{
		return Client != nullptr;
	}

	bool LiveLinkSource::RequestSourceShutdown()
	{
		Client = nullptr;
		SourceGuid.Invalidate();

		if (Face.Stop())
		{
			UE_LOG(PICOOpenXRMovement, Error, TEXT("At least one of the trackers cannot stop."));
		}
		return true;
	}

	FText LiveLinkSource::GetSourceType() const
	{
		return LOCTEXT("PICOLiveLinkSourceType", "PICO OpenXR SDK");
	}

	FText LiveLinkSource::GetSourceMachineName() const
	{
		if (IHeadMountedDisplayModule::IsAvailable())
		{
			const FString DeviceName = IHeadMountedDisplayModule::Get().GetDeviceSystemName();
			return FText::FromString(DeviceName);
		}
		return LOCTEXT("PICOLiveLinkMachineName", "PICO Device");
	}

	FText LiveLinkSource::GetSourceStatus() const
	{
		if (bAnySupported)
		{
			return LOCTEXT("PICOLiveLinkStatusSupported", "Active");
		}
		return LOCTEXT("PICOLiveLinkStatusNotSupported", "Not Supported");
	}

	void LiveLinkSource::Tick(float DeltaTime)
	{
		UpdateMovementSubjects();
	}

	template <typename SubjectT>
	void LiveLinkSource::InitializeMovementSubject(TOptional<FLiveLinkSubjectKey>& Key, SubjectT& Subject)
	{
		if (Key)
		{
			if (Key->Source.IsValid())
			{ // If the key was already in use. Remove it.
				Client->RemoveSubject_AnyThread(*Key);
			}
			Key.Reset();
		}
		if (Subject.IsSupported())
		{
			Key = FLiveLinkSubjectKey(SourceGuid, Subject.Name);
			if (!Subject.Start())
			{
				UE_LOG(PICOOpenXRMovement, Error, TEXT("Tracker for LiveLink subject %s cannot start."), *Subject.Name.ToString());
			}
			using Role = typename std::remove_reference_t<decltype(Subject)>::Role;
			Client->PushSubjectStaticData_AnyThread(*Key, Role::StaticClass(), Subject.StaticData());
		}
		else
		{
			UE_LOG(PICOOpenXRMovement, Log, TEXT("LiveLink subject %s is not supported."), *Subject.Name.ToString());
		}
	}
	void LiveLinkSource::InitializeMovementSubjects()
	{
		check(IsInGameThread());
		InitializeMovementSubject(KeyFace, Face);
	}
	template <typename SubjectT>
	void LiveLinkSource::UpdateMovementSubject(const TOptional<FLiveLinkSubjectKey>& Key, SubjectT& Subject)
	{
		if (Key)
		{
			const bool bPreviousFrameValid = Subject.IsLastFrameValid();
			auto FrameData = Subject.FrameData();
			const bool bFrameValid = Subject.IsLastFrameValid();
			if (bPreviousFrameValid != bFrameValid)
			{
				UE_LOG(PICOOpenXRMovement, Log, TEXT("LiveLink subject %s became %s."), *Subject.Name.ToString(), bFrameValid ? TEXT("valid") : TEXT("invalid"));
			}
			if (bFrameValid)
			{
				Client->PushSubjectFrameData_AnyThread(*Key, MoveTemp(FrameData));
			}
		}
	}

	void LiveLinkSource::UpdateMovementSubjects()
	{
		check(IsInGameThread());
		if (IsSourceStillValid())
		{
			UpdateMovementSubject(KeyFace, Face);
		}
	}
} // namespace PICOMovement
#undef LOCTEXT_NAMESPACE
