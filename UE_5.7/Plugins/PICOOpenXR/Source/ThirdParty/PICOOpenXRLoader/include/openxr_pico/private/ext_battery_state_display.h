/*
** Copyright (c) 2025-2026 PICO Technology Co., Ltd. All rights reserved.
**
** This header declares extension types distributed by PICO for the
** PICO OpenXR plugin. It is not part of the Khronos OpenXR registry in this
** package unless published upstream in a future revision.
*/

#ifndef PICOXRRUNTIME_EXT_BATTERY_STATE_DISPLAY_H
#define PICOXRRUNTIME_EXT_BATTERY_STATE_DISPLAY_H

// XR_EXT_interaction_profile_battery_state_display is a preprocessor guard. Do not pass it to API calls.
#define XR_EXT_interaction_profile_battery_state_display 1
#define XR_EXT_interaction_profile_battery_state_display_SPEC_VERSION 1
#define XR_EXT_INTERACTION_PROFILE_BATTERY_STATE_DISPLAY_EXTENSION_NAME "XR_EXT_interaction_profile_battery_state_display"
typedef XrFlags64 XrBatteryStateDisplayStateFlagsEXT;

static const XrStructureType XR_TYPE_BATTERY_STATE_DISPLAY_EXT = (XrStructureType)1000836000;

// Flag bits for XrBatteryStateDisplayStateFlagsEXT
static const XrBatteryStateDisplayStateFlagsEXT XR_BATTERY_STATE_DISPLAY_STATE_VALID_BIT_EXT = 0x00000001;
static const XrBatteryStateDisplayStateFlagsEXT XR_BATTERY_STATE_DISPLAY_STATE_CHARGING_BIT_EXT = 0x00000002;
static const XrBatteryStateDisplayStateFlagsEXT XR_BATTERY_STATE_DISPLAY_STATE_PLUGGED_IN_BIT_EXT = 0x00000004;
static const XrBatteryStateDisplayStateFlagsEXT XR_BATTERY_STATE_DISPLAY_STATE_NO_BATTERY_BIT_EXT = 0x00000008;

// XrBatteryStateDisplayEXT extends XrInteractionProfileState
typedef struct XrBatteryStateDisplayEXT {
    XrStructureType                       type;
    void* XR_MAY_ALIAS                    next;
    XrBatteryStateDisplayStateFlagsEXT    stateFlags;
    float                                 batteryLevel;
} XrBatteryStateDisplayEXT;


#endif //PICOXRRUNTIME_EXT_BATTERY_STATE_DISPLAY_H
