/**
 * @file client.hpp
 * @author Ethan Van Ruyskensvelde (Main developer)
 * @brief Define the class Client
 * @date 05/12/2024
 *
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "arg_parser.hpp"
#include "flags.hpp"
#include "message_queue/message_queue.hpp"
#include "text.hpp"

#include <arpa/inet.h>
#include <atomic>
#include <csignal>
#include <memory.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <unistd.h>

using namespace std;

constexpr int DEFAULT_PORT = 1234;        // Default port
constexpr int BUFFER_SIZE_MESSAGE = 1024; // Buffer size for the message
constexpr int MAX_LENGTH_PSEUDO = 30;     // Max length of the pseudo
constexpr uint8_t CURRENT_VERSION = 1;

enum class ConnectionState {
    Disconnected, // before the call to connect() or after logOut()
    Connecting,   // once connect() is done
    Connected,    // once logOn() is done
};

class Client {
  private:
    atomic_int sockFd_ = -1;
    sockaddr_in serverAddrIn_;
    const ChatFlags &flags_;
    pthread_t receiveThread_ = 0;
    pthread_mutex_t printMtx_ = PTHREAD_MUTEX_INITIALIZER;
    atomic<ConnectionState> connectionState_ = ConnectionState::Disconnected;
    string nickname_;
    MessageQueue queue_;
    atomic<int> exitCode_ = 0;

    /**
     * @brief Retreive IP & Port configuration.
     */
    void readIpConfig();

    /**
     * @brief Receive messages.
     */
    void receiveMessages();

    /**
     * @brief Send messages.
     */
    void sendMessages();

    /**
     * @brief Safely print a string on STDOUT on one line
     * @details Prevent concurrency between threads
     *
     * @param content The content
     * @param onSTDERR If the text is printed on STDERR
     */
    void safePrint(const Text &content, bool onSTDERR = false);

    // ### Thread Function ###

    /**
     * @brief Thread function to receive messages from the server.
     *
     * @param arg A pointer to the Client object to pass to the thread.
     */
    static void *receiveMessagesThreadFunc(void *arg);

    // ### Signals ####

    /**
     * @brief Signal handler (quick)
     *
     * @param signal The signal to handle
     */
    static void signalHandler(int signal);

    /**
     * @brief To handle signals (not the handler)
     *
     * @param canBeEOF Flag that indiquates if this can be called after a Ctrl+D
     * or end of socket
     */
    void handleSignalsSafely(bool canBeEOF = false);

    /**
     * @brief Init signal handlers
     *
     * @return bool If the operation succeded
     */
    bool initSignals();

    // #### --manuel flag ###

    /**
     * @brief Print the content of and empty the message queue.
     */
    void flushQueue();

    /**
     * @brief Add a new message in the message queue.
     *
     * @param message The message
     */
    void addToQueue(const Message &message);

    /**
     * @brief Edit a text and remove all "-" characters
     *
     * @param txt The text
     */
    static void removeHyphens(string &txt);

  public:
    // ### Constructors and destructor ###

    /**
     * @brief Constructor for Client
     *
     * @param args The arguments given to the program
     */
    Client(const ArgParser &args);

    /**
     * @brief Destrucctor for Client
     */
    ~Client();

    // ### Methods ###

    /**
     * @brief Run the client
     */
    void run();

    /**
     * @brief Log the client out properly
     *
     */
    void logOut();

    /**
     * @brief Log the client on properly
     *
     * @return true Operation succeded
     * @return false Operation failed
     */
    bool logOn();

    // ### Getters ####

    /**
     * @brief Get the state of the connection
     *
     * @return ConnectionState
     */
    ConnectionState getState() const;

    /**
     * @brief Get the exit code of the last execution of the client
     *
     * @return int
     */
    int getExitCode() const;
};

#endif
