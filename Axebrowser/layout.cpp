#include "layout.hpp"
#include <iostream>

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int width, const int height, bool isDirectChildOfBody) {
    auto box = std::make_shared<Box>(Box{ 0, 0, width, 0, node });

    // Special handling for body and its children
    bool isBody = (node->tag == "body");

    // Start yOffset at 0 for body's direct children to avoid the gap
    int yOffset = (isBody || isDirectChildOfBody) ? 0 : 10;

    for (const auto& child : node->children) {
        // information about parent to children
        auto childBox = CreateLayoutTree(child, width, height, isBody);
        childBox->y = yOffset;
        yOffset += childBox->height;
        box->children.push_back(childBox);
    }

    // Estimate text dimensions
    int charsPerLine = static_cast<int>(width / (16.0f * 0.6f));
    int numLines = static_cast<int>(std::ceil(static_cast<float>(node->text.length()) / charsPerLine));
    int textHeight = static_cast<int>(numLines * 16.0f * 1.2f);

    // Calculate height based on content and element type
    if (!node->children.empty()) // For body, don't add extra height beyond content height
        box->height = yOffset;
    else
        box->height = textHeight + std::stoi(box->node->style.properties["margin"]);

    // Special case: if this is the body element, make it fill the viewport height
    if (isBody)
        box->height = height;

    return box;
}
