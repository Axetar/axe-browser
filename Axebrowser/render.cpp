#include "render.hpp"
#include <string>

#pragma comment(lib, "dwrite")
#pragma comment(lib, "d2d1")

bool D2DResources::Initialize(HWND hwnd) {
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        factory.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(dwriteFactory.GetAddressOf())
    );
    if (FAILED(hr)) return false;

    hr = dwriteFactory->CreateTextFormat(
        L"Arial", nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        16.0f, L"en-us",
        textFormat.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    return SUCCEEDED(RecreateRenderTarget(hwnd))
        && SUCCEEDED(renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.8f, 0.9f, 1.0f), &defaultFillBrush)) 
        && SUCCEEDED(renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &borderBrush)) 
        && SUCCEEDED(renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &textBrush));
}

HRESULT D2DResources::RecreateRenderTarget(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    D2D1_SIZE_U size = D2D1::SizeU(
        static_cast<UINT32>(rc.right - rc.left),
        static_cast<UINT32>(rc.bottom - rc.top)
    );

    HRESULT hr = factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, size),
        &renderTarget
    );

    // Recreate brushes after recreating render target
    if (SUCCEEDED(hr)) {
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.8f, 0.9f, 1.0f), &defaultFillBrush);
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &borderBrush);
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &textBrush);
    }

    return hr;
}

void D2DResources::Cleanup() {
    textBrush.Reset();
    borderBrush.Reset();
    defaultFillBrush.Reset();
    renderTarget.Reset();
    textFormat.Reset();
    dwriteFactory.Reset();
    factory.Reset();
}

void RenderBox(const std::shared_ptr<Box>& box, D2DResources& res, int parentX, int parentY) {
    if (!box || !box->node || !res.renderTarget) return;

    const FLOAT x = static_cast<FLOAT>(parentX + box->x);
    const FLOAT y = static_cast<FLOAT>(parentY + box->y);
    const FLOAT width = static_cast<FLOAT>(box->width);
    const FLOAT height = static_cast<FLOAT>(box->height);

    // Draw box background and border
    const D2D1_RECT_F rect = D2D1::RectF(x, y, x + width, y + height);
    res.renderTarget->FillRectangle(rect, res.defaultFillBrush.Get());
    res.renderTarget->DrawRectangle(rect, res.borderBrush.Get());

    // Draw text
    if (!box->node->text.empty()) {
        const std::wstring text(box->node->text.begin(), box->node->text.end());
        const D2D1_RECT_F textRect = D2D1::RectF(
            x + 5.0f, y + 5.0f,
            x + width - 5.0f,
            y + height - 5.0f
        );

        res.renderTarget->DrawText(
            text.c_str(),
            static_cast<UINT32>(text.length()),
            res.textFormat.Get(),
            textRect,
            res.textBrush.Get()
        );
    }

    // Render children
    for (const auto& child : box->children)
        RenderBox(child, res, static_cast<int>(x), static_cast<int>(y));
}