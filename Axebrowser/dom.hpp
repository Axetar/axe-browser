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


// Style that goes into html
struct finalStyle {
    std::unordered_map<std::string, std::string> properties; // e.g., {"color", "red", "font-size", "16px"}
};

struct Node {
    std::string tag;
    std::string text;
    std::vector<std::shared_ptr<Node>> children;

    finalStyle style = { { {"padding", "15"} } };  // Store only resolved style
};


struct Box {
    int x, y = 0;
    int width, height = 0;
    std::shared_ptr<Node> node;
    std::vector<std::shared_ptr<Box>> children;
};