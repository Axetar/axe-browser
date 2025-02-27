#include "parser.hpp"

std::vector<Token> Tokenize(const std::string& html) {
    std::vector<Token> tokens;
    std::string buffer;
    bool in_tag = false;

    for (char c : html) {
        if (c == '<') {
            if (!buffer.empty()) {
                tokens.push_back({ TEXT, buffer });
                buffer.clear();
            }
            in_tag = true;
        }
        else if (c == '>') {
            if (!buffer.empty()) {
                if (buffer[0] == '/')
                    tokens.push_back({ TAG_CLOSE, buffer.substr(1) });
                else
                    tokens.push_back({ TAG_OPEN, buffer });
                buffer.clear();
            }
            in_tag = false;
        }
        else
            buffer += c;
    }

    return tokens;
}

std::shared_ptr<Node> ParseTokens(const std::vector<Token>& tokens) {
    auto root = std::make_shared<Node>();
    std::vector<std::shared_ptr<Node>> nodeStack{ root };

    for (const auto& token : tokens) {
        if (token.type == TAG_OPEN) {
            auto newNode = std::make_shared<Node>();
            newNode->tag = token.value;
            nodeStack.back()->children.push_back(newNode);
            nodeStack.push_back(newNode);
        }
        else if (token.type == TAG_CLOSE && nodeStack.size() > 1)
            nodeStack.pop_back();
        else if (token.type == TEXT)
            nodeStack.back()->text = token.value;
    }

    return root;
}