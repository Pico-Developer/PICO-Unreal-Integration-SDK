//
// Created by bytedance on 2025/2/26.
//

#ifndef PICOXRRUNTIME_PICO_LIGHT_ESTIMATION_EXTENSION_H
#define PICOXRRUNTIME_PICO_LIGHT_ESTIMATION_EXTENSION_H


#include "openxr.h"

#if PLATFORM_ANDROID
#include "VulkanRHIPrivate.h"
#endif

/**
 *  XrStructureType : 1200392100 ~ 1200392209
 */

#ifdef __cplusplus
extern "C" {
#endif

#define XR_PICO_light_estimation  1
#define XR_PICO_light_estimation_SPEC_VERSION 1
#define XR_PICO_LIGHT_ESTIMATION_EXTENSION_NAME "XR_PICO_light_estimation"


// Define the structure type constant for Light Estimation
static const XrStructureType XR_TYPE_SYSTEM_LIGHT_ESTIMATION_PROPERTIES_PICO = (XrStructureType)1010028000;

// Define the system property structure for Light Estimation
typedef struct XrSystemLightEstimationPropertiesPICO {
    XrStructureType       type;             // XR_TYPE_SYSTEM_LIGHT_ESTIMATION_PROPERTIES_PICO
    void*                 next;                       // Pointer to the extension structure
    XrBool32              supportsLightEstimation; // Indicates whether the Light Estimation feature is supported
    XrBool32              supportsEnvironmentTexture;
    XrBool32              supportsSphericalHarmonics;
} XrSystemLightEstimationPropertiesPICO;


static const XrStructureType XR_TYPE_SENSE_DATA_PROVIDER_CREATE_INFO_LIGHT_ESTIMATION_PICO  = (XrStructureType) 1010028001;


typedef XrFlags64 XrLightEstimationCreateFlagsPICO;
static const XrLightEstimationCreateFlagsPICO XR_LIGHT_ESTIMATION_CREATE_SPHERICAL_HARMONICS_BIT_PICO = 0x00000001;
static const XrLightEstimationCreateFlagsPICO XR_LIGHT_ESTIMATION_CREATE_ENVIRONMENT_TEXTURE_BIT_PICO = 0x00000002;


typedef struct XrSenseDataProviderCreateInfoLightEstimationPICO {
    XrStructureType                                            type;        // XR_TYPE_SENSE_DATA_PROVIDER_CREATE_INFO_ENVIRONMENT_TEXTURE_PICO
    const void*                                                next;
    XrLightEstimationCreateFlagsPICO                             createFlags; // Creation flag bits used to specify the light estimation configuration
} XrSenseDataProviderCreateInfoLightEstimationPICO;


typedef enum XrEnvironmentTextureResolutionPICO {
    XR_ENVIRONMENT_TEXTURE_RESOLUTION_8_8_PICO = 0,     // 8x8 resolution
    XR_ENVIRONMENT_TEXTURE_RESOLUTION_16_16_PICO = 1,   // 16x16 resolution
    XR_ENVIRONMENT_TEXTURE_RESOLUTION_32_32_PICO = 2,   // 32x32 resolution
    XR_ENVIRONMENT_TEXTURE_RESOLUTION_64_64_PICO = 3,   // 64x64 resolution
    XR_ENVIRONMENT_TEXTURE_RESOLUTION_128_128_PICO = 4,  // 128x128 resolution
    XR_ENVIRONMENT_TEXTURE_RESOLUTION_MAX_ENUM_PICO = 0x7FFFFFFF
} XrEnvironmentTextureResolutionPICO;

typedef enum XrEnvironmentTextureTransferTypePICO {
    XR_ENVIRONMENT_TEXTURE_TRANSFER_TYPE_RAW_PICO = 0,                // Using CPU pointer
    XR_ENVIRONMENT_TEXTURE_TRANSFER_TYPE_GLES_PICO = 1,            // OpenGLES texture
    XR_ENVIRONMENT_TEXTURE_TRANSFER_TYPE_VULKAN_PICO = 2,       // Vulkan Image
    XR_ENVIRONMENT_TEXTURE_TRANSFER_TYPE_MAX_ENUM_PICO = 0x7FFFFFFF
} XrEnvironmentTextureTransferTypePICO;

typedef enum XrEnvironmentTexturePixelFormatPICO {
    XR_ENVIRONMENT_TEXTURE_PIXEL_FORMAT_RGB_16FLOAT_PICO  = 0,  // 16-bit floating-point RGB format
    XR_ENVIRONMENT_TEXTURE_PIXEL_FORMAT_RGBA_16FLOAT_PICO  = 1,
    XR_ENVIRONMENT_TEXTURE_PIXEL_FORMAT_MAX_ENUM_PICO = 0x7FFFFFFF,
} XrEnvironmentTexturePixelFormatPICO;


static const XrStructureType XR_TYPE_ENVIRONMENT_TEXTURE_CREATE_CONFIG_INFO_PICO  = (XrStructureType) 1010028002;
// Define the XrEnvironmentTextureCreateConfigInfoPICO structure
typedef struct XrEnvironmentTextureCreateConfigInfoPICO {
    XrStructureType type;                                  // Structure type
    const void* next;                                      // Pointer to the extension structure
    XrEnvironmentTexturePixelFormatPICO pixelFormat;       // Pixel format
    XrEnvironmentTextureResolutionPICO  resolution;        // Texture resolution
    XrEnvironmentTextureTransferTypePICO  transferType; // File format
} XrEnvironmentTextureCreateConfigInfoPICO;

// Define the XrLightEstimationDataStatePICO enumeration for PICO extension
typedef enum XrLightEstimationDataStatePICO {
    XR_LIGHT_ESTIMATION_DATA_STATE_VALID_PICO  = 0,   // Light estimation is enabled
    XR_LIGHT_ESTIMATION_DATA_STATE_INVALID_PICO = 1,   // Light estimation is unavailable, and the current data is the last valid data
    XR_LIGHT_ESTIMATION_DATA_STATE_MAX_ENUM_PICO = 0x7FFFFFFF,
} XrLightEstimationDataStatePICO;

// Define the static constant value for PICO extension, used for Light Estimation
static const XrSpatialEntityComponentTypePICO XR_SPATIAL_ENTITY_COMPONENT_TYPE_LIGHT_ESTIMATION_PICO = (XrSpatialEntityComponentTypePICO)1010028000;

static const XrStructureType XR_TYPE_SPATIAL_ENTITY_LIGHT_ESTIMATION_DATA_PICO = (XrStructureType)1010028007;

// Define the XrSpatialEntityLightEstimationDataPICO structure for PICO extension
typedef struct XrSpatialEntityLightEstimationDataPICO {
    XrStructureType type;                // Structure type
    void* next;                          // Extension chain pointer
    XrLightEstimationDataStatePICO state;      // Light estimation state
} XrSpatialEntityLightEstimationDataPICO;

// Define the XrStructureType value for PICO extension, used for Light Estimation

// Define the XrLightEstimationInfoBaseHeaderPICO structure for PICO extension
typedef struct XrLightEstimationInfoBaseHeaderPICO {
    XrStructureType type;                 // XR_TYPE_LIGHT_ESTIMATION_BASE_HEADER_PICO
    void* next;                     // Pointer to the extension structure
} XrLightEstimationInfoBaseHeaderPICO;

static const XrStructureType XR_TYPE_LIGHT_ESTIMATION_INFO_ENVIRONMENT_TEXTURE_RAW_PICO  = (XrStructureType) 1010028003;
typedef struct XrLightEstimationInfoEnvironmentTextureRawPICO {
    XrStructureType                        type;        // XR_TYPE_LIGHT_ESTIMATION_INFO_ENVIRONMENT_RAW_PICO
    void*                                  next;        // Pointer to the extension structure
    XrEnvironmentTexturePixelFormatPICO    pixelFormat; // Pixel format (e.g., RGB_16FLOAT, RGBA_32FLOAT)
    uint32_t                               bufferLen;  // Size of the buffer in bytes
    uint8_t*                               cubemapRightImage;
    uint8_t*                               cubemapLeftImage;
    uint8_t*                               cubemapTopImage;
    uint8_t*                               cubemapBottomImage;
    uint8_t*                               cubemapFrontImage;
    uint8_t*                               cubemapBackImage;
} XrLightEstimationInfoEnvironmentTextureRawPICO;

//    h*w*3*sizeof(float16_t);
static const XrStructureType XR_TYPE_LIGHT_ESTIMATION_INFO_ENVIRONMENT_TEXTURE_GLES_PICO  = (XrStructureType) 1010028004;
typedef struct XrLightEstimationInfoEnvironmentTextureGlesPICO {
    XrStructureType             type;
    const void* XR_MAY_ALIAS    next;
    uint32_t                    cubeTexture;
} XrLightEstimationInfoEnvironmentTextureGlesPICO;


// Define the XrStructureType value for PICO extension, used for Light Estimation
static const XrStructureType XR_TYPE_SPATIAL_ENTITY_LIGHT_ESTIMATION_GET_INFO_PICO = (XrStructureType)1010028006;

// Define the XrSpatialEntityLightEstimationGetInfoPICO structure for PICO extension
typedef struct XrSpatialEntityLightEstimationGetInfoPICO {
    XrStructureType                     type;           // XR_TYPE_SPATIAL_ENTITY_LIGHT_ESTIMATION_GET_INFO_PICO
    const void*                         next;           // Pointer to the extension structure
    XrSpatialEntityIdPICO               entity;         // Unique identifier of the spatial entity
    XrSpatialEntityComponentTypePICO    componentType;  // Spatial entity component type
} XrSpatialEntityLightEstimationGetInfoPICO;


static const XrStructureType XR_TYPE_LIGHT_ESTIMATION_INFO_SPHERICAL_HARMONICS_PICO  = (XrStructureType) 1010028005;

typedef enum XrSphericalHarmonicsKindPICO {
    XR_SPHERICAL_HARMONICS_KIND_TOTAL_PICO = 0,
    XR_SPHERICAL_HARMONICS_KIND_MAX_ENUM_PICO = 0x7FFFFFFF
} XrSphericalHarmonicsKindPICO;

typedef struct XrLightEstimationInfoSphericalHarmonicsPICO {
    XrStructureType                 type;
    const void* XR_MAY_ALIAS        next;
    XrSphericalHarmonicsKindPICO    kind;
    uint32_t                        coeCapacityInput;
    uint32_t                        coeCountOutput;
    float*                          coefficients;
} XrLightEstimationInfoSphericalHarmonicsPICO;


static const XrStructureType XR_TYPE_ENVIRONMENT_TEXTURE_VULKAN_CREATE_CONFIG_INFO_PICO  = (XrStructureType) 1010028008;

#if PLATFORM_ANDROID
//XR_TYPE_LIGHT_ESTIMATE_INFO_ENVIRONMENT_TEXTURE_V_PICO
static const XrStructureType XR_TYPE_LIGHT_ESTIMATION_INFO_ENVIRONMENT_TEXTURE_VULKAN_PICO  = (XrStructureType) 1010028009;

typedef struct XrEnvironmentTextureVulkanCreateConfigInfoPICO {
    XrStructureType       type;        // Structure type
    const void*           next;        // Pointer to the extension structure
    uint32_t              createFlags; // VkImageCreateFlags
    uint32_t              usageFlags;  // VkImageUsageFlags
} XrEnvironmentTextureVulkanCreateConfigInfoPICO;

typedef struct XrLightEstimationInfoEnvironmentTextureVulkanPICO { //
    XrStructureType type;        // XR_TYPE_LIGHT_ESTIMATION_INFO_ENVIRONMENT_VULKAN_IMAGE_PICO
    void*           next;        // Pointer to the extension structure
    uint32_t        arrayLayers; // Number of array layers of the image (e.g., 6 for a cubemap, 1 for a 2D texture)
    VkImage         image;       // Handle of the Vulkan image object (VkImage)
    int             mipLevels;
    int             format;
} XrLightEstimationInfoEnvironmentTextureVulkanPICO;
#endif
    

typedef XrResult (XRAPI_ATTR *PFN_xrEnumerateEnvironmentTextureResolutionsPICO)(
    XrInstance                                  instance,                   // Instance handle.
    uint32_t                                    resolutionCapacityInput,    // Capacity of the provided resolution array.
    uint32_t*                                   resolutionCountOutput,      // Returns the number of available resolutions or the number of written resolutions.
    XrEnvironmentTextureResolutionPICO*         resolutions                // User-provided array to store resolution data.
);
typedef XrResult (XRAPI_ATTR *PFN_xrEnumerateEnvironmentTexturePixelFormatsPICO)(
    XrInstance instance,                                // Instance handle
    uint32_t  pixelFormatCapacityInput,                 // Capacity of the provided pixel format array
    uint32_t* pixelFormatCountOutput,                   // Returns the number of available pixel formats or the number of written pixel formats
    XrEnvironmentTexturePixelFormatPICO* pixelFormats   // User-provided array to store pixel format data
);

typedef XrResult (XRAPI_ATTR *PFN_xrEnumerateEnvironmentTextureTransferTypesPICO)(
    XrInstance instance,                                 // Instance handle
    uint32_t transferTypeCapacityInput,                   // Capacity of the provided environment texture file format array
    uint32_t* transferTypeCountOutput,                    // Gets the supported formats for passing environment texture data
    XrEnvironmentTextureTransferTypePICO* transferTypes   // User-provided array to store the transfer format data of the environment texture
);


typedef XrResult (XRAPI_ATTR *PFN_xrReleaseEnvironmentTextureImagePICO)(
        XrSession session,
        XrLightEstimationInfoBaseHeaderPICO* image
);


#ifndef XR_NO_PROTOTYPES
#ifdef XR_EXTENSION_PROTOTYPES

XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateEnvironmentTextureResolutionsPICO(
    XrInstance                                  instance,                   // Instance handle.
    uint32_t                                    resolutionCapacityInput,    // Capacity of the provided resolution array.
    uint32_t*                                   resolutionCountOutput,      // Returns the number of available resolutions or the number of written resolutions.
    XrEnvironmentTextureResolutionPICO*         resolutions);                  // User-provided array to store resolution data.

XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateEnvironmentTexturePixelFormatsPICO(
    XrInstance instance,                                // Instance handle
    uint32_t  pixelFormatCapacityInput,                 // Capacity of the provided pixel format array
    uint32_t* pixelFormatCountOutput,                   // Returns the number of available pixel formats or the number of written pixel formats
    XrEnvironmentTexturePixelFormatPICO* pixelFormats   // User-provided array to store pixel format data
);

XRAPI_ATTR XrResult XRAPI_CALL xrEnumerateEnvironmentTextureTransferTypesPICO(
    XrInstance instance,                                 // Instance handle
    uint32_t  transferTypeCapacityInput,                   // Capacity of the provided environment texture file format array
    uint32_t* transferTypeCountOutput,                    // Gets the supported formats for passing environment texture data
    XrEnvironmentTextureTransferTypePICO* transferTypes   // User-provided array to store the transfer format data of the environment texture
);

XRAPI_ATTR XrResult XRAPI_CALL  xrReleaseEnvironmentTextureImagePICO(
         XrSession session,
         XrLightEstimationInfoBaseHeaderPICO* image
         );


#endif /* XR_EXTENSION_PROTOTYPES */
#endif /* !XR_NO_PROTOTYPES */

#ifdef __cplusplus
}
#endif

#endif //PICOXRRUNTIME_PICO_LIGHT_ESTIMATION_EXTENSION_H


