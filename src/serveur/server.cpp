/**
 * @file server.cpp
 * @author Ethan Van Ruyskensvelde (Main developer)
 * @brief Header file for the server class
 * @date 05/12/2024
 *
 */

#include "server.hpp"
#include "../common/header/header.hpp"
#include "../common/receive_message/receive_message.hpp"
#include "../common/safe_write/safe_write.hpp"
#include "../common/signal/mask.hpp"

#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <sys/uio.h>
#include <unistd.h>

using namespace std;

volatile sig_atomic_t exitFlag = false;

// ### Constructors ###
Server::Server() = default;

// ### Destructor ###
Server::~Server() {
    disconnectAllClients();
    closeServerSocket();
    waitAllThreads();
    if (pthread_mutex_destroy(&mapMtx_) != 0
        or pthread_mutex_destroy(&fdMtx_) != 0) {
        cerr << "Err: échec de la destruction du mutex." << endl;
    }
    if (sem_destroy(&finishedThreads) != 0) {
        cerr << "Err: échec de la destruction du sémaphore." << endl;
    }
}

// ### Private methods ###

bool Server::startListening() {
    if (listen(serverSockFd_, ACCEPT_BACKLOG) != 0) {
        cerr << "Err: échec lors de l'écoute des connexions." << endl;
        return false;
    }
    return true;
}

int Server::acceptNewClient() {
    socklen_t addresslen = 0;

    int newClientSockFd = accept(serverSockFd_, nullptr, &addresslen);
    if (newClientSockFd < 0) {
        if (errno != EINTR and errno != EBADF) {
            cerr << "Err: Échec de l'acceptation du nouveau client." << endl;
        }
        return -1;
    }

    pthread_mutex_lock(&mapMtx_);
    bool reachedMaxClientsConnected =
        (mapSocketToClient_.size() >= MAX_CLIENTS_CONNECTED);
    pthread_mutex_unlock(&mapMtx_);

    if (reachedMaxClientsConnected) {
        cerr << "Err: Trop de clients connectés." << endl;
        if (close(newClientSockFd) != 0) {
            perror("close");
        }
        return -1;
    }

    // get the Nickname
    string nickname;
    string emptyMessage;
    if (receiveMessage(newClientSockFd, nickname, emptyMessage, CURRENT_VERSION)
        != ReceiveMessageReturnVal::SUCCESS) {
        cerr << "Err: Échec du serrage de main." << endl;
        if (close(newClientSockFd) != 0) {
            perror("close");
        }
        return -1;
    }

    uint8_t response = 1;
    if (findSocketByName(nickname) != -1) {
        cerr << "Err: Il y a déjà une connexion avec le nom d'utilisateur "
             << nickname << "." << endl;
        response = 0;
    }

    if (!safeWrite(newClientSockFd, reinterpret_cast<char *>(&response),
                   sizeof(response))) {
        cerr << "Err: La réponse n'a pas pu être envoyée." << endl;
        if (close(newClientSockFd) != 0) {
            perror("close");
        }
        return -1;
    }

    if (response == 0) return -1;

    pthread_mutex_lock(&mapMtx_);
    mapSocketToClient_[newClientSockFd] = nickname;
    pthread_mutex_unlock(&mapMtx_);

    cerr << "[+] Client connecté: " << nickname << endl;

    return newClientSockFd;
}

void Server::disconnectClient(int clientSockFd) {
    pthread_mutex_lock(&fdMtx_);
    pthread_mutex_lock(&mapMtx_);
    string nickname = mapSocketToClient_[clientSockFd];

    auto threadIt = mapSocketToThread_.find(clientSockFd);
    if (threadIt != mapSocketToThread_.end()) {
        mapSocketToThread_.erase(threadIt);
    } else {
        cerr << "Err: Le thread n'a pas été trouvé dans la liste." << endl;
    }

    auto clientIt = mapSocketToClient_.find(clientSockFd);
    if (clientIt != mapSocketToClient_.end()) {
        mapSocketToClient_.erase(clientIt);
    } else {
        cerr << "Err: Le socket n'a pas été trouvé dans la liste." << endl;
    }

    if (close(clientSockFd) != 0) {
        cerr << "Err: Échec de la fermeture du socket client - "
             << strerror(errno) << endl;
    }

    pthread_mutex_unlock(&mapMtx_);
    pthread_mutex_unlock(&fdMtx_);
    cerr << "[-] Client déconnecté: " << nickname << endl;
}

