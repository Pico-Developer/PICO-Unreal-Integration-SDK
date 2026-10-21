//
// Created by Administrator on 2024/10/22.
//

#ifndef ANDROID_CAMERAAPI_H
#define ANDROID_CAMERAAPI_H
#include <unordered_map>

typedef struct PxrVector3f_ {
    float    x;
    float    y;
    float    z;
} PxrVector3f;

typedef struct PxrQuaternionf_ {
    float    x;
    float    y;
    float    z;
    float    w;
} PxrQuaternionf;

typedef struct Pose_ {
    PxrVector3f position;
    PxrQuaternionf orientation;
} Pose;
typedef struct CameraFrame_ {
    uint32_t width;
    uint32_t height;
    uint64_t time;
    uint32_t size;
    unsigned char *data;
    Pose pose;
    int status;
} CameraFrame;
typedef struct RGBCameraParamsNew_ {
    // Intrinsics
    double fx;
    double fy;
    double cx;
    double cy;
    // Extrinsics
    PxrVector3f l_pos;
    PxrQuaternionf l_rot;
    PxrVector3f r_pos;
    PxrQuaternionf r_rot;
} RGBCameraParamsNew;
struct KeyValuePair {
    const char *key;
    const char *value;
};

typedef void (*CapturelibCallBack)(int type);

typedef std::unordered_map<std::string, std::string> StringDictionary;
extern "C" {
#if PLATFORM_ANDROID
typedef void setUEEnv(JavaVM *vm,JNIEnv *env,jclass *GGameActivityClassID,jobject *GGameActivityThis);
#endif
typedef bool closeCamera();
typedef bool openCameraAsync(const KeyValuePair *pairs, int count);
typedef void setConfigureDefault();
typedef void setConfigure(bool enableMvHevc, int videoFps);
typedef void setConfigureMap(const KeyValuePair *pairs, int count);
typedef bool startPerformance(int mode, int width, int height);
typedef bool startPreview(void *surface, int mode, int width, int height);
typedef void setCameraFrameBuffer(CameraFrame *f);
typedef void setCapturelibCallBack(CapturelibCallBack callback);
typedef bool getCameraExtrinsics(int *leftCameraExtrinsicsCount, double **leftCameraExtrinsics,
                         int *rightCameraExtrinsicsCount, double **rightCameraExtrinsics);
typedef bool getCameraIntrinsics(int width, int height, double h_fov, double v_fov, int *count,
                         double **rateArray);
typedef bool getCameraParametersNew(int width, int height, RGBCameraParamsNew *paramsNew);

}


#endif //ANDROID_CAMERAAPI_H
