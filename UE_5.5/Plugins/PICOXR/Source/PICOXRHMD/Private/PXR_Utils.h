// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Materials/MaterialInterface.h"
#include "PXR_Utils.generated.h"

UCLASS()
class UPICOContentResourceFinder : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
		UMaterial* StereoLayerDepthMat;
};

class FPICOXRUtils 
{
public:
	static FQuat ConvertXRQuatToUnrealQuat(FQuat InQuat);

	static FVector ConvertXRVectorToUnrealVector(FVector InVector, float Scale);

	static FQuat ConvertUnrealQuatToXRQuat(FQuat InQuat);

	static FVector ConvertUnrealVectorToXRVector(FVector InVector, float Scale);

	static FMatrix MakeProjection(float ZNear, float ZFar, float UpTan, float DownTan, float LeftTan, float RightTan, bool leftHanded);

	static FMatrix44f MakeUnprojectionMatrix(float UpTan, float DownTan, float LeftTan, float RightTan);
	
	static FMatrix44f MakeProjectionMatrix(float UpTan, float DownTan, float LeftTan, float RightTan);
};

inline FQuat FPICOXRUtils::ConvertXRQuatToUnrealQuat(FQuat InQuat)
{
	return FQuat(-InQuat.Z, InQuat.X, InQuat.Y, -InQuat.W);
}

inline FVector FPICOXRUtils::ConvertXRVectorToUnrealVector(FVector InVector, float Scale)
{
	return FVector(-InVector.Z * Scale, InVector.X * Scale, InVector.Y * Scale);
}

inline FQuat FPICOXRUtils::ConvertUnrealQuatToXRQuat(FQuat InQuat)
{
	return  FQuat{ InQuat.Y, InQuat.Z, -InQuat.X, -InQuat.W };
}

inline FVector FPICOXRUtils::ConvertUnrealVectorToXRVector(FVector InVector, float Scale)
{
	if (InVector.IsZero())
		return FVector{ 0.0f, 0.0f, 0.0f };

	return FVector{ InVector.Y / Scale, InVector.Z / Scale, -InVector.X / Scale };
}

inline FMatrix FPICOXRUtils::MakeProjection(float ZNear, float ZFar, float UpTan, float DownTan, float LeftTan, float RightTan, bool leftHanded)
{
	float handednessScale = leftHanded ? 1.0f : -1.0f;

	// A projection matrix is very like a scaling from NDC, so we can start with that.
	float projXScale = 2.0f / (LeftTan + RightTan);
	float projXOffset = (LeftTan - RightTan) * projXScale * 0.5f;
	float projYScale = 2.0f / (UpTan + DownTan);
	float projYOffset = (UpTan - DownTan) * projYScale * 0.5f;
	FMatrix projection;
	projection.M[0][0] = projXScale;
	projection.M[0][1] = 0.0f;
	projection.M[0][2] = handednessScale * projXOffset;
	projection.M[0][3] = 0.0f;
	projection.M[1][0] = 0.0f;
	projection.M[1][1] = projYScale;
	projection.M[1][2] = handednessScale * -projYOffset;
	projection.M[1][3] = 0.0f;
	projection.M[2][0] = 0.0f;
	projection.M[2][1] = 0.0f;
	if (FGenericPlatformMath::IsFinite(ZFar))
	{
		projection.M[2][2] = -handednessScale * ZFar / (ZNear - ZFar);
		projection.M[2][3] = (ZFar * ZNear) / (ZNear - ZFar);
	}
	else
	{
		projection.M[2][2] = handednessScale;
		projection.M[2][3] = -ZNear;
	}
	projection.M[3][0] = 0.0f;
	projection.M[3][1] = 0.0f;
	projection.M[3][2] = handednessScale;
	projection.M[3][3] = 0.0f;

	return projection.GetTransposed();
}

inline FMatrix44f FPICOXRUtils::MakeUnprojectionMatrix(float UpTan, float DownTan, float LeftTan, float RightTan)
{
	FMatrix44f Matrix = FMatrix44f::Identity;

	// Scale
	Matrix.M[0][0] = RightTan + LeftTan;
	Matrix.M[1][1] = UpTan + DownTan;

	// Offset
	Matrix.M[0][3] = -LeftTan;
	Matrix.M[1][3] = -DownTan;
	Matrix.M[2][3] = 1.0;

	return Matrix;
}

inline FMatrix44f FPICOXRUtils::MakeProjectionMatrix(float UpTan, float DownTan, float LeftTan, float RightTan)
{
	const float tanAngleWidth = RightTan + LeftTan;
	const float tanAngleHeight = UpTan + DownTan;

	FMatrix44f Matrix = FMatrix44f::Identity;

	// Scale
	Matrix.M[0][0] = 1.0f / tanAngleWidth;
	Matrix.M[1][1] = 1.0f / tanAngleHeight;

	// Offset
	Matrix.M[0][3] = LeftTan / tanAngleWidth;
	Matrix.M[1][3] = DownTan / tanAngleHeight;
	Matrix.M[2][3] = -1.0f;

	return Matrix;
}

class PICOXRHMD_API FPICOXRVersionHelper
{
public:
	static bool GetRuntimeAPIVersion(int32 &InCurrentSystemVersion);
	static bool IsThisVersionOrGreater(int32 CheckingVersion);

private:
	static int32 CurrentSystemVersion;
};