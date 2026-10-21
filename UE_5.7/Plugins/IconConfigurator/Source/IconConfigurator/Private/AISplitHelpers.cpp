// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "AISplitHelpers.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Guid.h"
#include "HAL/FileManager.h"

// ---------------------------------------------------------------
//  Standalone SHA-256 (RFC 6234 / FIPS 180-4)
// ---------------------------------------------------------------
namespace
{
	static const uint32 kK[64] = {
		0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
		0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
		0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
		0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
		0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
		0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
		0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
		0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
	};

	inline uint32 RR(uint32 x, uint32 n) { return (x >> n) | (x << (32 - n)); }
	inline uint32 Ch(uint32 x, uint32 y, uint32 z) { return (x & y) ^ (~x & z); }
	inline uint32 Maj(uint32 x, uint32 y, uint32 z) { return (x & y) ^ (x & z) ^ (y & z); }
	inline uint32 Sig0(uint32 x) { return RR(x,2)  ^ RR(x,13) ^ RR(x,22); }
	inline uint32 Sig1(uint32 x) { return RR(x,6)  ^ RR(x,11) ^ RR(x,25); }
	inline uint32 sig0(uint32 x) { return RR(x,7)  ^ RR(x,18) ^ (x >> 3);  }
	inline uint32 sig1(uint32 x) { return RR(x,17) ^ RR(x,19) ^ (x >> 10); }

	void SHA256Block(const uint8 Data[64], uint32 H[8])
	{
		uint32 W[64];
		for (int i = 0; i < 16; ++i)
			W[i] = ((uint32)Data[i*4]<<24)|((uint32)Data[i*4+1]<<16)|((uint32)Data[i*4+2]<<8)|(uint32)Data[i*4+3];
		for (int i = 16; i < 64; ++i)
			W[i] = sig1(W[i-2]) + W[i-7] + sig0(W[i-15]) + W[i-16];

		uint32 a=H[0],b=H[1],c=H[2],d=H[3],e=H[4],f=H[5],g=H[6],h=H[7];
		for (int i = 0; i < 64; ++i)
		{
			uint32 T1 = h + Sig1(e) + Ch(e,f,g) + kK[i] + W[i];
			uint32 T2 = Sig0(a) + Maj(a,b,c);
			h=g; g=f; f=e; e=d+T1; d=c; c=b; b=a; a=T1+T2;
		}
		H[0]+=a; H[1]+=b; H[2]+=c; H[3]+=d;
		H[4]+=e; H[5]+=f; H[6]+=g; H[7]+=h;
	}

	void SHA256Raw(const uint8* Data, int32 Len, uint8 OutHash[32])
	{
		uint32 H[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};

		uint8 Block[64];
		int32 BlockLen = 0;
		int32 Processed = 0;

		auto FlushBlock = [&]() { SHA256Block(Block, H); BlockLen = 0; ++Processed; };
		auto AddByte = [&](uint8 B)
		{
			Block[BlockLen++] = B;
			if (BlockLen == 64) FlushBlock();
		};

		for (int32 i = 0; i < Len; ++i) AddByte(Data[i]);

		AddByte(0x80);
		while (BlockLen != 56) AddByte(0x00);

		uint64 BitLen = (uint64)(Processed * 64 + Len) * 8;
		for (int i = 7; i >= 0; --i) AddByte((uint8)(BitLen >> (i*8)));

		for (int i = 0; i < 8; ++i)
		{
			OutHash[i*4+0] = (uint8)(H[i] >> 24);
			OutHash[i*4+1] = (uint8)(H[i] >> 16);
			OutHash[i*4+2] = (uint8)(H[i] >> 8);
			OutHash[i*4+3] = (uint8)(H[i]);
		}
	}
} // anonymous namespace

// ---------------------------------------------------------------
//  JSON helpers
// ---------------------------------------------------------------