void Server::disconnectAllClients() {
    pthread_mutex_lock(&mapMtx_);
    auto copySTC(mapSocketToClient_);
    auto copySTT(mapSocketToThread_);
    pthread_mutex_unlock(&mapMtx_);

    for (const auto &pair : copySTC) {
        if (shutdown(pair.first, SHUT_RD) < 0) {
            cerr << "Err: Une connexion n'a pas pu être fermée." << endl;
            pthread_cancel(copySTT[pair.first]); //< Force quit
            sem_post(&finishedThreads);
        }
    }
}

void *Server::handleClientThreadFunc(void *arg) {
    int clientSockFd = reinterpret_cast<long>(arg);
    Server &server = Server::getInstance();

    if (not setSigMask(true)) { // Ignore signals
        server.disconnectClient(clientSockFd);
        sem_post(&server.finishedThreads);
        return nullptr;
    }

    pthread_mutex_lock(&server.mapMtx_);
    string nicknameSender = server.mapSocketToClient_[clientSockFd];
    pthread_mutex_unlock(&server.mapMtx_);

    ReceiveMessageReturnVal readMsgRet;

    string nicknameDest;
    string message;
    do {
        readMsgRet = receiveMessage(clientSockFd, nicknameDest, message,
                                    CURRENT_VERSION);
        if (static_cast<bool>(readMsgRet)) continue;

        int destSockFd = server.findSocketByName(nicknameDest);
        if (destSockFd == -1) {
            string emptyNickname;
            string disconnectedDestMessage =
                "Cette personne (" + nicknameDest + ") n'est pas connectée.";

            SendMessageReturnVal ret = server.sendMessage(
                clientSockFd, emptyNickname, disconnectedDestMessage);
            if (ret == SendMessageReturnVal::BROKEN_PIPE) {
                break;
            } else if (ret != SendMessageReturnVal::SUCCESS) {
                cerr << "Err: Échec de l'envoi du message signalant que "
                        "l'utilisateur n'est pas connecté."
                     << endl;
            }

        } else {
            SendMessageReturnVal ret =
                server.sendMessage(destSockFd, nicknameSender, message);
            if (ret == SendMessageReturnVal::BROKEN_PIPE) {
                break;
            } else if (ret != SendMessageReturnVal::SUCCESS) {
                cerr << "Err: Échec de l'envoi du message." << endl;
            }
        }
    } while (readMsgRet == ReceiveMessageReturnVal::SUCCESS);

    if (readMsgRet == ReceiveMessageReturnVal::MESSAGE_TOO_LONG) {
        server.sendTooLongMessage(clientSockFd);
    }

    server.disconnectClient(clientSockFd);
    sem_post(&server.finishedThreads);
    return nullptr;
}

void Server::signalHandler(int signal) {
    if (signal == SIGINT or signal == SIGTERM) {
        exitFlag = true;
    }
}

void Server::sendTooLongMessage(const int clientSockFd) {
    string emptyNickname;
    string tooLongMessageWarning = TOO_LONG_MESSAGE_WARNING;

    if (sendMessage(clientSockFd, emptyNickname, tooLongMessageWarning)
        != SendMessageReturnVal::SUCCESS) {
        cerr << "Err: échec de l'envoi de l'avertissement pour message trop "
                "long."
             << endl;
    }
}

void Server::closeServerSocket() {
    if (serverSockFd_ == -1) {
        return;
    }

    if (close(serverSockFd_) != 0) {
        cerr << "Err: échec de la fermeture du socket du serveur." << endl;
    }

    serverSockFd_ = -1;
}

int Server::findSocketByName(const string &nickname) {
    int ret = -1;

    pthread_mutex_lock(&mapMtx_);
    for (const auto &pair : mapSocketToClient_) {
        if (pair.second == nickname) {
            ret = pair.first;
            break;
        }
    }
    pthread_mutex_unlock(&mapMtx_);

    return ret;
}

// ### Public methods ###

