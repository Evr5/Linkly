/**
 * @file send_message.cpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Source file of sending routine
 * @date 2024
 *
 */

#include "send_message.hpp"
#include "../header/header.hpp"

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

SendMessageReturnVal sendMessage(int sockFd, const string &nickname,
                                 const string &message, uint8_t version) {

    uint8_t nicknameSize = nickname.size();
    uint16_t messageSize = message.size();
    uint16_t totalSize = sizeof(PacketHeader) + messageSize + nicknameSize;

    PacketHeader header{version, htons(totalSize), nicknameSize};

    struct iovec iov[3];
    iov[0].iov_base = &header;
    iov[0].iov_len = sizeof(header);
    iov[1].iov_base = const_cast<char *>(nickname.data());
    iov[1].iov_len = nicknameSize;
    iov[2].iov_base = const_cast<char *>(message.data());
    iov[2].iov_len = messageSize;

    size_t numBytes = sizeof(PacketHeader) + nicknameSize + messageSize,
           iovcnt = sizeof(iov) / sizeof(struct iovec);

    int bytesWritten = writev(sockFd, iov, iovcnt);

    if (bytesWritten == static_cast<int>(numBytes)) {
        return SendMessageReturnVal::SUCCESS;
    } else if (bytesWritten < 0) {
        if (errno != EINTR) {
            cerr << "Err: " << strerror(errno) << endl;
            return SendMessageReturnVal::WRITE_FAILED;
        }
    }

    return SendMessageReturnVal::COULD_NOT_WRITE_ALL_BYTES;
}
