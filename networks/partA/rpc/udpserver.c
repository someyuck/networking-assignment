#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>

int main()
{
    int sock;
    socklen_t client1Size, client2Size;
    struct sockaddr_in serverAddr, client1Addr, client2Addr;
    char serverMsg1[100], serverMsg2[100], client1Msg[100], client2Msg[100];

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

    // wait for clients to say they're ready
    client1Size = sizeof(client1Addr);
    if (recvfrom(sock, client1Msg, sizeof(client1Msg), 0, (struct sockaddr *)&client1Addr, &client1Size) < 0)
    {
        fprintf(stderr, "ERROR: recvfrom : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("client 1 message: %s\n", client1Msg);

    client2Size = sizeof(client2Addr);
    if (recvfrom(sock, client2Msg, sizeof(client2Msg), 0, (struct sockaddr *)&client2Addr, &client2Size) < 0)
    {
        fprintf(stderr, "ERROR: recvfrom : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }
    printf("client 2 message: %s\n", client2Msg);

    // send confirmation to clients to begin game
    client1Size = sizeof(client1Addr);
    if (sendto(sock, "begin", strlen("begin"), 0, (struct sockaddr *)&client1Addr, client1Size) < 0)
    {
        fprintf(stderr, "ERROR: sendto : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }

    client2Size = sizeof(client2Addr);
    if (sendto(sock, "begin", strlen("begin"), 0, (struct sockaddr *)&client2Addr, client2Size) < 0)
    {
        fprintf(stderr, "ERROR: sendto : errno(%d): %s\n", errno, strerror(errno));
        close(sock);
        return -1;
    }

    // ==========================================================================================================
    // now play the game
    printf("\n\nGame begins\n");

    int flag = 1;

    while (flag == 1) // play
    {
        // clear buffers
        memset(serverMsg1, 0, sizeof(serverMsg1));
        memset(serverMsg2, 0, sizeof(serverMsg2));
        memset(client1Msg, 0, sizeof(client1Msg));
        memset(client2Msg, 0, sizeof(client2Msg));

        // receive message from client 1
        if (recvfrom(sock, client1Msg, sizeof(client1Msg), 0, (struct sockaddr *)&client1Addr, &client1Size) < 0)
        {
            fprintf(stderr, "ERROR: recv : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }
        printf("client 1 message: %s\n", client1Msg);

        // receive message from client 2
        if (recvfrom(sock, client2Msg, sizeof(client2Msg), 0, (struct sockaddr *)&client2Addr, &client2Size) < 0)
        {
            fprintf(stderr, "ERROR: recv : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }
        printf("client 2 message: %s\n", client2Msg);

        // make judgement:
        long c1 = strtol(client1Msg, NULL, 10);
        long c2 = strtol(client2Msg, NULL, 10);
        if (c1 == c2)
        {
            strcpy(serverMsg1, "Draw");
            strcpy(serverMsg2, "Draw");
            printf("Draw\n");
        }
        else if (c2 == (c1 + 1) % 3) // client 2 wins
        {
            strcpy(serverMsg1, "Lose");
            strcpy(serverMsg2, "Win");
            printf("Client 2 wins\n");
        }
        else if (c1 == (c2 + 1) % 3) // client 1 wins
        {
            strcpy(serverMsg1, "Win");
            strcpy(serverMsg2, "Lose");
            printf("Client 1 wins\n");
        }

        // send response to clients
        if (sendto(sock, serverMsg1, strlen(serverMsg1), 0, (struct sockaddr *)&client1Addr, client1Size) < 0)
        {
            fprintf(stderr, "ERROR: send : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }
        if (sendto(sock, serverMsg2, strlen(serverMsg2), 0, (struct sockaddr *)&client2Addr, client2Size) < 0)
        {
            fprintf(stderr, "ERROR: send : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }

        // take in clients' permission messages on whether to continue the game or not
        memset(serverMsg1, 0, sizeof(serverMsg1));
        memset(serverMsg2, 0, sizeof(serverMsg2));
        memset(client1Msg, 0, sizeof(client1Msg));
        memset(client2Msg, 0, sizeof(client2Msg));

        // receive message from client 1
        if (recvfrom(sock, client1Msg, sizeof(client1Msg), 0, (struct sockaddr *)&client1Addr, &client1Size) < 0)
        {
            fprintf(stderr, "ERROR: recv : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }
        printf("client 1 message: %s\n", client1Msg);

        // receive message from client 2
        if (recvfrom(sock, client2Msg, sizeof(client2Msg), 0, (struct sockaddr *)&client2Addr, &client2Size) < 0)
        {
            fprintf(stderr, "ERROR: recv : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }
        printf("client 2 message: %s\n", client2Msg);

        // decide
        if(strcmp(client1Msg, "1") == 0 && strcmp(client2Msg, "1") == 0) //  continue playing
        {
            strcpy(serverMsg1, "continue");
            strcpy(serverMsg2, "continue");
        }
        else if(strcmp(client1Msg, "1") == 0 && strcmp(client2Msg, "0") == 0)
        {
            strcpy(serverMsg1, "client2quit");
            strcpy(serverMsg2, "client2quit");
            flag = 0;
        }
        else if(strcmp(client1Msg, "0") == 0 && strcmp(client2Msg, "1") == 0)
        {
            strcpy(serverMsg1, "client1quit");
            strcpy(serverMsg2, "client1quit");
            flag = 0;
        }
        else if(strcmp(client1Msg, "0") == 0 && strcmp(client2Msg, "0") == 0)
        {
            strcpy(serverMsg1, "quit");
            strcpy(serverMsg2, "quit");
            flag = 0;
        }
        
        
        // send the decision
        if (sendto(sock, serverMsg1, strlen(serverMsg1), 0, (struct sockaddr *)&client1Addr, client1Size) < 0)
        {
            fprintf(stderr, "ERROR: send : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }
        if (sendto(sock, serverMsg2, strlen(serverMsg2), 0, (struct sockaddr *)&client2Addr, client2Size) < 0)
        {
            fprintf(stderr, "ERROR: send : errno(%d): %s\n", errno, strerror(errno));
            close(sock);
            return -1;
        }
    }

    // game over, close sockets
    printf("Game over.\n");

    close(sock);
}