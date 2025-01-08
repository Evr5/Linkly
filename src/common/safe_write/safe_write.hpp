/**
 * @file safe_write.hpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Header file of write syscall
 * @date 2024
 *
 */

#ifndef SAFE_WRITE_HPP
#define SAFE_WRITE_HPP

#include <stdio.h>
#include <unistd.h>

/**
 * @brief Write size bytes from the buffer into fd.
 *
 * @return True if size bytes were written; otherwise, false.
 */
bool safeWrite(int fd, const char *buffer, size_t size);

#endif
