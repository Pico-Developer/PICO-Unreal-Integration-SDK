// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PICOSpatialUSDModule.h"
#include "PICOSpatialUsdHeaders.h"

class UMaterialInterface;
class UMaterialExpression;
class FShaderGraphUsdConvertorPICO;

enum ENodeTypePICO
{
    MNP_Unsupported,
    MNP_Constant,
    MNP_Constant2Vector,
    MNP_Constant3Vector,
    MNP_Constant4Vector,
    MNP_TextureSample,
    MNP_TextureSampleParameter2D,
    MNP_TextureCoordinate,
    MNP_TextureObject,
    MNP_CameraPosition,
    MNP_CameraDirection,
    MNP_Normalize,
    MNP_WorldPosition,
    MNP_Multiply,
    MNP_Add,
    MNP_Lerp,
    MNP_Sine,
    MNP_Cosine,
    MNP_Arcsine,
    MNP_Arccosine,
    MNP_Tangent,
    MNP_Arctangent2,
    MNP_Power,
    MNP_Sqrt,
    MNP_Abs,
    MNP_Exp,
    MNP_Exp2,
    MNP_Log,
    MNP_Log2,
    MNP_Log10,
    MNP_DDX,
    MNP_DDY,
    MNP_Frac,
    MNP_Distance,
    MNP_DotProduct,
    MNP_Max,
    MNP_Min,
    MNP_OneMinus,
    MNP_Floor,
    MNP_Ceil,
    MNP_CrossProduct,
    MNP_Clamp,
    MNP_Modulo,
    MNP_If,
    MNP_Divide,
    MNP_Subtract,
    MNP_Time,
    MNP_MF_PI,
    MNP_MF_CameraDirection,
    MNP_ScalarParameter,
    MNP_VectorParameter,
    MNP_StaticSwitchParameter,
    MNP_RootNode
};

#if USE_USD_SDK
class FExpressionBasePICO
{
public:
    FExpressionBasePICO(FShaderGraphUsdConvertorPICO* InConvertor, UMaterialExpression* InExpression);
    virtual ~FExpressionBasePICO();
    FExpressionBasePICO(const FExpressionBasePICO&) = delete;
    FExpressionBasePICO& operator=(const FExpressionBasePICO&) = delete;
    FExpressionBasePICO(FExpressionBasePICO&&) = delete;
    FExpressionBasePICO& operator=(FExpressionBasePICO&&) = delete;

    virtual void DefineShader(pxr::UsdStageRefPtr Stage) {}
    virtual bool ConnectInput() { return true; }

    // Returns the ordered list of USD types that this node's MaterialX nodedef
    // actually supports.  The list is used by ResolveOperatingType() to map
    // a raw "desired" type (= max of upstream outputs) to the best valid variant.
    //
    // Selection rules (applied inside ResolveOperatingType):
    //   1. Pick the minimum-dimension variant whose dim >= desired dim.
    //   2. If both a color and a vector variant exist at that dimension, prefer color.
    //   3. If desired dim exceeds the dimension of every supported variant, clamp to
    //      the maximum supported variant.  The connect phase will then insert a
    //      convert node to downcast the upstream output; the node itself must not
    //      be defined beyond its supported variant set.
    //
    // Default: all standard types, suitable for fully-generic arithmetic nodes
    // (Add, Multiply, Lerp, etc.). Override in subclasses with a restricted variant set
    // (e.g. Normalize -> {Float2, Float3, Float4}; no float, no color).
    virtual TArray<pxr::SdfValueTypeName> GetSupportedVariants() const;

    // Resolve a raw desired type (= max of upstream outputs) to the best-matching
    // operating type within this node's supported variant set.
    // Non-virtual: delegates entirely to GetSupportedVariants() so subclasses only
    // need to declare the variant list.
    pxr::SdfValueTypeName ResolveOperatingType(const pxr::SdfValueTypeName& DesiredType) const;

    pxr::SdfValueTypeName GetInputType(int Index);
    pxr::SdfValueTypeName GetOutputType(int Index);

    FShaderGraphUsdConvertorPICO* Convertor;
    UMaterialExpression* Owner;
    ENodeTypePICO NodeType;
    FString NodeName;
    bool bIsSupportedNode;
    bool bIsParameterNode;
    bool bDefinedUsdShader;

    pxr::UsdShadeShader UsdShader;
    TMap<int, pxr::UsdShadeInput> ShaderInputs;
    TMap<int, pxr::UsdShadeOutput> ShaderOutputs;

    // Populated by FShaderGraphUsdConvertorPICO::RunForwardTypePropagation()
    // before DefineShader(); stores the **operating type** for this node
    // (= max of all directly-connected upstream output types).
    // For most nodes this equals the output type seen by downstream nodes,
    // but they are semantically distinct.  When adding a scalar-reduction node
    // (e.g. magnitude / dotproduct, always float output), do NOT return this
    // field from GetNodeOutputTypeForPropagation(); return Float instead.
    pxr::SdfValueTypeName InferredUsdType;
};

class FShaderGraphUsdConvertorPICO
{
public:
    FShaderGraphUsdConvertorPICO(const UMaterialInterface* InMaterialInterface, pxr::UsdStageRefPtr InStage);

