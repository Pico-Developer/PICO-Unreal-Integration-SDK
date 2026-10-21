// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_PassthroughFunctionLibrary.h"
#include "PICO_PassthroughModule.h"

bool UPassthroughFunctionLibraryPICO::GetSupportedPassthroughPICO(bool& Support, bool& HasColor, bool& HasDepth)
{
	return FPICOOpenXRPassthroughModule::Get().GetPassthroughPlugin().GetSupportedPassthrough(Support, HasColor, HasDepth);
}
