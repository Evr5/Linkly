/**
 * @file server.hpp
 * @author Ethan Van Ruyskensvelde (Main developer)
 * @brief Header file for the server class
 * @date 05/12/2024
 *
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "../common/send_message/send_message.hpp"

#include <atomic>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <sys/types.h>
#include <unordered_map>

using namespace std;

constexpr int DEFAULT_PORT = 1234;
constexpr int ACCEPT_BACKLOG = 5;
constexpr int MAX_CLIENTS_CONNECTED = 1000;
constexpr int MAX_LENGTH_MESSAGE = 1024;
constexpr int MAX_LENGTH_NICKNAME = 30;
constexpr uint8_t CURRENT_VERSION = 1;
const string TOO_LONG_MESSAGE_WARNING = "Votre message est trop long !";

class Server {
  private:
    int serverSockFd_;
    int port_;

    pthread_mutex_t mapMtx_ PTHREAD_MUTEX_INITIALIZER,
        fdMtx_ PTHREAD_MUTEX_INITIALIZER;

    sem_t finishedThreads;
    atomic<unsigned> startedThreads = 0;

    /**
     * @brief Map each socket to the name of its corresponding client.
     */
    unordered_map<int, string> mapSocketToClient_;

    /**
     * @brief Map each socket to the thread responsible of handling that
     * socket/client.
     */
    unordered_map<int, pthread_t> mapSocketToThread_;

    /**
     * @brief Wait for all running threads to end their execution
     */
    void waitAllThreads();

    /**
     * @brief Start listening for incoming connections.
     * Exit if the call to listen fails.
     */
    bool startListening();

    /**
     * @brief Accept a new client.
     *
     * @return int Return the new client socket in case of success; otherwise,
     * -1.
     */
    int acceptNewClient();

    /**
     * @brief Close the specified client socket and removes the corresponding
     * thread from the map of threads.
     * This function doesn't stop the thread that was handling the given client.
     *
     * @param clientSockFd The socket of the client to disconnect.
     */
    void disconnectClient(int clientSockFd);

    /**
     * @brief Disconnect all the connected clients.
     * This function makes every tread for every client stop.
     */
    void disconnectAllClients();

    /**
     * @brief Thread function to handle a client.
     *
     * @param arg The client socket cast to a void*.
     * @return void* Return a pointer to void.
     */
    static void *handleClientThreadFunc(void *arg);

    /**
     * @brief Handle signals.
     *
     * @param signal The signal to handle.
     */
    static void signalHandler(int signal);

    /**
     * @brief Send a message to the client notifying them that their message is
     * too long.
     *
     * @param clientSockFd The client's socket.
     */
    void sendTooLongMessage(const int clientSockFd);

    /**
     * @brief close the server's socket.
     */
    void closeServerSocket();

    /**
     * @brief Find the socket corresponding to the given client.
     *
     * @param nickname The client's nickname.
     *
     * @return int The socket if it was found; otherwise, -1.
     */
    int findSocketByName(const string &nickname);

    /**
     * @brief Send a message associated with a nickname to the client
     * associated with the given socket.
     *
     * @param destSockFd The client's sockets.
     * @param nickname The nickname.
     * @param message The message.
     *
     * @return SendMessageReturnVal An enum that holds values for success and
     * the possible errors.
     */
    SendMessageReturnVal sendMessage(int destSockFd, const string &nickname,
                                     const string &message);

    /**
     * @brief Handle signals received
     *
     */
    void handleSignalsSafely();

    /**
     * @brief Initialize signal handler
     *
     * @return bool If the operation succeded
     */
    bool initSignals();

  public:
    /**
     * @brief Construct a new Server object.
     */
    Server();

    /**
     * @brief Destroy the Server object.
     */
    ~Server();

    /**
     * @brief Initialize a Server object.
     */
    bool init();

    /**
     * @brief Run the server.
     *
     * @return int The exit code of the server
     */
    int run();

    /**
     * @brief Get the instance of the server.
     *
     * @return Server& Return a reference of the instance of the server.
     */
    static Server &getInstance();
};

#endif // SERVER_HPP
