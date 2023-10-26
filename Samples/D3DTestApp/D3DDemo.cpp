#pragma once

#define NOMINMAX
#include <Windows.h>

#include "CathodeRetro/CathodeRetro.h"

#include "DemoHandler.h"
#include "D3D11GraphicsDevice.h"

class D3DDemoHandler : public IDemoHandler
{
public:
  D3DDemoHandler(HWND hwnd)
  {
    graphicsDevice = ID3D11GraphicsDevice::Create(hwnd);
  }

  virtual void ResizeBackbuffer(uint32_t width, uint32_t height)
  {
    graphicsDevice->UpdateWindowSize(width, height);
    cathodeRetro->SetOutputSize(width, height);
  }


  std::unique_ptr<CathodeRetro::ITexture> CreateRGBATexture(uint32_t width, uint32_t height, uint32_t *rgbaData) override
  {
    return graphicsDevice->CreateTexture(width, height, CathodeRetro::TextureFormat::RGBA_Unorm8, rgbaData);
  }

  void Render(
    const CathodeRetro::ITexture *currentFrame,
    const CathodeRetro::ITexture *prevFrame,
    CathodeRetro::ScanlineType scanlineType) override
  {
    graphicsDevice->ClearBackbuffer();
    cathodeRetro->Render(currentFrame, prevFrame, scanlineType, nullptr);
    graphicsDevice->Present();
  }

  void UpdateCathodeRetroSettings(
    CathodeRetro::SignalType sigType,
    uint32_t inputWidth,
    uint32_t inputHeight,
    const CathodeRetro::SourceSettings &sourceSettings,
    const CathodeRetro::ArtifactSettings &artifactSettings,
    const CathodeRetro::TVKnobSettings &knobSettings,
    const CathodeRetro::OverscanSettings &overscanSettings,
    const CathodeRetro::ScreenSettings &screenSettings) override
  {
    if (cathodeRetro == nullptr)
    {
      cathodeRetro = std::make_unique<CathodeRetro::CathodeRetro>(
        graphicsDevice.get(),
        sigType,
        inputWidth,
        inputHeight,
        sourceSettings,
        artifactSettings,
        knobSettings,
        overscanSettings,
        screenSettings);
    }
    else
    {
      cathodeRetro->UpdateSettings(
        sigType,
        inputWidth,
        inputHeight,
        sourceSettings,
        artifactSettings,
        knobSettings,
        overscanSettings,
        screenSettings);
    }
  }

private:
  std::unique_ptr<CathodeRetro::CathodeRetro> cathodeRetro;
  std::unique_ptr<ID3D11GraphicsDevice> graphicsDevice;
};


// This function is called by the main demo code to create our demo handler
std::unique_ptr<IDemoHandler> MakeDemoHandler(HWND hwnd)
{
  return std::make_unique<D3DDemoHandler>(hwnd);
}
