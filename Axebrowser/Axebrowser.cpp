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

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_SIZE:
            if (g_d2dResources.renderTarget) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                g_d2dResources.RecreateRenderTarget(hwnd);
                layoutRoot = CreateLayoutTree(layoutRoot->node, rc.right);
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
				MessageBox(hwnd, L"D2DERR_RECREATE_TARGET", L"Error", MB_OK); // Temp
                g_d2dResources.Cleanup();
                g_d2dResources.Initialize(hwnd);

                // Recreate layout
                //RECT rc;
               // GetClientRect(hwnd, &rc);
                //layoutRoot = CreateLayoutTree(layoutRoot->node, rc.right);
            }

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void PrintDOMTree(const std::shared_ptr<Node>& node, int depth = 0) {
    for (int i = 0; i < depth; ++i) std::cout << "  ";
    std::cout << "Tag: " << node->tag << std::endl;

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

    const std::string html = "<html><body><div><h1>Example Domain</h1><p>Wow This is the most amazing thing ever however the text isn't going to fit if it wraps down and the dimentiosn will be readjusted</p><p>This is the text that isnt going to fit </p></div></body></html>";
    auto tokens = Tokenize(html);
    auto domRoot = ParseTokens(tokens);

  
    // In WinMain, after parsing the tokens and creating the DOM tree
    PrintDOMTree(domRoot);

    RECT rc;
    GetClientRect(hwnd, &rc);
    layoutRoot = CreateLayoutTree(domRoot, rc.right);

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