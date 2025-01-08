/**
 * @file send_message.hpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Header file of sending routine
 * @date 2024
 *
 */

#ifndef SEND_MESSAGE_HPP
#define SEND_MESSAGE_HPP

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
 * @brief Return value of the sending routine
 *
 */
enum class SendMessageReturnVal {
    SUCCESS = 0,
    COULD_NOT_WRITE_ALL_BYTES,
    WRITE_FAILED,
    BROKEN_PIPE
};

/**
 * @brief Send the given message with the given nickname to the given recipient.
 *
 * @param sockFd The recipient's socket.
 * @param nickname The nickname associated with the message.
 * @param nickname The message.
 * @param nickname The protocol version.
 */
SendMessageReturnVal sendMessage(int sockFd, const string &nickname,
                                 const string &message, uint8_t version);

#endif
