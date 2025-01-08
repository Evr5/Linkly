/**
 * @file main.cpp
 * @author Bilal Vandenberge (Main developer)
 * @brief Main execution of the server
 * @date 2024
 *
 */

#include "server.hpp"

#include <iostream>
using namespace std;

int main() {
    Server &server = Server::getInstance();

    if (!server.init()) {
        cerr << "Err: erreur lors de l'initialisation du serveur." << endl;
        return 1;
    }

    return server.run();
}
