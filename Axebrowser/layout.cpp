#include "layout.hpp"
#include <iostream>

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int width, const int height) {
    int margin = stoi(node->style.properties["margin"]);
    int padding = stoi(node->style.properties["padding"]);
    auto box = std::make_shared<Box>(Box{ margin, margin, width - margin * 2, height - margin * 2, node });

    // Start yOffset at 0 for body's direct children to avoid the gap
    int yOffset = padding;

    for (const auto& child : node->children) {

        // In box width/height marign is already added and then just need to add padding for the child content
        auto childBox = CreateLayoutTree(child, std::max(0, box->width - padding * 2), box->height - padding * 2);
        childBox->y = yOffset;
		childBox->x += padding;
        yOffset += childBox->height;
        box->children.push_back(childBox);
    }

	// ADD: Make dynamic height calculation
    int fontSize = 16.0f; //stoi(node->style.properties["font-size"]);
    int charsPerLine = static_cast<int>(std::max(0, box->width - padding * 2) / (fontSize * 0.6f));
    int numLines = static_cast<int>(ceil(static_cast<float>(node->text.length()) / charsPerLine));
    int textHeight = static_cast<int>(numLines * fontSize * 1.1f);

    // if there are children, height is sum of children's heights
    box->height = margin + padding + (!node->children.empty() ? yOffset : textHeight);

    // Body element should take full height
    if (box->node->tag == "body")
        box->height = height;

    return box;
}

