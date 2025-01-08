/**
 * @file safe_read.cpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Source file of read syscall
 * @date 2024
 *
 */

#include "safe_read.hpp"

#include <cerrno>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
using namespace std;

bool safeRead(int fd, char *buffer, size_t size) {
    unsigned readCount = 0;
    int ret;
    do {
        ret = read(fd, &buffer[readCount], size - readCount);

        if (ret == 0) {
            return false;
        } else if (ret > 0) {
            readCount += ret;
        } else {
            if (errno != EINTR) {
                if (errno == ECONNRESET)
                    cerr << "Err: L'autre partie a annulé la connexion."
                         << endl;
                else perror("read");
                return false;
            }
        }

    } while (readCount < size);

    return true;
}
