#include "layout.hpp"
#include <iostream>

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int available_width, int available_height, int parent_x, int parent_y) {
    auto box = std::make_shared<Box>();
    box->node = node;

    // style values
    int margin = stoi(node->style.properties["margin"]);
    int padding = stoi(node->style.properties["padding"]);

    // box model values (all sides equal)
    box->margin = { margin, margin, margin, margin };
    box->padding = { padding, padding, padding, padding };

    // outer dimensions (including margins)
    box->outer_width = available_width - box->margin.left - box->margin.right;
    box->outer_height = available_height - box->margin.top - box->margin.bottom;

    // content area (inside padding)
    box->content_width = box->outer_width - box->padding.left - box->padding.right;
    box->content_height = box->outer_height - box->padding.top - box->padding.bottom;

    // Set positions relative to parent
    box->outer_x = parent_x + box->margin.left;
    box->outer_y = parent_y + box->margin.top;
    box->content_x = box->outer_x + box->padding.left;
    box->content_y = box->outer_y + box->padding.top;

    // children in content area
    int y_offset = 0;
    for (const auto& child : node->children) {
        auto child_box = CreateLayoutTree(
            child,
            box->content_width,
            box->content_height,
            box->content_x,
            box->content_y + y_offset
        );

        y_offset += child_box->outer_height;
        box->children.push_back(child_box);
    }

    // Calculate text height if no children
    if (node->children.empty()) {
        int fontSize = 16;
        int charsPerLine = box->content_width / (fontSize * 0.6f);
        int numLines = static_cast<int>(ceil(node->text.length())) / charsPerLine;
        box->content_height = numLines * fontSize * 1.1f;
    }
    else {
        box->content_height = y_offset;
    }

    // Update outer dimensions to match content + padding
    box->outer_height = box->padding.top + box->padding.bottom + box->content_height;
    box->outer_width = box->padding.left + box->padding.right + box->content_width;

    // Handle body element special case
    if (node->tag == "body") {
        box->outer_height = available_height;
        box->content_height = available_height;
    }

    return box;
}