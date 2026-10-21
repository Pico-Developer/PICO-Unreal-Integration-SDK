#ifndef EXT_HAPTIC_PARAMETRIC_H_
#define EXT_HAPTIC_PARAMETRIC_H_ 1

/*
** Copyright 2017-2025 The Khronos Group Inc.
**
** SPDX-License-Identifier: Apache-2.0 OR MIT
*/

/*
** This header is generated from the Khronos OpenXR XML API Registry.
**
*/


#ifdef __cplusplus
extern "C" {
#endif


#ifndef XR_EXT_haptic_parametric

// XR_EXT_haptic_parametric is a preprocessor guard. Do not pass it to API calls.
#define XR_EXT_haptic_parametric 1

#define XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT 500


#define XR_HAPTIC_PARAMETRIC_VIBRATION_EXTEND_DURATION_EXT 50000000


#define XR_HAPTIC_PARAMETRIC_FREQUENCY_MIN_HZ_EXT 1


#define XR_HAPTIC_PARAMETRIC_FREQUENCY_MAX_HZ_EXT 1000

#define XR_EXT_haptic_parametric_SPEC_VERSION 1
#define XR_EXT_HAPTIC_PARAMETRIC_EXTENSION_NAME "XR_EXT_haptic_parametric"
#define XR_TYPE_HAPTIC_PARAMETRIC_VIBRATION_EXT ((XrStructureType) 1000775000U)
#define XR_TYPE_HAPTIC_PARAMETRIC_PROPERTIES_EXT ((XrStructureType) 1000775001U)
#define XR_TYPE_SYSTEM_HAPTIC_PARAMETRIC_PROPERTIES_EXT ((XrStructureType) 1000775002U)

typedef enum XrHapticParametricStreamFrameTypeEXT {
    XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_NONE_EXT = 0,
    XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_FIRST_FRAME_EXT = 1,
    XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_INTERMEDIATE_FRAME_EXT = 2,
    XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_LAST_FRAME_EXT = 3,
    XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_MAX_ENUM_EXT = 0x7FFFFFFF
} XrHapticParametricStreamFrameTypeEXT;
typedef struct XrHapticParametricPropertiesEXT {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    XrDuration                  idealFrameSubmissionRate;
    XrDuration                  minimumFirstFrameDuration;
    float                       minFrequencyHz;
    float                       maxFrequencyHz;
} XrHapticParametricPropertiesEXT;

typedef struct XrHapticParametricPointEXT {
    XrDuration    time;
    float         value;
} XrHapticParametricPointEXT;

typedef struct XrHapticParametricTransientEXT {
    XrDuration    time;
    float         amplitude;
    float         frequency;
} XrHapticParametricTransientEXT;

typedef struct XrHapticParametricVibrationEXT {
    XrStructureType                          type;
    const void* XR_MAY_ALIAS                 next;
    uint32_t                                 amplitudePointCount;
    const XrHapticParametricPointEXT*        amplitudePoints;
    uint32_t                                 frequencyPointCount;
    const XrHapticParametricPointEXT*        frequencyPoints;
    uint32_t                                 transientCount;
    const XrHapticParametricTransientEXT*    transients;
    float                                    minFrequencyHz;
    float                                    maxFrequencyHz;
    XrHapticParametricStreamFrameTypeEXT     streamFrameType;
} XrHapticParametricVibrationEXT;

typedef struct XrSystemHapticParametricPropertiesEXT {
    XrStructureType       type;
    void* XR_MAY_ALIAS    next;
    XrBool32              supportsParametricHaptics;
} XrSystemHapticParametricPropertiesEXT;

typedef XrResult (XRAPI_PTR *PFN_xrHapticParametricGetPropertiesEXT)(XrSession session, const XrHapticActionInfo* hapticActionInfo, XrHapticParametricPropertiesEXT* parametricProperties);

#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES
XRAPI_ATTR XrResult XRAPI_CALL xrHapticParametricGetPropertiesEXT(
    XrSession                                   session,
    const XrHapticActionInfo*                   hapticActionInfo,
    XrHapticParametricPropertiesEXT*            parametricProperties);
#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */
#endif /* XR_EXT_haptic_parametric */

#ifdef __cplusplus
}
#endif

#endif
