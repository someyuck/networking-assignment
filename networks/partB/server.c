#include "defs.h"

int main()
{
    int sock;
    socklen_t clientSize;
    struct sockaddr_in serverAddr, clientAddr;
    char serverMsg[4096];

    memset(serverMsg, 0, sizeof(serverMsg));

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

    // receive message from user
    clientSize = sizeof(clientAddr);
    char *message = receiveAndAggregate(sock, &clientAddr, &clientSize);
    if (message == NULL)
    {
        printf("an error occured\n");
        close(sock);
        return -1;
    }
    else
        printf("Client message: %s\n", message);

    // prepare response:
    if (strcmp(message, "hey! wassup?") == 0)
        strcpy(serverMsg, "nothin much really, just chillin");
    else
        strcpy(serverMsg, "what does that mean?");

    /*To check retransmission, uncomment the below while loop. This prevents the sender from sending some text and subsequently closing,
     so that the client can keep retransmitting unacknowledged packets. Keep it commented otherwise.*/
    // while (1);

    clientSize = sizeof(clientAddr);
    if (sequenceAndSend(serverMsg, sock, &clientAddr, &clientSize) < 0)
    {
        fprintf(stderr, "ERROR: could not send\n");
        close(sock);
        return -1;
    }
    printf("reply sent\n");

    // convo over, close sockets
    close(sock);
}