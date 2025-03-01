#pragma once
#include "dom.hpp"

std::shared_ptr<Box> CreateLayoutTree(const std::shared_ptr<Node>& node, int width);