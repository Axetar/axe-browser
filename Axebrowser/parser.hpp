#pragma once
#include <vector>
#include <string>
#include "dom.hpp"

enum TokenType { TAG_OPEN, TAG_CLOSE, TEXT };

struct Token {
    enum TokenType type;
    std::string value;
};

void CombineHTMLCSS(std::shared_ptr<Node>& node, const std::vector<Tstyle>& css);
std::vector<Tstyle> ParseCSS(const std::string& css);
std::shared_ptr<Node> ParseHTML(const std::string& html);