    virtual ~FShaderGraphUsdConvertorPICO();
    FShaderGraphUsdConvertorPICO(const FShaderGraphUsdConvertorPICO&) = delete;
    FShaderGraphUsdConvertorPICO& operator=(const FShaderGraphUsdConvertorPICO&) = delete;
    FShaderGraphUsdConvertorPICO(FShaderGraphUsdConvertorPICO&&) = delete;
    FShaderGraphUsdConvertorPICO& operator=(FShaderGraphUsdConvertorPICO&&) = delete;

    bool Convert();
    bool CanConvert() { return !bHasUnsupportedNode; }
    void SetLogContext(int32 InMaterialSequence, const FString& InOutputUsdPath);
    FString GetLogPrefix() const;
    const FString& GetLastError() const { return LastError; }

    ENodeTypePICO GetExpressionType(const UMaterialExpression* Expression);
    EMaterialValueType InferenceInputType(UMaterialExpression* Expression, int InputIndex);
    FExpressionBasePICO* FindNode(UMaterialExpression* Expression);

    // Forward type propagation: must be called after CreateAndValidateNodes() and before
    // DefineAllUsdShaders().  Fills InferredUsdType on every node using topological-order
    // DFS so that each node only considers its direct upstream connections.
    void RunForwardTypePropagation();

    // ---------- Static type helpers (used by Expression DefineShader implementations) ----------

    // Returns the MaterialX nodedef type-suffix string (e.g. "color4", "vector3", "float").
    static FString GetMxTypeSuffix(const pxr::SdfValueTypeName& Type);

    // Returns true if the type carries color-space semantics (color3f / color4f).
    static bool IsColorUsdType(const pxr::SdfValueTypeName& Type);

    // Returns a numeric rank for ordering USD types by component width.
    // Higher rank = more components:
    //   Color4f=8, Float4=7, Color3f=6, Float3=5, Float2=4, Float=1.
    // Exposed as a public static so Expression subclasses can compare types
    // inside GetSupportedVariants().
    static int32 GetTypeRank(const pxr::SdfValueTypeName& Type);

    // Returns the number of scalar components in a USD type:
    //   Float=1, Float2=2, Float3/Color3f=3, Float4/Color4f=4.
    // Exposed as a public static for use in GetSupportedVariants() overrides.
    static int32 GetTypeDimension(const pxr::SdfValueTypeName& Type);

    // Broadcast a single float scalar as the default value for the given USD type.
    // For vector/color types the scalar is broadcast to all channels.
    static void SetDefaultInputValue(pxr::UsdShadeInput& Input, float DefaultFloat,
        const pxr::SdfValueTypeName& Type);

    // Map a color USD type to its vector equivalent (Color3f→Float3, Color4f→Float4).
    // Used for nodes (e.g. normalize) that only accept vector inputs in MaterialX.
    static pxr::SdfValueTypeName ToVectorEquivalent(const pxr::SdfValueTypeName& Type);

    // Getter for USD Stage
    pxr::UsdStageRefPtr GetStage() const { return Stage; }

    // Generate a unique shader path by appending random suffix if path already exists
    FString GenerateUniqueShaderPath(const FString& DesiredPath);

    // Create a type conversion USD shader node with unique naming
    pxr::UsdShadeShader CreateConvertOrSwizzleShader(pxr::SdfValueTypeName FromType, pxr::SdfValueTypeName ToType);

    /**
     * Get texture file path from UTexture for USD export
     * @param Texture The texture to get path from
     * @param Stage The USD stage to calculate relative path
     * @return Relative or absolute path to the texture file
     */
    static FString GetTexturePathFromTexture(UTexture* Texture, pxr::UsdStageRefPtr Stage);

private:
    const UMaterialInterface* MaterialInterface;
    TMap<UMaterialExpression*, FExpressionBasePICO*> MaterialExpressionToPICONodeMap;
    TSet<UMaterialExpression*> ReachableMaterialExpressions;

    bool bHasUnsupportedNode = false;
    int32 MaterialSequence = 0;
    FString OutputUsdPath;
    FString MaterialDebugPath;
    FString LastError;

    FExpressionBasePICO* CreateExpressionPICO(UMaterialExpression* Expression);
    void SetFailure(const FString& Step, const FString& Reason);
    bool Fail(const FString& Step, const FString& Reason);

    // Helper functions for Convert()
    const UMaterial* ValidateAndCastMaterial() const;
    bool ValidateShadingModel(const UMaterial* Material, EMaterialShadingModel& OutShadingModel) const;

    void SetupUsdStageDefaults();
    void CreatePreviewGeometry();
    bool CreateUsdMaterialWithProperties(const UMaterial* Material, pxr::UsdShadeMaterial& UsdMaterial);
    bool CreateSurfaceShader(const UMaterial* Material, EMaterialShadingModel ShadingModel,
        pxr::UsdShadeMaterial& UsdMaterial, pxr::UsdShadeShader& OutSurfaceShader);

    // Material expression node processing
    bool ProcessMaterialExpressions(const UMaterial* Material);
    void CreateAndValidateNodes(const UMaterial* Material);
    bool DefineAllUsdShaders();
    bool ConnectAllNodeInputs();

    // Material property connections
    bool ConnectMaterialPropertiesToSurface(const UMaterial* Material, EMaterialShadingModel ShadingModel,
        pxr::UsdShadeShader& SurfaceShader);

    pxr::UsdStageRefPtr Stage;
};
#endif // USE_USD_SDK
