Texture2D<float4> g_sourceTexture : register(t0);
Texture2D<float2> g_scanlinePhases : register(t1);

sampler g_sampler : register(s0);

cbuffer consts : register(b0)
{
  uint g_samplesPerColorburstCycle;
  float g_tint;
  float g_saturation;
  float g_brightness;
  float g_blackLevel;
  float g_whiteLevel;
  float g_temporalArtifactReduction;
}

static const float pi = 3.1415926535897932384626433832795028841971f;


// This shader demodulates the chroma information to get the I and Q color space values out of it (Y is easy, we already have it, it's just the
//  luma value).
//
// This process is called NTSC color demodulation, which is a type of QAM demodulation.
// Just like filtering the luma from the chroma (see the comment in CompositeToSVideo.hlsl), rather than doing any sort of fancy filtering here,
// the absolute best results I've gotten were from a super-basic average (which very nicely removes a very specific frequency and all integer
//  multiples of it), which is also exactly what we need here.
//
// Additionally, if we want temporal artifact reduction, we're doing this whole calculation twice (we have a float4(lumaA, chromaA, lumaB, chromaB)
//  texture representing the same frame with two different color phases), and we use the temporal artifact reduction value to blend between them,
//  which eliminates alternating-phase-related flickering that systems like the NES could generate.
float4 main(float2 inTexCoord : TEX): SV_TARGET
{
  float2 inputTexelSize = float2(ddx(inTexCoord).x, ddy(inTexCoord).y);

  uint sampleXIndex = uint(round(inTexCoord.x / inputTexelSize.x - 0.5));

  // We're going to sample chroma at double our actual colorburst cycle to eliminate some deeply rough artifacting on the edges.
  //  In this case we're going to average by DOUBLE the color burst cycle - doing just a single cycle ends up with a LOT of edge artifacting on
  //  things, which are too strong. 2x is much softer but not so large that you can really notice that it's extra.
  uint filterWidth = g_samplesPerColorburstCycle * 2;

  float2 relativePhase = g_scanlinePhases.Sample(g_sampler, inTexCoord.y) + g_tint;

  // This is the chroma decode process, it's a QAM demodulation.
  //  You multiply the chroma signal by a reference waveform and its quadrature (Basically, sin and cos at a given time) and then filter
  //  out the chroma frequency (here done by a box filter (an average)). What you're left with are the approximate I and Q color space
  //  values for this part of the image.
  float4 centerSample = g_sourceTexture.Sample(g_sampler, inTexCoord);
  float2 Y = centerSample.xz;

  // $TODO This could be made likely more efficient by basically doing two passes: one to generate a four-component texture with both sets of
  //  sin and cos-multiplied chroma values, and then THIS shader to average them, which would allow doing half the samples in this pass
  //  as we could take advantage of bilinear filtering.
  float4 IQ;
  {
    float2 s, c;
    sincos(2.0 * pi * (float(sampleXIndex) / g_samplesPerColorburstCycle + relativePhase), s, c);
    IQ = centerSample.yyww  * float4(s, -c).xzyw;
  }

  {
    int iterEnd = int((filterWidth - 1) / 2);
    for (int i = 1; i <= iterEnd; i++)
    {
      {
        float2 coord = inTexCoord + float2(i, 0) * inputTexelSize;
        float2 chroma = g_sourceTexture.Sample(g_sampler, coord).yw;
        float2 s, c;
        sincos(2.0 * pi * (float(sampleXIndex + i) / g_samplesPerColorburstCycle + relativePhase), s, c);
        IQ += chroma.xxyy  * float4(s, -c).xzyw;
      }

      {
        float2 coord = inTexCoord - float2(i, 0) * inputTexelSize;
        float2 chroma = g_sourceTexture.Sample(g_sampler, coord).yw;
        float2 s, c;
        sincos(2.0 * pi * (float(sampleXIndex - i) / g_samplesPerColorburstCycle + relativePhase), s, c);
        IQ += chroma.xxyy  * float4(s, -c).xzyw;
      }
    }

    if ((filterWidth & 1) == 0)
    {
      // We have an odd remainder (because we have an even filter width), so sample 0.5x each endpoint
      {
        float2 coord = inTexCoord + float2(iterEnd + 1, 0) * inputTexelSize;
        float2 chroma = g_sourceTexture.Sample(g_sampler, coord).yw;
        float2 s, c;
        sincos(2.0 * pi * (float(sampleXIndex + iterEnd + 1) / g_samplesPerColorburstCycle + relativePhase), s, c);
        IQ += 0.5 * chroma.xxyy  * float4(s, -c).xzyw;
      }

      {
        float2 coord = inTexCoord - float2(iterEnd + 1, 0) * inputTexelSize;
        float2 chroma = g_sourceTexture.Sample(g_sampler, coord).yw;
        float2 s, c;
        sincos(2.0 * pi * (float(sampleXIndex - iterEnd - 1) / g_samplesPerColorburstCycle + relativePhase), s, c);
        IQ += 0.5 * chroma.xxyy  * float4(s, -c).xzyw;
      }
    }

    IQ /= filterWidth;
  }

  // Adjust our components, first Y to account for the signal's black/white level (and user-chosen brightness), then IQ for saturation (Which includes the
  //  signal's saturation scale)
  Y = (Y - g_blackLevel) / (g_whiteLevel - g_blackLevel) * g_brightness;
  IQ *= g_saturation.xxxx;

  Y.x = lerp(Y.x, Y.y, g_temporalArtifactReduction * 0.5);
  IQ.xy = lerp(IQ.xy, IQ.zw, g_temporalArtifactReduction * 0.5);

  // Do some gamma adjustments (values effectively based on eyeballing the results of NTSC signals from NES, SNES, and Genesis consoles)
  Y.x = pow(saturate(Y.x), 2.0 / 2.2);
  float iqSat = saturate(length(IQ.xy));
  IQ.xy *= pow(iqSat, 2.0 / 2.2) / max(0.00001, iqSat);

  return float4(Y.x, IQ.xy, 1);
}