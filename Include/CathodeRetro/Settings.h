#pragma once

#include <cinttypes>
#include <string.h>

namespace CathodeRetro
{
  enum class SignalType
  {
    RGB,          // Perfect RGB end to end.
    SVideo,       // Keep luma and chroma separate - chroma modulation artifacting but not channel mixing artifacts.
    Composite,    // Combine luma and chroma, necessitating a separation pass which will introduce channel mixing
                  //  artifacts.
  };


  enum class ScanlineType
  {
    Odd,                // This is an "odd" interlaced frame, the (1-based) odd scanlines will be full brightness.
    Even,               // This is an "even" interlaced frame

    Progressive = Odd,  // If doing Progressive scan, you'll want to set scanlineStrength to 0 & always use Odd frames.
  };


  enum class MaskType
  {
    SlotMask,
    ShadowMask,
    ApertureGrille,
  };


  struct Vec2
  {
    float x;
    float y;
  };


  struct Color
  {
    float r;
    float g;
    float b;
    float a;
  };


  // This structure describes the properties of the hypothetical source "machine" that is generating the composite or
  //  S-Video signal. Specifically, it gives scanline timings and color-cycle-to-pixel ratios and phase offsets, as
  //  well as a display pixel ratio.
  struct SourceSettings
  {
    bool operator==(const SourceSettings &other) const { return memcmp(this, &other, sizeof(*this)) == 0; }
    bool operator!=(const SourceSettings &other) const { return memcmp(this, &other, sizeof(*this)) != 0; }

    // The display aspect ratio of an input pixel when displayed.
    //  Use 1/1 for square input pixels, but for the NES/SNES it's 8/7
    float inputPixelAspectRatio = 1.0f;

    // the number of color cycles to pad on either side of the signal texture (so that filtering won't have visible
    //  rtifacts on the left and right sides). Defaults to 2, but can be set to 0 if you don't need padding (like for a
    //  real signal, or a generated signal that already has expected overscan on the sides).
    uint32_t sidePaddingColorCycleCount = 2;

    // This is the common denominator of all phase generation values (kept as a fraction to maintain numerical
    //  precision, because in practice they're all rational values). So if the denominator is 3, a value of 1 would be
    //  1/3rd of a color cycle, 2 would be  2/3rds, etc.
    //
    // Basically, all subsequent values are fractions with this as the denominator.
    uint32_t denominator = 1;

    // This is a measure of how many cycles of the color carrier wave there are per pixel, and the answer is
    //  usually <= 1.
    uint32_t colorCyclesPerInputPixel = 1;

    // Phase is measured in multiples of the color cycle.

    // This is what fraction of the color cycle the first line of the first frame starts at
    uint32_t initialFramePhase = 0;

    // This is what fraction of the color cycle the phase increments every scanline
    uint32_t phaseIncrementPerLine = 0;

    // Some systems had different phase changes per frame, and so this breaks down "how different is the starting phase
    //  of a frame from one frame to the next" into even and odd frame deltas.
    uint32_t phaseIncrementPerEvenFrame = 0;
    uint32_t phaseIncrementPerOddFrame = 0;
  };


  // This describes how noisy the generated signal is. Think of this as a bad cable or RF transmission between the
  //  nice, cleanly-generated source signal and the back of the TV.
  struct ArtifactSettings
  {
    bool operator==(const ArtifactSettings &other) const { return memcmp(this, &other, sizeof(*this)) == 0; }
    bool operator!=(const ArtifactSettings &other) const { return memcmp(this, &other, sizeof(*this)) != 0; }

    float ghostVisibility = 0.0f;           // How visible is the ghost
    float ghostSpreadScale = 0.71f;         // How far does each sample of the ghost spread from the last
    float ghostDistance    = 3.1f;          // How far from center is the ghost's center

    float noiseStrength = 0.0f;             // How much noise is added to the signal

    float instabilityScale = 0.0f;          // How much horizontal wobble to have on the screen per scanline

    float temporalArtifactReduction = 0.0f; // How much to blend between two different phases to reduce temporal aliasing
  };


  // These settings describe the screen properties of the virtual CRT TV that is displaying the image: things like how
  //  curved is the screen and the appearance of the mask and scanlines.
  struct ScreenSettings
  {
    bool operator==(const ScreenSettings &other) const { return memcmp(this, &other, sizeof(*this)) == 0; }
    bool operator!=(const ScreenSettings &other) const { return memcmp(this, &other, sizeof(*this)) != 0; }

    // How much to barrel distort our picture horizontally and vertically to emulate a curved screen
    Vec2 distortion = { 0.0f, 0.0f };

    // How much additional rounding of the edges we want to emulate a screen that didn't have a rectangular bezel shape
    Vec2 screenEdgeRounding = { 0.0f, 0.0f };

    // How much to round the corners (to emulate an old TV with rounded corners)
    float cornerRounding = 0.0f;

    // What type of mask, as well as how much the mask affects the output (both its scale and its strength, and "depth"
    //  which is the level of the darkest part of the mask)
    MaskType maskType = MaskType::SlotMask;
    float maskScale = 1.0f;
    float maskStrength = 0.0f;
    float maskDepth = 0.5f;

    // How much of the previous frame to keep around on the next frame.
    float phosphorPersistence = 0.0f;

    // How powerful the scanlines are
    float scanlineStrength = 0.0f;

    // How much the "glass" in front of the "phosphors" diffuses the light passing through it.
    float diffusionStrength = 0.0f;

    // The color around the edges of the screen
    Color borderColor = { 0.05f, 0.05f, 0.05f, 1.0f };
  };


  // These settings mimic the knobs that would have been on the front of an old CRT.
  struct TVKnobSettings
  {
    bool operator==(const TVKnobSettings &other) const { return memcmp(this, &other, sizeof(*this)) == 0; }
    bool operator!=(const TVKnobSettings &other) const { return memcmp(this, &other, sizeof(*this)) != 0; }

    float saturation = 1.0f;    // The saturation of the decoded signal (where 1.0 is the default)
    float brightness = 1.0f;    // The brightnesss of the decoded signal
    float tint = 0.0f;          // An additional tint to apply to the output signal
    float sharpness = 0.0f;     // 0 is unfiltered, 1 is "fully sharpened", -1 is "fully blurred"
  };


  // These settings describe how much overscan we want to have: that is, how many input pixels of the source image get
  //  cut off by the "bevel" of the TV.
  struct OverscanSettings
  {
    bool operator==(const OverscanSettings &other) const { return memcmp(this, &other, sizeof(*this)) == 0; }
    bool operator!=(const OverscanSettings &other) const { return memcmp(this, &other, sizeof(*this)) != 0; }

    uint32_t overscanLeft = 0;
    uint32_t overscanRight = 0;
    uint32_t overscanTop = 0;
    uint32_t overscanBottom = 0;
  };
}