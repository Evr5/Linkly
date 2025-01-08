/**
 * @file arg_parser.cpp
 * @author Ethan Van Ruyskensvelde (Main developer)
 * @brief Handle and check the arguments given to the program
 * @date 01/11/2024
 *
 */

#include "arg_parser.hpp"

#include <boost/range/algorithm/find.hpp>
#include <cstring>
#include <iostream>
#include <string>
using namespace std;

// ##### Private Methods ####

void ArgParser::checkArgc() const {
    if (argc_ < 2) {
        cerr << "chat pseudo_utilisateur [--bot] [--manuel]" << endl;
        exit(1);
    } else if (argc_ > 5) {
        cerr << "Err: Trop d'arguments donnés." << endl;
        exit(9);
    }
}

void ArgParser::checkName(const string &name) const {
    if (name.size() > MAX_NAME_LENGTH) {
        cerr << "Err: Le pseudo de l'utilisateur '" + name + "' est trop long."
             << endl;
        exit(2);
    }

    if (boost::range::find(INVALID_NAMES, name) != INVALID_NAMES.end()) {
        cerr << "Err: Le pseudo de l'utilisateur '" + name + "' est interdit"
             << endl;
        exit(3);
    }

    bool containsInvalidChar =
        any_of(INVALID_CHARS.begin(), INVALID_CHARS.end(), [&](char c) {
            return boost::range::find(name, c) != name.end();
        });
    if (containsInvalidChar) {
        cerr << "Err: Le pseudo de l'utilisateur '" + name
                    + "' contient des caractères interdits"
             << endl;
        exit(3);
    }
}

// #### Internal Setters ####

void ArgParser::setFlag(const char *const flag) {
    if (strcmp(flag, BOT_FLAG) == 0) {
        flags_.bot = true;
    } else if (strcmp(flag, MANUEL_FLAG) == 0) {
        flags_.manuel = true;
    } else if (strcmp(flag, BALISE_FLAG) == 0) {
        flags_.balise = true;

    } else {
        cerr << "Err: L'argument '" + string{flag} + "' est inconnu." << endl;
        exit(13);
    }
}

void ArgParser::setFlags() {
    for (int i = 2; i < argc_; i++) setFlag(argv_[i]);
}

void ArgParser::setNames() {
    speaker_ = argv_[1];

    checkName(speaker_);
}

// ##### Public Methods ####

ArgParser::ArgParser(int argc, char *argv[]) : argc_{argc}, argv_{argv} {
    checkArgc();
    setNames();
    setFlags();
}

ArgParser::~ArgParser() = default;

const ChatFlags &ArgParser::getFlags() const noexcept { return flags_; }

string ArgParser::getSpeaker() const noexcept { return speaker_; }
