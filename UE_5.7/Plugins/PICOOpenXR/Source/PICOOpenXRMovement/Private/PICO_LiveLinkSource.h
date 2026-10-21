#pragma once

#include "CoreMinimal.h"
#include "ILiveLinkSource.h"
#include "LiveLinkTypes.h"
#include "Roles/LiveLinkBasicRole.h"
#include "Tickable.h"

#include "PICO_FaceTrackingComponent.h"

#define LOCTEXT_NAMESPACE "PICOLiveLink"

namespace PICOLiveLink
{
	template <typename PICODataFields, typename PICOState, typename RoleTypeStaticData, typename RoleTypeFrameData, typename RoleT>
	class TSubject
	{
	public:
		explicit TSubject();
		using Role = RoleT;

		const FLiveLinkSubjectName Name;

		FLiveLinkStaticDataStruct StaticData() const;
		FLiveLinkFrameDataStruct FrameData();
		bool IsLastFrameValid() const { return bLastFrameIsValid; };
		bool Start();
		bool Stop();
		static bool IsSupported();

	private:
		bool bLastFrameIsValid;
		bool bStarted;
		PICOState LastState;

		void UpdateFrame(RoleTypeFrameData& FrameData);
	};

	using FFaceSubject = TSubject<EFaceBlendShapePICO, FFaceStatePICO, FLiveLinkBaseStaticData, FLiveLinkBaseFrameData, ULiveLinkBasicRole>;
	
	class LiveLinkSource : public ILiveLinkSource, public FTickableGameObject
	{
	public:
		LiveLinkSource();
		virtual ~LiveLinkSource() override = default;

		// ILiveLinkSource implementation

		virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
		virtual bool IsSourceStillValid() const override;
		virtual bool RequestSourceShutdown() override;
		virtual FText GetSourceType() const override;
		virtual FText GetSourceMachineName() const override;
		virtual FText GetSourceStatus() const override;

		// FTickableGameObject implementation

		virtual void Tick(float DeltaTime) override;
		virtual bool IsTickable() const override { return bAnySupported && Client; };
		virtual TStatId GetStatId() const override
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FPICOLiveLink, STATGROUP_Tickables);
		}
		virtual bool IsTickableInEditor() const override { return true; }
		virtual bool IsTickableWhenPaused() const override { return true; }

	private:
		template <typename SubjectT>
		void InitializeMovementSubject(TOptional<FLiveLinkSubjectKey>& Key, SubjectT& Subject);
		void InitializeMovementSubjects();
		template <typename SubjectT>
		void UpdateMovementSubject(const TOptional<FLiveLinkSubjectKey>& Key, SubjectT& Subject);
		void UpdateMovementSubjects();

		// LiveLink Data
		ILiveLinkClient* Client{ nullptr };
		FGuid SourceGuid;
		const bool bAnySupported;
		TOptional<FLiveLinkSubjectKey> KeyFace;
		FFaceSubject Face;
	};
} // namespace PICOLiveLink
#undef LOCTEXT_NAMESPACE
