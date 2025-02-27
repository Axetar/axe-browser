#pragma once
#include <vector>
#include <string>
#include "dom.hpp"

enum TokenType { TAG_OPEN, TAG_CLOSE, TEXT };

struct Token {
    enum TokenType type;
    std::string value;
};

std::vector<Token> Tokenize(const std::string& html);
std::shared_ptr<Node> ParseTokens(const std::vector<Token>& tokens);