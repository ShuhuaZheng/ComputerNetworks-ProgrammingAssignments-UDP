/* 
 * COEN 233 Programming Assignment 1 
 * packet.h
 * 
 * Student Name: Shuhua Zheng 
 * Student ID: 1289984
 *
 */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CLIENT_ID_MAX   255
#define PAYLOAD_LENGTH_MAX  255

typedef unsigned char uint8; // used for 1 byte 
typedef unsigned short uint16; // used for 2 bytes 

//init constants
const uint16 PACKET_ID_START = 0XFFFF;
const uint16 PACKET_ID_END = 0XFFFF;

enum TYPES_OF_PACKET {
    DATA = 0XFFF1,
    ACK = 0XFFF2,
    REJECT = 0XFFF3
};

// frist 4 are Reject Sub Codes, the last one is ACK 
enum SERVER_REPLY_CODE {
    OUT_OF_SEQUENCE = 0XFFF4,
    LENGTH_MISMATCH = 0XFFF5,
    PACKET_ID_END_MISSING = 0XFFF6,
    DUPLICATE_PACKET = 0XFFF7,
    NO_ERROR = 0XFFF8   // namely, ACK 
};

struct PACKET {
    uint8 clientId;
    enum TYPES_OF_PACKET packetType;
    uint8 segmentNum;
    uint8 length;
    char *payload;
    enum SERVER_REPLY_CODE reply ;
};

// used by server to creat Ack or Rej Packet 
int createAckRejPack(struct PACKET packet, uint8* msg) {
    memcpy(msg, &PACKET_ID_START, 2);       // copy start of Packet id to msg, 2 bytes
    memcpy(msg + 2, &packet.clientId, 1);   // copy client ID of Packet id to msg, 1 byte  
    memcpy(msg + 3, &packet.packetType, 2); // copy packetType to msg, 2 bytes  

    int size = 0;  //size of the message

    switch (packet.packetType) {
        case ACK:
            memcpy(msg + 5, &packet.segmentNum, 1); // copy segment No to msg, 1 byte 
            size = 5 + 1;                           // update size 
            break;

        case REJECT:
            memcpy(msg + 5, &packet.reply , 2); // copy Reject sub Code to msg, 2 bytes 
            memcpy(msg + 7, &packet.segmentNum, 1); // copy Segment Number to msg, 1 byte
            size = 7 + 1;                           // update size 
    }

    memcpy(msg + size, &PACKET_ID_END, 2);
    return size + 2;
}

// used by client to create Data packet 
int createDataPack(struct PACKET packet, uint8* msg) {
    memcpy(msg, &PACKET_ID_START, 2);       // copy start of Packet id to msg, 2 bytes
    memcpy(msg + 2, &packet.clientId, 1);   // copy client ID of Packet id to msg, 1 byte  
    memcpy(msg + 3, &packet.packetType, 2); // copy packetType to msg, 2 bytes  

    int size = 0;  //size of the message

    memcpy(msg + 5, &packet.segmentNum, 1);         // copy segment No to msg, 1 byte 

    if (packet.length > PAYLOAD_LENGTH_MAX)         // check whether packet len <= 255 
        packet.length = 255;                        // adjust the packet len 

    memcpy(msg + 6, &packet.length, 1);             // copy packet length to msg, 1 byte  
    memcpy(msg + 7, packet.payload, packet.length); // copy packet content, namely payload to msg, max 255 bytes  

    size = 7 + packet.length;   // update size 

    memcpy(msg + size, &PACKET_ID_END, 2); // append packet ID end to msg 

    return size + 2;
}

//SERVER: decode the received message and decide the type of SERVER_REPLY_CODE to reply 
enum SERVER_REPLY_CODE parseMsgServer (uint8* receivedMsg, long msgLen, struct PACKET* packet, int* sequence) {
    memcpy(&packet->clientId, receivedMsg + 2, 1);
    memcpy(&packet->packetType, receivedMsg + 3, 2);

    int expectedSeq = sequence[packet->clientId] + 1;
    memcpy(&packet->segmentNum, receivedMsg + 5, 1);
    memcpy(&packet->length, receivedMsg + 6, 1);

    // check whether the end of packet is missing 
    if (memcmp(&PACKET_ID_END, receivedMsg + msgLen - 2, 2) != 0) 
        return PACKET_ID_END_MISSING;

    // check whether length of payload corresponds with Length of packet 
    if (7 + packet->length + 2 != msgLen) 
        return LENGTH_MISMATCH;
    
    // check whether the segment number of packet is correct 
    if (packet->segmentNum < expectedSeq) {
        return DUPLICATE_PACKET;
    } else if (packet->segmentNum > expectedSeq) {
        return OUT_OF_SEQUENCE;
    } else {
        // update current sequence number for next check of next packet
        sequence[packet->clientId] = packet->segmentNum;
        if(sequence[packet->clientId] == 5)
            sequence[packet->clientId] = 0;
    }
      
    char data[packet->length + 1];  // 
    data[packet->length] = 0;    // use 0 to mark the end of string
    memcpy(data, receivedMsg + 7, packet->length);
    packet->payload = data;
        
    return NO_ERROR;
}

//Client: decode the received message and decide whether it's ACK or REJ 
enum SERVER_REPLY_CODE parseMsgClient(uint8* receivedMsg, long msgLen, struct PACKET* packet) {
    memcpy(&packet->clientId, receivedMsg + 2, 1);
    memcpy(&packet->packetType, receivedMsg + 3, 2);

    switch (packet->packetType) {
        case ACK:
            memcpy(&packet->segmentNum, receivedMsg + 5, 1);
            break;
        case REJECT:
            memcpy(&packet->reply , receivedMsg + 5, 2);
            memcpy(&packet->segmentNum, receivedMsg + 7, 1);
    }
    return NO_ERROR;
}


