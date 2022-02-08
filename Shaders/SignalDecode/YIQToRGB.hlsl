Texture2D<float4> g_sourceTexture : register(t0);
RWTexture2D<float4> g_outputTexture : register(u0);

cbuffer consts : register(b0)
{
  float g_gamma;
}


[numthreads(8, 8, 1)]
void main(uint2 dispatchThreadID : SV_DispatchThreadID)
{
	float3 YIQ = g_sourceTexture.Load(uint3(dispatchThreadID, 0)).rgb;

  // Do the YIQ to RGB conversion and then gamma-adjust it
  float3x3 mat = float3x3(
    1.0,       1.0,       1.0,
    0.946882, -0.274788, -1.108545,
    0.623557, -0.635691,  1.7090047);
  float3 RGB = pow(saturate(mul(YIQ, mat)), g_gamma);

  g_outputTexture[dispatchThreadID] = float4(RGB, 1);
}
