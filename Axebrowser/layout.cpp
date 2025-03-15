#include "layout.hpp"
#include <iostream>

// ADD: Checks for margins/Padding becoming negative in width calculation
std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int width, const int height, bool isDirectChildOfBody) {
    int margin = stoi(node->style.properties["margin"]);
    int padding = stoi(node->style.properties["padding"]);
    auto box = std::make_shared<Box>(Box{ margin, margin + padding, width - margin * 2, 0, node });

    // Special handling for body and its children
    bool isBody = (node->tag == "body");

    // Start yOffset at 0 for body's direct children to avoid the gap
    int yOffset = (isBody || isDirectChildOfBody) ? 0 : padding;

    for (const auto& child : node->children) {
        // information about parent to children
        auto childBox = CreateLayoutTree(child, std::max(0, width - margin * 2 - padding * 2), height, isBody);
        childBox->y = yOffset;
		childBox->x += padding;
        yOffset += childBox->height;
        box->children.push_back(childBox);
    }

	// ADD: Make dynamic height calculation
    int fontSize = 16.0f; //stoi(node->style.properties["font-size"]);
    int charsPerLine = static_cast<int>(std::max(0, width - margin * 2) / (fontSize * 0.6f));
    int numLines = static_cast<int>(ceil(static_cast<float>(node->text.length()) / charsPerLine));
    int textHeight = static_cast<int>(numLines * fontSize * 1.1f);

    // if there are children, height is sum of children's heights
    box->height = margin + padding + (!node->children.empty() ? yOffset : textHeight);

    // Body element should take full height
    if (isBody)
        box->height = height;

    return box;
}

