#pragma once
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

// Parse inital CSS
struct Tstyle {
    std::string selector;  // Example: ".container", "#header", "p"
    std::unordered_map<std::string, std::string> properties; // e.g., {"color", "red"}
};
// std::vector<Tstyle> stylesheet;


// struct so expansion can later be made
struct finalStyle {
    std::unordered_map<std::string, std::string> properties;
};

struct Node {
    std::string tag;
    std::string text;
    std::vector<std::shared_ptr<Node>> children;

    finalStyle style = { {{"padding", "0"}, {"margin", "0"}} };  // Store only resolved style
};


struct Box {
    // Full box dimensions (including margins)
    int outer_x, outer_y;
    int outer_width, outer_height;

    // Individual box model components
    struct {
        int left, right, top, bottom;
    } margin, padding;

    // Content area (inside padding)
    int content_x, content_y;
    int content_width, content_height;

    std::shared_ptr<Node> node;
    std::vector<std::shared_ptr<Box>> children;
};