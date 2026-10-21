// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_OpenXRRenderModelPMC.h"

#include "PICO_HMDFunctionLibrary.h"
#include "PICO_HMDModule.h"

#include "Containers/StringConv.h"
#include "KismetProceduralMeshLibrary.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/Crc.h"
#include "Misc/ScopeLock.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	static constexpr bool bFlipWinding = false;
	static constexpr bool bInvertNormals = false;
	static constexpr bool bCoordXformOnly = true;
	static constexpr int32 TexCacheMaxMB = 32;
	static constexpr bool bEnableTexCache = true;
	static constexpr bool bEnableGlbCache = true;

	struct FTexCacheEntry
	{
		UTexture2D* Tex = nullptr;
		int64 Size = 0;
		double LastAccess = 0.0;
	};
	static FCriticalSection GTexCacheMutex;
	static TMap<uint64, FTexCacheEntry> GImageTexCache;
	static int64 GImageTexCacheBytes = 0;
	static int32 GActivePMCInstances = 0;
	static FCriticalSection GGlbCacheMutex;
	static TMap<int32, TArray<uint8>> GGlbCacheByHand;
	// TODO: Replace Hand-keyed GLB cache with cacheId-keyed cache (requires exposing cacheId from GetRenderModelGLBBytes) to support multiple controller variants/skins.
	// TODO: Add parsed glTF cache (bufferViews/accessors/materials/meshes) keyed by cacheId/CRC to skip JSON parsing and reduce per-spawn CPU cost.
	struct FGLTFBufferViewPICO
	{
		int32 BufferIndex = 0;
		int32 ByteOffset = 0;
		int32 ByteLength = 0;
		int32 ByteStride = 0;
	};

	struct FGLTFAccessorPICO
	{
		int32 BufferViewIndex = INDEX_NONE;
		int32 ByteOffset = 0;
		int32 ComponentType = 0;
		int32 Count = 0;
		FString Type;
		bool bNormalized = false;
	};

	struct FGLTFPrimitivePICO
	{
		int32 IndicesAccessor = INDEX_NONE;
		int32 PositionAccessor = INDEX_NONE;
		int32 NormalAccessor = INDEX_NONE;
		int32 UV0Accessor = INDEX_NONE;
		int32 UV1Accessor = INDEX_NONE;
		int32 MaterialIndex = INDEX_NONE;
	};

	struct FGLTFMeshPICO
	{
		TArray<FGLTFPrimitivePICO> Primitives;
	};

	struct FGLTFNodePICO
	{
		FString Name;
		int32 MeshIndex = INDEX_NONE;
		TArray<int32> Children;
		FTransform LocalTransform = FTransform::Identity;
	};

	struct FGLTFImagePICO
	{
		int32 BufferViewIndex = INDEX_NONE;
		FString MimeType;
	};

	struct FGLTFTexturePICO
	{
		int32 SourceImage = INDEX_NONE;
	};

	struct FGLTFMaterialPICO
	{
		int32 BaseColorTex = INDEX_NONE;
		FLinearColor BaseColorFactor = FLinearColor(1, 1, 1, 1);
		int32 NormalTex = INDEX_NONE;
		float NormalScale = 1.0f;
		int32 MetallicRoughnessTex = INDEX_NONE;
		float MetallicFactor = 1.0f;
		float RoughnessFactor = 1.0f;
		int32 OcclusionTex = INDEX_NONE;
		float OcclusionStrength = 1.0f;
		int32 EmissiveTex = INDEX_NONE;
		FLinearColor EmissiveFactor = FLinearColor(0, 0, 0, 0);
		int32 BaseColorTexCoord = 0;
		int32 NormalTexCoord = 0;
		int32 MetallicRoughnessTexCoord = 0;
		int32 EmissiveTexCoord = 0;
		int32 OcclusionTexCoord = 0;
		FVector2D BaseColorOffset = FVector2D(0, 0);
		FVector2D BaseColorScale = FVector2D(1, 1);
		FVector2D NormalOffset = FVector2D(0, 0);
		FVector2D NormalScale2D = FVector2D(1, 1);
		FVector2D MetallicRoughnessOffset = FVector2D(0, 0);
		FVector2D MetallicRoughnessScale = FVector2D(1, 1);
		FVector2D EmissiveOffset = FVector2D(0, 0);
		FVector2D EmissiveScale = FVector2D(1, 1);
		FVector2D OcclusionOffset = FVector2D(0, 0);
		FVector2D OcclusionScale = FVector2D(1, 1);
	};

	static bool ParseGLTFCore(const TSharedPtr<FJsonObject>& RootObj, TArray<FGLTFBufferViewPICO>& OutBufferViews, TArray<FGLTFAccessorPICO>& OutAccessors, TArray<FGLTFMeshPICO>& OutMeshes, TArray<FGLTFNodePICO>& OutNodes, TArray<int32>& OutSceneRootNodes, FString& OutError);
	static bool ParseGLTFMaterials(const TSharedPtr<FJsonObject>& RootObj, TArray<FGLTFImagePICO>& OutImages, TArray<FGLTFTexturePICO>& OutTextures, TArray<FGLTFMaterialPICO>& OutMaterials, FString& OutError);
	static void BuildNodeWorldTransforms(const TArray<FGLTFNodePICO>& Nodes, const TArray<int32>& RootNodes, TArray<FTransform>& OutWorldTransforms);
	static bool ReadPositions(const TArray<uint8>& Bin, const TArray<FGLTFBufferViewPICO>& BufferViews, const TArray<FGLTFAccessorPICO>& Accessors, int32 AccessorIndex, const FTransform& NodeTransform, TArray<FVector>& OutPositions, FString& OutError);
	static bool ReadNormals(const TArray<uint8>& Bin, const TArray<FGLTFBufferViewPICO>& BufferViews, const TArray<FGLTFAccessorPICO>& Accessors, int32 AccessorIndex, const FTransform& NodeTransform, TArray<FVector>& OutNormals, FString& OutError);
	static bool ReadUV0(const TArray<uint8>& Bin, const TArray<FGLTFBufferViewPICO>& BufferViews, const TArray<FGLTFAccessorPICO>& Accessors, int32 AccessorIndex, TArray<FVector2D>& OutUV0, FString& OutError);
	static bool ReadIndices(const TArray<uint8>& Bin, const TArray<FGLTFBufferViewPICO>& BufferViews, const TArray<FGLTFAccessorPICO>& Accessors, int32 AccessorIndex, TArray<int32>& OutTriangles, FString& OutError);
	static void ForceFlipWindingAndNormals(const TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals);
	static UTexture2D* CreateTexture2DFromImageBytes(const TArray<uint8>& InBytes, bool bSRGB);

	static bool ReadUInt32LE(const TArray<uint8>& Data, int64 Offset, uint32& OutValue)
	{
		if (Offset < 0 || Offset + 4 > Data.Num())
		{
			return false;
		}
		OutValue = (uint32)Data[Offset]
			| ((uint32)Data[Offset + 1] << 8)
			| ((uint32)Data[Offset + 2] << 16)
			| ((uint32)Data[Offset + 3] << 24);
		return true;
	}

	static bool LoadGLTFJsonAndBinFromGLBBytes(const TArray<uint8>& FileBytes, FString& OutJson, TArray<uint8>& OutBin, FString& OutError)
	{
		uint32 Magic = 0;
		uint32 Version = 0;
		uint32 TotalLength = 0;
		if (!ReadUInt32LE(FileBytes, 0, Magic) || !ReadUInt32LE(FileBytes, 4, Version) || !ReadUInt32LE(FileBytes, 8, TotalLength))
		{
			OutError = TEXT("Invalid GLB header");
			return false;
		}

		if (Magic != 0x46546C67 || Version != 2 || (int64)TotalLength != FileBytes.Num())
		{
			OutError = FString::Printf(TEXT("Unsupported GLB (magic=0x%08x version=%u length=%u file=%d)"), Magic, Version, TotalLength, FileBytes.Num());
			return false;
		}

		int64 Cursor = 12;
		bool bFoundJson = false;
		bool bFoundBin = false;
		while (Cursor + 8 <= FileBytes.Num())
		{
			uint32 ChunkLength = 0;
			uint32 ChunkType = 0;
			if (!ReadUInt32LE(FileBytes, Cursor, ChunkLength) || !ReadUInt32LE(FileBytes, Cursor + 4, ChunkType))
			{
				OutError = TEXT("Invalid GLB chunk header");
				return false;
			}
			Cursor += 8;
			if (Cursor + (int64)ChunkLength > FileBytes.Num())
			{
				OutError = TEXT("Invalid GLB chunk length");
				return false;
			}

			if (ChunkType == 0x4E4F534A)
			{
				const ANSICHAR* JsonPtr = reinterpret_cast<const ANSICHAR*>(FileBytes.GetData() + Cursor);
				FUTF8ToTCHAR Converted(JsonPtr, (int32)ChunkLength);
				OutJson = FString(Converted.Length(), Converted.Get());
				bFoundJson = true;
			}
			else if (ChunkType == 0x004E4942)
			{
				OutBin.Reset();
				OutBin.Append(FileBytes.GetData() + Cursor, ChunkLength);
				bFoundBin = true;
			}

			Cursor += ChunkLength;
		}

		if (!bFoundJson)
		{
			OutError = TEXT("GLB missing JSON chunk");
			return false;
		}

		if (!bFoundBin)
		{
			OutError = TEXT("GLB missing BIN chunk");
			return false;
		}

		return true;
	}

	static bool ParseGLTFCore(const TSharedPtr<FJsonObject>& RootObj, TArray<FGLTFBufferViewPICO>& OutBufferViews, TArray<FGLTFAccessorPICO>& OutAccessors, TArray<FGLTFMeshPICO>& OutMeshes, TArray<FGLTFNodePICO>& OutNodes, TArray<int32>& OutSceneRootNodes, FString& OutError)
	{
		OutBufferViews.Reset();
		OutAccessors.Reset();
		OutMeshes.Reset();
		OutNodes.Reset();
		OutSceneRootNodes.Reset();

		const TArray<TSharedPtr<FJsonValue>>* BufferViewsArr = nullptr;
		if (RootObj->TryGetArrayField(TEXT("bufferViews"), BufferViewsArr))
		{
			for (const TSharedPtr<FJsonValue>& V : *BufferViewsArr)
			{
				const TSharedPtr<FJsonObject> Obj = V->AsObject();
				if (!Obj.IsValid())
				{
					OutError = TEXT("Invalid bufferViews entry");
					return false;
				}

				FGLTFBufferViewPICO BV;
				BV.BufferIndex = Obj->HasField(TEXT("buffer")) ? (int32)Obj->GetNumberField(TEXT("buffer")) : 0;
				BV.ByteOffset = Obj->HasField(TEXT("byteOffset")) ? (int32)Obj->GetNumberField(TEXT("byteOffset")) : 0;
				BV.ByteLength = Obj->HasField(TEXT("byteLength")) ? (int32)Obj->GetNumberField(TEXT("byteLength")) : 0;
				BV.ByteStride = Obj->HasField(TEXT("byteStride")) ? (int32)Obj->GetNumberField(TEXT("byteStride")) : 0;
				OutBufferViews.Add(BV);
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* AccessorsArr = nullptr;
		if (!RootObj->TryGetArrayField(TEXT("accessors"), AccessorsArr))
		{
			OutError = TEXT("glTF missing accessors");
			return false;
		}

		for (const TSharedPtr<FJsonValue>& V : *AccessorsArr)
		{
			const TSharedPtr<FJsonObject> Obj = V->AsObject();
			if (!Obj.IsValid())
			{
				OutError = TEXT("Invalid accessor entry");
				return false;
			}

			FGLTFAccessorPICO A;
			A.BufferViewIndex = Obj->HasField(TEXT("bufferView")) ? (int32)Obj->GetNumberField(TEXT("bufferView")) : INDEX_NONE;
			A.ByteOffset = Obj->HasField(TEXT("byteOffset")) ? (int32)Obj->GetNumberField(TEXT("byteOffset")) : 0;
			A.ComponentType = Obj->HasField(TEXT("componentType")) ? (int32)Obj->GetNumberField(TEXT("componentType")) : 0;
			A.Count = Obj->HasField(TEXT("count")) ? (int32)Obj->GetNumberField(TEXT("count")) : 0;
			A.Type = Obj->HasField(TEXT("type")) ? Obj->GetStringField(TEXT("type")) : FString();
			A.bNormalized = Obj->HasField(TEXT("normalized")) ? Obj->GetBoolField(TEXT("normalized")) : false;
			OutAccessors.Add(A);
		}

		const TArray<TSharedPtr<FJsonValue>>* MeshesArr = nullptr;
		if (!RootObj->TryGetArrayField(TEXT("meshes"), MeshesArr))
		{
			OutError = TEXT("glTF missing meshes");
			return false;
		}

		for (const TSharedPtr<FJsonValue>& V : *MeshesArr)
		{
			const TSharedPtr<FJsonObject> MeshObj = V->AsObject();
			if (!MeshObj.IsValid())
			{
				OutError = TEXT("Invalid mesh entry");
				return false;
			}

			const TArray<TSharedPtr<FJsonValue>>* PrimsArr = nullptr;
			if (!MeshObj->TryGetArrayField(TEXT("primitives"), PrimsArr))
			{
				OutError = TEXT("Mesh missing primitives");
				return false;
			}

			FGLTFMeshPICO Mesh;
			for (const TSharedPtr<FJsonValue>& PV : *PrimsArr)
			{
				const TSharedPtr<FJsonObject> PrimObj = PV->AsObject();
				if (!PrimObj.IsValid())
				{
					OutError = TEXT("Invalid primitive entry");
					return false;
				}

				const int32 Mode = PrimObj->HasField(TEXT("mode")) ? (int32)PrimObj->GetNumberField(TEXT("mode")) : 4;
				if (Mode != 4)
				{
					continue;
				}

				FGLTFPrimitivePICO Prim;
				Prim.IndicesAccessor = PrimObj->HasField(TEXT("indices")) ? (int32)PrimObj->GetNumberField(TEXT("indices")) : INDEX_NONE;

				const TSharedPtr<FJsonObject>* AttrObjPtr = nullptr;
				if (!PrimObj->TryGetObjectField(TEXT("attributes"), AttrObjPtr) || AttrObjPtr == nullptr || !AttrObjPtr->IsValid())
				{
					OutError = TEXT("Primitive missing attributes");
					return false;
				}
				const TSharedPtr<FJsonObject> AttrObj = *AttrObjPtr;

				if (!AttrObj->HasField(TEXT("POSITION")))
				{
					OutError = TEXT("Primitive missing POSITION");
					return false;
				}

				Prim.PositionAccessor = (int32)AttrObj->GetNumberField(TEXT("POSITION"));
				Prim.NormalAccessor = AttrObj->HasField(TEXT("NORMAL")) ? (int32)AttrObj->GetNumberField(TEXT("NORMAL")) : INDEX_NONE;
				Prim.UV0Accessor = AttrObj->HasField(TEXT("TEXCOORD_0")) ? (int32)AttrObj->GetNumberField(TEXT("TEXCOORD_0")) : INDEX_NONE;
				Prim.UV1Accessor = AttrObj->HasField(TEXT("TEXCOORD_1")) ? (int32)AttrObj->GetNumberField(TEXT("TEXCOORD_1")) : INDEX_NONE;
				Prim.MaterialIndex = PrimObj->HasField(TEXT("material")) ? (int32)PrimObj->GetNumberField(TEXT("material")) : INDEX_NONE;

				Mesh.Primitives.Add(Prim);
			}
			OutMeshes.Add(Mesh);
		}

		const TArray<TSharedPtr<FJsonValue>>* NodesArr = nullptr;
		if (!RootObj->TryGetArrayField(TEXT("nodes"), NodesArr))
		{
			OutError = TEXT("glTF missing nodes");
			return false;
		}

		OutNodes.Reserve(NodesArr->Num());
		for (const TSharedPtr<FJsonValue>& V : *NodesArr)
		{
			const TSharedPtr<FJsonObject> NodeObj = V->AsObject();
			if (!NodeObj.IsValid())
			{
				OutError = TEXT("Invalid node entry");
				return false;
			}

			FGLTFNodePICO Node;
			NodeObj->TryGetStringField(TEXT("name"), Node.Name);
			Node.MeshIndex = NodeObj->HasField(TEXT("mesh")) ? (int32)NodeObj->GetNumberField(TEXT("mesh")) : INDEX_NONE;

			const TArray<TSharedPtr<FJsonValue>>* ChildrenArr = nullptr;
			if (NodeObj->TryGetArrayField(TEXT("children"), ChildrenArr))
			{
				for (const TSharedPtr<FJsonValue>& CV : *ChildrenArr)
				{
					Node.Children.Add((int32)CV->AsNumber());
				}
			}

			FVector Translation = FVector::ZeroVector;
			FVector Scale = FVector::OneVector;
			FQuat Rotation = FQuat::Identity;

			const TArray<TSharedPtr<FJsonValue>>* TArr = nullptr;
			if (NodeObj->TryGetArrayField(TEXT("translation"), TArr) && TArr->Num() == 3)
			{
				Translation = FVector((float)(*TArr)[0]->AsNumber(), (float)(*TArr)[1]->AsNumber(), (float)(*TArr)[2]->AsNumber());
			}

			const TArray<TSharedPtr<FJsonValue>>* SArr = nullptr;
			if (NodeObj->TryGetArrayField(TEXT("scale"), SArr) && SArr->Num() == 3)
			{
				Scale = FVector((float)(*SArr)[0]->AsNumber(), (float)(*SArr)[1]->AsNumber(), (float)(*SArr)[2]->AsNumber());
			}

			const TArray<TSharedPtr<FJsonValue>>* RArr = nullptr;
			if (NodeObj->TryGetArrayField(TEXT("rotation"), RArr) && RArr->Num() == 4)
			{
				Rotation = FQuat((float)(*RArr)[0]->AsNumber(), (float)(*RArr)[1]->AsNumber(), (float)(*RArr)[2]->AsNumber(), (float)(*RArr)[3]->AsNumber());
			}

			Node.LocalTransform = FTransform(Rotation, Translation, Scale);
			OutNodes.Add(Node);
		}

		const int32 SceneIndex = RootObj->HasField(TEXT("scene")) ? (int32)RootObj->GetNumberField(TEXT("scene")) : 0;
		const TArray<TSharedPtr<FJsonValue>>* ScenesArr = nullptr;
		if (!RootObj->TryGetArrayField(TEXT("scenes"), ScenesArr) || !ScenesArr->IsValidIndex(SceneIndex))
		{
			OutError = TEXT("glTF missing scenes");
			return false;
		}

		const TSharedPtr<FJsonObject> SceneObj = (*ScenesArr)[SceneIndex]->AsObject();
		if (!SceneObj.IsValid())
		{
			OutError = TEXT("Invalid scene");
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* SceneNodesArr = nullptr;
		if (!SceneObj->TryGetArrayField(TEXT("nodes"), SceneNodesArr))
		{
			OutError = TEXT("Scene missing nodes");
			return false;
		}

		for (const TSharedPtr<FJsonValue>& NV : *SceneNodesArr)
		{
			OutSceneRootNodes.Add((int32)NV->AsNumber());
		}

		return true;
	}

	static bool ParseGLTFMaterials(const TSharedPtr<FJsonObject>& RootObj, TArray<FGLTFImagePICO>& OutImages, TArray<FGLTFTexturePICO>& OutTextures, TArray<FGLTFMaterialPICO>& OutMaterials, FString& OutError)
	{
		OutImages.Reset();
		OutTextures.Reset();
		OutMaterials.Reset();

		const TArray<TSharedPtr<FJsonValue>>* ImagesArr = nullptr;
		if (RootObj->TryGetArrayField(TEXT("images"), ImagesArr))
		{
			for (const TSharedPtr<FJsonValue>& V : *ImagesArr)
			{
				const TSharedPtr<FJsonObject> Obj = V->AsObject();
				if (!Obj.IsValid()) continue;
				FGLTFImagePICO Img;
				Img.BufferViewIndex = Obj->HasField(TEXT("bufferView")) ? (int32)Obj->GetNumberField(TEXT("bufferView")) : INDEX_NONE;
				Obj->TryGetStringField(TEXT("mimeType"), Img.MimeType);
				OutImages.Add(Img);
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* TexturesArr = nullptr;
		if (RootObj->TryGetArrayField(TEXT("textures"), TexturesArr))
		{
			for (const TSharedPtr<FJsonValue>& V : *TexturesArr)
			{
				const TSharedPtr<FJsonObject> Obj = V->AsObject();
				if (!Obj.IsValid()) continue;
				FGLTFTexturePICO Tex;
				Tex.SourceImage = Obj->HasField(TEXT("source")) ? (int32)Obj->GetNumberField(TEXT("source")) : INDEX_NONE;
				OutTextures.Add(Tex);
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* MaterialsArr = nullptr;
		if (RootObj->TryGetArrayField(TEXT("materials"), MaterialsArr))
		{
			for (const TSharedPtr<FJsonValue>& V : *MaterialsArr)
			{
				const TSharedPtr<FJsonObject> Obj = V->AsObject();
				if (!Obj.IsValid()) continue;
				FGLTFMaterialPICO Mat;
				const TSharedPtr<FJsonObject>* PBRObjPtr = nullptr;
				if (Obj->TryGetObjectField(TEXT("pbrMetallicRoughness"), PBRObjPtr) && PBRObjPtr && PBRObjPtr->IsValid())
				{
					const TSharedPtr<FJsonObject> PBR = *PBRObjPtr;
					const TSharedPtr<FJsonObject>* BCTexPtr = nullptr;
					if (PBR->TryGetObjectField(TEXT("baseColorTexture"), BCTexPtr) && BCTexPtr && BCTexPtr->IsValid())
					{
						if ((*BCTexPtr)->HasField(TEXT("index"))) Mat.BaseColorTex = (int32)(*BCTexPtr)->GetNumberField(TEXT("index"));
						if ((*BCTexPtr)->HasField(TEXT("texCoord"))) Mat.BaseColorTexCoord = (int32)(*BCTexPtr)->GetNumberField(TEXT("texCoord"));
						const TSharedPtr<FJsonObject>* ExtObjPtr = nullptr;
						if ((*BCTexPtr)->TryGetObjectField(TEXT("extensions"), ExtObjPtr) && ExtObjPtr && ExtObjPtr->IsValid())
						{
							const TSharedPtr<FJsonObject>* XformPtr = nullptr;
							if ((*ExtObjPtr)->TryGetObjectField(TEXT("KHR_texture_transform"), XformPtr) && XformPtr && XformPtr->IsValid())
							{
								const TArray<TSharedPtr<FJsonValue>>* Off = nullptr;
								if ((*XformPtr)->TryGetArrayField(TEXT("offset"), Off) && Off->Num() == 2)
								{
									Mat.BaseColorOffset = FVector2D((float)(*Off)[0]->AsNumber(), (float)(*Off)[1]->AsNumber());
								}
								const TArray<TSharedPtr<FJsonValue>>* Scl = nullptr;
								if ((*XformPtr)->TryGetArrayField(TEXT("scale"), Scl) && Scl->Num() == 2)
								{
									Mat.BaseColorScale = FVector2D((float)(*Scl)[0]->AsNumber(), (float)(*Scl)[1]->AsNumber());
								}
								if ((*XformPtr)->HasField(TEXT("texCoord")))
								{
									Mat.BaseColorTexCoord = (int32)(*XformPtr)->GetNumberField(TEXT("texCoord"));
								}
							}
						}
					}
					const TArray<TSharedPtr<FJsonValue>>* BCF = nullptr;
					if (PBR->TryGetArrayField(TEXT("baseColorFactor"), BCF) && BCF->Num() == 4)
					{
						Mat.BaseColorFactor = FLinearColor(
							(float)(*BCF)[0]->AsNumber(),
							(float)(*BCF)[1]->AsNumber(),
							(float)(*BCF)[2]->AsNumber(),
							(float)(*BCF)[3]->AsNumber());
					}
					const TSharedPtr<FJsonObject>* MRTexPtr = nullptr;
					if (PBR->TryGetObjectField(TEXT("metallicRoughnessTexture"), MRTexPtr) && MRTexPtr && MRTexPtr->IsValid())
					{
						if ((*MRTexPtr)->HasField(TEXT("index"))) Mat.MetallicRoughnessTex = (int32)(*MRTexPtr)->GetNumberField(TEXT("index"));
						if ((*MRTexPtr)->HasField(TEXT("texCoord"))) Mat.MetallicRoughnessTexCoord = (int32)(*MRTexPtr)->GetNumberField(TEXT("texCoord"));
						const TSharedPtr<FJsonObject>* ExtObjPtr = nullptr;
						if ((*MRTexPtr)->TryGetObjectField(TEXT("extensions"), ExtObjPtr) && ExtObjPtr && ExtObjPtr->IsValid())
						{
							const TSharedPtr<FJsonObject>* XformPtr = nullptr;
							if ((*ExtObjPtr)->TryGetObjectField(TEXT("KHR_texture_transform"), XformPtr) && XformPtr && XformPtr->IsValid())
							{
								const TArray<TSharedPtr<FJsonValue>>* Off = nullptr;
								if ((*XformPtr)->TryGetArrayField(TEXT("offset"), Off) && Off->Num() == 2)
								{
									Mat.MetallicRoughnessOffset = FVector2D((float)(*Off)[0]->AsNumber(), (float)(*Off)[1]->AsNumber());
								}
								const TArray<TSharedPtr<FJsonValue>>* Scl = nullptr;
								if ((*XformPtr)->TryGetArrayField(TEXT("scale"), Scl) && Scl->Num() == 2)
								{
									Mat.MetallicRoughnessScale = FVector2D((float)(*Scl)[0]->AsNumber(), (float)(*Scl)[1]->AsNumber());
								}
								if ((*XformPtr)->HasField(TEXT("texCoord")))
								{
									Mat.MetallicRoughnessTexCoord = (int32)(*XformPtr)->GetNumberField(TEXT("texCoord"));
								}
							}
						}
					}
					if (PBR->HasField(TEXT("metallicFactor"))) Mat.MetallicFactor = (float)PBR->GetNumberField(TEXT("metallicFactor"));
					if (PBR->HasField(TEXT("roughnessFactor"))) Mat.RoughnessFactor = (float)PBR->GetNumberField(TEXT("roughnessFactor"));
				}
				const TSharedPtr<FJsonObject>* NormPtr = nullptr;
				if (Obj->TryGetObjectField(TEXT("normalTexture"), NormPtr) && NormPtr && NormPtr->IsValid())
				{
					if ((*NormPtr)->HasField(TEXT("index"))) Mat.NormalTex = (int32)(*NormPtr)->GetNumberField(TEXT("index"));
					if ((*NormPtr)->HasField(TEXT("scale"))) Mat.NormalScale = (float)(*NormPtr)->GetNumberField(TEXT("scale"));
					if ((*NormPtr)->HasField(TEXT("texCoord"))) Mat.NormalTexCoord = (int32)(*NormPtr)->GetNumberField(TEXT("texCoord"));
					const TSharedPtr<FJsonObject>* ExtObjPtr = nullptr;
					if ((*NormPtr)->TryGetObjectField(TEXT("extensions"), ExtObjPtr) && ExtObjPtr && ExtObjPtr->IsValid())
					{
						const TSharedPtr<FJsonObject>* XformPtr = nullptr;
						if ((*ExtObjPtr)->TryGetObjectField(TEXT("KHR_texture_transform"), XformPtr) && XformPtr && XformPtr->IsValid())
						{
							const TArray<TSharedPtr<FJsonValue>>* Off = nullptr;
							if ((*XformPtr)->TryGetArrayField(TEXT("offset"), Off) && Off->Num() == 2)
							{
								Mat.NormalOffset = FVector2D((float)(*Off)[0]->AsNumber(), (float)(*Off)[1]->AsNumber());
							}
							const TArray<TSharedPtr<FJsonValue>>* Scl = nullptr;
							if ((*XformPtr)->TryGetArrayField(TEXT("scale"), Scl) && Scl->Num() == 2)
							{
								Mat.NormalScale2D = FVector2D((float)(*Scl)[0]->AsNumber(), (float)(*Scl)[1]->AsNumber());
							}
							if ((*XformPtr)->HasField(TEXT("texCoord")))
							{
								Mat.NormalTexCoord = (int32)(*XformPtr)->GetNumberField(TEXT("texCoord"));
							}
						}
					}
				}
				const TSharedPtr<FJsonObject>* OcclPtr = nullptr;
				if (Obj->TryGetObjectField(TEXT("occlusionTexture"), OcclPtr) && OcclPtr && OcclPtr->IsValid())
				{
					if ((*OcclPtr)->HasField(TEXT("index"))) Mat.OcclusionTex = (int32)(*OcclPtr)->GetNumberField(TEXT("index"));
					if ((*OcclPtr)->HasField(TEXT("strength"))) Mat.OcclusionStrength = (float)(*OcclPtr)->GetNumberField(TEXT("strength"));
					if ((*OcclPtr)->HasField(TEXT("texCoord"))) Mat.OcclusionTexCoord = (int32)(*OcclPtr)->GetNumberField(TEXT("texCoord"));
					const TSharedPtr<FJsonObject>* ExtObjPtr = nullptr;
					if ((*OcclPtr)->TryGetObjectField(TEXT("extensions"), ExtObjPtr) && ExtObjPtr && ExtObjPtr->IsValid())
					{
						const TSharedPtr<FJsonObject>* XformPtr = nullptr;
						if ((*ExtObjPtr)->TryGetObjectField(TEXT("KHR_texture_transform"), XformPtr) && XformPtr && XformPtr->IsValid())
						{
							const TArray<TSharedPtr<FJsonValue>>* Off = nullptr;
							if ((*XformPtr)->TryGetArrayField(TEXT("offset"), Off) && Off->Num() == 2)
							{
								Mat.OcclusionOffset = FVector2D((float)(*Off)[0]->AsNumber(), (float)(*Off)[1]->AsNumber());
							}
							const TArray<TSharedPtr<FJsonValue>>* Scl = nullptr;
							if ((*XformPtr)->TryGetArrayField(TEXT("scale"), Scl) && Scl->Num() == 2)
							{
								Mat.OcclusionScale = FVector2D((float)(*Scl)[0]->AsNumber(), (float)(*Scl)[1]->AsNumber());
							}
							if ((*XformPtr)->HasField(TEXT("texCoord")))
							{
								Mat.OcclusionTexCoord = (int32)(*XformPtr)->GetNumberField(TEXT("texCoord"));
							}
						}
					}
				}
				const TSharedPtr<FJsonObject>* EmisPtr = nullptr;
				if (Obj->TryGetObjectField(TEXT("emissiveTexture"), EmisPtr) && EmisPtr && EmisPtr->IsValid())
				{
					if ((*EmisPtr)->HasField(TEXT("index"))) Mat.EmissiveTex = (int32)(*EmisPtr)->GetNumberField(TEXT("index"));
					if ((*EmisPtr)->HasField(TEXT("texCoord"))) Mat.EmissiveTexCoord = (int32)(*EmisPtr)->GetNumberField(TEXT("texCoord"));
					const TSharedPtr<FJsonObject>* ExtObjPtr = nullptr;
					if ((*EmisPtr)->TryGetObjectField(TEXT("extensions"), ExtObjPtr) && ExtObjPtr && ExtObjPtr->IsValid())
					{
						const TSharedPtr<FJsonObject>* XformPtr = nullptr;
						if ((*ExtObjPtr)->TryGetObjectField(TEXT("KHR_texture_transform"), XformPtr) && XformPtr && XformPtr->IsValid())
						{
							const TArray<TSharedPtr<FJsonValue>>* Off = nullptr;
							if ((*XformPtr)->TryGetArrayField(TEXT("offset"), Off) && Off->Num() == 2)
							{
								Mat.EmissiveOffset = FVector2D((float)(*Off)[0]->AsNumber(), (float)(*Off)[1]->AsNumber());
							}
							const TArray<TSharedPtr<FJsonValue>>* Scl = nullptr;
							if ((*XformPtr)->TryGetArrayField(TEXT("scale"), Scl) && Scl->Num() == 2)
							{
								Mat.EmissiveScale = FVector2D((float)(*Scl)[0]->AsNumber(), (float)(*Scl)[1]->AsNumber());
							}
							if ((*XformPtr)->HasField(TEXT("texCoord")))
							{
								Mat.EmissiveTexCoord = (int32)(*XformPtr)->GetNumberField(TEXT("texCoord"));
							}
						}
					}
				}
				const TArray<TSharedPtr<FJsonValue>>* EmisF = nullptr;
				if (Obj->TryGetArrayField(TEXT("emissiveFactor"), EmisF) && EmisF->Num() == 3)
				{
					Mat.EmissiveFactor = FLinearColor(
						(float)(*EmisF)[0]->AsNumber(),
						(float)(*EmisF)[1]->AsNumber(),
						(float)(*EmisF)[2]->AsNumber(), 0.0f);
				}
				OutMaterials.Add(Mat);
			}
		}
		return true;
	}

	static int32 GetComponentSizeForComponentType(int32 ComponentType)
	{
		switch (ComponentType)
		{
		case 5120: return 1;
		case 5121: return 1;
		case 5122: return 2;
		case 5123: return 2;
		case 5125: return 4;
		case 5126: return 4;
		default: return 0;
		}
	}

	static bool ReadAccessorBytes(const TArray<uint8>& Bin, const TArray<FGLTFBufferViewPICO>& BufferViews, const FGLTFAccessorPICO& Accessor, int32 ElementSize, const uint8*& OutData, int32& OutStride, FString& OutError)
	{
		if (Accessor.BufferViewIndex == INDEX_NONE || !BufferViews.IsValidIndex(Accessor.BufferViewIndex))
		{
			OutError = TEXT("Accessor missing/invalid bufferView");
			return false;
		}

		if (Accessor.Count <= 0 || ElementSize <= 0)
		{
			OutError = TEXT("Accessor invalid count/elementSize");
			return false;
		}

		const FGLTFBufferViewPICO& BV = BufferViews[Accessor.BufferViewIndex];
		const int32 Offset = BV.ByteOffset + Accessor.ByteOffset;
		const int32 Stride = BV.ByteStride > 0 ? BV.ByteStride : ElementSize;
		if (Stride < ElementSize)
		{
			OutError = TEXT("Accessor stride smaller than element size");
			return false;
		}

		const int64 Required = (int64)Offset + (int64)Stride * (int64)(Accessor.Count - 1) + (int64)ElementSize;
		if (Offset < 0 || Required > Bin.Num())
		{
			OutError = TEXT("Accessor out of bounds");
			return false;
		}

		OutData = Bin.GetData() + Offset;
		OutStride = Stride;
		return true;
	}

	static FVector ConvertPositionGltfToUnreal(const FVector& P)
	{
		return FVector(-P.Z, P.X, P.Y) * 100.0f;
	}

	static FVector ConvertVectorGltfToUnreal(const FVector& V)
	{
		return FVector(-V.Z, V.X, V.Y);
	}

	static bool ReadPositions(const TArray<uint8>& Bin, const TArray<FGLTFBufferViewPICO>& BufferViews, const TArray<FGLTFAccessorPICO>& Accessors, int32 AccessorIndex, const FTransform& NodeTransform, TArray<FVector>& OutPositions, FString& OutError)
	{
		if (!Accessors.IsValidIndex(AccessorIndex))
		{
			OutError = TEXT("Invalid POSITION accessor");
			return false;
		}
		const FGLTFAccessorPICO& A = Accessors[AccessorIndex];
		if (A.ComponentType != 5126 || A.Type != TEXT("VEC3"))
		{
			OutError = TEXT("Unsupported POSITION accessor format");
			return false;
		}

		const uint8* Data = nullptr;
		int32 Stride = 0;
		if (!ReadAccessorBytes(Bin, BufferViews, A, 12, Data, Stride, OutError))
		{
			return false;
		}

		OutPositions.Reset();
		OutPositions.Reserve(A.Count);
		for (int32 i = 0; i < A.Count; i++)
		{
			const float* F = reinterpret_cast<const float*>(Data + (int64)Stride * i);
			const FVector GltfP(F[0], F[1], F[2]);
			const FVector GltfWorld = NodeTransform.TransformPosition(GltfP);
			OutPositions.Add(ConvertPositionGltfToUnreal(GltfWorld));
		}
		return true;
	}

	static bool ReadNormals(const TArray<uint8>& Bin, const TArray<FGLTFBufferViewPICO>& BufferViews, const TArray<FGLTFAccessorPICO>& Accessors, int32 AccessorIndex, const FTransform& NodeTransform, TArray<FVector>& OutNormals, FString& OutError)
	{
		if (AccessorIndex == INDEX_NONE)
		{
			OutNormals.Reset();
			return true;
		}
		if (!Accessors.IsValidIndex(AccessorIndex))
		{
			OutError = TEXT("Invalid NORMAL accessor");
			return false;
		}
		const FGLTFAccessorPICO& A = Accessors[AccessorIndex];
		if (A.ComponentType != 5126 || A.Type != TEXT("VEC3"))
		{
			OutError = TEXT("Unsupported NORMAL accessor format");
			return false;
		}

		const uint8* Data = nullptr;
		int32 Stride = 0;
		if (!ReadAccessorBytes(Bin, BufferViews, A, 12, Data, Stride, OutError))
		{
			return false;
		}

		OutNormals.Reset();
		OutNormals.Reserve(A.Count);
		for (int32 i = 0; i < A.Count; i++)
		{
			const float* F = reinterpret_cast<const float*>(Data + (int64)Stride * i);
			const FVector GltfN(F[0], F[1], F[2]);
			const FVector GltfWorldN = NodeTransform.TransformVector(GltfN).GetSafeNormal();
			OutNormals.Add(ConvertVectorGltfToUnreal(GltfWorldN).GetSafeNormal());
		}
		return true;
	}

	static bool ReadUV0(const TArray<uint8>& Bin, const TArray<FGLTFBufferViewPICO>& BufferViews, const TArray<FGLTFAccessorPICO>& Accessors, int32 AccessorIndex, TArray<FVector2D>& OutUV0, FString& OutError)
	{
		if (AccessorIndex == INDEX_NONE)
		{
			OutUV0.Reset();
			return true;
		}
		if (!Accessors.IsValidIndex(AccessorIndex))
		{
			OutError = TEXT("Invalid TEXCOORD_0 accessor");
			return false;
		}
		const FGLTFAccessorPICO& A = Accessors[AccessorIndex];
		if (A.ComponentType != 5126 || A.Type != TEXT("VEC2"))
		{
			OutError = TEXT("Unsupported TEXCOORD_0 accessor format");
			return false;
		}

		const uint8* Data = nullptr;
		int32 Stride = 0;
		if (!ReadAccessorBytes(Bin, BufferViews, A, 8, Data, Stride, OutError))
		{
			return false;
		}

		OutUV0.Reset();
		OutUV0.Reserve(A.Count);
		for (int32 i = 0; i < A.Count; i++)
		{
			const float* F = reinterpret_cast<const float*>(Data + (int64)Stride * i);
			OutUV0.Add(FVector2D(F[0], F[1]));
		}
		return true;
	}

	static bool ReadIndices(const TArray<uint8>& Bin, const TArray<FGLTFBufferViewPICO>& BufferViews, const TArray<FGLTFAccessorPICO>& Accessors, int32 AccessorIndex, TArray<int32>& OutTriangles, FString& OutError)
	{
		if (AccessorIndex == INDEX_NONE)
		{
			OutError = TEXT("Primitive missing indices");
			return false;
		}
		if (!Accessors.IsValidIndex(AccessorIndex))
		{
			OutError = TEXT("Invalid indices accessor");
			return false;
		}

		const FGLTFAccessorPICO& A = Accessors[AccessorIndex];
		if (A.Type != TEXT("SCALAR"))
		{
			OutError = TEXT("Unsupported indices accessor type");
			return false;
		}

		const int32 ComponentSize = GetComponentSizeForComponentType(A.ComponentType);
		if (ComponentSize == 0)
		{
			OutError = TEXT("Unsupported indices componentType");
			return false;
		}

		const uint8* Data = nullptr;
		int32 Stride = 0;
		if (!ReadAccessorBytes(Bin, BufferViews, A, ComponentSize, Data, Stride, OutError))
		{
			return false;
		}

		if (A.Count % 3 != 0)
		{
			OutError = TEXT("Indices count is not multiple of 3");
			return false;
		}

		OutTriangles.Reset();
		OutTriangles.Reserve(A.Count);
		for (int32 i = 0; i < A.Count; i++)
		{
			const uint8* Ptr = Data + (int64)Stride * i;
			uint32 Index = 0;
			if (A.ComponentType == 5121)
			{
				Index = (uint32)(*Ptr);
			}
			else if (A.ComponentType == 5123)
			{
				Index = (uint32)(Ptr[0] | (Ptr[1] << 8));
			}
			else if (A.ComponentType == 5125)
			{
				Index = (uint32)(Ptr[0] | (Ptr[1] << 8) | (Ptr[2] << 16) | (Ptr[3] << 24));
			}
			else
			{
				OutError = TEXT("Unsupported indices componentType");
				return false;
			}
			OutTriangles.Add((int32)Index);
		}

		return true;
	}

	static void BuildNodeWorldTransforms(const TArray<FGLTFNodePICO>& Nodes, const TArray<int32>& RootNodes, TArray<FTransform>& OutWorldTransforms)
	{
		OutWorldTransforms.SetNum(Nodes.Num());

		TArray<bool> bVisited;
		bVisited.SetNumZeroed(Nodes.Num());

		TFunction<void(int32, const FTransform&)> Visit = [&](int32 NodeIndex, const FTransform& ParentWorld)
		{
			if (!Nodes.IsValidIndex(NodeIndex))
			{
				return;
			}
			if (bVisited[NodeIndex])
			{
				return;
			}
			bVisited[NodeIndex] = true;

			const FTransform World = Nodes[NodeIndex].LocalTransform * ParentWorld;
			OutWorldTransforms[NodeIndex] = World;

			for (int32 Child : Nodes[NodeIndex].Children)
			{
				Visit(Child, World);
			}
		};

		for (int32 Root : RootNodes)
		{
			Visit(Root, FTransform::Identity);
		}

		for (int32 i = 0; i < Nodes.Num(); i++)
		{
			if (!bVisited[i])
			{
				OutWorldTransforms[i] = Nodes[i].LocalTransform;
			}
		}
	}

	static void ForceFlipWindingAndNormals(const TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals)
	{
		if (!bFlipWinding)
		{
			return;
		}
		if (Vertices.Num() == 0 || Triangles.Num() < 3)
		{
			return;
		}
		for (int32 t = 0; t + 2 < Triangles.Num(); t += 3)
		{
			Swap(Triangles[t + 1], Triangles[t + 2]);
		}
	}

	static UTexture2D* CreateTexture2DFromImageBytes(const TArray<uint8>& InBytes, bool bSRGB)
	{
		IImageWrapperModule& ImgWrap = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		EImageFormat Fmt = ImgWrap.DetectImageFormat(InBytes.GetData(), InBytes.Num());
		if (Fmt == EImageFormat::Invalid) return nullptr;
		TSharedPtr<IImageWrapper> Wrapper = ImgWrap.CreateImageWrapper(Fmt);
		if (!Wrapper.IsValid() || !Wrapper->SetCompressed(InBytes.GetData(), InBytes.Num())) return nullptr;
		TArray<uint8> Raw;
		if (!Wrapper->GetRaw(ERGBFormat::RGBA, 8, Raw)) return nullptr;
		const int32 W = Wrapper->GetWidth();
		const int32 H = Wrapper->GetHeight();
		if (W <= 0 || H <= 0) return nullptr;
		UTexture2D* Tex = UTexture2D::CreateTransient(W, H, PF_B8G8R8A8);
		if (!Tex) return nullptr;
		Tex->SRGB = bSRGB;
		void* MipData = Tex->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		const uint8* Src = Raw.GetData();
		uint8* Dst = static_cast<uint8*>(MipData);
		const int32 PixelCount = W * H;
		for (int i = 0; i < PixelCount; i++)
		{
			Dst[4 * i + 0] = Src[4 * i + 2];
			Dst[4 * i + 1] = Src[4 * i + 1];
			Dst[4 * i + 2] = Src[4 * i + 0];
			Dst[4 * i + 3] = Src[4 * i + 3];
		}
		Tex->GetPlatformData()->Mips[0].BulkData.Unlock();
		Tex->UpdateResource();
		return Tex;
	}

	static UTexture2D* GetOrCreateTextureFromImageBytes(const TArray<uint8>& ImgBytes, bool bSRGB)
	{
		if (bEnableTexCache)
		{
			const uint32 Crc = FCrc::MemCrc32(ImgBytes.GetData(), ImgBytes.Num());
			const uint64 Key = (uint64(ImgBytes.Num()) << 32) | uint64(Crc);
			{
				FScopeLock L(&GTexCacheMutex);
				if (FTexCacheEntry* Found = GImageTexCache.Find(Key))
				{
					if (Found->Tex && IsValid(Found->Tex))
					{
						Found->LastAccess = FPlatformTime::Seconds();
						return Found->Tex;
					}
				}
			}
			if (UTexture2D* NewTex = CreateTexture2DFromImageBytes(ImgBytes, bSRGB))
			{
				const int32 MaxMB = FMath::Max(1, TexCacheMaxMB);
				const int64 MaxBytes = int64(MaxMB) * 1024 * 1024;
				FScopeLock L(&GTexCacheMutex);
				const int64 Size = ImgBytes.Num();
				while (GImageTexCache.Num() > 0 && (GImageTexCacheBytes + Size > MaxBytes))
				{
					double Oldest = 1e300;
					uint64 OldKey = 0;
					for (auto& Pair : GImageTexCache)
					{
						if (Pair.Value.LastAccess < Oldest)
						{
							Oldest = Pair.Value.LastAccess;
							OldKey = Pair.Key;
						}
					}
					if (OldKey != 0)
					{
						GImageTexCacheBytes -= GImageTexCache[OldKey].Size;
						GImageTexCache.Remove(OldKey);
					}
					else
					{
						break;
					}
				}
				FTexCacheEntry Entry;
				Entry.Tex = NewTex;
				Entry.Size = ImgBytes.Num();
				Entry.LastAccess = FPlatformTime::Seconds();
				GImageTexCache.Add(Key, Entry);
				GImageTexCacheBytes += Entry.Size;
				return NewTex;
			}
			return nullptr;
		}
		return CreateTexture2DFromImageBytes(ImgBytes, bSRGB);
	}
}

bool UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModelPICO()
{
	return LoadFromOpenXRRenderModel(Hand);
}

bool UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel(EControllerHand InHand)
{
	TArray<uint8> GlbBytes;
	FString Error;
	bool bHaveGlb = false;
	if (bEnableGlbCache)
	{
		FScopeLock L(&GGlbCacheMutex);
		if (const TArray<uint8>* Found = GGlbCacheByHand.Find((int32)InHand))
		{
			GlbBytes = *Found;
			bHaveGlb = true;
		}
	}
	if (!bHaveGlb)
	{
		if (!FPICOOpenXRHMDModule::Get().GetXRPlugin().GetRenderModelGLBBytes(InHand, GlbBytes, Error))
		{
			UE_LOG(LogPICOOpenXRHMD, Verbose, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: %s"), *Error);
			return false;
		}
		if (bEnableGlbCache)
		{
			FScopeLock L(&GGlbCacheMutex);
			GGlbCacheByHand.Add((int32)InHand, GlbBytes);
		}
	}

	FString JsonText;
	TArray<uint8> Bin;
	if (!LoadGLTFJsonAndBinFromGLBBytes(GlbBytes, JsonText, Bin, Error))
	{
		UE_LOG(LogPICOOpenXRHMD, Verbose, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: %s"), *Error);
		return false;
	}

	TSharedPtr<FJsonObject> RootObj;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: Failed to parse glTF JSON"));
		return false;
	}

	TArray<FGLTFBufferViewPICO> BufferViews;
	TArray<FGLTFAccessorPICO> Accessors;
	TArray<FGLTFMeshPICO> Meshes;
	TArray<FGLTFNodePICO> Nodes;
	TArray<int32> SceneRootNodes;
	if (!ParseGLTFCore(RootObj, BufferViews, Accessors, Meshes, Nodes, SceneRootNodes, Error))
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: %s"), *Error);
		return false;
	}

	TArray<FGLTFImagePICO> Images;
	TArray<FGLTFTexturePICO> Textures;
	TArray<FGLTFMaterialPICO> Materials;
	if (!ParseGLTFMaterials(RootObj, Images, Textures, Materials, Error))
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: %s"), *Error);
		return false;
	}

	TMap<int32, UTexture2D*> ImageIndexToTexture;

	TArray<FTransform> NodeWorldTransforms;
	BuildNodeWorldTransforms(Nodes, SceneRootNodes, NodeWorldTransforms);

	ClearAllMeshSections();

	int32 SectionIndex = 0;
	int32 TotalVertices = 0;
	int32 TotalTriangles = 0;
	for (int32 NodeIndex = 0; NodeIndex < Nodes.Num(); NodeIndex++)
	{
		const int32 MeshIndex = Nodes[NodeIndex].MeshIndex;
		if (MeshIndex == INDEX_NONE || !Meshes.IsValidIndex(MeshIndex))
		{
			continue;
		}

		const FTransform& NodeXf = NodeWorldTransforms.IsValidIndex(NodeIndex) ? NodeWorldTransforms[NodeIndex] : FTransform::Identity;
		const FGLTFMeshPICO& Mesh = Meshes[MeshIndex];
		for (const FGLTFPrimitivePICO& Prim : Mesh.Primitives)
		{
			TArray<FVector> Vertices;
			TArray<int32> Triangles;
			TArray<FVector> Normals;
			TArray<FVector2D> UV0;
			TArray<FProcMeshTangent> Tangents;
			TArray<FLinearColor> Colors;

			if (!ReadPositions(Bin, BufferViews, Accessors, Prim.PositionAccessor, NodeXf, Vertices, Error))
			{
				UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: %s"), *Error);
				continue;
			}
			if (!ReadNormals(Bin, BufferViews, Accessors, Prim.NormalAccessor, NodeXf, Normals, Error))
			{
				UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: %s"), *Error);
				continue;
			}
			int32 ChosenUVAccessor = Prim.UV0Accessor;
			const FGLTFMaterialPICO* MatForUV = Materials.IsValidIndex(Prim.MaterialIndex) ? &Materials[Prim.MaterialIndex] : nullptr;
			if (MatForUV && MatForUV->BaseColorTexCoord == 1 && Prim.UV1Accessor != INDEX_NONE)
			{
				ChosenUVAccessor = Prim.UV1Accessor;
			}
			if (!ReadUV0(Bin, BufferViews, Accessors, ChosenUVAccessor, UV0, Error))
			{
				UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: %s"), *Error);
				continue;
			}
			if (!ReadIndices(Bin, BufferViews, Accessors, Prim.IndicesAccessor, Triangles, Error))
			{
				UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: %s"), *Error);
				continue;
			}

			if (UV0.Num() != Vertices.Num()) UV0.SetNumZeroed(Vertices.Num());
			if (!bCoordXformOnly)
			{
				ForceFlipWindingAndNormals(Vertices, Triangles, Normals);
				Normals.Reset();
				Tangents.Reset();
				if (Vertices.Num() > 0 && Triangles.Num() > 0)
				{
					UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UV0, Normals, Tangents);
					if (bInvertNormals)
					{
						for (FVector& N : Normals) { N *= -1.0f; }
						for (FProcMeshTangent& T : Tangents) { T.TangentX *= -1.0f; }
					}
				}
			}
			else
			{
				if (Normals.Num() != 0 && Normals.Num() != Vertices.Num())
				{
					Normals.Reset();
				}
				Tangents.Reset();
			}

			CreateMeshSection_LinearColor(SectionIndex, Vertices, Triangles, Normals, UV0, Colors, Tangents, false);
			UMaterialInterface* BaseMat = OverrideMaterial;
			if (BaseMat)
			{
				UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(BaseMat, this);
				const FGLTFMaterialPICO* Mat = Materials.IsValidIndex(Prim.MaterialIndex) ? &Materials[Prim.MaterialIndex] : nullptr;
				if (Mat)
				{
					MID->SetVectorParameterValue(TEXT("BaseColorFactor"), Mat->BaseColorFactor);
					MID->SetScalarParameterValue(TEXT("MetallicFactor"), Mat->MetallicFactor);
					MID->SetScalarParameterValue(TEXT("RoughnessFactor"), Mat->RoughnessFactor);
					MID->SetScalarParameterValue(TEXT("NormalScale"), Mat->NormalScale);
					MID->SetVectorParameterValue(TEXT("EmissiveFactor"), Mat->EmissiveFactor);
					MID->SetVectorParameterValue(TEXT("UVScale"), FLinearColor(Mat->BaseColorScale.X, Mat->BaseColorScale.Y, 0, 0));
					MID->SetVectorParameterValue(TEXT("UVOffset"), FLinearColor(Mat->BaseColorOffset.X, Mat->BaseColorOffset.Y, 0, 0));

					auto GetTexFromIndex = [&](int32 TexIndex, bool bSRGB)->UTexture2D*
					{
						if (!Textures.IsValidIndex(TexIndex)) return nullptr;
						const int32 ImgIndex = Textures[TexIndex].SourceImage;
						if (!Images.IsValidIndex(ImgIndex)) return nullptr;
						if (UTexture2D** Found = ImageIndexToTexture.Find(ImgIndex)) return *Found;
						const FGLTFImagePICO& Img = Images[ImgIndex];
						if (Img.BufferViewIndex == INDEX_NONE || !BufferViews.IsValidIndex(Img.BufferViewIndex)) return nullptr;
						const FGLTFBufferViewPICO& BV = BufferViews[Img.BufferViewIndex];
						if (BV.ByteOffset < 0 || BV.ByteOffset + BV.ByteLength > Bin.Num()) return nullptr;
						TArray<uint8> ImgBytes;
						ImgBytes.Append(Bin.GetData() + BV.ByteOffset, BV.ByteLength);
						UTexture2D* Tex = GetOrCreateTextureFromImageBytes(ImgBytes, bSRGB);
						if (Tex) ImageIndexToTexture.Add(ImgIndex, Tex);
						return Tex;
					};

					if (Mat->BaseColorTex != INDEX_NONE)
					{
						if (UTexture2D* T = GetTexFromIndex(Mat->BaseColorTex, true))
						{
							MID->SetTextureParameterValue(TEXT("BaseColorTexture"), T);
						}
					}
					if (Mat->NormalTex != INDEX_NONE)
					{
						if (UTexture2D* T = GetTexFromIndex(Mat->NormalTex, false))
						{
							MID->SetTextureParameterValue(TEXT("NormalTexture"), T);
						}
					}
					int32 ORMTexIdx = Mat->MetallicRoughnessTex;
					if (ORMTexIdx != INDEX_NONE)
					{
						if (UTexture2D* T = GetTexFromIndex(ORMTexIdx, false))
						{
							MID->SetTextureParameterValue(TEXT("ORMTexture"), T);
						}
					}
					if (Mat->EmissiveTex != INDEX_NONE)
					{
						if (UTexture2D* T = GetTexFromIndex(Mat->EmissiveTex, true))
						{
							MID->SetTextureParameterValue(TEXT("EmissiveTexture"), T);
						}
					}
				}
				SetMaterial(SectionIndex, MID);
			}
			SectionIndex++;
			TotalVertices += Vertices.Num();
			TotalTriangles += Triangles.Num() / 3;
		}
	}

	if (SectionIndex <= 0)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: No mesh sections built"));
		return false;
	}

	UE_LOG(LogPICOOpenXRHMD, Display, TEXT("UOpenXRRenderModelPMCPICO::LoadFromOpenXRRenderModel: Built %d sections, %d verts, %d tris"), SectionIndex, TotalVertices, TotalTriangles);
	return true;
}

void UOpenXRRenderModelPMCPICO::BeginPlay()
{
	Super::BeginPlay();
	FramesSinceLastAttempt = 0;
	PrimaryComponentTick.SetTickFunctionEnable(bAutoReload);
}

UOpenXRRenderModelPMCPICO::UOpenXRRenderModelPMCPICO(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	if (!OverrideMaterial)
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(TEXT("/PICOOpenXR/Materials/M_OpenXRRenderModel_PICO.M_OpenXRRenderModel_PICO"));
		if (MatFinder.Succeeded())
		{
			OverrideMaterial = MatFinder.Object;
		}
		else
		{
			UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("UOpenXRRenderModelPMCPICO: Default material not found: /PICOOpenXR/Materials/M_OpenXRRenderModel_PICO"));
		}
	}
}

bool UOpenXRRenderModelPMCPICO::TryLoadRenderModel()
{
	LoadAttempts++;
	FramesSinceLastAttempt = 0;
	if (bMeshLoaded)
	{
		PrimaryComponentTick.SetTickFunctionEnable(true);
		return true;
	}

	const bool bSuccess = LoadFromOpenXRRenderModelPICO();
	if (bSuccess)
	{
		bMeshLoaded = true;
		PrimaryComponentTick.SetTickFunctionEnable(true);
		return true;
	}

	if (!bAutoReload)
	{
		PrimaryComponentTick.SetTickFunctionEnable(false);
	}

	return false;
}

void UOpenXRRenderModelPMCPICO::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bMeshLoaded)
	{
		FramesSinceLastAttempt++;
		if (FramesSinceLastAttempt >= RetryEveryNFrames)
		{
			FramesSinceLastAttempt = 0;
			TryLoadRenderModel();
		}
		return;
	}

	{
		FTransform WorldXf;
		FString Error;
		if (FPICOOpenXRHMDModule::Get().GetXRPlugin().GetRenderModelWorldTransform(Hand, WorldXf, Error))
		{
			SetWorldTransform(WorldXf, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

void UOpenXRRenderModelPMCPICO::OnRegister()
{
	Super::OnRegister();
	{
		FScopeLock L(&GTexCacheMutex);
		GActivePMCInstances++;
	}
}

void UOpenXRRenderModelPMCPICO::OnUnregister()
{
	bool bShouldClear = false;
	{
		FScopeLock L(&GTexCacheMutex);
		GActivePMCInstances = FMath::Max(0, GActivePMCInstances - 1);
		bShouldClear = (GActivePMCInstances == 0);
	}
	bool bCleared = false;
	if (bShouldClear)
	{
		{
			FScopeLock L(&GTexCacheMutex);
			GImageTexCache.Empty();
			GImageTexCacheBytes = 0;
		}
		{
			FScopeLock L(&GGlbCacheMutex);
			GGlbCacheByHand.Empty();
		}
		bCleared = true;
	}
	if (bCleared)
	{
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("UOpenXRRenderModelPMCPICO: Cleared caches (no active instances)"));
	}
	Super::OnUnregister();
}
