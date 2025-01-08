/**
 * @file safe_read.hpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Header file of read syscall
 * @date 2024
 *
 */

#ifndef SAFE_READ_HPP
#define SAFE_READ_HPP

#include <stdio.h>
#include <unistd.h>

/**
 * @brief Read size bytes from fd into buffer.
 *
 * @note This function restarts the read if it was interrupted by a sys-call.
 * Do not call this function with size=0.
 *
 * @return True only if size bytes were read;
 * false if EOF was read or less than size bytes could be read.
 */
bool safeRead(int fd, char *buffer, size_t size);

#endif
