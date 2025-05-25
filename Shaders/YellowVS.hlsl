// VS.hlsl
cbuffer CameraBuffer : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD;
};

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output;
    float4 pos = float4(input.position, 1.0f);
    output.position = mul(mul(mul(pos, world), view), proj);
    output.normal = input.normal;
    output.texcoord = input.texcoord;
    return output;
}