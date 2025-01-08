/**
 * @file flags.hpp
 * @author Bilal Vandenberge (Main developer)
 * @brief Header file for the ChatFlags struct
 * @date 2024
 *
 */

#ifndef FLAGS_HPP
#define FLAGS_HPP

#include <atomic>
using namespace std;

/**
 * @brief Store the options given with the flags --bot and --manuel
 *
 */
struct ChatFlags {
    atomic_bool bot, manuel, balise;

    // #### Constructor: ####

    /**
     * @brief Construct a new ChatFlags object
     *
     * @param b The --bot flag
     * @param m  The --manuel flag
     * @param ba The --balise flag
     *
     */
    ChatFlags(bool b = false, bool m = false, bool ba = false);

    /**
     * @brief The equality operator
     *
     * @param other The other ChatFlags object
     * @return bool (this == other)
     */
    bool operator==(const ChatFlags &other) const noexcept;
};

#endif
