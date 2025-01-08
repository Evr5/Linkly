/**
 * @file mask.hpp
 * @author Bilal Vandenberge (Main developer)
 * @brief Header file for sigmask operations
 * @date 2024
 *
 */

#ifndef SIGMASK_HPP
#define SIGMASK_HPP

/**
 * @brief block/unblock signal handling in current thread
 *
 * @param block If the mask is blocking (1) or not (0) the signals
 */
bool setSigMask(bool block);

#endif