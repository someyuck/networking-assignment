#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>

int main()
{
    int sock;
    socklen_t clientSize;
    struct sockaddr_in serverAddr, clientAddr;
    char serverMsg[4096], clientMsg[4096];

    memset(serverMsg, 0, sizeof(serverMsg));
    memset(clientMsg, 0, sizeof(clientMsg));

    // create a socket for UDP communication
    sock = socket(AF_INET, SOCK_DGRAM, 0); // IP; unreliable, connection-less communication.
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

    // here we don't establish connections, and directly send/receive messages

    // receive message from client
    clientSize = sizeof(clientAddr);
    if (recvfrom(sock, clientMsg, sizeof(clientMsg), 0, (struct sockaddr *)&clientAddr, &clientSize) < 0)
    {
        fprintf(stderr, "ERROR: recvfrom : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("client message: %s\n", clientMsg);

    // prepare response:
    if (strcmp(clientMsg, "hey! wassup?") == 0)
        strcpy(serverMsg, "nothin much really, just chillin");
    else
        strcpy(serverMsg, "what does that mean?");

    // send response to client
    if (sendto(sock, serverMsg, strlen(serverMsg), 0, (struct sockaddr*)&clientAddr, clientSize) < 0)
    {
        fprintf(stderr, "ERROR: send : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("reply sent\n");

    // convo over, close sockets
    close(sock);
}