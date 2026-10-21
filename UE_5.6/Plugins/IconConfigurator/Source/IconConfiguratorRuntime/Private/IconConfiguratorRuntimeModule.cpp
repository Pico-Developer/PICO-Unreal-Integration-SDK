// Copyright PICO Technology Co., Ltd. All rights reserved.

#include "Modules/ModuleManager.h"

/**
 * Minimal runtime module for IconConfigurator plugin.
 * Its sole purpose is to participate in Android packaging so that
 * the UPL (IconConfigurator_UPL_Android.xml) gets mounted and
 * AndroidManifest.xml is properly injected with icon resource references.
 */
class FIconConfiguratorRuntimeModule : public IModuleInterface
{
};

IMPLEMENT_MODULE(FIconConfiguratorRuntimeModule, IconConfiguratorRuntime)
