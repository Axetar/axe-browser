#include "layout.hpp"
#include <iostream>

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int width) {
    auto box = std::make_shared<Box>(Box{ 0, 0, width, 0, node });
    int fontSize = 8;

    int yOffset = 10;

    for (const auto& child : node->children) {
        auto childBox = CreateLayoutTree(child, width);
        childBox->y = yOffset;
        yOffset += childBox->height;
        box->children.push_back(childBox);
    }

    // Estimate text dimensions
    int charsPerLine = static_cast<int>(width / (8 * 0.6f));
    int numLines = static_cast<int>(std::ceil(static_cast<float>(node->text.length()) / charsPerLine));
    int textHeight = static_cast<int>(numLines * 8 * 1.2f);

    if (!node->children.empty())
        box->height = yOffset;
    else
       box->height = textHeight + std::stoi(node->style.properties["padding"]);

    return box;
}



