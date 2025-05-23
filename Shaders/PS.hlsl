// PS.hlsl
// Shader Model 6.7
struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

[shader("pixel")]
float4 main(PSInput input) : SV_TARGET
{
    // Simple color: blueish, for testing
    return float4(1, 1, 0.1, 1.0);
}