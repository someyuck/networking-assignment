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
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        fprintf(stderr, "ERROR: connect : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("connected to server successfully\n");

    // ==========================================================================================================

    // first wait for confirmation from server to begin game
    if (recv(sock, serverMsg, sizeof(serverMsg), 0) < 0)
    {
        fprintf(stderr, "ERROR: recv : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    if (strcmp(serverMsg, "begin") != 0)
    {
        printf("server ordered me to close\n");
        close(sock);
        return -1;
    }

    // now start playing the game
    printf("\n\n");
    printf("Welcome to Rock, Paper, Scissors!\n");
    printf("Game begins!\n\n");

    char perm[5]; // represents client's choice whether to continue the game or not

    while (1)
    {
        // clear buffers
        memset(serverMsg, 0, sizeof(serverMsg));
        memset(clientMsg, 0, sizeof(clientMsg));

        // prompt user for input:
        printf("Enter your choice (0 for Rock, 1 for Paper, 2 for Scissors): ");
        fgets(clientMsg, sizeof(clientMsg), stdin);
        clientMsg[strlen(clientMsg) - 1] = '\0'; // overwrite the last '\n'

        // send it to server
        if (send(sock, clientMsg, strlen(clientMsg), 0) < 0)
        {
            fprintf(stderr, "ERROR: send : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }

        // receive reply from server
        if (recv(sock, serverMsg, sizeof(serverMsg), 0) < 0)
        {
            fprintf(stderr, "ERROR: recv : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }

        // display result to user
        if (strcmp(serverMsg, "Draw") != 0)
            printf("You ");
        printf("%s\n", serverMsg);

        // ask user if they want to continue playing
        printf("\nDo you want to play another game (enter 1 for Yes, 0 for No)? ");
        fgets(perm, sizeof(perm), stdin);
        perm[strlen(perm) - 1] = '\0'; // overwrite the last '\n'
        // tell server
        if (send(sock, perm, strlen(perm), 0) < 0)
        {
            fprintf(stderr, "ERROR: send : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }

        // get server's reply
        memset(serverMsg, 0, sizeof(serverMsg));
        if (recv(sock, serverMsg, sizeof(serverMsg), 0) < 0)
        {
            fprintf(stderr, "ERROR: recv : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }
        if (strcmp(serverMsg, "continue") == 0)
            continue;
        else if (strcmp(perm, "1") == 0 && strcmp(serverMsg, "client2quit") == 0)
        {
            printf("Sorry, but client 2 does not want to play.\n");
            break;
        }
        else if(strcmp(perm, "0") == 0)
            break;
    }

    printf("quitting...\n");

    // game over, close socket
    close(sock);
}