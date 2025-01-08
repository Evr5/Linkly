/**
 * @file client.cpp
 * @author Ethan Van Ruyskensvelde (Main developer),
 *         Bilal Vandenberge (Perfective maintenance)
 * @brief Define the class Client
 * @date 05/12/2024
 *
 */

#include "client.hpp"
#include "../common/receive_message/receive_message.hpp"
#include "../common/safe_read/safe_read.hpp"
#include "../common/send_message/send_message.hpp"
#include "../common/signal/mask.hpp"
#include "arg_parser.hpp"
#include "message_queue/message_queue.hpp"

#include <arpa/inet.h>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <memory.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string>
#include <unistd.h>

using namespace std;

// ### Signal flags ###
volatile sig_atomic_t pipeFlag = 0, exitFlag = 0, termFlag = 0;

// ### Constructor ###
Client::Client(const ArgParser &args)
    : flags_(args.getFlags()), nickname_(args.getSpeaker()) {};

// ### Destructor ###
Client::~Client() {
    logOut();
    if (pthread_mutex_destroy(&printMtx_) != 0) {
        safePrint(Text("Err: échec de la destruction du mutex."), true);
    }
}

// ### Public methods ###
bool Client::logOn() {
    if (exitCode_) {
        return false; //< Cannot connect (critical error already encountered)
    }

    if (not initSignals()) return false;

    readIpConfig();

    // Create the socket
    sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd_ < 1) {
        safePrint(Text("Err: Le socket n'a pas pu être créé."), true);
        exitCode_ = 11;
        if (close(sockFd_) != 0) {
            perror("close");
        }
        return false;
    }

    // Connect to the server
    while (connect(sockFd_, reinterpret_cast<struct sockaddr *>(&serverAddrIn_),
                   sizeof(serverAddrIn_))
           != 0) {
        if (errno == EINTR) {
            handleSignalsSafely();
            return false;
        };
        safePrint(Text("Err: Échec de la connexion au serveur."), true);
        if (close(sockFd_) != 0) {
            perror("close");
        }
        exitCode_ = 10;
        return false;
    }

    connectionState_ = ConnectionState::Connecting;

    const string &nickname = nickname_;
    const string emptyMessage;
    if (sendMessage(sockFd_, nickname, emptyMessage, CURRENT_VERSION)
        != SendMessageReturnVal::SUCCESS) {
        safePrint(Text("Err: Échec du serrage de main avec le serveur."), true);
        if (close(sockFd_) != 0) {
            perror("close");
        }
        connectionState_ = ConnectionState::Disconnected;
        return false;
    };

    // Check whether the server accepted our request
    uint8_t response;
    if (!safeRead(sockFd_, reinterpret_cast<char *>(&response),
                  sizeof(response))) {
        safePrint(
            Text("Err: Échec de la réception de la réponse venant du serveur."),
            true);
        if (close(sockFd_) != 0) {
            perror("close");
        }
        connectionState_ = ConnectionState::Disconnected;
        return false;
    }
    if (response != 1) {
        safePrint(Text("Err: Pseudonyme non-accepté par le serveur."), true);
        if (close(sockFd_) != 0) {
            perror("close");
        }
        connectionState_ = ConnectionState::Disconnected;
        return false;
    }
    safePrint(Text("Session ouverte."), true);

    connectionState_ = ConnectionState::Connected;
    return true;
}

void Client::flushQueue() {
    while (not queue_.empty()) {
        const string &author = queue_.front().sender,
                     &content = queue_.front().content;
        safePrint(Text(author, content, flags_.bot, flags_.balise));
        queue_.pop();
    }
}

void Client::addToQueue(const Message &message) {
    cout << "\a" << flush; //< Ring the bell
    if (not queue_.push(message)) {
        flushQueue();
        safePrint({message.sender, message.content, flags_.bot, flags_.balise});
    }
}

void Client::logOut() {
    if (connectionState_ == ConnectionState::Disconnected) return;
    connectionState_ = ConnectionState::Disconnected;
    if (sockFd_ == -1) return;
    if (shutdown(sockFd_, SHUT_RD) < 0) {
        safePrint(Text("Err: Échec lors de la fermeture du socket."), true);
    }

    if (receiveThread_ != 0) {
        pthread_join(receiveThread_, nullptr);
        receiveThread_ = 0; //< Reinitialize the TID
    }

    if (close(sockFd_) < 0) {
        safePrint(Text("Err: Échec lors de la fermeture de la connexion."),
                  true);
    }
    sockFd_ = -1;
}

