#include "defs.h"

int main()
{
    int sock;
    socklen_t serverSize;
    struct sockaddr_in serverAddr;
    char clientMsg[4096];

    memset(clientMsg, 0, sizeof(clientMsg));

    // create a socket for UDP communication
    sock = socket(AF_INET, SOCK_DGRAM, 0); // IP; unreliable, connection-less communication.
    if (sock < 0)
    {
        fprintf(stderr, "ERROR: socket : errno(%d): %s\n", errno, strerror(errno));
        return -1;
    }
    printf("socket created successfully\n");

    // make socket nonblocking
    struct timeval timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 1000; // make it not wait much to check for incoming data
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeVal, sizeof(timeVal)) < 0)
    {
        fprintf(stderr, "ERROR: setsockopt : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }

    // set IP and port -- same as that on server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // prompt user for input:
    printf("Enter your message: ");
    fgets(clientMsg, sizeof(clientMsg), stdin);
    clientMsg[strlen(clientMsg) - 1] = '\0'; // overwrite the last '\n'

    // send message to server
    serverSize = sizeof(serverAddr);
    if(sequenceAndSend(clientMsg, sock, &serverAddr, &serverSize) < 0)
    {
        fprintf(stderr, "ERROR: could not send\n");
        close(sock);
        return -1;
    }
    printf("message sent to server\n");

    // receive reply from server
    serverSize = sizeof(serverAddr);
    char *message = receiveAndAggregate(sock, &serverAddr, &serverSize);
    if(message == NULL)
        printf("an error occured\n");
    else
        printf("server's reply: %s\n", message);

    // close socket
    close(sock);
}