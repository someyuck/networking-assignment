# Instructions to test

## For Part A
### For basic
1. For the TCP server-client programs, open a terminal in the `partA/basic` directory and run `make tcp` (or `make`). Then run `./tcpserver` in one terminal and `tcpclient` in another terminal to begin.
2. For the UDP server-client programs, open a terminal in the `partA/basic` directory and run `make udp` (or `make`). Then run `./udpserver` in one terminal and `udpclient` in another terminal to begin.
3. To remove the executables, run `make clean`. Ignore errors that may come up due to some executables not being present (as their source files were not compiled)

### For rpc 
1. For the TCP version, open a terminal in the `partA/rpc` directory and run `make tcp` (or `make`). Then open two more terminals and run '`./tcpserver`, `./tcpclient1` and `./tcpclient2` to begin.
2. For the UDP version, open a terminal in the `partA/rpc` directory and run `make udp` (or `make`). Then open two more terminals and run '`./udpserver`, `./udpclient1` and `./udpclient2` to begin.
3. To remove the executables, run `make clean`. Ignore errors that may come up due to some executables not being present (as their source files were not compiled)

## For Part B
1. Open a terminal in the `partB` directory and run `make`. Then open another terminal, and run `./server` in one and `./client` in the other to begin.
2. To remove the executables, run `make clean`.
3. For partB, to test the retransmission of packets in case of the sender not receivng acknowledgements of some packets from the receiver, uncomment part of functions.c (lines 216 to 217), and part of server.c (line 66). Keep them commented to enable full acknowledgement of received packets.

# Networking Report (Specification 4)

## How is your implementation of data sequencing and retransmission different from traditional TCP? (If there are no apparent differences, you may mention that)

### Data Sequencing
- In my implementation, I use fixed-size chunks (10 bytes of the string message) to divide up the message string, and index each packet in the order in which they appear in the original message. That is, the first 10 bytes make packet 0, the next 10 make packet 1 and so on. However, in the standard TCP implementation, the sequence number of the first packet (SYN) can be arbitrary, in order to defend against security risks wherein attckers may predict TCP sequences.
- In sending ACKs, I simply set its sequence number to be the that of the packet which is being acknowledged. In traditional TCP, the ACK packet's sequence number specifies the byte to which the data has been received.

### Retransmission
- For retransmitting a packet in case the sender does not receive its acknowledgement, my implementation uses a timeout (100 ms). I keep track of the last time (to nanosecond precision) each packet was transmitted, and the time elapsed since then (also in ns precision). If the time elapsed exceeds the timeout, I retransmit that packet. If however, at any moment I receive an ACK, I mark that packet as having been acknowledged.

Traditional TCP uses two approaches:
- Duplicate cumulative acknowledgements (dupacks): In this approach, the receiver sends acknowledgements cumulatively (whereas mine sends one as soon as it receives a packet, no matter the order), so if one packet went missing, it will not acknowledge any packets with higher sequence numbers, and keep repeating the last acknolegement. If three such duplicate ACKs are detected, the sender retransmits the last unaclnowledged packet.
- Retransmission Timeout (RTO): Similar to what I have done, the sender initialises a timer for each packet sent, and retransmits if an ACK is not received for it within that time. The difference is that the sender would then double the timer duration, resulting in an exponential increase in wait times of the sender, whereas I keep the timeout constant (at 100 ms). Also, the sender makes an estimate of the round-trip time (RTT) for the arrival of the acknowledgement, with a mathematical formula typically used to determine the intial timeout value, or using an algorithm to determine it.

## How can you extend your implementation to account for flow control? You may ignore deadlocks.

Flow control is used by TCP to regulate the amount of data the sender sends so as to not overwhelm the receiver. The receiver specifies the maximum number of additional bytes it can receive, called receiving window size, and the sender sends new bytes according to this limit. This information is sent along with the TCP segment, with each acknowledgment packet.

To implement this in UDP:

1. I would add an integer field in the `struct packet` in `defs.h`, for the receiving window size, in bytes.
2. First I would have the receiver send a packet containing its initial receiving window size. The sender will send packets according to this.
3. I would maintain a sliding window at the sender end, which would contain the packets which are to be sent next, and whose size would be less than or equal to the receiving window size provided by the receiver in the latest acknowledgement message.
4. The sender would then send the packets in the sliding window and wait for the receiver to send an ACK for these packets. Only when it receives an ACK, would it move the window to the right to send further packets. From this new ACK, it would update the sliding window size as well based on the new receiving window size specified by the receiver.
5. The sender would keep sending packets, waiting for ACK and moving the sliding window till the time all packets have been sent with ACKs received for all of them.
6. To implement the sliding window, all we need to do is to maintain two indices, a left and a right one denoting the start and the end of the window. The sender simply sends the packets in this window.
7. If the receiver specifies a receiving window size of zero, the sender will halt sending further packets and wait for the receiver to send a packet containing a nonzero receiving window size, so that it can resume sending packets. Note that since we can ignore deadlocks, I assume that no such ACK message will be lost.


# Assumptions
1. For partA/basic, after one run of the TCP client-server pair programs, wait for some time, as immediately after a run, the address may still be in use (bind() throws an error). Waiting for some time would free it up and then we can run both programs again.
2. For partA/rpc, run the UDP programs with the server run first, then the clients, as I make the server wait for a confirmation from the clients that they are ready to play. For the TCP programs, run in any order.