#include "PICOSpatialExpressionTextureSample.h"

#if USE_USD_SDK

void FExpressionTextureSamplePICO::DefineShader(pxr::UsdStageRefPtr Stage)
{
	if (!Stage)
	{
		return;
	}

	if (UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Owner))
	{
		// Check if default texture is supported (if it exists)
		if (TextureSample->Texture && !IsTextureSupported(TextureSample->Texture))
		{
			UE_LOG(LogPICOSpatialUSD, Warning,
				TEXT("Failed to define shader for TextureSample node '%s': Unsupported default texture type"),
				*Owner->GetName());
			return;
		}

		const FString GraphPath = Convertor->GenerateUniqueShaderPath(FString("/Root/Material/") + Owner->GetName() + "_nodegraph");
		if (GraphPath.IsEmpty())
		{
			return;
		}

		pxr::UsdShadeNodeGraph NodeGraph = pxr::UsdShadeNodeGraph::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*GraphPath)));

		pxr::UsdShadeInput FileGraphInput = NodeGraph.CreateInput(pxr::TfToken("file"), pxr::SdfValueTypeNames->Asset);
		FileGraphInput.GetAttr().SetColorSpace(pxr::TfToken("srgb_texture"));
		FileGraphInput.Set(pxr::SdfAssetPath(""));

		pxr::UsdShadeInput DefaultGraphInput = NodeGraph.CreateInput(pxr::TfToken("default"), pxr::SdfValueTypeNames->Color4f);
		DefaultGraphInput.Set(pxr::GfVec4f(0.0f, 0.0f, 0.0f, 0.0f));
		DefaultGraphInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));

		pxr::UsdShadeInput TexCoordGraphInput = NodeGraph.CreateInput(pxr::TfToken("texcoord"), pxr::SdfValueTypeNames->Float2);
		TexCoordGraphInput.Set(pxr::GfVec2f(0.0f, 0.0f));

		if (TextureSample->Texture)
		{
			FString TexturePath = FShaderGraphUsdConvertorPICO::GetTexturePathFromTexture(TextureSample->Texture, Stage);
			if (!TexturePath.IsEmpty())
			{
				FileGraphInput.Set(pxr::SdfAssetPath(TCHAR_TO_UTF8(*TexturePath)));
			}
		}

		{
			const FString ImagePath = GraphPath + "/Image";
			UsdShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ImagePath)));
			UsdShader.CreateIdAttr().Set(pxr::TfToken("ND_tiledimage_color4"));

			pxr::UsdShadeInput FileInput = UsdShader.CreateInput(pxr::TfToken("file"), pxr::SdfValueTypeNames->Asset);
			FileInput.GetAttr().SetColorSpace(pxr::TfToken("srgb_texture"));
			FileInput.ConnectToSource(FileGraphInput);

			pxr::UsdShadeInput DefaultInput = UsdShader.CreateInput(pxr::TfToken("default"), pxr::SdfValueTypeNames->Color4f);
			DefaultInput.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
			DefaultInput.ConnectToSource(DefaultGraphInput);

			pxr::UsdShadeInput TexCoordInput = UsdShader.CreateInput(pxr::TfToken("texcoord"), pxr::SdfValueTypeNames->Float2);
			TexCoordInput.Set(pxr::GfVec2f(0.0f, 0.0f));
			// TODO(PICOSpatialUSD): Fix runtime issue where connecting texcoord inside NodeGraph causes invisible result; re-enable when resolved.

			UsdShader.CreateInput(pxr::TfToken("filtertype"), pxr::SdfValueTypeNames->String).Set(std::string("linear"));
			UsdShader.CreateInput(pxr::TfToken("frameendaction"), pxr::SdfValueTypeNames->String).Set(std::string("constant"));
			UsdShader.CreateInput(pxr::TfToken("frameoffset"), pxr::SdfValueTypeNames->Int).Set(0);
			UsdShader.CreateInput(pxr::TfToken("framerange"), pxr::SdfValueTypeNames->String).Set(std::string(""));
			UsdShader.CreateInput(pxr::TfToken("realworldimagesize"), pxr::SdfValueTypeNames->Float2).Set(pxr::GfVec2f(1.0f, 1.0f));
			UsdShader.CreateInput(pxr::TfToken("realworldtilesize"), pxr::SdfValueTypeNames->Float2).Set(pxr::GfVec2f(1.0f, 1.0f));
			UsdShader.CreateInput(pxr::TfToken("uvoffset"), pxr::SdfValueTypeNames->Float2).Set(pxr::GfVec2f(0.0f, 0.0f));
			UsdShader.CreateInput(pxr::TfToken("uvtiling"), pxr::SdfValueTypeNames->Float2).Set(pxr::GfVec2f(1.0f, 1.0f));

			pxr::UsdShadeOutput RGBAOutput = UsdShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Color4f);

			pxr::UsdShadeOutput RGBAOutGraph = NodeGraph.CreateOutput(pxr::TfToken("RGBA"), pxr::SdfValueTypeNames->Color4f);
			RGBAOutGraph.ConnectToSource(RGBAOutput);
			ShaderOutputs.Add(5, RGBAOutGraph);

			const FString ConvertPath = GraphPath + "/ConvertColor4ToColor3";
			pxr::UsdShadeShader ConvertShader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*ConvertPath)));
			ConvertShader.CreateIdAttr().Set(pxr::TfToken("ND_convert_color4_color3"));
			pxr::UsdShadeInput ConvertIn = ConvertShader.CreateInput(pxr::TfToken("in"), pxr::SdfValueTypeNames->Color4f);
			ConvertIn.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
			ConvertIn.ConnectToSource(RGBAOutput);
			pxr::UsdShadeOutput ConvertOut = ConvertShader.CreateOutput(pxr::TfToken("out"), pxr::SdfValueTypeNames->Color3f);

			pxr::UsdShadeOutput RGBOutGraph = NodeGraph.CreateOutput(pxr::TfToken("RGB"), pxr::SdfValueTypeNames->Color3f);
			RGBOutGraph.ConnectToSource(ConvertOut);
			ShaderOutputs.Add(0, RGBOutGraph);

			const FString SeparatePath = GraphPath + "/Separate4";
			pxr::UsdShadeShader Separate4Shader = pxr::UsdShadeShader::Define(Stage, pxr::SdfPath(TCHAR_TO_UTF8(*SeparatePath)));
			Separate4Shader.CreateIdAttr().Set(pxr::TfToken("ND_separate4_color4"));
			pxr::UsdShadeInput SepIn = Separate4Shader.CreateInput(pxr::TfToken("in"), pxr::SdfValueTypeNames->Color4f);
			SepIn.GetAttr().SetColorSpace(pxr::TfToken("lin_srgb"));
			SepIn.ConnectToSource(RGBAOutput);

			pxr::UsdShadeOutput ROut = Separate4Shader.CreateOutput(pxr::TfToken("outr"), pxr::SdfValueTypeNames->Float);
			pxr::UsdShadeOutput GOut = Separate4Shader.CreateOutput(pxr::TfToken("outg"), pxr::SdfValueTypeNames->Float);
			pxr::UsdShadeOutput BOut = Separate4Shader.CreateOutput(pxr::TfToken("outb"), pxr::SdfValueTypeNames->Float);
			pxr::UsdShadeOutput AOut = Separate4Shader.CreateOutput(pxr::TfToken("outa"), pxr::SdfValueTypeNames->Float);

			pxr::UsdShadeOutput ROutGraph = NodeGraph.CreateOutput(pxr::TfToken("R"), pxr::SdfValueTypeNames->Float);
			ROutGraph.ConnectToSource(ROut);
			ShaderOutputs.Add(1, ROutGraph);

			pxr::UsdShadeOutput GOutGraph = NodeGraph.CreateOutput(pxr::TfToken("G"), pxr::SdfValueTypeNames->Float);
			GOutGraph.ConnectToSource(GOut);
			ShaderOutputs.Add(2, GOutGraph);

			pxr::UsdShadeOutput BOutGraph = NodeGraph.CreateOutput(pxr::TfToken("B"), pxr::SdfValueTypeNames->Float);
			BOutGraph.ConnectToSource(BOut);
			ShaderOutputs.Add(3, BOutGraph);

			pxr::UsdShadeOutput AOutGraph = NodeGraph.CreateOutput(pxr::TfToken("A"), pxr::SdfValueTypeNames->Float);
			AOutGraph.ConnectToSource(AOut);
			ShaderOutputs.Add(4, AOutGraph);
		}

		const int32 InputCount = Owner->CountInputs();
		for (int32 Index = 0; Index < InputCount; ++Index)
		{
			FExpressionInput* Input = Owner->GetInput(Index);
			if (Input == &TextureSample->Coordinates)
			{
				ShaderInputs.Add(Index, TexCoordGraphInput);
			}
			else if (Input == &TextureSample->TextureObject)
			{
				ShaderInputs.Add(Index, FileGraphInput);
			}
			else
			{
				FString PlaceholderNameStr;
				pxr::SdfValueTypeName PlaceholderType = pxr::SdfValueTypeNames->Float;

				if (Input == &TextureSample->MipValue)
				{
					PlaceholderNameStr = TEXT("PICO_PLACEHOLDER_MipValue");
					PlaceholderType = pxr::SdfValueTypeNames->Float;
				}
				else if (Input == &TextureSample->CoordinatesDX)
				{
					PlaceholderNameStr = TEXT("PICO_PLACEHOLDER_CoordinatesDX");
					PlaceholderType = pxr::SdfValueTypeNames->Float2;
				}
				else if (Input == &TextureSample->CoordinatesDY)
				{
					PlaceholderNameStr = TEXT("PICO_PLACEHOLDER_CoordinatesDY");
					PlaceholderType = pxr::SdfValueTypeNames->Float2;
				}
				else if (Input == &TextureSample->AutomaticViewMipBiasValue)
				{
					PlaceholderNameStr = TEXT("PICO_PLACEHOLDER_AutomaticViewMipBiasValue");
					PlaceholderType = pxr::SdfValueTypeNames->Float;
				}
				else
				{
					PlaceholderNameStr = FString::Printf(TEXT("PICO_PLACEHOLDER_Input%d"), Index);
					PlaceholderType = pxr::SdfValueTypeNames->Float;
				}

				pxr::TfToken PlaceholderName(TCHAR_TO_UTF8(*PlaceholderNameStr));
				pxr::UsdShadeInput PlaceholderInput = NodeGraph.CreateInput(PlaceholderName, PlaceholderType);
				if (PlaceholderType == pxr::SdfValueTypeNames->Float2)
				{
					PlaceholderInput.Set(pxr::GfVec2f(0.0f, 0.0f));
				}
				else
				{
					PlaceholderInput.Set(0.0f);
				}

				ShaderInputs.Add(Index, PlaceholderInput);

				UE_LOG(LogPICOSpatialUSD, Verbose,
					TEXT("%s[PIN][PLACEHOLDER] TextureSample '%s' InputIndex=%d Placeholder=%s"),
					Convertor ? *Convertor->GetLogPrefix() : TEXT("[USD_MTLX][#0000][MAT=<null>]"),
					*Owner->GetName(), Index, *PlaceholderNameStr);
			}
		}

		bDefinedUsdShader = true;
	}
}

