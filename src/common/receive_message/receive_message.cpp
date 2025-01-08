/**
 * @file receive_message.cpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Source file of receiving routine
 * @date 2024
 *
 */

#include "receive_message.hpp"
#include "../../serveur/server.hpp"
#include "../header/header.hpp"
#include "../safe_read/safe_read.hpp"

#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <sys/uio.h>
#include <unistd.h>

using namespace std;

ReceiveMessageReturnVal receiveMessage(int sockFd, string &nickname,
                                       string &message, uint8_t version) {
    PacketHeader header;
    if (!safeRead(sockFd, reinterpret_cast<char *>(&header),
                  sizeof(PacketHeader))) {
        return ReceiveMessageReturnVal::READ_ERROR;
    }

    uint16_t totalSize = ntohs(header.totalSize);
    if (header.version != version) {
        cerr << "Err: version incorrecte" << endl;
        return ReceiveMessageReturnVal::INVALID_VERSION;
    }
    uint8_t nicknameSize = header.nicknameSize;
    uint16_t messageSize = totalSize - sizeof(PacketHeader) - nicknameSize;

    if (nicknameSize > MAX_LENGTH_NICKNAME) {
        cerr << "Err: Pseudo trop long." << endl;
        return ReceiveMessageReturnVal::NICKNAME_TOO_LONG;
    }
    if (messageSize > MAX_LENGTH_MESSAGE) {
        cerr << "Err: Message trop long." << endl;
        return ReceiveMessageReturnVal::MESSAGE_TOO_LONG;
    }

    // allocate enough space in the buffers to read into them
    nickname.resize(nicknameSize);
    message.resize(messageSize);

    if (nicknameSize > 0) {
        if (!safeRead(sockFd, nickname.data(), nicknameSize)) {
            return ReceiveMessageReturnVal::READ_ERROR;
        }
    }

    if (messageSize > 0) {
        if (!safeRead(sockFd, message.data(), messageSize)) {
            return ReceiveMessageReturnVal::READ_ERROR;
        }
    }

    return ReceiveMessageReturnVal::SUCCESS;
}
