/**
 * @file message_queue.hpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Header file of the MessageQueue class
 * @date 2024
 *
 */

#ifndef MESSAGE_QUEUE_HPP
#define MESSAGE_QUEUE_HPP

#include <pthread.h>
#include <queue>
#include <string>

using namespace std;

/**
 * @brief Represent a message
 */
struct Message {
    string sender, content;
};

/**
 * @class MessageQueue
 * @brief Store messages in FIFO order thread-safely.
 */
class MessageQueue {
  private:
    queue<Message> queue_;
    unsigned numBytes_ = 0; // the number of bytes enqueued
    pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;
    static constexpr unsigned MAX_QUEUE_SIZE = 4096;

  public:
    /**
     * @brief Construct a new MessageQueue object
     *
     */
    MessageQueue() = default;

    /**
     * @brief Destroy the MessageQueue object
     *
     */
    ~MessageQueue() = default;

    /**
     * @brief Push the given message in the queue.
     *
     * @param message The message to be pushed.
     */
    bool push(const Message &message);

    /**
     * @brief Push the given message in the queue.
     *
     * @param sender The sender's nickname.
     * @param messageContent The content of the message.
     */
    bool push(const string &sender, const string &messageContent);

    /**
     * @brief Return the number of enqueued bytes.
     */
    unsigned getNumBytes();

    /**
     * @brief Retrieve the message at the front of the queue without removing
     * it.
     */
    const Message &front();

    /**
     * @brief Remove the message at the front of the queue.
     *
     * @note The behavior is undefined if the queue is empty. Ensure the queue
     * is not empty before calling this function.
     */
    void pop();

    /**
     * @brief Check whether the queue is empty.
     */
    bool empty();
};

#endif
