#include "parser.hpp"
#include <iostream>
#include <functional>

// ADD: Whitespace trim for complex CSS
std::string TrimWhitespace(const std::string& str) {
    const size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) 
        return "";

    return str.substr(first, (str.find_last_not_of(" \t") - first + 1));
}

std::vector<Tstyle> ParseCSS(const std::string& css) {
    std::vector<Tstyle> css_rules;

    // Extract CSS content between <style> tags
    const size_t style_start = css.find("<style>");
    const size_t style_end = css.find("</style>");

    if (style_start == std::string::npos || style_end == std::string::npos)
        return css_rules;

    const std::string style_content = css.substr(
        style_start + 7,  // Length of "<style>"
        style_end - (style_start + 7)
    );

    // CSS parsing states
    enum class ParseState {
        Selector,
        PropertyName,
        PropertyValue
    };

    ParseState state = ParseState::Selector;
    Tstyle current_rule;
    std::string text;
	std::string property_name;
    char previous_char = '\0';


    for (const char c : style_content) {
        if (c == ' ') // Beginning and ending white space will be needed to trim for complex css
            continue;

        // Add comment handling

        switch (state) {
            case ParseState::Selector:
                if (c == '{') {
                    current_rule.selector = text;
                    text.clear();
                    state = ParseState::PropertyName;
                }
                else if (c != '\n' && c != '\r')
                    text += c;

                break;

            case ParseState::PropertyName:
                if (c == ':')
                    state = ParseState::PropertyValue;
                else if (c == '}') {
                    css_rules.push_back(current_rule);
                    current_rule = Tstyle();
                    state = ParseState::Selector;
                }
                else
                    property_name += c;

                break;

            case ParseState::PropertyValue:
                if (c == ';') {

                    current_rule.properties[property_name] = text;
                    text.clear();
					property_name.clear();
                    state = ParseState::PropertyName;
                }
                else
                    text += c;

                break;
        }

        previous_char = c; // Use for comment handling
    }

    // Debug output
    for (const auto& rule : css_rules) {
        std::cout << "Selector: " << rule.selector << "\n";
        for (const auto& property : rule.properties) {
            std::cout << "  " << property.first << ": " << property.second << "\n";
        }
    }

    return css_rules;
}

// Parse HTML
std::shared_ptr<Node> ParseHTML(const std::string& html) {
    std::vector<Token> tokens;
    std::string buffer;
    bool in_tag = false;
    auto root = std::make_shared<Node>();
    std::vector<std::shared_ptr<Node>> nodeStack{ root };

    const size_t body_start = html.find("<body>");
    const size_t body_end = html.find("</body>");

    const std::string body = html.substr(
        body_start, 
        body_end - (body_start)
    );

    // Parse into token
    for (char c : body) {
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

    // Tokens into DOM Tree
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

    return root->children[0]; // return body tag as first instead of blank space
}

void CombineHTMLCSS(std::shared_ptr<Node>& node, const std::vector<Tstyle>& css) {
    for (const auto& rule : css) {
        if (node->tag == rule.selector) {
            for (const auto& property : rule.properties) {
                node->style.properties[property.first] = property.second;
            }
        }
    }
    for (auto& child : node->children) // Remove const qualifier
        CombineHTMLCSS(child, css);
}