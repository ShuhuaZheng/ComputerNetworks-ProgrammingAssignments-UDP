/* 
 * COEN 233 Programming Assignment 2 
 * client.c
 * 
 * Student Name: Shuhua Zheng 
 * Student ID: 1289984
 *
 */ 

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <unistd.h>

#include "packet.h"

#define PORT 1234
#define MAXDATASIZE 1024 


int clientSocket;   // create a socket 
struct sockaddr_in server; // a structure containing an internet address 
socklen_t addrSize; // socklen_t is an unsigned opaque integral type of 
                    // len of at least 32 bits

//Initial array for storing the message receive from server
uint8 receiveMessage[MAXDATASIZE];

//print the sending or receiving message
void printMsg(unsigned char *string, int size) {
    switch(string[3]) {
        case 0XF9: 
            printf("Server Response: Not Paid.\n");
            break; 
        case 0XFA: 
            printf("Server Response: Subscriber does not exist.\n");
            break; 
        case 0XFB: 
            printf("Server Response: Access permitted.\n");
    }
    
    //print receive response in byte
    for (int i = 0; i < size; i++) {
        if (i > 0)
            printf(":");
        printf("%02X", *(string + i));
    }

    printf("\n");
}

//send message to server and receive message from server
void sendAndGet(uint8 clientId, uint8 segNum, uint64 cellNum, uint8 technology) {
    uint8 payloadLen = 5;    // Technology(1 byte) + Source Subscriber No(4 bytes)

    struct Packet packet; 

    if (clientId > 255) {
        packet.clientID = 0; 
    } else {
        packet.clientID = clientId; 
    }
    
    packet.segNum = segNum; 
    packet.subscriber.technology = technology; 

    if (cellNum > 4294967295) {
        packet.subscriber.cellNum = 0; 
    } else {
        packet.subscriber.cellNum = cellNum; 
    }
    
    packet.length = payloadLen; 
    
    uint8 sendMsg[PACKET_SIZE]; // used to store the message to be sent 

    createRequestMsg(sendMsg, packet);

    //Send message to server
    sendto(clientSocket, sendMsg, PACKET_SIZE, 0, (struct sockaddr *) &server, addrSize);
    
    //print send message
    printf("Sending packet to server...\n");
    printMsg(sendMsg, PACKET_SIZE);
    printf("\n");
    
    struct timeval timer;
    timer.tv_sec = 3;      // set timeout 3 sec
    timer.tv_usec = 0; 
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timer, sizeof(timer)) != 0)
        perror("ERROR: setsockopt failed.\n");
    
    //set receive size and sending message counter
    int recvSize = -1;
    int counter = 3;

    while (counter > 0) {
        //receive message from server
        recvSize = (int) recvfrom(clientSocket, receiveMessage, MAXDATASIZE, 0, (struct sockaddr *) &server, &addrSize);

        if(recvSize < 0)
            perror("ERROR: recvfrom error");
        else
            break;

        counter--;
    }

    //after 3 times, if recvSize still = -1, server does not respond
    if (recvSize < 0) {
        printf("Server does not respond.\n\n");
        exit(1);
    } else {
        printMsg(receiveMessage, recvSize);
    }
    printf("\n");
    printf("**********************************************\n"); 

}

int main() {
    // create UDP socket, the arguments are domain, type, protocol 
    // and check whether the socket has been created successfully 
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("ERROR: creating socket failed.\n"); 
        exit(1); 
    }

    // set configuration 
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // set size variable
    addrSize = sizeof(server);
    
    printf("1. Existing subscriber with paid status.\n");
    sendAndGet(1, 1, 4085546805, TECHNOLOGY_4G);

    printf("2. Subscriber does not exist because subscriber number cannot be found.\n");
    sendAndGet(1, 2, 4086200285, TECHNOLOGY_4G);

    printf("3. Subscriber does not exist because subscriber number cannot be found.\n");
    printf("Testing boundary condition: subscriber number goes beyond 4294967295\n");
    sendAndGet(1, 3, 8888888888, TECHNOLOGY_4G);

    printf("4. Subscriber does not exist because technology does not match.\n");
    sendAndGet(1, 4, 4086808821, TECHNOLOGY_5G);

    printf("5. Existing subscriber with unpaid status.\n");
    sendAndGet(1, 5, 4086668821, TECHNOLOGY_3G);

    printf("6. Server is not working.\n");
    sendAndGet(1, 6, 4086808821, TECHNOLOGY_2G);

    close(clientSocket);
    return 0;
}
