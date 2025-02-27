#include "layout.hpp"

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int parentWidth) {
    auto box = std::make_shared<Box>();
    box->node = node;
    box->width = parentWidth;
    box->height = 30;

    int yOffset = 0;
    for (const auto& child : node->children) {
        auto childBox = CreateLayoutTree(child, parentWidth);
        childBox->y = yOffset;
        yOffset += childBox->height;
        box->children.push_back(childBox);
    }

    if (!node->children.empty())
        box->height = yOffset;

    return box;
}