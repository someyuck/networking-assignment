#include "defs.h"

struct packet **sequence(char *message)
{
    int len = strlen(message);
    int totalPackets = len / CHUNK_SIZE;

    if (totalPackets * CHUNK_SIZE < len)
        totalPackets++;

    struct packet **list = (struct packet **)malloc(sizeof(struct packet *) * totalPackets);

    for (int i = 0; i < totalPackets; i++)
    {
        list[i] = (struct packet *)malloc(sizeof(struct packet));
        list[i]->total = totalPackets;
        list[i]->index = i;
        int j;
        for (j = 0; j < CHUNK_SIZE && (i * CHUNK_SIZE + j < len); j++)
            list[i]->chunk[j] = message[i * CHUNK_SIZE + j];
        list[i]->chunk[j] = '\0';
    }

    return list;
}

char *aggregate(struct packet **packets)
{
    int totalPackets = packets[0]->total;
    char *message = (char *)malloc(sizeof(char) * ((totalPackets - 1) * CHUNK_SIZE + strlen(packets[totalPackets - 1]->chunk) + 1));
    message[0] = '\0';
    for (int i = 0; i < totalPackets; i++)
        strcat(message, packets[i]->chunk);

    return message;
}

int sendACK(int acknum, int sockfd, struct sockaddr_in *peerAddr, socklen_t peerSize)
{
    struct ack *ACK = (struct ack *)malloc(sizeof(struct ack));
    ACK->acknum = acknum;

    if (sendto(sockfd, ACK, sizeof(struct ack), 0, (struct sockaddr *)peerAddr, peerSize) < 0)
    {
        fprintf(stderr, "ERROR: sendto : errno(%d) : %s\n", errno, strerror(errno));
        close(sockfd);
        free(ACK);
        return 0;
    }
    return 1;
}

struct ack *receiveACK(int sockfd, struct sockaddr_in *peerAddr, socklen_t *peerSize)
{
    struct ack *ACK = (struct ack *)malloc(sizeof(struct ack));

    // only raise if error is not cause of no input due to timer
    if (recvfrom(sockfd, ACK, sizeof(struct ack), 0, (struct sockaddr *)peerAddr, peerSize) < 0)
    {
        free(ACK);
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return NULL;
        fprintf(stderr, "ERROR: recvfrom : errno(%d) : %s\n", errno, strerror(errno));
        close(sockfd);
        return NULL;
    }

    return ACK;
}

int sequenceAndSend(char *message, int sockfd, struct sockaddr_in *peerAddr, socklen_t *peerSize)
{
    struct packet **list = sequence(message);
    int numPackets = list[0]->total;

    // contains nanoseconds since last transmit, and last transmit time respectively. -1 if that packet has not been sent yet, -2 if received ACK
    long long nsSinceLastTransmit[numPackets];
    long long lastTransmitTime[numPackets];
    for (int i = 0; i < numPackets; i++)
    {
        nsSinceLastTransmit[i] = -1;
        lastTransmitTime[i] = -1;
    }
    int curPacket = 0;
    int numACKS = 0;

    struct timespec start;

    while (numACKS < numPackets)
    {
        // send next packet
        if (curPacket < numPackets)
        {
            if (sendto(sockfd, list[curPacket], sizeof(struct packet), 0, (struct sockaddr *)peerAddr, *peerSize) < 0)
            {
                fprintf(stderr, "ERROR: sendto : errno(%d) : %s\n", errno, strerror(errno));
                close(sockfd);
                return -1;
            }
            printf("[+] Sent packet %d\n", curPacket + 1);

            // set transmit time
            clock_gettime(CLOCK_MONOTONIC, &start);
            lastTransmitTime[curPacket] = start.tv_nsec;
            nsSinceLastTransmit[curPacket] = 0;

            curPacket++;
        }

        // check for something, if an ACK is received, mark it in the array; if timeout for any sent packet, retransmit
        numACKS = waitForACK(list, nsSinceLastTransmit, lastTransmitTime, numPackets, sockfd, peerAddr, peerSize);
        if (numACKS < 0)
            return -1;
    }

    return numPackets;
}

