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
    socklen_t serverSize;
    struct sockaddr_in serverAddr;
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

    // set IP and port -- same as that on server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // here we don't establish connections, and directly send/receive messages

    // prompt user for input:
    printf("Enter your message: ");
    fgets(clientMsg, sizeof(clientMsg), stdin);
    clientMsg[strlen(clientMsg) - 1] = '\0'; // overwrite the last '\n'

    // send message to server
    if (sendto(sock, clientMsg, strlen(clientMsg), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        fprintf(stderr, "ERROR: sendto : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("message sent to server\n");

    // receive response from server
    serverSize = sizeof(serverAddr);
    if (recvfrom(sock, serverMsg, sizeof(serverMsg), 0, (struct sockaddr*)&serverAddr, &serverSize) < 0)
    {
        fprintf(stderr, "ERROR: recvfrom : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }

    // display to user
    printf("Server's response: %s\n", serverMsg);

    // close socket
    close(sock);
}