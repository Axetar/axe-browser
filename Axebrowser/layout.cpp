#include "layout.hpp"
#include <iostream>

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int width) {
    auto box = std::make_shared<Box>(Box{ 0, 0, width, 30, node });

    int yOffset = 10;
    for (const auto& child : node->children) {
        auto childBox = CreateLayoutTree(child, width);
        childBox->y = yOffset;
        yOffset += childBox->height;
        box->children.push_back(childBox);
    }

    // Estimate characters per line (assuming average character width is ~0.6 * font size)
    int charsPerLine = static_cast<int>(width / (node->style.fontSize * 0.6f));

    // Calculate number of lines required
    int numLines = static_cast<int>(std::ceil(static_cast<float>(node->text.length()) / charsPerLine));

    // Compute text height based on line count
    int textHeight = static_cast<int>(numLines * node->style.fontSize * 1.2f);

    if (!node->children.empty())
        box->height = yOffset - box->y;
    else
        box->height = textHeight + node->style.padding;

    return box;
}
