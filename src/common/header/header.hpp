/**
 * @file header.hpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Header file of the PacketHeader struct
 * @date 2024
 *
 */

#ifndef HEADER_HPP
#define HEADER_HPP

#include <cstdint>

/**
 * @brief Represent the header of a packet.
 *
 * @note "__attribute__((packed))" removes padding, ensuring that the header is
 * 4 bytes long.
 */
struct __attribute__((packed)) PacketHeader {
    uint8_t version;
    uint16_t totalSize;
    uint8_t nicknameSize;
};

#endif // HEADER_HPP
