#include "render.hpp"
#include <algorithm>

#pragma comment(lib, "dwrite")
#pragma comment(lib, "d2d1")

ID2D1Factory* pD2DFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
IDWriteFactory* pDWriteFactory = nullptr;
IDWriteTextFormat* pTextFormat = nullptr;

void InitializeD2DResources(HWND hwnd) {
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&pDWriteFactory)
    );

    pDWriteFactory->CreateTextFormat(
        L"Arial", nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        16.0f, L"en-us", &pTextFormat
    );

    pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    RECT rc;
    GetClientRect(hwnd, &rc);
    pD2DFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right, rc.bottom)),
        &pRenderTarget
    );
}

void CleanupD2DResources() {
    pRenderTarget->Release();
    pD2DFactory->Release();
    pTextFormat->Release();
    pDWriteFactory->Release();
}

void RenderBox(const std::shared_ptr<Box>& box, int parentX, int parentY) {
    if (!box || !box->node || !pRenderTarget)
        return;

    int x = parentX + box->x;
    int y = parentY + box->y;

    ID2D1SolidColorBrush* pFillBrush = nullptr, * pBorderBrush = nullptr;

    auto createBrush = [&](const D2D1::ColorF& color, ID2D1SolidColorBrush** brush) -> bool {
        HRESULT hr = pRenderTarget->CreateSolidColorBrush(color, brush);
        return SUCCEEDED(hr);
        };

    if (!createBrush(D2D1::ColorF(0.8f, 0.9f, 1.0f), &pFillBrush))
        return;
    if (!createBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &pBorderBrush)) {
        pFillBrush->Release(); // Release previous brush
        return;
    }

    D2D1_RECT_F rect = D2D1::RectF(static_cast<FLOAT>(x), static_cast<FLOAT>(y), static_cast<FLOAT>(x + box->width), static_cast<FLOAT>(y + box->height));
    pRenderTarget->FillRectangle(rect, pFillBrush);
    pRenderTarget->DrawRectangle(rect, pBorderBrush);

    pFillBrush->Release();
    pBorderBrush->Release();

    if (!box->node->text.empty()) { // Text in the node
        ID2D1SolidColorBrush* pTextBrush = nullptr;

        if (!createBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &pTextBrush))
            return;

        std::wstring text(box->node->text.begin(), box->node->text.end());
        D2D1_RECT_F textRect = D2D1::RectF(static_cast<FLOAT>(x + 5), static_cast<FLOAT>(y + 5), static_cast<FLOAT>(x + box->width - 5), static_cast<FLOAT>(y + box->height - 5));


        pRenderTarget->DrawText(
            text.c_str(), static_cast<UINT32>(text.length()),
            pTextFormat, textRect, pTextBrush
        );

        pTextBrush->Release();
    }

    for (const auto& child : box->children)
        RenderBox(child, x, y);
}