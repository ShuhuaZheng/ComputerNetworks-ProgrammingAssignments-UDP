/* 
 * COEN 233 Programming Assignment 1 
 * server.c
 * 
 * Student Name: Shuhua Zheng 
 * Student ID: 1289984
 *
 */ 

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "packet.h"

#define PORT 1234
#define MAXDATASIZE 1024

int main() {
    int serverSocket;
    struct sockaddr_in server; //a structure containing an internet address
    socklen_t addrSize;

    // create UDP socket, the arguments are domain, type, protocol
    // and check whether the socket has been created successfully 
    if((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("ERROR: Creating socket failed.\n");
        exit(1);
    }

    // set configuration 
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");


    // Check if sever has been binded successfully 
    if(bind(serverSocket, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror("ERROR: Binding socket failed.\n");
        exit(1);
    }

    
    printf("===========================================\n");
    printf("Server is running. Feed me packets....\n");
    printf("===========================================\n");

    int sequence[10] = {0}; // used to check store and check previous segments number 
    int testTimes = 0;

    while(11) {
        if(testTimes == 10) // already received 10 packets, stops 
            break;
        else
            testTimes++;

        uint8 buffer[MAXDATASIZE];
        addrSize = sizeof(server);
        
        fflush(stdout); // used to clear the buffer and accept the next string 
        
        // start receiving client message
        int recvSize = (int) recvfrom(serverSocket, buffer, 1024, 0, (struct sockaddr *) &server, &addrSize);
        if (recvSize == -1) {
            perror("ERROR: recvfrom error");
            break;
        }

        //set received data packet
        struct PACKET receivedDataPack;
        enum SERVER_REPLY_CODE reply  = parseMsgServer(buffer, recvSize, &receivedDataPack, sequence);

        // make response packet as an ACK packet
        struct PACKET packet;
        packet.clientId = receivedDataPack.clientId;
        packet.packetType = ACK;
        packet.segmentNum = receivedDataPack.segmentNum;

        // check whether there is a reject error
        switch (reply) {
            case OUT_OF_SEQUENCE:
                printf("\nWrong packet:\n");
                printf("ERROR: out of sequence\n");
                break;
            case LENGTH_MISMATCH:
                printf("\nWrong packet:\n");
                printf("ERROR: length mismatch\n");
                break;
            case PACKET_ID_END_MISSING:
                printf("\nWrong packet:\n");
                printf("ERROR: end of packet id missing\n");
                break;
            case DUPLICATE_PACKET:
                printf("\nWrong packet:\n");
                printf("ERROR: duplicate packet\n");
                break;
            case NO_ERROR:
                printf("\nReceived correct packet NO. %d \n", receivedDataPack.segmentNum);
                break;
        }

        if (reply != NO_ERROR) {
            packet.packetType = REJECT; 
            packet.reply = reply; 
        }

        // make response packet to buffer and get the packet length
        int packLen = createAckRejPack(packet, buffer);

        // send packet to client and check whether there is an error 
        if (sendto(serverSocket, buffer, packLen, 0, (struct sockaddr *) &server, addrSize) == -1) {
            perror("Error: sendto error");
            break;
        }

    }
    
    close(serverSocket);
    return 0;
}
