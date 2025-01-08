/**
 * @file mask.cpp
 * @author Bilal Vandenberge (Main developer)
 * @brief Source file for sigmask operations
 * @date 2024
 *
 */

#include "mask.hpp"

#include <iostream>
#include <signal.h>
using namespace std;

bool setSigMask(bool block) {
    sigset_t emptySet;
    sigaddset(&emptySet, SIGINT);
    sigaddset(&emptySet, SIGPIPE);
    sigaddset(&emptySet, SIGTERM);
    if (pthread_sigmask(block ? SIG_BLOCK : SIG_UNBLOCK, &emptySet, NULL)
        != 0) {
        cerr << "Err: Le programme ne peut altérater son masque de signaux."
             << endl;
        return false;
    }
    return true;
}
