/*!
 * \file       main.cpp
 * \brief      This is the main file, here the streams of data from server to client and viceversa are set.
 * \author     Enrico Fraccaroli
 * \date       4 May 2015
 * \copyright
 *  RadMud (C) Copyright 2015 by Enrico Fraccaroli.
 *  Permission to copy, use, modify, sell and distribute this software is granted
 *  provided this copyright notice appears in all copies. This software is provided
 *  "as is" without express or implied warranty, and with no claim as to its suitability
 *  for any purpose.
 */

#ifdef __linux__
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#elif __APPLE__
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#elif __CYGWIN__
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#elif _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WS2tcpip.h>
#include <windows.h>
#endif

#include <termios.h>
#include <iostream>
#include <signal.h>
#include <netdb.h>
#include <cstring>
#include <stdio.h>

#include "utils.h"

///////////////////////////////////////////////////////////
// DEFINES
/// Client Version
#define VERSION "1.0.0"
/// where to connect to (name or IP)
#define HOSTNAME "localhost"
/// port to connect to
#define PORT     4000
/// Time to wait in seconds.
#define COMMS_WAIT_SEC      0
/// Time to wait in microseconds.
#define COMMS_WAIT_USEC     500000
/// Return the bigger value.
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
/// Return the smaller value.
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
///////////////////////////////////////////////////////////

/// Set by signal handler
static bool stop_now = false;
/// Time we last sent something to the MUD.
time_t last_send;
/// The number of bytes received from server.
static double bandwidth_in = 0;
/// The number of bytes sent to server.
static double bandwidth_out = 0;
/// The number of bytes saved due to compression.
static double bandwidth_unc = 0;
/// The number of messages received from server.
static double messages_in = 0;
/// The number of messages sent to server.
static double messages_out = 0;
/// The server's socket.
static int srvsocket;

///////////////////////////////////////////////////////////
// Forward Functions
void bailout(int sig);
int ProcessIO();
///////////////////////////////////////////////////////////

