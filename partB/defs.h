#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#define CHUNK_SIZE 10 // fixed sized chunks
#define timeout 100000000 // 0.1s in ns

struct packet
{
    int total;
    int index;
    char chunk[CHUNK_SIZE + 1]; // for '\0'
};

struct ack
{
    int acknum;
};

// functions:

// string -> list of chunk struct pointers, also returning the number of chunks
struct packet **sequence(char *message);

// list of chunks -> string
char *aggregate(struct packet **packets);

// send ACK packet over UDP
int sendACK(int acknum, int sockfd, struct sockaddr_in *peerAddr, socklen_t peerSize);

// receive an ACK packet over UDP
struct ack *receiveACK(int sockfd, struct sockaddr_in *peerAddr, socklen_t *peerSize);

// sequence a given string using sequencer, and send each over UDP. Also check for ACK using waitForACK
int sequenceAndSend(char *message, int sockfd, struct sockaddr_in *peerAddr, socklen_t *peerSize);

// check for ack of a given packet, retransmit if timeout occurs
int waitForACK(struct packet **packetList, long long *nsSinceLastTransmit, long long *lastTransmitTime, int numPackets, int sockfd, struct sockaddr_in *peerAddr, socklen_t *peerSize);

//  receive each packet using UDP, send ACK, sort and then aggregate to get the whole messagete
char *receiveAndAggregate(int sockfd, struct sockaddr_in *peerAddr, socklen_t *peerSize);
int comparator(const void *a, const void *b); // for qsort
