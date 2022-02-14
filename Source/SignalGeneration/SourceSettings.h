#pragma once

#include <cinttypes>

namespace NTSCify::SignalGeneration
{
  enum class SignalType
  {
    SVideo,       // Keep luma and chroma separate - meaning you get chroma modulation artifacting but not channel mixing artifacts.
    Composite,    // Blend luma and chroma, necessitating a separation pass which will introduce channel mixing artifacts
  };


  // This structure contains the settings used for the generation of the actual clean SVideo/composite signal, and represent the properties of the source of the 
  //  signal (i.e. the "machine" that is generating the signal)
  struct SourceSettings
  {
    SignalType signalType = SignalType::Composite;

    // This is the common denominator of all phase generation values (kept as a fraction to maintain numerical precision, because in
    //  practice they're all rational values). So if the denominator is 3, a value of 1 would be 1/3rd of a color cycle, 2 would be 
    //  2/3rds, etc. 
    //
    // Basically, all subsequent values are fractions with this as the denominator.
    uint32_t denominator = 1;

    // This is a measure of how many cycles of the color carrier wave there are per pixel, and the answer is usually <= 1.
    uint32_t colorCyclesPerInputPixel = 1;

    // Phase is measured in multiples of the color cycle.

    // This is what fraction of the color cycle the first line of the first frame starts at
    uint32_t initialFramePhase = 0;

    // This is what fraction of the color cycle the phase increments every scanline
    uint32_t phaseIncrementPerLine = 0;

    // Some systems had different phase changes per frame, and so this breaks down "how different is the starting phase of a frame
    //  from one frame to the next" into even and odd frame deltas.
    uint32_t phaseIncrementPerEvenFrame = 0;
    uint32_t phaseIncrementPerOddFrame = 0;
  };


  // Timings for NES- and SNES-like video generation
  static constexpr SourceSettings k_NESLikeSourceSettings =
  {
    SignalType::Composite,
    3,        // NES timings are all in multiples of 1/3rd 
    2,        // NES has 2/3rds of a color phase for every pixel (i.e. it has a 33% greater horizontal resolution than the color signal can represent)
    0,        // The starting phase of the NES doesn't really matter, so just pick 0
    1,        // Every line the phase of the color cycle offsets by 1/3rd of the color
    2,        // On even frames, the color cycle offsets by 2/3rds of the color cycle
    1,        // On odd frames, the color cycle offsets by 1/3rd of the color cycle (making the frames alternate, since 2/3 + 1/3 == 1)
  };


  // Timings for CGA (And likely other PC board)-like video generation, 320px horizontal resolution
  static constexpr SourceSettings k_CGA320LikeSourceSettings =
  {
    SignalType::Composite,
    2,        // CGA deals in multiples of 1/2
    1,        // Every pixel is half of a color subcarrier wave
    0,        // Start at phase 0
    0,        // The CGA doesn't change phase at all per line (or per frames)
    0,        //  ...
    0,        //  ...
  };


  // Timings for CGA (And likely other PC board)-like video generation, 640px horizontal resolution
  static constexpr SourceSettings k_CGA640LikeSourceSettings =
  {
    SignalType::Composite,
    4,        // CGA at 640 pixels wide deals in multiples of 1/4
    1,        // Every pixel is a quarter a color subcarrier wave (half of the 320 span since we have twice the number of pixels)
    0,        // Start at phase 0
    0,        // The CGA doesn't change phase at all per line (or per frames)
    0,        //  ...
    0,        //  ...
  };
}