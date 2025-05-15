#include "render.hpp"
#include <string>
#include <iostream>
#include <iomanip>
#include <cstdio>

#pragma comment(lib, "dwrite")
#pragma comment(lib, "d2d1")

bool D2DResources::Initialize(HWND hwnd) {
    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        factory.GetAddressOf()
    );
    if (FAILED(hr)) 
        return false;

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(dwriteFactory.GetAddressOf())
    );
    if (FAILED(hr)) 
        return false;

    hr = dwriteFactory->CreateTextFormat(
        L"Arial", nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        16.0f, L"en-us",
        textFormat.GetAddressOf()
    );
    if (FAILED(hr)) 
        return false;

    textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

    return SUCCEEDED(RecreateRenderTarget(hwnd))
        && SUCCEEDED(renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.8f, 0.9f, 1.0f), &defaultFillBrush)) 
        && SUCCEEDED(renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &borderBrush)) 
        && SUCCEEDED(renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &textBrush))
	    && SUCCEEDED(renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 1.0f, 0.2f), &marginBrush))
		&& SUCCEEDED(renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.5f, 0.0f, 0.2f), &paddingBrush));
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
        // Create margin brush (blue with 20% opacity)
        renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(0.0f, 0.0f, 1.0f, 0.2f),
            &marginBrush
        );

        // Create padding brush (orange with 20% opacity)
        renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(1.0f, 0.5f, 0.0f, 0.2f),
            &paddingBrush
        );
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.8f, 0.9f, 1.0f), &defaultFillBrush);
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f), &borderBrush);
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &textBrush);
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 1.0f, 0.2f), &marginBrush);
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.5f, 0.0f, 0.2f), &paddingBrush);
	}
	else {
		// Cleanup if failed
		Cleanup();
		MessageBox(hwnd, L"Failed to create render target", L"Error", MB_OK);
    }

    return hr;
}
D2D1::ColorF HexToRGB(const std::string& hex) {
    if (hex.size() != 7 || hex[0] != '#') {
        std::cerr << "Invalid hex color format!" << std::endl;
        return D2D1::ColorF(0, 0, 0); // Return black color in case of error
    }

    int r, g, b;
    sscanf_s(hex.c_str(), "#%02x%02x%02x", &r, &g, &b);

    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    return D2D1::ColorF(rf, gf, bf);
}

void D2DResources::Cleanup() {
    textBrush.Reset();
    borderBrush.Reset();
    defaultFillBrush.Reset();
	marginBrush.Reset();
	paddingBrush.Reset();
    renderTarget.Reset();
    textFormat.Reset();
    dwriteFactory.Reset();
    factory.Reset();
}

void RenderBox(const std::shared_ptr<Box>& box, D2DResources& res, int parentX, int parentY) {
    if (!box || !res.renderTarget) return;

    // Draw content background
    D2D1_RECT_F content_rect = D2D1::RectF(
        box->content_x,
        box->content_y,
        box->content_x + box->content_width,
        box->content_y + box->content_height
    );

    // Draw content background
    const auto& bgColor = box->node->style.properties["background"];
    if (!bgColor.empty()) {
        ComPtr<ID2D1SolidColorBrush> backgroundBrush;
        res.renderTarget->CreateSolidColorBrush(HexToRGB(bgColor), &backgroundBrush);
        //res.renderTarget->FillRectangle(content_rect, backgroundBrush.Get());
    }

    // Draw text
    if (!box->node->text.empty()) {
        std::wstring text(box->node->text.begin(), box->node->text.end());
        res.renderTarget->DrawText(
            text.c_str(),
            static_cast<UINT32>(text.length()),
            res.textFormat.Get(),
            content_rect,
            res.textBrush.Get()
        );
    }

    // Render children using content area as new parent coordinates
    for (const auto& child : box->children)
        RenderBox(child, res, box->content_x, box->content_y);
}