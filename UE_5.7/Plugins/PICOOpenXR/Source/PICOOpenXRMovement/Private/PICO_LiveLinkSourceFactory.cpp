// Fill out your copyright notice in the Description page of Project Settings.

#include "PICO_LiveLinkSourceFactory.h"
#include "PICO_MovementModule.h"
#include "ILiveLinkClient.h"

#define LOCTEXT_NAMESPACE "PICOLiveLink"

FText UPICOLiveLinkSourceFactory::GetSourceDisplayName() const
{
	return LOCTEXT("PICOLiveLinkSourceName", "PICO Live Link");
}

FText UPICOLiveLinkSourceFactory::GetSourceTooltip() const
{
	return LOCTEXT("PICOLiveLinkSourceTooltip", "PICO Live Link Source");
}

UPICOLiveLinkSourceFactory::EMenuType UPICOLiveLinkSourceFactory::GetMenuType() const
{
	if (IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		const ILiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);

		if (!FPICOOpenXRMovementModule::Get().IsLiveLinkSourceValid() || !LiveLinkClient.HasSourceBeenAdded(FPICOOpenXRMovementModule::Get().GetLiveLinkSource()))
		{
			return EMenuType::MenuEntry;
		}
	}
	return EMenuType::Disabled;
}

TSharedPtr<ILiveLinkSource> UPICOLiveLinkSourceFactory::CreateSource(const FString& ConnectionString) const
{
	return FPICOOpenXRMovementModule::Get().GetLiveLinkSource();
}

#undef LOCTEXT_NAMESPACE