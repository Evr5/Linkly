/**
 * @file message_queue.cpp
 * @author Lucas Verbeiren (Main developer)
 * @brief Source file of the MessageQueue class
 * @date 2024
 *
 */

#include "message_queue.hpp"

bool MessageQueue::push(const Message &message) {
    size_t newMessageSize = message.sender.size() + message.content.size()
                            + 2; // +2 for the null-terminator

    pthread_mutex_lock(&mutex_);
    bool enoughSpace = (numBytes_ + newMessageSize < MAX_QUEUE_SIZE);
    if (enoughSpace) {
        queue_.push(message);
        numBytes_ += newMessageSize;
    }
    pthread_mutex_unlock(&mutex_);
    return enoughSpace;
}

bool MessageQueue::push(const string &sender, const string &messageContent) {
    Message message{sender, messageContent};
    return push(message);
}

unsigned MessageQueue::getNumBytes() {
    pthread_mutex_lock(&mutex_);

    unsigned numBytes = numBytes_;

    pthread_mutex_unlock(&mutex_);

    return numBytes;
}

const Message &MessageQueue::front() {
    pthread_mutex_lock(&mutex_);

    Message &message = queue_.front();

    pthread_mutex_unlock(&mutex_);

    return message;
}

void MessageQueue::pop() {
    pthread_mutex_lock(&mutex_);

    if (!queue_.empty()) {
        size_t poppedMessageSize = queue_.front().sender.size()
                                   + queue_.front().content.size()
                                   + 2; // +2 for the null-terminator
        queue_.pop();
        numBytes_ -= poppedMessageSize;
    }

    pthread_mutex_unlock(&mutex_);
}

bool MessageQueue::empty() {
    pthread_mutex_lock(&mutex_);

    bool isEmpty = queue_.empty();

    pthread_mutex_unlock(&mutex_);

    return isEmpty;
}
