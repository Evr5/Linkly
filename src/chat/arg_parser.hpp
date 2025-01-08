/**
 * @file arg_parser.hpp
 * @author Ethan Van Ruyskensvelde (Main developer)
 * @brief Handle and check the arguments given to the program
 * @date 01/11/2024
 *
 */

#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

#include "flags.hpp"

#include <array>
#include <string>
using namespace std;

static constexpr int MAX_NAME_LENGTH =
    30; // Maximum length of the nickname is 30 bytes

/**
 * @brief Parse and store program arguments as the speaker name, listener name
 * and --bot and --manuel flags
 *
 * @note Exit the program if the argument content or number are invalid
 */
class ArgParser {
  private:
    string speaker_ = "speaker";
    int argc_;
    char **argv_;
    ChatFlags flags_;

    /**
     * @brief Exit if too few or too many arguments
     */
    void checkArgc() const;

    /**
     * @brief Exit if a nickname is invalid
     */
    void checkName(const string &name) const;

    // #### Internal Setters ####

    /**
     * @brief Set the given flag or exit if it is invalid
     */
    void setFlag(const char *const flag);

    /**
     * @brief Set all the flags
     */
    void setFlags();

    /**
     * @brief Set all the names
     */
    void setNames();

  public:
    static constexpr array<const char, 4> INVALID_CHARS{'/', '-', '[', ']'};
    static constexpr array<const char *, 2> INVALID_NAMES{".", ".."};
    static constexpr char const *BOT_FLAG{"--bot"}, *MANUEL_FLAG{"--manuel"},
        *BALISE_FLAG{"--balise"};

    // #### Constructors and destructor ####

    /**
     * @brief Construct a new ArgParser object
     *
     */
    ArgParser() = default;

    /**
     * @brief Construct a new ArgParser object
     */
    ArgParser(int argc, char **argv);

    // #### Destructor ####

    /**
     * @brief Destruct an ArgParser object
     */
    virtual ~ArgParser();

    // #### Getters #####

    /**
     * @brief Get the --bot and --manuel flags
     */
    virtual const ChatFlags &getFlags() const noexcept;

    /**
     * @brief Get the speaker name
     */
    virtual string getSpeaker() const noexcept;
};

#endif
