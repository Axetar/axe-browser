#pragma once
#include <memory>
#include <vector>
#include <string>

struct Node {
    std::string tag;
    std::string text;
    std::vector<std::shared_ptr<Node>> children;
};

struct Box {
    int x, y = 0;
    int width, height = 0;
    std::shared_ptr<Node> node;
    std::vector<std::shared_ptr<Box>> children;
};