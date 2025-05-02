#include <windows.h>
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <dwrite.h>
#include <wrl/client.h>
#include "dom.hpp"
#include "parser.hpp"
#include "layout.hpp"
#include "render.hpp"

std::shared_ptr<Box> layoutRoot;
D2DResources g_d2dResources;
std::shared_ptr<Box> phoveredBox = nullptr;


static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SIZE:
            if (g_d2dResources.renderTarget) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                g_d2dResources.RecreateRenderTarget(hwnd);
                layoutRoot = CreateLayoutTree(layoutRoot->node, rc.right, rc.bottom - rc.top);

                InvalidateRect(hwnd, nullptr, TRUE); // Force a full repaint
            }
            break;

        case WM_PAINT: {
            PAINTSTRUCT ps;

            BeginPaint(hwnd, &ps);

            g_d2dResources.renderTarget->BeginDraw();
            g_d2dResources.renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

            RenderBox(layoutRoot, g_d2dResources);

            HRESULT hr = g_d2dResources.renderTarget->EndDraw();
            if (hr == D2DERR_RECREATE_TARGET) { 
				MessageBox(hwnd, L"D2DERR_RECREATE_TARGET", L"Error", MB_OK); 
                g_d2dResources.Cleanup();
                g_d2dResources.Initialize(hwnd);

                // Re-create layout 
                RECT rc;
                GetClientRect(hwnd, &rc);
                layoutRoot = CreateLayoutTree(layoutRoot->node, rc.right, rc.bottom - rc.top);
            }

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_MOUSEMOVE: {
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(hwnd, &pt);

			// Check if the mouse is over a box
            auto hitTest = [](std::shared_ptr<Box> box, int x, int y, auto&& self) -> std::shared_ptr<Box> {
                for (auto it = box->children.rbegin(); it != box->children.rend(); it++) {
                    auto childHit = self(*it, x, y, self);
                    if (childHit)
                        return childHit;
                }
                if (x >= box->content_x && x <= box->content_x + box->content_width && y >= box->content_y && y <= box->content_y + box->content_height) {
                    return box;
                }
                return nullptr;
            };

            std::shared_ptr<Box> hoveredBox = hitTest(layoutRoot, pt.x, pt.y, hitTest);

            if (hoveredBox && hoveredBox != phoveredBox) {
				phoveredBox = hoveredBox;
                //std::cout << "Mouse over: " << hoveredBox->node->tag << std::endl;

                // (+) Need to switch to just a region repaint
				InvalidateRect(hwnd, nullptr, TRUE); // Force a full repaint
				UpdateWindow(hwnd);
                g_d2dResources.renderTarget->BeginDraw();

                // Full margin rect minues border/padding/content area
                D2D1_RECT_F marginOuter = D2D1::RectF(
                    hoveredBox->outer_x - hoveredBox->margin.left,
                    hoveredBox->outer_y - hoveredBox->margin.top,
                    hoveredBox->outer_x + hoveredBox->outer_width + hoveredBox->margin.right,
                    hoveredBox->outer_y + hoveredBox->outer_height + hoveredBox->margin.bottom
                );

                D2D1_RECT_F marginInner = D2D1::RectF(
                    hoveredBox->outer_x,
                    hoveredBox->outer_y,
                    hoveredBox->outer_x + hoveredBox->outer_width,
                    hoveredBox->outer_y + hoveredBox->outer_height
                );

                // Geometry group
                ComPtr<ID2D1RectangleGeometry> outerGeom;
                ComPtr<ID2D1RectangleGeometry> innerGeom;
                g_d2dResources.factory->CreateRectangleGeometry(marginOuter, &outerGeom);
                g_d2dResources.factory->CreateRectangleGeometry(marginInner, &innerGeom);

                ID2D1Geometry* geometries[] = { outerGeom.Get(), innerGeom.Get() };
                ComPtr<ID2D1GeometryGroup> marginGeom;
                g_d2dResources.factory->CreateGeometryGroup(D2D1_FILL_MODE_ALTERNATE, geometries, 2, &marginGeom);

                // Draw only the margin ring
                g_d2dResources.renderTarget->FillGeometry(marginGeom.Get(), g_d2dResources.marginBrush.Get());


                // Border rect MINUS content area
                D2D1_RECT_F paddingOuter = D2D1::RectF(
                    hoveredBox->outer_x,
                    hoveredBox->outer_y,
                    hoveredBox->outer_x + hoveredBox->outer_width,
                    hoveredBox->outer_y + hoveredBox->outer_height
                );

                D2D1_RECT_F paddingInner = D2D1::RectF(
                    hoveredBox->content_x,
                    hoveredBox->content_y,
                    hoveredBox->content_x + hoveredBox->content_width,
                    hoveredBox->content_y + hoveredBox->content_height
                );

                // Create geometry group
                ComPtr<ID2D1RectangleGeometry> outerGeom1;
                ComPtr<ID2D1RectangleGeometry> innerGeom1;
                g_d2dResources.factory->CreateRectangleGeometry(paddingOuter, &outerGeom1);
                g_d2dResources.factory->CreateRectangleGeometry(paddingInner, &innerGeom1);

                ID2D1Geometry* geometries1[] = { outerGeom1.Get(), innerGeom1.Get() };
                ComPtr<ID2D1GeometryGroup> paddingGeom;
                g_d2dResources.factory->CreateGeometryGroup(D2D1_FILL_MODE_ALTERNATE,geometries1, 2,&paddingGeom);

                // Draw padding ring
                g_d2dResources.renderTarget->FillGeometry(paddingGeom.Get(), g_d2dResources.paddingBrush.Get());
                g_d2dResources.renderTarget->EndDraw();
            }
            break; // allow rest of mouse events to be handled
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void PrintDOMTree(const std::shared_ptr<Node>& node, int depth = 0) {
    for (int i = 0; i < depth; ++i) 
        std::cout << "  ";
    std::cout << "Tag: " << node->tag << ": " << node->style.properties["padding"] << std::endl;

    for (const auto& child : node->children)
        PrintDOMTree(child, depth + 1);
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"AxeBrowserClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        wc.lpszClassName, L"Axe Browser",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, nullptr, nullptr, hInstance, nullptr
    );

    g_d2dResources.Initialize(hwnd);

    FILE* stream;
	AllocConsole();
	freopen_s(&stream, "CONOUT$", "w", stdout);

    const std::string html = "<html><body><div><h1>Example Domain</h1><p1>This domain is for use in illustrative examples in documents. You may use this domain in literature without prior coordination or asking for permission.</p><p><a href = 'https://www.iana.org/domains/example'>More information...</a></p></div> </body> <style> div { background: #FFC0CB; margin: 25; padding: 10; } p1 { background: #FFFFFF; margin: 0; padding: 25; } </style></html>";
    auto csRoot = ParseCSS(html);
    auto domRoot = ParseHTML(html);

	CombineHTMLCSS(domRoot, csRoot);

    // In WinMain, after parsing the tokens and creating the DOM tree
    PrintDOMTree(domRoot);

    RECT rc;
    GetClientRect(hwnd, &rc);
    layoutRoot = CreateLayoutTree(domRoot, rc.right, rc.bottom - rc.top);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_d2dResources.Cleanup();

    return 0;
}