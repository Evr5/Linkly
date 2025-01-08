/**
 * @file safe_write.cpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Source file of write syscall
 * @date 2024
 *
 */

#include "safe_write.hpp"

#include <cstring>
#include <errno.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;

bool safeWrite(int fd, const char *buffer, size_t size) {
    while (true) {
        int bytes_written = write(fd, buffer, size);

        if (bytes_written == static_cast<int>(size)) {
            return true;
        } else if (bytes_written < 0) {
            if (errno != EINTR) {
                if (errno == EPIPE)
                    cerr << "Err: L'autre partie a cassé la connexion"
                            "pendant l'écriture"
                         << endl;
                else perror("write");
                return false;
            }
        } else {
            return false;
        }
    }
}