static FAISplitRegionConfig ParseRegionConfig(const TSharedPtr<FJsonObject>& RegObj, const FString& RegionName)
{
	FAISplitRegionConfig R;
	if (!RegObj.IsValid()) return R;

	// Log raw region JSON for diagnostics
	FString RegionRaw;
	TSharedRef<TJsonWriter<>> W = TJsonWriterFactory<>::Create(&RegionRaw);
	FJsonSerializer::Serialize(RegObj.ToSharedRef(), W);
	UE_LOG(LogTemp, Log, TEXT("AISplit region[%s] raw JSON: %s"), *RegionName, *RegionRaw);

	{
		const TSharedPtr<FJsonObject>* AuthObj;
		if (RegObj->TryGetObjectField(TEXT("auth"), AuthObj) && (*AuthObj).IsValid())
		{
			double AppIdD = 0.0;
			if ((*AuthObj)->TryGetNumberField(TEXT("app_id"), AppIdD))
				R.Auth.AppId = static_cast<int64>(AppIdD);
			(*AuthObj)->TryGetStringField(TEXT("salt"), R.Auth.Salt);
		}
	}
	{
		const TSharedPtr<FJsonObject>* TosObj;
		if (RegObj->TryGetObjectField(TEXT("tos"), TosObj) && (*TosObj).IsValid())
		{
			(*TosObj)->TryGetStringField(TEXT("endpoint"),          R.Tos.Endpoint);
			(*TosObj)->TryGetStringField(TEXT("bucket"),            R.Tos.Bucket);
			(*TosObj)->TryGetStringField(TEXT("ak"),                R.Tos.Ak);
			(*TosObj)->TryGetStringField(TEXT("public_url_prefix"), R.Tos.PublicUrlPrefix);
			(*TosObj)->TryGetStringField(TEXT("object_dir"),        R.Tos.ObjectDir);
		}
	}
	{
		const TSharedPtr<FJsonObject>* ApiObj;
		if (RegObj->TryGetObjectField(TEXT("api"), ApiObj) && (*ApiObj).IsValid())
		{
			(*ApiObj)->TryGetStringField(TEXT("gen_url"),  R.Api.GenUrl);
			(*ApiObj)->TryGetStringField(TEXT("info_url"), R.Api.InfoUrl);
			(*ApiObj)->TryGetStringField(TEXT("tt_env"),   R.Api.TtEnv);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AISplit region[%s] parsed: appId=%lld salt=%s endpoint=%s bucket=%s genUrl=%s infoUrl=%s"),
		*RegionName, R.Auth.AppId, *R.Auth.Salt, *R.Tos.Endpoint, *R.Tos.Bucket, *R.Api.GenUrl, *R.Api.InfoUrl);

	return R;
}

// ---------------------------------------------------------------
//  Public API
// ---------------------------------------------------------------

bool AISplitHelpers::ParseTccConfig(const FString& ResponseBody, FAISplitTccConfig& OutConfig)
{
	// Parse outer: { "data": { "ai_split_config": "<stringified JSON>" } }
	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AISplit: TCC outer JSON parse failed"));
		return false;
	}

	const TSharedPtr<FJsonObject>* DataObj;
	if (!Root->TryGetObjectField(TEXT("data"), DataObj) || !(*DataObj).IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AISplit: TCC JSON missing 'data'"));
		return false;
	}

	FString AiSplitConfigStr;
	if (!(*DataObj)->TryGetStringField(TEXT("ai_split_config"), AiSplitConfigStr))
	{
		UE_LOG(LogTemp, Warning, TEXT("AISplit: TCC JSON missing 'ai_split_config'"));
		return false;
	}

	// Clean up backticks/whitespace
	AiSplitConfigStr = AiSplitConfigStr.Replace(TEXT("`"), TEXT("")).TrimStartAndEnd();

	// Log the full inner JSON in chunks of 900 chars so nothing is lost
	{
		const int32 ChunkSize = 900;
		const int32 TotalLen  = AiSplitConfigStr.Len();
		for (int32 Offset = 0; Offset < TotalLen; Offset += ChunkSize)
		{
			UE_LOG(LogTemp, Log, TEXT("AISplit inner cfg [%d/%d]: %s"),
				Offset, TotalLen, *AiSplitConfigStr.Mid(Offset, ChunkSize));
		}
	}

	// Parse middle: { "ai_split": { ... } }
	TSharedPtr<FJsonObject> Mid;
	TSharedRef<TJsonReader<>> MidReader = TJsonReaderFactory<>::Create(AiSplitConfigStr);
	if (!FJsonSerializer::Deserialize(MidReader, Mid) || !Mid.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AISplit: ai_split_config inner JSON parse failed"));
		return false;
	}

	const TSharedPtr<FJsonObject>* AiSplitObj;
	if (!Mid->TryGetObjectField(TEXT("ai_split"), AiSplitObj) || !(*AiSplitObj).IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AISplit: inner JSON missing 'ai_split'"));
		return false;
	}

	// Timeout
	{
		const TSharedPtr<FJsonObject>* TmObj;
		if ((*AiSplitObj)->TryGetObjectField(TEXT("timeout_ms"), TmObj) && (*TmObj).IsValid())
		{
			double V = 0.0;
			if ((*TmObj)->TryGetNumberField(TEXT("connect"),       V)) OutConfig.TimeoutMs.Connect       = static_cast<int32>(V);
			if ((*TmObj)->TryGetNumberField(TEXT("read"),          V)) OutConfig.TimeoutMs.Read          = static_cast<int32>(V);
			if ((*TmObj)->TryGetNumberField(TEXT("poll_interval"), V)) OutConfig.TimeoutMs.PollInterval  = static_cast<int32>(V);
		}
	}

	// region_config
	const TSharedPtr<FJsonObject>* RegionMapObj;
	if ((*AiSplitObj)->TryGetObjectField(TEXT("region_config"), RegionMapObj) && RegionMapObj && (*RegionMapObj).IsValid())
	{
		for (auto& Pair : (*RegionMapObj)->Values)
		{
			const TSharedPtr<FJsonObject>* RegObj;
			if (Pair.Value->TryGetObject(RegObj) && RegObj && (*RegObj).IsValid())
			{
				FAISplitRegionConfig RC = ParseRegionConfig(*RegObj, Pair.Key);
				OutConfig.RegionConfig.Add(Pair.Key, RC);
			}
		}
	}

	OutConfig.bEnable = true;
	UE_LOG(LogTemp, Log, TEXT("AISplit: TCC config parsed, regions: %d"), OutConfig.RegionConfig.Num());
	return true;
}