int waitForACK(struct packet **packetList, long long *nsSinceLastTransmit, long long *lastTransmitTime,
               int numPackets, int sockfd, struct sockaddr_in *peerAddr, socklen_t *peerSize)
{
    // assume that socket is non waiting
    struct ack *ACK = receiveACK(sockfd, peerAddr, peerSize);
    if (ACK != NULL) // received something
    {
        printf("[+] Received acknowledgement for packet %d\n", ACK->acknum + 1);
        nsSinceLastTransmit[ACK->acknum] = -2;
        lastTransmitTime[ACK->acknum] = -2;
    }

    // update time elapsed since last transmit of each sent packet
    // and retransmit if timeout has been breached
    struct timespec cur;

    int numACKS = 0;
    for (int i = 0; i < numPackets; i++)
    {

        if (lastTransmitTime[i] == -2 && nsSinceLastTransmit[i] == -2) // ACK already received, no need to retransmit
        {
            numACKS++;
            continue;
        }

        clock_gettime(CLOCK_MONOTONIC, &cur);

        if (lastTransmitTime[i] != -1) // has been sent at least once, and whose ACK hasn't already been received
            nsSinceLastTransmit[i] = cur.tv_nsec;

        if (nsSinceLastTransmit[i] >= timeout) // retransmit
        {
            printf("[-] Did not receive acknowledgement for packet %d\n", i + 1);
            if (sendto(sockfd, packetList[i], sizeof(struct packet), 0, (struct sockaddr *)peerAddr, *peerSize) < 0)
            {
                fprintf(stderr, "ERROR: sendto : errno(%d) : %s\n", errno, strerror(errno));
                close(sockfd);
                return -1;
            }
            printf("[+] Resent packet %d\n", i + 1);

            // set transmit time
            clock_gettime(CLOCK_MONOTONIC, &cur);
            lastTransmitTime[i] = cur.tv_nsec;
            nsSinceLastTransmit[i] = 0;
        }
    }

    return numACKS;
}

// functions for receiving side

char *receiveAndAggregate(int sockfd, struct sockaddr_in *peerAddr, socklen_t *peerSize)
{
    int numPackets = 0;
    struct packet **list = NULL; // realloc this

    // receive packets
    struct packet *buf = (struct packet *)malloc(sizeof(struct packet));
    int totalPackets = __INT_MAX__;

    int *receivedPackets;

    while (numPackets < totalPackets)
    {
        memset(buf, 0, sizeof(struct packet));
        // *peerSize = sizeof(*peerAddr);
        if (recvfrom(sockfd, buf, sizeof(struct packet), 0, (struct sockaddr *)peerAddr, peerSize) >= 0)
        {
            if (numPackets == 0) // setup
            {
                totalPackets = buf->total;
                receivedPackets = (int *)calloc(buf->total, sizeof(int));
            }

            // mark packet as received
            if (receivedPackets[buf->index] == 0) // only for unique chunks (if not already received)
            {
                receivedPackets[buf->index] = 1;

                // add to packet list
                list = (struct packet **)realloc(list, sizeof(struct packet *) * (numPackets + 1));
                list[numPackets] = (struct packet *)malloc(sizeof(struct packet));
                list[numPackets]->index = buf->index;
                list[numPackets]->total = buf->total;
                memset(list[numPackets]->chunk, 0, sizeof(list[numPackets]->chunk));
                strcpy(list[numPackets]->chunk, buf->chunk);

                numPackets++;

                // send ACK
                int flag = 1;

                /* Comment this part (next 2 lines) out to enable sending ACK for each packet.
                Uncomment to not send ACK for every third packet, to test retransmission*/
                // if ((buf->index % 3) == 2)
                //     flag = 0;

                if (flag == 1)
                {
                    if (sendACK(buf->index, sockfd, peerAddr, *peerSize) == 0)
                    {
                        fprintf(stderr, "ERROR: sendto : errno(%d) : %s\n", errno, strerror(errno));
                        close(sockfd);
                        return NULL;
                    }
                    printf("[+] Sent ACK for packet %d\n", buf->index + 1);
                }
            }
        }
    }

    free(buf);

    // now arrange the packets in order
    qsort(list, totalPackets, sizeof(struct packet *), comparator);

    return aggregate(list);
}

int comparator(const void *a, const void *b)
{
    int ans = 0;

    if (((struct packet *)a)->index < ((struct packet *)b)->index)
        ans = -1;
    else if (((struct packet *)a)->index == ((struct packet *)b)->index)
        ans = 0;
    else if (((struct packet *)a)->index > ((struct packet *)b)->index)
        ans = 1;

    return ans;
}
