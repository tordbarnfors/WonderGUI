// One triangle covering the whole mip level being drawn, no vertex buffer
// needed. Texture coordinates run 0 to 1 over the visible part.

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 texUV : TEXCOORD;
};


VS_OUTPUT main(uint vertexId : SV_VertexID)
{
    VS_OUTPUT output;

    output.texUV = float2((vertexId << 1) & 2, vertexId & 2);
    output.position = float4(output.texUV.x * 2.0f - 1.0f, 1.0f - output.texUV.y * 2.0f, 0.0f, 1.0f);

    return output;
}
