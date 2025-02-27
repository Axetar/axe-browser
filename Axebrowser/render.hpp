#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>
#include <windows.h>
#include <memory>
#include "dom.hpp"

using Microsoft::WRL::ComPtr;

struct D2DResources {
    ComPtr<ID2D1Factory> factory;
    ComPtr<ID2D1HwndRenderTarget> renderTarget;
    ComPtr<IDWriteFactory> dwriteFactory;
    ComPtr<IDWriteTextFormat> textFormat;
    ComPtr<ID2D1SolidColorBrush> defaultFillBrush;
    ComPtr<ID2D1SolidColorBrush> borderBrush;
    ComPtr<ID2D1SolidColorBrush> textBrush;

    bool Initialize(HWND hwnd);
    void Cleanup();
    HRESULT RecreateRenderTarget(HWND hwnd);
};

void RenderBox(const std::shared_ptr<Box>& box, D2DResources& resources, int parentX = 0, int parentY = 0);