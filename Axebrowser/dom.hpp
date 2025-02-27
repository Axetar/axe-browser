#pragma once
#include <memory>
#include <vector>
#include <string>
#include <d2d1.h>

struct Node {
    std::string tag;
    std::string text;
    std::vector<std::shared_ptr<Node>> children;

    // Style information
    struct Style {
        float fontSize = 16.0f;
        D2D1_COLOR_F color = D2D1::ColorF(D2D1::ColorF::Black);
        D2D1_COLOR_F backgroundColor = D2D1::ColorF(0.8f, 0.9f, 1.0f);
    } style;
};

struct Box {
    int x, y = 0;
    int width, height = 0;
    std::shared_ptr<Node> node;
    std::vector<std::shared_ptr<Box>> children;
};