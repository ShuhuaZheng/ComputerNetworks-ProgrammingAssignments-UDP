/* 
 * COEN 233 Programming Assignment 2
 * packet.h
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

#define PACKET_SIZE 14
#define MAX_CLIENT_ID 255
#define MAX_PAYLOAD_LENGTH 255
#define MAX_SOURCE_SUBSCRIBER_NO 4294967295

// define frequenlty used data type 
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64; 

//initialize constants
const uint16 START_OF_PACKET_ID = 0XFFFF;
const uint16 END_OF_PACKET_ID = 0XFFFF;

const uint16 ACC_PER = 0XFFF8;
const uint16 NOT_PAID = 0XFFF9;
const uint16 NOT_EXIST = 0XFFFA;
const uint16 ACCESS_OK = 0XFFFB;

const uint8 TECHNOLOGY_2G = '2';
const uint8 TECHNOLOGY_3G = '3';
const uint8 TECHNOLOGY_4G = '4';
const uint8 TECHNOLOGY_5G = '5';

// Subscriber information saved in verification database 
struct Subscriber {
    uint32 cellNum;
    uint8 technology;
    uint8 payStatus;
};

// Packet that sent between client and server 
struct Packet {
    uint8 clientID;
    uint8 segNum;
    uint8 length;
    struct Subscriber subscriber;
};


// Server creates response message based on the Packet received from client 
void createResponseMsg(struct Packet packet, uint8 *replyMsg, uint16 replyType){
    memcpy(replyMsg, &START_OF_PACKET_ID, 2);   
    memcpy(replyMsg + 2, &packet.clientID, 1);  

    switch(replyType) {
        case 0: 
            memcpy(replyMsg + 3, &ACCESS_OK, 2);
            break; 
        case 1: 
            memcpy(replyMsg + 3, &NOT_PAID, 2);
            break; 
        case 2: 
            memcpy(replyMsg + 3, &NOT_EXIST, 2);
    }

    memcpy(replyMsg + 5, &packet.segNum, 1);
    memcpy(replyMsg + 6, &packet.length, 1);
    memcpy(replyMsg + 7, &packet.subscriber.technology, 1);
    memcpy(replyMsg + 8, &packet.subscriber.cellNum, 4);

    memcpy(replyMsg + 12, &END_OF_PACKET_ID, 2);
    printf("\n");
}

// Client creates request message 
void createRequestMsg(uint8 *msg, struct Packet packet) {
    memcpy(msg, &START_OF_PACKET_ID, 2);
    memcpy(msg + 2, &packet.clientID, 1);
    memcpy(msg + 3, &ACC_PER, 2);
    memcpy(msg + 5, &packet.segNum, 1);
    memcpy(msg + 6, &packet.length, 1);
    memcpy(msg + 7, &packet.subscriber.technology, 1);
    memcpy(msg + 8, &packet.subscriber.cellNum, 4);
    memcpy(msg + 12, &END_OF_PACKET_ID, 2);
}

//read Message and get Packet
struct Packet parseMsgServer(uint8 *msg) {
    struct Packet Packet;
   
    memcpy(&Packet.clientID, msg + 2, 1);
    memcpy(&Packet.segNum, msg + 5, 1);
    memcpy(&Packet.length, msg + 6, 1);
    memcpy(&Packet.subscriber.technology, msg + 7, 1);
    memcpy(&Packet.subscriber.cellNum, msg + 8, 4);
    
    return Packet;
}
