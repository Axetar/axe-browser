#include "layout.hpp"
#include <iostream>

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int width, const int height, bool isDirectChildOfBody) {
	int margin = stoi(node->style.properties["margin"]);
    auto box = std::make_shared<Box>(Box{margin, margin, width - margin * 2, 0, node });

    // Special handling for body and its children
    bool isBody = (node->tag == "body");

    // Start yOffset at 0 for body's direct children to avoid the gap
    int yOffset = (isBody || isDirectChildOfBody) ? 0 : 10;

    for (const auto& child : node->children) {
        // information about parent to children
        auto childBox = CreateLayoutTree(child, width - margin * 2, height, isBody);
        childBox->y = yOffset;
        yOffset += childBox->height;
        box->children.push_back(childBox);
    }

    // Estimate text dimensions
    int fontSize = 16.0f; //stoi(node->style.properties["font-size"]);
    int charsPerLine = static_cast<int>((width) / (fontSize * 0.6f)); // - stoi(node->style.properties["padding"]) * 2
    int numLines = static_cast<int>(ceil(static_cast<float>(node->text.length()) / charsPerLine));
    int textHeight = static_cast<int>(numLines * fontSize * 1.2f);

     // if there are children, height is sum of children's heights
     box->height = margin + (!node->children.empty() ? yOffset : textHeight);

	// Body element should take full height
    if (isBody)
        box->height = height;

    return box;
}
