/**
 * @file main.cpp
 * @author Bilal Vandenberge (Main developer)
 * @brief Main execution of the client
 * @date 2024
 *
 */

#include "arg_parser.hpp"
#include "client.hpp"

int main(int argc, char *argv[]) {
    ArgParser args(argc, argv);
    Client client(args);
    if (client.logOn()) {
        client.run();
    }

    return client.getExitCode();
}
