/**
 * @file receive_message.hpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Header file of receiving routine
 * @date 2024
 *
 */

#ifndef RECEIVE_MESSAGE_CPP
#define RECEIVE_MESSAGE_CPP

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <sys/uio.h>
#include <unistd.h>

using namespace std;

/**
 * @brief Return value of the receiving routine
 *
 */
enum class ReceiveMessageReturnVal {
    SUCCESS = 0,
    NICKNAME_TOO_LONG,
    MESSAGE_TOO_LONG,
    READ_ERROR,
    INVALID_VERSION
};

/**
 * @brief Read one message (and nickname) into the given nickname and message
 * buffers.
 *
 * @param sockFd The socket to read from.
 * @param nickname The nickname buffer.
 * @param message The message buffer.
 * @param version The protocol version.
 *
 * @return ReceiveMessageReturnVal An enum holding values for success and
 * different errors.
 */
ReceiveMessageReturnVal receiveMessage(int sockFd, string &nickname,
                                       string &message, uint8_t version);

#endif
