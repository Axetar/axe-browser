#pragma once
#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>
#include "dom.hpp"

extern ID2D1Factory* pD2DFactory;
extern ID2D1HwndRenderTarget* pRenderTarget;
extern IDWriteFactory* pDWriteFactory;
extern IDWriteTextFormat* pTextFormat;

void InitializeD2DResources(HWND hwnd);
void CleanupD2DResources();
void RenderBox(const std::shared_ptr<Box>& box, int parentX = 0, int parentY = 0);