bool Server::init() {
    if (sem_init(&finishedThreads, 0, 0) != 0) {
        cerr << "Err: Le sémaphore n'a pas pu être initialisé." << endl;
        return false;
    }

    // Get the port from the environment variable PORT_SERVEUR and if not found,
    // set default port to 1234
    port_ = DEFAULT_PORT;
    const char *port = getenv("PORT_SERVEUR");
    if (port) {
        int portNum = atoi(port);
        if (portNum > 1 && portNum < 65535) {
            port_ = portNum;
        }
    }

    // Create the socket
    serverSockFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSockFd_ < 0) {
        cerr << "Err: Le socket n'a pas pu être créé - " << strerror(errno)
             << endl;
        return false;
    }

    // Allow the reuse of the port
    int opt = 1;
    if (setsockopt(serverSockFd_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))
        != 0) {
        cerr << "Err: La réutilisation du port/adresse n'a pas pu être activée."
             << endl;
        return false;
    }

    struct sockaddr_in address;

    // Set the listening address and port, reserve the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<uint16_t>(port_));

    if (bind(serverSockFd_, reinterpret_cast<sockaddr *>(&address),
             sizeof(address))
        != 0) {
        cerr << "Err: Échec de la liaison entre le socket et le port." << endl;
        return false;
    }

    return initSignals();
}

int Server::run() {
    if (not setSigMask(false)) {
        return 1;
    }

    cerr << "Le serveur est en cours d'exécution." << endl;
    startListening();

    // Exit this loop when receiving SIGINT
    while (true) {
        handleSignalsSafely();
        int newClientSockFd = acceptNewClient();
        if (serverSockFd_ == -1) break; // Server shutting down

        pthread_mutex_lock(&fdMtx_);
        if (newClientSockFd > 0) {
            pthread_t thread;
            int ret = pthread_create(&thread, nullptr, handleClientThreadFunc,
                                     reinterpret_cast<void *>(newClientSockFd));

            if (ret == 0) {
                pthread_mutex_lock(&mapMtx_);
                mapSocketToThread_[newClientSockFd] = thread;
                pthread_mutex_unlock(&mapMtx_);

                if (pthread_detach(thread) != 0) { // Abort
                    if (shutdown(newClientSockFd, SHUT_RD) != 0) {
                        pthread_cancel(thread);
                        sem_post(&finishedThreads);
                    }
                }
                ++startedThreads;
            }
        }
        pthread_mutex_unlock(&fdMtx_);
    }

    return 0;
}

Server &Server::getInstance() {
    static Server instance;
    return instance;
}

SendMessageReturnVal Server::sendMessage(int destSockFd, const string &nickname,
                                         const string &message) {

    uint8_t nicknameSize = nickname.size();
    uint16_t messageSize = message.size();
    uint16_t totalSize = sizeof(PacketHeader) + messageSize + nicknameSize;

    PacketHeader header{CURRENT_VERSION, htons(totalSize), nicknameSize};

    struct iovec iov[3];
    iov[0].iov_base = &header;
    iov[0].iov_len = sizeof(header);
    iov[1].iov_base = const_cast<char *>(nickname.data());
    iov[1].iov_len = nicknameSize;
    iov[2].iov_base = const_cast<char *>(message.data());
    iov[2].iov_len = messageSize;

    size_t numBytes = sizeof(PacketHeader) + nicknameSize + messageSize,
           iovcnt = sizeof(iov) / sizeof(struct iovec);

    while (true) {
        int bytesWritten = writev(destSockFd, iov, iovcnt);

        if (bytesWritten == static_cast<int>(numBytes)) {
            return SendMessageReturnVal::SUCCESS;
        } else if (bytesWritten < 0) {
            if (errno == EPIPE) {
                return SendMessageReturnVal::BROKEN_PIPE;
            }

            if (errno != EINTR) {
                cerr << "Err: " << strerror(errno) << endl;
                return SendMessageReturnVal::WRITE_FAILED;
            }
        } else {
            return SendMessageReturnVal::COULD_NOT_WRITE_ALL_BYTES;
        }
    }
}

void Server::waitAllThreads() {
    unsigned max = startedThreads;
    for (unsigned i = 0; i < max; ++i) {
        sem_wait(&finishedThreads);
    }
    cerr << "Tous les clients ont été déconnectés." << endl;
    startedThreads = 0;
}

void Server::handleSignalsSafely() {
    if (exitFlag) {
        closeServerSocket();
        exitFlag = false;
    }
}

bool Server::initSignals() {
    // Handle signals
    struct sigaction sa;
    sa.sa_flags = 0; //< Disable SA_RESTART
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR
        or sigaction(SIGINT, &sa, NULL) == -1
        or sigaction(SIGTERM, &sa, NULL) == -1) {
        cerr << "Err: Échec de l'assignation de gestionnaire de signaux"
             << endl;
        return false;
    }
    return true;
}