bool FExpressionTextureSamplePICO::ConnectInput()
{
	if (!Owner || !bDefinedUsdShader)
	{
		return false;
	}

	UMaterialExpressionTextureSample* TextureSample = Cast<UMaterialExpressionTextureSample>(Owner);
	if (!TextureSample)
	{
		return false;
	}

	bool bAllConnectionsSucceeded = true;

	auto FindInputIndex = [&](const FExpressionInput* Target) -> int32
	{
		const int32 InputCount = Owner->CountInputs();
		for (int32 Index = 0; Index < InputCount; ++Index)
		{
			if (Owner->GetInput(Index) == Target)
			{
				return Index;
			}
		}
		return INDEX_NONE;
	};

	const int32 CoordinatesIndex = FindInputIndex(&TextureSample->Coordinates);
	if (CoordinatesIndex != INDEX_NONE && TextureSample->Coordinates.IsConnected() && TextureSample->Coordinates.Expression)
	{
		FExpressionBasePICO* ConnectedNode = Convertor->FindNode(TextureSample->Coordinates.Expression);
		if (ConnectedNode && ConnectedNode->bDefinedUsdShader)
		{
			if (!ConnectedNode->ShaderOutputs.Contains(TextureSample->Coordinates.OutputIndex))
			{
				UE_LOG(LogPICOSpatialUSD, Warning,
					TEXT("Failed to connect Coordinates input for TextureSample node: %s (Missing source output index %d)"),
					*NodeName, TextureSample->Coordinates.OutputIndex);
				bAllConnectionsSucceeded = false;
			}
			else if (!ShaderInputs.Contains(CoordinatesIndex))
			{
				UE_LOG(LogPICOSpatialUSD, Warning,
					TEXT("Failed to connect Coordinates input for TextureSample node: %s (Missing target input index %d)"),
					*NodeName, CoordinatesIndex);
				bAllConnectionsSucceeded = false;
			}
			else
			{
				pxr::UsdShadeInput TexCoordInput = ShaderInputs[CoordinatesIndex];
				pxr::UsdShadeOutput SourceOutput = ConnectedNode->ShaderOutputs[TextureSample->Coordinates.OutputIndex];
				pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(TextureSample->Coordinates.OutputIndex);
				pxr::SdfValueTypeName InputType = TexCoordInput.GetTypeName();

				if (OutputType && InputType && OutputType != InputType)
				{
					pxr::UsdShadeShader ConvertShader = Convertor->CreateConvertOrSwizzleShader(OutputType, InputType);
					pxr::UsdShadeInput ConvertInput = ConvertShader ? ConvertShader.GetInput(pxr::TfToken("in")) : pxr::UsdShadeInput();
					pxr::UsdShadeOutput ConvertOutput = ConvertShader ? ConvertShader.GetOutput(pxr::TfToken("out")) : pxr::UsdShadeOutput();

					if (!ConvertShader || !ConvertInput || !ConvertOutput || !ConvertInput.ConnectToSource(SourceOutput) || !TexCoordInput.ConnectToSource(ConvertOutput))
					{
						UE_LOG(LogPICOSpatialUSD, Warning,
							TEXT("Failed to connect Coordinates input for TextureSample node: %s (Type conversion %s -> %s)"),
							*NodeName,
							UTF8_TO_TCHAR(OutputType.GetAsToken().GetString().c_str()),
							UTF8_TO_TCHAR(InputType.GetAsToken().GetString().c_str()));
						bAllConnectionsSucceeded = false;
					}
				}
				else if (TexCoordInput && !TexCoordInput.ConnectToSource(SourceOutput))
				{
					UE_LOG(LogPICOSpatialUSD, Warning,
						TEXT("Failed to connect Coordinates input for TextureSample node: %s"),
						*NodeName);
					bAllConnectionsSucceeded = false;
				}
			}
		}
		else
		{
			UE_LOG(LogPICOSpatialUSD, Warning,
				TEXT("Failed to connect Coordinates input for TextureSample node: %s (Missing connected node)"),
				*NodeName);
			bAllConnectionsSucceeded = false;
		}
	}

	const int32 TextureObjectIndex = FindInputIndex(&TextureSample->TextureObject);
	if (TextureObjectIndex != INDEX_NONE && TextureSample->TextureObject.IsConnected() && TextureSample->TextureObject.Expression)
	{
		if (FExpressionBasePICO* ConnectedNode = Convertor->FindNode(TextureSample->TextureObject.Expression))
		{
			if (!ConnectedNode->ShaderOutputs.Contains(TextureSample->TextureObject.OutputIndex))
			{
				UE_LOG(LogPICOSpatialUSD, Warning,
					TEXT("Failed to connect TextureObject input for TextureSample node: %s (Missing source output index %d)"),
					*NodeName, TextureSample->TextureObject.OutputIndex);
				bAllConnectionsSucceeded = false;
			}
			else if (!ShaderInputs.Contains(TextureObjectIndex))
			{
				UE_LOG(LogPICOSpatialUSD, Warning,
					TEXT("Failed to connect TextureObject input for TextureSample node: %s (Missing target input index %d)"),
					*NodeName, TextureObjectIndex);
				bAllConnectionsSucceeded = false;
			}
			else
			{
				pxr::UsdShadeInput FileInput = ShaderInputs[TextureObjectIndex];
				if (!FileInput)
				{
					UE_LOG(LogPICOSpatialUSD, Warning,
						TEXT("Failed to connect TextureObject input for TextureSample node: %s"),
						*NodeName);
					bAllConnectionsSucceeded = false;
				}
				else
				{
					FileInput.ClearSource();
					pxr::UsdShadeOutput SourceOutput = ConnectedNode->ShaderOutputs[TextureSample->TextureObject.OutputIndex];
					pxr::SdfValueTypeName OutputType = ConnectedNode->GetOutputType(TextureSample->TextureObject.OutputIndex);
					pxr::SdfValueTypeName InputType = FileInput.GetTypeName();

					if (OutputType && InputType && OutputType != InputType)
					{
						pxr::UsdShadeShader ConvertShader = Convertor->CreateConvertOrSwizzleShader(OutputType, InputType);
						pxr::UsdShadeInput ConvertInput = ConvertShader ? ConvertShader.GetInput(pxr::TfToken("in")) : pxr::UsdShadeInput();
						pxr::UsdShadeOutput ConvertOutput = ConvertShader ? ConvertShader.GetOutput(pxr::TfToken("out")) : pxr::UsdShadeOutput();

						if (!ConvertShader || !ConvertInput || !ConvertOutput || !ConvertInput.ConnectToSource(SourceOutput) || !FileInput.ConnectToSource(ConvertOutput))
						{
							UE_LOG(LogPICOSpatialUSD, Warning,
								TEXT("Failed to connect TextureObject input for TextureSample node: %s (Type conversion %s -> %s)"),
								*NodeName,
								UTF8_TO_TCHAR(OutputType.GetAsToken().GetString().c_str()),
								UTF8_TO_TCHAR(InputType.GetAsToken().GetString().c_str()));
							bAllConnectionsSucceeded = false;
						}
					}
					else if (!FileInput.ConnectToSource(SourceOutput))
					{
						UE_LOG(LogPICOSpatialUSD, Warning,
							TEXT("Failed to connect TextureObject input for TextureSample node: %s"),
							*NodeName);
						bAllConnectionsSucceeded = false;
					}
				}
			}
		}
		else
		{
			UE_LOG(LogPICOSpatialUSD, Warning,
				TEXT("Failed to connect TextureObject input for TextureSample node: %s (Missing connected node)"),
				*NodeName);
			bAllConnectionsSucceeded = false;
		}
	}

	if (TextureSample->MipValue.IsConnected() ||
		TextureSample->CoordinatesDX.IsConnected() ||
		TextureSample->CoordinatesDY.IsConnected() ||
		TextureSample->AutomaticViewMipBiasValue.IsConnected())
	{
		UE_LOG(LogPICOSpatialUSD, Warning,
			TEXT("Ignored connecting unsupported inputs for TextureSample node: %s"),
			*NodeName);
	}

	return bAllConnectionsSucceeded;
}
#endif // USE_USD_SDK