void Client::run() {
    if (not setSigMask(true)) {
        exitCode_ = 9;
        logOut();
        return;
    }

    if (pthread_create(&receiveThread_, nullptr, receiveMessagesThreadFunc,
                       this)
        != 0) {
        safePrint(Text("Err: Impossible de créer le thread de réception."),
                  true);
        exitCode_ = 5;
        return;
    }

    if (not setSigMask(false)) {
        exitCode_ = 9;
        logOut();
        return;
    }

    sendMessages();
}

// ### Private methods ###

void Client::readIpConfig() {
    // IP
    serverAddrIn_.sin_family = AF_INET; // Use IPv4
    const char *ipEnv = getenv("IP_SERVEUR");
    if (!ipEnv || inet_pton(AF_INET, ipEnv, &serverAddrIn_.sin_addr) != 1) {
        cerr << "Utilisation de l'adresse IP du serveur par défaut (loopback)."
             << endl;
        serverAddrIn_.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }

    // Port
    const char *portEnv = getenv("PORT_SERVEUR");
    int port = 0;
    if (portEnv) {
        port = atoi(portEnv);
    }
    if (port < 1 || port > 65535) {
        cerr << "Utilisation du port par défaut." << endl;
        port = DEFAULT_PORT;
    }
    serverAddrIn_.sin_port = htons(static_cast<uint16_t>(port));
}

void Client::receiveMessages() {
    string nickname;
    string message;

    while (connectionState_ == ConnectionState::Connected
           and receiveMessage(sockFd_, nickname, message, CURRENT_VERSION)
                   == ReceiveMessageReturnVal::SUCCESS) {
        if (nickname.empty())
            safePrint(Text(message, flags_.balise),
                      true); //< All server log are displayed on STDERR
        else if (not flags_.manuel)
            safePrint(Text(nickname, message, flags_.bot, flags_.balise));
        else addToQueue(Message{nickname, message});
    }
}

void Client::sendMessages() {
    while (connectionState_ == ConnectionState::Connected) {
        string messageWithNickname;
        cin.clear();
        while (connectionState_ == ConnectionState::Connected
               and getline(cin, messageWithNickname)) {
            handleSignalsSafely();
            // Remove the first word and the space -> the nickname
            size_t spaceIndex = messageWithNickname.find(' ');
            if (spaceIndex == string::npos) continue;

            string nickname = messageWithNickname.substr(0, spaceIndex);
            removeHyphens(nickname); // "chat-chat" to "chat chat"
            string message = messageWithNickname.substr(spaceIndex + 1);

            if (nickname != nickname_
                and sendMessage(sockFd_, nickname, message, CURRENT_VERSION)
                        != SendMessageReturnVal::SUCCESS) {
                safePrint(Text("Err: Le message n'a pas été envoyé."), true);
            } else {
                if (not flags_.bot)
                    safePrint(
                        Text(nickname_, message, flags_.bot, flags_.balise));
                flushQueue();
            }
        }
        handleSignalsSafely(true);
    }
}

void Client::safePrint(const Text &msg, bool onSTDERR) {
    pthread_mutex_lock(&printMtx_);
    if (onSTDERR) cerr << msg << endl;
    else cout << msg << endl;
    pthread_mutex_unlock(&printMtx_);
}

// ### Thread Function ###

void *Client::receiveMessagesThreadFunc(void *arg) {
    Client *client = static_cast<Client *>(arg);
    client->receiveMessages();
    kill(getpid(), SIGPIPE); //< Prevent main thread
    return nullptr;
}

// ### Signals ###

void Client::signalHandler(int signal) {
    if (signal == SIGINT) {
        exitFlag = 1;
    } else if (signal == SIGPIPE) {
        pipeFlag = 1;
    } else if (signal == SIGTERM) {
        termFlag = 1;
    }
}

bool Client::initSignals() {
    struct sigaction sa;
    sa.sa_flags = 0; //< Disable SA_RESTART
    sa.sa_handler = Client::signalHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1
        or sigaction(SIGINT, &sa, NULL) == -1
        or sigaction(SIGTERM, &sa, NULL) == -1) {
        safePrint(Text("Err: Les signaux du programme sont tenaces."), true);
        exitCode_ = 8;
        return false;
    };
    return true;
}

void Client::handleSignalsSafely(bool canBeEOF) {
    if (pipeFlag or termFlag
        or (exitFlag and connectionState_ == ConnectionState::Disconnected)
        or (not exitFlag and canBeEOF)) {
        logOut();
        exitCode_ = termFlag ? 6 : pipeFlag ? 7 : exitFlag ? 4 : 0;
    } else if (exitFlag and flags_.manuel) {
        flushQueue();
    }
    pipeFlag = termFlag = exitFlag = 0;
}

// ### Getters ###

ConnectionState Client::getState() const { return connectionState_; }

int Client::getExitCode() const { return exitCode_; }

void Client::removeHyphens(string &txt) {
    for (char &chr : txt)
        if (chr == '-') chr = ' ';
}
