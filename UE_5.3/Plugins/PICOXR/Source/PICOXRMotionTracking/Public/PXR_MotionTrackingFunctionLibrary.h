// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "PXR_MotionTrackingTypes.h"
#include "PXR_MotionTrackingFunctionLibrary.generated.h"

UCLASS()
class  PICOXRMOTIONTRACKING_API UPICOXRMotionTrackingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/// <summary>
	/// Gets whether the current device supports face tracking.
	/// </summary>
	/// <param name="Supported">Indicates whether the device supports face tracking:
	/// * `true`: Support
	/// * `false`: Not support
	/// </param>
	/// <param name="SupportedModes"> The EPXRFaceTrackingMode enum. Returns the face tracking modes supported by the current device.
	/// - `PXR_FTM_NONE		  = (uint8)-1`: Do not enable face tracking
	/// - `PXR_FTM_FACE		  = 0`: Image-driven mode
	/// - `PXR_FTM_LIPS		  = 1`: Audio-driven mode
	/// - `PXR_FTM_FACE_LIPS_VIS = 2`: Hybrid-Viseme mode
	/// - `PXR_FTM_FACE_LIPS_BS  = 3`: Hybrid-Blendshape mode
	/// </param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool GetFaceTrackingSupported(bool& Supported, TArray<EPXRFaceTrackingMode>& SupportedModes);

	/// <summary>
	/// Gets the state of face tracking.
	/// @note Supported by PICO 4 Pro and PICO 4 Enterprise.
	/// </summary>
	/// <param name="IsTracking">Returns a bool indicating whether face tracking is working:
	/// * `true`: face tracking is working
	/// * `false`: face tracking has been stopped
	/// </param>
	/// <param name="TrackingState">Returns the face tracking state information, including the face tracking mode and face tracking state code.
	/// - `CurrentTrackingMode`: EPXRFaceTrackingMode enum
	/// 	- `PXR_FTM_NONE		  = (uint8)-1`: Do not enable face tracking
	/// 	- `PXR_FTM_FACE		  = 0`: Image-driven mode
	/// 	- `PXR_FTM_LIPS		  = 1`: Audio-driven mode
	/// 	- `PXR_FTM_FACE_LIPS_VIS = 2`: Hybrid-Viseme mode
	/// 	- `PXR_FTM_FACE_LIPS_BS  = 3`: Hybrid-Blendshape mode
	/// - `TrackingStateCode`: The [EPXRTrackingStateCode](https://developer-cn.pico-interactive.com/en/document/unreal/face-tracking/#94bd97fa) enum
	/// </param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool GetFaceTrackingState(bool& IsTracking, FPXRFaceTrackingState& TrackingState);

	/// <summary>
	/// Starts face tracking.
	/// @note Supported by PICO 4 Pro and PICO 4 Enterprise.
	/// </summary>
	/// <param name="StartInfo">Passes the information for starting face tracking. Here you need to choose the face tracking mode. For details, see the above EPXRFaceTrackingMode enum.</param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool StartFaceTracking(const FPXRFaceTrackingStartInfo& StartInfo);

	/// <summary>
	/// Stops face tracking.
	/// @note Supported by PICO 4 Pro and PICO 4 Enterprise.
	/// </summary>
	/// <param name="StopInfo">Passes the information for stopping face tracking:
	/// `Pause`: Bool. `true` indicates pausing face tracking and `false` indicates directly stopping face tracking.
	/// If you need to switch the face-tracking mode, you can set `Pause` to true and then call `StartFaceTracking` to select another mode, instead of completely stop it.
	/// </param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool StopFaceTracking(const FPXRFaceTrackingStopInfo& StopInfo);

	/// <summary>
	/// Gets face tracking data.
	/// @note Supported by PICO 4 Pro and PICO 4 Enterprise.
	/// </summary>
	/// <param name="GetInfo">Specifies the face tracking data you want.
	/// - DisplayTime: Int. Set as the prediction time. Currently, this parameter is only supported to be set as 0.
	/// </param>
	/// <param name="OutFaceTrackingData">Returns the desired face tracking data:
	/// - `BlendShapeWeights`: See the [EPXRFaceBlendShape](https://developer-cn.pico-interactive.com/en/document/unreal/face-tracking/#ab7d460b) enum for details. 
	/// - `TimeStamp`: Int. The time at which the data was retrieved.
	/// - `LaughingProb`: The probability of "laughing," ranging from 0 to 1, where 0 represents no smile, and 1 represents a hearty laugh.
	/// - `EyeValid`: Indicates whether the data for the eye area is valid.
	/// - `FaceValid`: Indicates whether the data for the facial area is valid.
	/// </param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool GetFaceTrackingData(const FPXRFaceTrackingDataGetInfo& GetInfo, FPXRFaceTrackingData& OutFaceTrackingData);

	/// <summary>
	/// Gets whether the current device supports eye tracking.
	/// </summary>
	/// <param name="Supported">
	/// Returns a bool indicating whether eye tracking is supported:
	/// * `true`: Supported
	/// * `false`: Not supported
	/// </param>
	/// <param name="SupportedModes">
	/// Returns the eye tracking modes supported by the current device:
	/// `PXR_ETM_BOTH`: The device supports tracking both eyes, namely binocular-tracking mode
	/// `PXR_ETM_NONE`: The device does not support eye tracking
	/// </param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool GetEyeTrackingSupported(bool& Supported, TArray<EPXREyeTrackingMode>& SupportedModes);

	/// <summary>
	/// Gets the state of eye tracking.
	/// @note Supported by PICO Neo3 Pro Eye, PICO 4 Pro, and PICO 4 Enterprise.
	/// </summary>
	/// <param name="IsTracking">Returns a bool that indicates whether eye tracking is working:
	/// * `true`: eye tracking is working
	/// * `false`: eye tracking has been stopped
	/// </param>
	/// <param name="TrackingState">Returns the eye tracking state information, including:
	/// - `CurrentTrackingMode`: The EPXREyeTrackingMode enum
	/// - `TrackingStateCode`:  The EPXRTrackingStateCode enum
	/// </param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool GetEyeTrackingState(bool& IsTracking, FPXREyeTrackingState& TrackingState);

	/// <summary>
	/// Starts eye tracking.
	/// @note Supported by PICO Neo3 Pro Eye, PICO 4 Pro, and PICO 4 Enterprise.
	/// </summary>
	/// <param name="StartInfo">Passes the information for starting eye tracking.
	/// - `NeedCalibration`: Whether your app needs eye tracking calibration
	/// - `StartMode`: EPXREyeTrackingMode enum
	/// </param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool StartEyeTracking(const FPXREyeTrackingStartInfo& StartInfo);

	/// <summary>
	/// Stops eye tracking.
	/// @note Supported by PICO Neo3 Pro Eye, PICO 4 Pro, and PICO 4 Enterprise.
	/// </summary>
	/// <param name="StopInfo">Passes the information for stopping eye tracking. Currently, you do not need to pass anything.</param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool StopEyeTracking(const FPXREyeTrackingStopInfo& StopInfo);

	/// <summary>Gets eye tracking data.
    /// @note Supported by PICO Neo3 Pro Eye, PICO 4 Pro, and PICO 4 Enterprise. To call this API, you need to enable eye tracking first for your project.
	/// </summary>
	/// <param name="WorldToMetersScale"> Float. Specifies how many Unreal units correspond to one meter in the real world. 100 by default. </param>
	/// <param name="GetInfo">Specifies the eye tracking data you want:
	/// - DisplayTime: Int. Set as the prediction time. If DisplayTime is set to 0, the prediction time of the current frame will be used as DisplayTime.
	/// - QueryPosition: Bool. Whether to query the position data.
	/// - QueryOrientation: Bool. Whether to query the orientation data.
	/// </param>
	/// <param name="OutEyeTrackingData">Returns the desired eye tracking data. The structure containing the following details:
    /// <ul>
    /// <li>`EyeType`: EPICOEye type:
	///		- Left: The left eye
	///		- Right: The right eye
	///		- Center: The midpoint between eyes
	/// </li>
    /// <li>`Position`: FVector, the current position of eyes </li>
	/// <li>`Orientation`: FRotator, the current eye gaze direction </li>
	/// <li>`bIsPoseValid`: Bool, whether the current eye's position data is valid </li>
	/// <li>`Openness`: Float, the openness data of the left eye or right eye </li>
	/// <li>`bIsOpennessValid`: Bool, whether the current eye's openness data is valid </li>
    /// </ul>
	/// </param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking", meta = (WorldToMetersScale = "100.0"))
		static bool GetEyeTrackingData(float WorldToMetersScale, const FPXREyeTrackingDataGetInfo& GetInfo, FPXREyeTrackingData& OutEyeTrackingData);

	/// <summary>
	/// Gets the openness of the left and right eyes.
	/// @note Only supported by PICO 4 Enterprise. To use this API, you must add `<meta-data android:name="pvr.app.et_tob_advance" android:value="true"/>` to the AndroidManifest.xml file.
	/// </summary>
	/// <param name="LeftEyeOpenness">The openness of the left eye, which is a float value ranges from `0.0` to `1.0`. `0.0` indicates completely closed, `1.0` indicates completely open.</param>
	/// <param name="RightEyeOpenness">The openness of the right eye, which is a float value ranges from `0.0` to `1.0`. `0.0` indicates completely closed, `1.0` indicates completely open.</param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool GetEyeOpenness(float& LeftEyeOpenness, float& RightEyeOpenness);

	/// <summary>
	/// Gets the information about the pupils of both eyes.
	/// @note Only supported by PICO 4 Enterprise. To use this API, you must add `<meta-data android:name="pvr.app.et_tob_advance" android:value="true"/>` to the AndroidManifest.xml file.
	/// </summary>
	/// <param name="EyePupilInfo">Returns the diameters and positions of both pupils.
	/// - LeftEyePupilDiameter: Float. The left eye pupil diameter in pixels.
	///	- RightEyePupilDiameter: Float. The right eye pupil diameter in pixels.
	///	- LeftEyePupilPosition: FVector2D. The location of the left eye pupil on the captured image. The image's coordinate system originates at (0,0) in the top-left corner, with the positive X-axis extending to the right and the positive Y-axis extending downward, measured in pixels.
	///	- RightEyePupilPosition: FVector2D. The location of the right eye pupil on the captured image. The image's coordinate system originates at (0,0) in the top-left corner, with the positive X-axis extending to the right and the positive Y-axis extending downward, measured in pixels.
	/// </param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool GetEyePupilInfo(FPXREyePupilInfo& EyePupilInfo);
		
	/// <summary>
	/// Retrieves the per-eye pose information.
	/// @note Only supported by PICO 4 Enterprise. To use this API, you must add `<meta-data android:name="pvr.app.et_tob_advance" android:value="true"/>` to the AndroidManifest.xml file.
	/// </summary>
	/// <param name="Timestamp">Int. The time at which the data was retrieved.</param>
	/// <param name="LeftEyePose"> FTransform. Indicates the location and rotation of the left eye.</param>
	/// <param name="RightEyePose">FTransform. Indicates the location and rotation of the right eye.</param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
	static bool GetPerEyePose(int64& Timestamp, FTransform& LeftEyePose, FTransform& RightEyePose);

	/// <summary>
	/// Retrieves the eye blink information.
	/// @note Only supported by PICO 4 Enterprise. To use this API, you must add `<meta-data android:name="pvr.app.et_tob_advance" android:value="true"/>` to the AndroidManifest.xml file.
	/// </summary>
	/// <param name="Timestamp"> Int. The time at which the data was retrieved.</param>
	/// <param name="bLeftBlink">Bool. Indicates whether the left eye is blinked (true) or not (false).</param>
	/// <param name="bRightBlink">Bool. Indicates whether the right eye is blinked (true) or not (false).</param>
	/// <returns>Bool:
    /// <ul>
    /// <li>`true`: success</li>
    /// <li>`false`: failure</li>
    /// </ul>
    /// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
	static bool GetEyeBlink(int64& Timestamp, bool& bLeftBlink, bool& bRightBlink);

	//UFUNCTION(BlueprintCallable, Category = "PXR|BodyTracking")
	static bool PXR_GetBodyTrackingSupported(bool& Supported, TArray<EPXRBodyTrackingMode>& SupportedModes);

	//UFUNCTION(BlueprintCallable, Category = "PXR|BodyTracking")
	static bool PXR_GetBodyTrackingState(bool& IsTracking, FPXRBodyTrackingState& TrackingState);

	//UFUNCTION(BlueprintCallable, Category = "PXR|BodyTracking")
	static bool PXR_StartBodyTracking(const FPXRBodyTrackingStartInfo& StartInfo);

	//UFUNCTION(BlueprintCallable, Category = "PXR|BodyTracking")
	static bool PXR_StopBodyTracking(const FPXRBodyTrackingStopInfo& StopInfo);

	//UFUNCTION(BlueprintCallable, Category = "PXR|BodyTracking", meta = (WorldToMetersScale = "100.0"))
	static bool PXR_GetBodyTrackingData(float WorldToMetersScale, const FPXRBodyTrackingDataGetInfo& GetInfo, FPXRBodyTrackingData& BodyTrackingData);

	//UFUNCTION(BlueprintCallable, Category = "PXR|BodyTracking")
	static bool PXR_StartBodyTrackingCalibApp();

	//UFUNCTION(BlueprintCallable, Category = "PXR|MotionTracker")
	static bool PXR_GetMotionTrackerConnectState(FPXRMotionConnectState& connectState);

	//UFUNCTION(BlueprintCallable, Category = "PXR|MotionTracker")
	static bool PXR_GetMotionTrackerType(EPXRMotionTrackerType& trackerType);

	//UFUNCTION(BlueprintCallable, Category = "PXR|MotionTracker")
	static bool PXR_GetMotionTrackerMode(EPXRMotionTrackerMode& trackerMode);

	//UFUNCTION(BlueprintCallable, Category = "PXR|MotionTracker", meta = (WorldToMetersScale = "100.0"))
	static bool PXR_GetMotionTrackerLocations(float WorldToMetersScale ,const FString& trackerSN, FPXRMotionTrackerLocations& locations);
};
