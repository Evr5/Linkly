/**
 * @file text.hpp
 * @author Ethan Van Ruyskensvelde (Main developer),
           Bilal Vandenberge (Perfective maintenance)
 * @brief Handles displaying messages based on conditions
 * @date text
 *
 */

#include "text.hpp"

using namespace std;

// #### Private Methods ####

void Text::formatNoBalise() {
    formattedMessage_ = (bot_ ? "[" : "[\x1B[4m") + nickname_
                        + (bot_ ? "] " : "\x1B[0m] ")
                        + (bot_ ? "" : ANSI_FULL_RESET) + message_;
}

void Text::formatBalise() {
    size_t index = 0;
    formattedMessage_ = (bot_ ? "[" : "[\x1B[4m") + nickname_
                        + (bot_ ? "] " : "\x1B[0m] ") + tagSearch(index)
                        + (bot_ ? "" : ANSI_FULL_RESET);
}

string Text::tagSearch(size_t &index, const string &currentColor) {
    const bool inTag = static_cast<bool>(index); //< Tag nesting
    if (inTag) {
        if (message_[index] != BEGIN_CHAR) {
            // The tag content does not start with '{'
            return "";
        } else ++index;
    }

    string output;
    for (size_t current = index; current < message_.size(); ++current) {
        if (inTag and message_[current] == END_CHAR) {
            // Only end on a "}" if we are not in a primary tags search
            index = current + 1;
            return output;
        } else if (message_[current] == TAG_CHAR) {
            size_t subindex = current + 1;
            string suboutput = basicTag(subindex, currentColor);
            if (suboutput.empty()) {
                output.push_back(TAG_CHAR);
            } else {
                current = subindex - 1;
                output.insert(output.end(), suboutput.begin(), suboutput.end());
                output.insert(output.end(), currentColor.begin(),
                              currentColor.end());
            }
        } else {
            output.push_back(message_[current]);
        }
    }

    return inTag ? "" : output;
}

bool Text::tagAt(const string &tag, size_t index) {
    if (message_.size() - index < tag.size()) {
        return false;
    }

    for (char chr : tag) {
        if (message_[index] != chr) {
            return false;
        }
        ++index;
    }

    return true;
}

string Text::basicTag(size_t &index, const string &currentColor) {
    for (const auto &tag : TAGS) {
        if (tagAt(get<0>(tag), index)) {
            size_t subindex = index + get<0>(tag).size();
            string suboutput = tagSearch(
                subindex, (get<2>(tag) ? ansiCode(get<1>(tag)) : currentColor));
            if (suboutput.empty()) {
                return suboutput;
            } else {
                index = subindex;
                return ansiCode(get<1>(tag)) + suboutput
                       + (not get<2>(tag) ? ansiCode(get<1>(tag), true) : "");
            }
        }
    }
    size_t subindex = index;
    string output = linkTag(subindex, currentColor);
    if (output.size()) index = subindex;
    return output;
}

string Text::linkTag(size_t &index, const string &currentColor) {
    if (not tagAt(LINK_TAG, index)) {
        return "";
    }
    index += LINK_TAG.size();
    size_t subindex = index;
    string txt = tagSearch(subindex, currentColor);
    if (txt.empty() or subindex >= message_.size()
        or message_[subindex] != BEGIN_CHAR)
        return ""; // Syntax error
    auto end = message_.find(END_CHAR, subindex);
    if (end >= message_.size()) return "";
    string link = message_.substr(subindex + 1, end - (subindex + 1));
    index = end + 1;
    return LINK_BEGIN + link + LINK_MIDDLE + txt + LINK_END;
}

string Text::ansiCode(int style, bool reset) {
    return bot_ ? ""
                : ((reset ? ANSI_RESET_CODE : ANSI_START_CODE)
                   + to_string(style) + "m");
}

// #### Public Methods ####

Text::Text(const string &nickname, const string &message, bool bot, bool balise)
    : nickname_(nickname), message_(message), bot_(bot), balise_(balise) {
    balise_ ? formatBalise() : formatNoBalise();
}
Text::Text(const string &message, bool balise)
    : message_(message), balise_(balise) {
    size_t index = 0;
    formattedMessage_ = balise_ ? tagSearch(index) : message;
}

Text::~Text() = default;

string Text::out() const noexcept { return formattedMessage_; }

string Text::in() const noexcept { return message_; }

string Text::who() const noexcept { return nickname_; }

ostream &operator<<(ostream &os, const Text &text) {
    os << text.formattedMessage_;
    return os;
}
