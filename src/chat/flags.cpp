/**
 * @file flags.hpp
 * @author Bilal Vandenberge (Main developer)
 * @brief Source file for the ChatFlags struct
 * @date 2024
 *
 */

#include "flags.hpp"

ChatFlags::ChatFlags(bool b, bool m, bool ba) : bot(b), manuel(m), balise(ba) {}

bool ChatFlags::operator==(const ChatFlags &other) const noexcept {
    return (this->bot == other.bot) and (this->manuel == other.manuel)
           and (this->balise == other.balise);
}