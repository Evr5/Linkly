/**
 * @file text.hpp
 * @author Ethan Van Ruyskensvelde (Main developer),
           Bilal Vandenberge (Perfective maintenance)
 * @brief Handles displaying messages based on conditions
 * @date 2024
 *
 */

#ifndef TEXT_HPP
#define TEXT_HPP

#include <array>
#include <iostream>
#include <string>
#include <tuple>
using namespace std;

static constexpr char TAG_CHAR = '\\', BEGIN_CHAR = '{', END_CHAR = '}';
static const array<const tuple<string, int, bool>, 15> TAGS{{
    // Color tags: {name, ANSI style}
    {"red", 31, true},
    {"yellow", 33, true},
    {"green", 32, true},
    {"blue", 34, true},
    {"magenta", 35, true},
    {"cyan", 36, true},
    {"white", 37, true},
    {"black", 30, true},

    // Style tags:
    {"bold", 1, false},
    {"underline", 4, false},
    {"dim", 2, false},
    {"italic", 3, false},
    {"inverse", 7, false},
    {"hidden", 8, false},
    {"strikethrough", 9, false},
}};
static const string ANSI_RESET_CODE("\033[2"), ANSI_START_CODE("\033["),
    ANSI_FULL_RESET("\033[0m"), LINK_TAG("link"), LINK_BEGIN("\033]8;;"),
    LINK_MIDDLE("\033\\"), LINK_END("\033]8;;\033\\");

/**
 * @brief Handle formatting of a message based on tags and bot chat flag
 *
 * @note We can directly print an instance of the Text class.
 */
class Text {
  private:
    // #### Private attributes ####
    string nickname_;
    string message_;
    bool bot_;
    bool balise_;
    string formattedMessage_;

    // #### Private methods ####

    /**
     * @brief Format the message and initialize the attribute formattedMessage_
     * with the output (no tags when --balise is not activated)
     *
     */
    void formatNoBalise();

    /**
     * @brief Format the message and initialize the attribute formattedMessage_
     * with the output (with tags when --balise is activated)
     *
     */
    void formatBalise();

    // #### Helpers ####

    /**
     * @brief Search for tags in the raw message and change them into ANSI codes
     *
     * @note This is a recursive method (To handle nested tags). If the arg
     * index > 0, we are formatting the content of a tag
     *
     * @param index The begin index of the content to parse in the message
     * @param currentColor The color of the current content (or empty string if
     * none)
     * @return size_t The number of treated characters
     */
    string tagSearch(size_t &index,
                     const string &currentColor = ANSI_FULL_RESET);

    /**
     * @brief Return if a tag is present at message_[index]
     *
     * @param tag The tag
     * @param index The index in the message
     * @return bool (message_[index:index+tag.size] == tag )
     */
    bool tagAt(const string &tag, size_t index);

    /**
     * @brief Handle a \link tag at message_[index]
     *
     * @param index The index where the tag content begins (right after the
     * '\link' expression)
     * @param currentColor The color of the current content (or empty string if
     * none)
     * @return string The formatted link (or empty string if there is a
     * formatting error)
     * @warning A formatting error happens when the expression is incomplete
     * (missing braces)
     */
    string linkTag(size_t &index, const string &currentColor);

    /**
     * @brief Handle a basic tag (e.g. \red) at message_[index]
     *
     * @param index The index where the tag content begins (right after the
     * '\tag' expression)
     * @param currentColor The color of the current content (or empty string if
     * none)
     * @return string The formatted link (or empty string if there is a
     * formatting error)
     * @warning A formatting error happens when the expression is incomplete
     * (missing braces)
     */
    string basicTag(size_t &index, const string &currentColor);

    /**
     * @brief Build an ANSI escape code
     *
     * @param style The style of the code
     * @param reset Is the code reseting that style ?
     * @return string The ANSI escape code (or empty string if the bot mode is
     * enabled)
     */
    string ansiCode(int style, bool reset = false);

  public:
    /**
     * @brief Constructor with the nickname, the message and the bot
     *
     * @param nickname The user's nickname
     * @param message The message to display
     * @param bot True if the bot flag is toggled on
     * @param balise True if the balise flag is toggled on
     *
     * @note Format the message with the nickname depending on the bot flag
     */
    Text(const string &nickname, const string &message, bool bot = false,
         bool balise = false);
    Text(const string &message, bool balise = false);

    /**
     * @brief Default destructor
     *
     */
    virtual ~Text();

    /**
     * @brief Get the raw message
     *
     * @return string
     */
    virtual string in() const noexcept;

    /**
     * @brief Get the formatted message
     *
     * @return string
     */
    virtual string out() const noexcept;

    /**
     * @brief Get the name of the message sender
     *
     * @return string
     */
    virtual string who() const noexcept;

    /**
     * @brief Overload the operator<< to print the message
     *
     * @note If we print a empty formated message, it will format the message
     * the default way (using defaultFormat())
     */
    friend ostream &operator<<(ostream &os, const Text &text);
};

#endif // TEXT_HPP
