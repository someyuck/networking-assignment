#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>

int main()
{
    int sock, clientSock;
    socklen_t clientSize;
    struct sockaddr_in serverAddr, clientAddr;
    char serverMsg[4096], clientMsg[4096];

    memset(serverMsg, 0, sizeof(serverMsg));
    memset(clientMsg, 0, sizeof(clientMsg));

    // create a socket for TCP communication
    sock = socket(AF_INET, SOCK_STREAM, 0); // IP; duplex and reliable, connection-based stream.
    if (sock < 0)
    {
        fprintf(stderr, "ERROR: socket : errno(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    printf("socket created successfully\n");

    // set IP and port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // add a name to the socket
    if (bind(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        fprintf(stderr, "ERROR: bind : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("socket bound successfully\n");

    // listen for a client (only one)
    if (listen(sock, 1) < 0)
    {
        fprintf(stderr, "ERROR: listen : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("listening for a connection request...\n");


    // accept an incoming connection
    clientSize = sizeof(clientAddr);
    clientSock = accept(sock, (struct sockaddr *)&clientAddr, &clientSize);
    if (clientSock < 0)
    {
        fprintf(stderr, "ERROR: accept : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("client connected successfully\n");

    // receive message from client
    if (recv(clientSock, clientMsg, sizeof(clientMsg), 0) < 0)
    {
        fprintf(stderr, "ERROR: recv : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        close(clientSock);
        return -1;
    }
    printf("client message: %s\n", clientMsg);

    // prepare response:
    if (strcmp(clientMsg, "hey! wassup?") == 0)
        strcpy(serverMsg, "nothin much really, just chillin");
    else
        strcpy(serverMsg, "what does that mean?");

    // send response to client
    if (send(clientSock, serverMsg, strlen(serverMsg), 0) < 0)
    {
        fprintf(stderr, "ERROR: send : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        close(clientSock);
        return -1;
    }
    printf("reply sent\n");

    // convo over, close sockets
    close(sock);
    close(clientSock);
}