/* 
 * COEN 233 Programming Assignment 2
 * server.c
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
#include <unistd.h>

#include "packet.h"

#define PORT 1234
#define MAXDATASIZE 1024


//absolute path of file, used to verify info sent by client 
const uint8 database[] = "/home/fergie/Desktop/ProgrammingAssignment2/verification_database.txt";

//get subscriber data from databse and put data into an array 
//called subscribers. return the number of subscribers
int extractData(struct Subscriber subscribers[], const char *database) {
    printf("**********************************************************\n");
    printf("  Extracting Subscriber Data from Verfication Database...\n");
    printf("**********************************************************\n");
    
    FILE *file;
    file = fopen(database, "r"); // open database and read the data 
    if (file == NULL)           // check if the file has been opened succseefully 
        perror("ERROR: open file failed.\n");

    size_t lineSize= 0;
    char *line = NULL;
    uint8 cellNum[11];
    //uint8 usrData[18];     
    uint8 usrData[17]; 

    getline(&line, &lineSize, file);    // skip the title line by not saving the content of line 
    
    int count = 0;                      //index of the subscriber
    printf("Subscriber Number\tTechnology\tPaid\n");

    while ((getline(&line, &lineSize, file)) != -1) {
        memcpy(usrData, line, 17); 

        for (int i = 0, j = 0; i < 12; i++) {
            if (usrData[i] != '-') {
                cellNum[j] = usrData[i];
                j++;
            }
        }
        
        cellNum[10] = '\0';  //append end of string
        subscribers[count].cellNum= (uint32) atol((const char*) cellNum);
        subscribers[count].technology =  usrData[14];
        subscribers[count].payStatus = usrData[16];
        
        // print all the subscribers' info to screen 
        printf("%u\t\t0%c\t\t%c\n",subscribers[count].cellNum,subscribers[count].technology,subscribers[count].payStatus);
        count++;
    }

    fclose(file);   // close file after extraction finishes
    
    return count;   // return the number of subscribers 
}


int main() {
    int serverSocket; 
    struct sockaddr_in server; // a structure containing an internet address 
    socklen_t addrSize;

    // create UDP socket, the arguments are domain, type, protocol 
    // and check whether the socket has been created successfully 
    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("ERROR: Creating socket failed.\n"); 
        exit(1); 
    }

    // set configuration 
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Check if server has been binded successfully 
    if (bind(serverSocket, (struct sockaddr *) &server, sizeof(server)) == -1) {
        perror("ERROR: Binding socket failed.\n");
        exit(1); 
    }

    struct Subscriber subscribers[20];  // used to store all the subscribers in database 

    //read file and store all the subscribers' info in subscribers 
    int count = extractData(subscribers, (const char*) database);
    
    printf("**********************************************************\n");
    printf(" Server is working. Feed me packets...\n");
    printf("**********************************************************\n");
    
    int counter = 0;

    addrSize = sizeof(server);  //Initialize size variable
    
    //while loop for receive packet from client
    while(1) {
        if(counter == 5) // test 5 normal packets and close server after that 
            break;
        else
            counter++;
        
        //receive any incoming UDP datagram.
        uint8 recvbuffer[MAXDATASIZE];  
        memset(recvbuffer, 0, MAXDATASIZE); // clear previous data

        int recvSize = - 1; 
        recvSize = (int) recvfrom(serverSocket, recvbuffer, MAXDATASIZE, 0, (struct sockaddr *) &server, &addrSize);

        if (recvSize == -1) {
            perror("ERROR: recvfrom error.\n"); 
            break; 
        }

        // convert the message received from client to a packet 
        struct Packet packet = parseMsgServer(recvbuffer);

        //process permission request packet
        printf("Receiving packets from client %d...\n\n", packet.clientID);
         

        for (int i = 0; i < count; i++) {   // loop through the subscriber in subscribers 
            uint8 respondMessage[PACKET_SIZE];

            if (packet.subscriber.cellNum != subscribers[i].cellNum && i != 2) {
                continue; 
            } else if (packet.subscriber.cellNum != subscribers[i].cellNum && i == 2) {
                    printf("Sorry, subscriber does not exits.\n"); 
                    printf("Reason: cannot find subscriber's number.\n");
                    printf("*************************************************************\n"); 

                    createResponseMsg(packet, respondMessage, 2);
                    sendto(serverSocket, respondMessage, PACKET_SIZE, 0, (struct sockaddr *) &server, addrSize);

            } else if (packet.subscriber.cellNum == subscribers[i].cellNum) {

                if (subscribers[i].technology == packet.subscriber.technology) {
                    if (subscribers[i].payStatus == '0') {
                        printf("Congratulations, subscriber exits.\n");
                        printf("Subscriber Number: found\nTechnology: matched\nPayment Status: Not Paid\n");
                        printf("*************************************************************\n"); 
                            
                        createResponseMsg(packet, respondMessage, 1);
                        sendto(serverSocket, respondMessage, PACKET_SIZE, 0, (struct sockaddr *) &server, addrSize);
                    } else {
                        printf("Subscriber exits.\n");
                        printf("Subscriber number: found\nTechnology: matched\nPayment Status: Paid\n");
                        printf("*************************************************************\n"); 

                        createResponseMsg(packet, respondMessage, 0);
                        sendto(serverSocket, respondMessage, PACKET_SIZE, 0, (struct sockaddr *) &server, addrSize);
                    }

                } else {
                    printf("Sorry, subscriber does not exits.\n"); 
                    printf("Reason: found subscriber number but technology does not match.\n");
                    printf("*************************************************************\n"); 

                    createResponseMsg(packet, respondMessage, 2);
                    sendto(serverSocket, respondMessage, PACKET_SIZE, 0, (struct sockaddr *) &server, addrSize);
                }

                i = 2; 
            }

        }
    }
    
    close(serverSocket);
    return 0;
}