FString AISplitHelpers::ComputeSHA256Hex(const FString& Input)
{
	FTCHARToUTF8 Conv(*Input);
	const uint8* Bytes = reinterpret_cast<const uint8*>(Conv.Get());
	int32 Len = Conv.Length();

	uint8 Hash[32];
	SHA256Raw(Bytes, Len, Hash);

	FString Hex;
	Hex.Reserve(64);
	for (int32 i = 0; i < 32; ++i)
	{
		Hex += FString::Printf(TEXT("%02x"), Hash[i]);
	}
	return Hex;
}

int64 AISplitHelpers::GetOrCreateDeviceId()
{
	const FString DirPath  = FPaths::ProjectSavedDir() / TEXT("AISplit");
	const FString FilePath = DirPath / TEXT("device_id");

	FString Content;
	if (FFileHelper::LoadFileToString(Content, *FilePath))
	{
		int64 Id = FCString::Atoi64(*Content.TrimStartAndEnd());
		if (Id != 0) return Id;
	}

	// Generate new ID from GUID hash
	FGuid Guid = FGuid::NewGuid();
	int64 NewId = static_cast<int64>(GetTypeHash(Guid.ToString()));
	if (NewId < 0) NewId = -NewId;
	if (NewId == 0) NewId = 1;

	IFileManager::Get().MakeDirectory(*DirPath, true);
	FFileHelper::SaveStringToFile(FString::Printf(TEXT("%lld"), NewId), *FilePath);
	return NewId;
}
