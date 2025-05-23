#include "layout.hpp"
#include <iostream>
#include <sstream>

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int available_width, int available_height, int parent_x, int parent_y) {
    auto box = std::make_shared<Box>();
    box->node = node;

    // style values

    // Four diff values of margin
    std::stringstream ss(node->style.properties["margin"]);
    std::string token;
    std::vector<std::string> margin1;

    while (std::getline(ss, token, ' '))
        margin1.push_back(token);
    

	box->margin.left = (margin1[0] != "auto") ? std::stoi(margin1[0]) : (available_width - stoi(node->style.properties["width"])) / 2;
    if (margin1.size() > 1)
        box->margin.right = (margin1[1] != "auto") ? std::stoi(margin1[0]) : (available_width - stoi(node->style.properties["width"])) / 2;
    else 
		box->margin.right = box->margin.left;
    if (margin1.size() > 2)
        box->margin.top = (margin1[2] != "auto") ? std::stoi(margin1[0]) : 0;
    if (margin1.size() > 3)
	    box->margin.bottom = (margin1[3] != "auto") ? std::stoi(margin1[0]) : 0;


    //int margin = (node->style.properties["margin"] == "auto") ? ((available_width - stoi(node->style.properties["width"])) / 2) : stoi(node->style.properties["margin"]);
    int padding = stoi(node->style.properties["padding"]);

    // box model values (all sides equal)
    //box->margin = { margin , margin, ((node->style.properties["margin"] == "auto") ? 0 : margin), ((node->style.properties["margin"] == "auto") ? 0 : margin)};
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
        int numLines = static_cast<int>(ceil(node->text.length())) / ((charsPerLine > 0) ? charsPerLine : 1);
        box->content_height = numLines * fontSize * 1.1f;
    }
    else
        box->content_height = y_offset;

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