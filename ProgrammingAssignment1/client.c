/* 
 * COEN 233 Programming Assignment 1 
 * client.c
 * 
 * Student Name: Shuhua Zheng 
 * Student ID: 1289984
 *
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "packet.h"

#define PORT 1234
#define MAXDATASIZE 1024


int clientSocket; // create a socket 
struct sockaddr_in server; //a structure containing an internet address
socklen_t addrSize; // socklen_t is an unsigned opaque integral type of 
                    // len of at least 32 bits

//test data info
uint8 TEST_DATA[] = "Testing data used for generating correct packets.";


//Initial array for storing the message receive from server
uint8 buffer[MAXDATASIZE];

//print sending message byte by byte
void printSendMsg (uint8 *string, int size) {
    for (int i = 0; i < size; i++) {
        if (i > 0)
            printf(":");
        printf("%02X", *(string + i));
    }
    printf("\n");
}

//print reply msg received from server 
void printResponse(struct PACKET receivedPack) {
    switch (receivedPack.packetType) {
        case ACK:
            printf("Received ACK from server for packet with segment %d\n", receivedPack.segmentNum);
            break;
        case REJECT:
            printf("Received REJECT from server for packet with segment %d\n", receivedPack.segmentNum);
            switch (receivedPack.reply) {
                case OUT_OF_SEQUENCE:
                    fprintf(stderr, "REJECT SUB CODE: OUT OF SEQUENCE\n"); // stderr, standard error stream 
                    break;
                case DUPLICATE_PACKET:
                    fprintf(stderr, "REJECT SUB CODE: DUPLICATE PACKET\n");
                    break;
                case LENGTH_MISMATCH:
                    fprintf(stderr, "REJECT SUB CODE: LENGTH MISMATCH\n");
                    break; 
                case PACKET_ID_END_MISSING:
                    fprintf(stderr, "REJECT SUB CODE: END OF PACKET ID MISSING\n");
                    break; 
                case NO_ERROR:
                    break;
            }
    }
}

//send message to server and receive feedback from server
void sendAndReceive(uint8* message, uint8 clientId, uint8 num, int type) {
    uint8 payloadLength = strlen((const char *)message);
    struct PACKET dataPack;
    dataPack.clientId = clientId;
    dataPack.packetType = DATA;
    dataPack.segmentNum = num;
    dataPack.payload = message;
    dataPack.length = payloadLength;

    int size = createDataPack(dataPack, buffer);
    
    // create packet with error: LENGTH NOT MATCH!
    if(type == 1)
        buffer[6] = 0; //change the length of packet to create an error packet 

    // create packet with error: DON'T HAVE the END PACKET ID!
    if (type == 2)
        size = size - 2; // eliminate the End of Packet id to create an arror 
    
    //set receive size and sending message counter
    long recvSize = -1;
    int counter = 3; // same packet can be sent at most 3 times  
    
    //send message to server
    sendto(clientSocket, buffer, size, 0, (struct sockaddr *) &server, sizeof(server));
    
    //show send message
    printf("\nSending packet with segment %d to server...\n", dataPack.segmentNum);
    printSendMsg(buffer, size); // print massage byte by byte
    printf("\n");
    
    // set timeout 3 sec
    struct timeval timer;
    timer.tv_sec = 3;      
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer)) < 0) 
        perror("ERROR: setsockopt failed.\n");
    
    while (counter > 0) {
        //receive message from server
        recvSize = recvfrom(clientSocket, buffer, MAXDATASIZE, 0,(struct sockaddr *) &server, &addrSize);

        if(recvSize < 0)
            perror("ERROR: recvfrom error\n");
        else
            break;
        //ever time counter--
        counter--;
    }
    
    //If recvSize still equals to -1, after the same packet sent to server 3 times
    // the server must be down and client should print an error message and exit. 
    if (recvSize < 0) {
        printf("\nServer does not respond.\n\n");
        exit(1);
    } else {
        // get packet information
        struct PACKET receivedPack;
        parseMsgClient(buffer, recvSize, &receivedPack);
        
        printResponse(receivedPack);    // print out the message received Ack or Rej
    }
    printf("\n");
    
}


int main() {
    // create UDP socket, the arguments are domain, type, protocol
    // and check whether the socket has been created successfully
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("ERROR: creating socket failed.\n");
        exit(1);
    }
    
    //set configuration 
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    printf("========================================================\n");
    printf("Client is working. Sending five correct packets...\n");
    printf("========================================================\n");

    //create and send five correct packets
    for (int i = 0; i < 5 ; i++) {
        sendAndReceive(TEST_DATA, 1, i + 1, 0);
    }
    
    //send another five packets include one correct packet and four with error
    printf("========================================================\n");
    printf("Client is still working. Sending another five packets...\n");
    printf("========================================================\n");
    
    //Send a correct packet!
    printf("1. Sending correct packet No. 1...\n");
    sendAndReceive(TEST_DATA,1,1,0);
    
    //Send a packet with error: not in sequence. Segment number is 3, should be 2 
    printf("2. Sending packet No. 2 with error: not in sequence...\n");
    sendAndReceive(TEST_DATA,1,3,0);
    
    //Send a packet with error: length not match. Changed length to 0 
    printf("3. Sending packet No. 3 with error: length not match...\n");
    sendAndReceive(TEST_DATA,1,2,1);
    
    //Send a packet with error: end of packet missing. Removed end of packet id 
    printf("4. Sending packet No. 4 with error: don't have the end of packet ID...\n");
    sendAndReceive(TEST_DATA,1,2,2);
    
    //Send a packet with error: duplicated packet. Resend No. 1 packet 
    printf("5. Sending packet No.5 with error: duplicated packet...\n");
    sendAndReceive(TEST_DATA,1,1,0);
    
    //Send a packet when server is down. Check whether client can detect the problem 
    printf("6. Sending packet to test client reaction when server is down...\n");
    sendAndReceive(TEST_DATA,1,2,0);
    
    close(clientSocket);
    return 0;
}