/// The main function.
int main() {
    time_t start, end;

    printf("RadClient version %s.\n", VERSION);
    printf("Connecting to %s, port %i...\n", HOSTNAME, PORT);

    // Structure describing our socket address.
    struct sockaddr_in srvsockaddr;

    // Standard termination signals.
    signal(SIGINT, bailout);
    signal(SIGTERM, bailout);
    signal(SIGHUP, bailout);

    // Create a TCP/IP stream socket.
    if ((srvsocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("creating socket");
        return 1;
    }

    // Fill in the the address structure
    memset(&srvsockaddr, '\0', sizeof(srvsockaddr));
    srvsockaddr.sin_port = htons(PORT);
    srvsockaddr.sin_family = AF_INET;
    srvsockaddr.sin_addr.s_addr = inet_addr(HOSTNAME);

    // Look up IP address, convert into number
    if (srvsockaddr.sin_addr.s_addr == INADDR_NONE) {
        struct hostent * hostinfo = gethostbyname(HOSTNAME);

        if (hostinfo == nullptr) {
            herror("finding hostname: " HOSTNAME);
            return 1;
        }
        srvsockaddr.sin_addr.s_addr = *(unsigned long *) hostinfo->h_addr;
    }

    // Connect to the socket
    if (connect(srvsocket, (struct sockaddr *) &srvsockaddr, sizeof srvsockaddr) == -1) {
        perror("connecting to MUD");
        return 1;
    }

    printf("Connected using socket %i.\n", srvsocket);

    time(&start);
    // Loop processing MUD input, and our keystrokes.
    do {
        // Process incoming comms, and commands from player.
        if (ProcessIO() != 0) {
            break;
        }
    }
    while (!stop_now);
    time(&end);

    // Close the socket.
    close(srvsocket);

    // Print the statistics.
    std::cout << "# Statistics:" << std::endl;
    std::cout << "|        Uptime........ " << difftime(end, start) << " \tseconds" << std::endl;
    std::cout << "|--> Bandwidth" << std::endl;
    std::cout << "|        In............ " << bandwidth_in << " \tBytes" << std::endl;
    std::cout << "|        Out........... " << bandwidth_out << " \tBytes" << std::endl;
    std::cout << "|        Uncompressed.. " << bandwidth_unc << " \tBytes" << std::endl;
    std::cout << "|        Saved......... " << (bandwidth_unc - bandwidth_in) << " \tBytes" << std::endl;
    std::cout << "|--> Messages" << std::endl;
    std::cout << "|        In............ " << messages_in << " \tMsgs" << std::endl;
    std::cout << "|        Out........... " << messages_out << " \tMsgs" << std::endl;
    std::cout << "# Goodbye!" << std::endl;

    return 0;
}

/// Sends a message
void Send(const char * msg) {
    if (send(srvsocket, msg, strlen(msg), 0) == -1) {
        perror("send");
    }
    else {
        last_send = time(NULL);
    }
}

/*!
 * \brief receives asynchronous output, waits COMMS_WAIT_SEC/COMMS_WAIT_USEC and then returns.
 * \return -1 in case of Error, 0 in case of Timeout, 1 in case the connection is clodes.
 */
int ProcessIO() {
    fd_set in_set;
    fd_set exc_set;
    struct timeval timeout;

    // loop processing input
    for (;;) {

        FD_ZERO(&in_set);
        FD_ZERO(&exc_set);

        // add our socket
        FD_SET(srvsocket, &in_set);
        FD_SET(srvsocket, &exc_set);

        // and stdin
        FD_SET(STDIN_FILENO, &in_set);
        FD_SET(STDIN_FILENO, &exc_set);

        timeout.tv_sec = COMMS_WAIT_SEC;
        timeout.tv_usec = COMMS_WAIT_USEC; /* wait half a second */

        /// time  limit expired? return
        if (select(UMAX(srvsocket, STDIN_FILENO) + 1, &in_set, NULL, &exc_set, &timeout) == 0) {
            return 0;
        }

        // check for exception on our socket
        if (FD_ISSET(srvsocket, &exc_set)) {
            printf("Exception occurred on socket\n");
            return -1;  // out of receive loop
        }

        // check for input on our socket
        if (FD_ISSET(srvsocket, &in_set)) {
            std::vector<char> buffer(4096);
            std::vector<unsigned char> compressed;
            std::vector<unsigned char> uncompressed;
            int nRead;

            nRead = read(srvsocket, &buffer[0], buffer.size());

            // input but zero length? must have closed the connection
            if (nRead == 0) {
                printf("Connection closed\n");
                return 1;
            }
            // less than zero bytes? error on connection
            if (nRead < 0) {
                perror("read from MUD");
                return -1;
            }

            ///////////////////////////////////////////////////
            // COMPRESSED
            for (int i = 0; i < nRead; i++) {
                compressed.push_back(buffer[i]);
            }

            // create decompressed output
            InflateStream(compressed, uncompressed);

            for (unsigned int i = 0; i < uncompressed.size(); i++) {
                std::cout << uncompressed[i] << std::flush;
            }
            fflush(stdout);

            // Update Bandwidth statistics.
            bandwidth_in += compressed.size();
            bandwidth_unc += uncompressed.size();

            // Update Messages statistics.
            messages_in += 1;

            compressed.clear();
            uncompressed.clear();
            ///////////////////////////////////////////////
        }

        // check for input on the keyboard
        if (FD_ISSET(STDIN_FILENO, &in_set)) {
            static char buf[1000];
            int nRead;

            nRead = read(STDIN_FILENO, buf, sizeof(buf) - 1);

            // less than zero bytes? error on stdin
            if (nRead < 0) {
                perror("input from player");
                return -1;
            }

            // Update Bandwidth statistics.
            bandwidth_out += nRead;

            // Update Messages statistics.
            messages_out += 1;

            // terminate buffer, send results
            buf[nRead] = 0;  // terminating null
            Send(buf);
        }
    }
}

/*!
 * \brief Print an information string every time a Signal is caught.
 * \param sig the signal that has been caught.
 */
void bailout(int sig) {
    printf("**** Terminated by player on signal %i ****\n\n", sig);
    stop_now = 1;
}

/*!
 * This function disable the echo of the keyboard.
 */
void HideStdinKeystrokes() {
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    // We want to disable echo.
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

/*!
 * This function re-enable the echo of the keyboard.
 */
void ShowStdinKeystrokes() {
    termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    // We want to reenable echo.
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

