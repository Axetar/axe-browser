#pragma once
#include "dom.hpp"

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int available_width, int available_height, int parent_x = 0, int parent_y = 0);