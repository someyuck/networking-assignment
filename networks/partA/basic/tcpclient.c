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
    struct sockaddr_in serverAddr;
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


    // set IP and port -- same as that on server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // request a connection to server (server is listening for this)
    if(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        fprintf(stderr, "ERROR: connect : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("connected to server successfully\n");

    // prompt user for input:
    printf("Enter your message: ");
    fgets(clientMsg, sizeof(clientMsg), stdin);
    clientMsg[strlen(clientMsg) - 1] = '\0'; // overwrite the last '\n'

    // send it to server
    if(send(sock, clientMsg, strlen(clientMsg), 0) < 0)
    {
        fprintf(stderr, "ERROR: send : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("message sent to server\n");

    // receive reply from server
    if(recv(sock, serverMsg, sizeof(serverMsg), 0) < 0)
    {
        fprintf(stderr, "ERROR: recv : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    
    // display to user
    printf("Server's response: %s\n", serverMsg);

    // close socket
    close(sock);
}