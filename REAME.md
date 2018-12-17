# Programming Assignments for Computer Networks Course 

## Programming Assignment 1 
One client connects to one server based using customized protocol on top of UDP protocol for sending information to the server. 

First, client sends five correct packets to the server and the server acknowledges with ACK for the five correct packets. 

Then, client sends another five packets to the server, emulating one correct packet and four packets with errors. The server acknowledges with ACK for the correct packet and with corresponding Rejct sub codes for packets with errors. 

Finally, server will close after receiving total 10 packets. Client will send one more packet to server after server is closed and the client will start an ack_timer for the packet. If the ACK for the packet does not arrive before the timeout, the client will retransmit the packet and restart the ack_timer. The ack_timer will be reset for a total of 3 times. If no ACK was received from the server after sending the same packet 3 times, the client will generate "Server does not respond". 

### Primitives of Protocol - Assignment 1
Start of Packet Identifier ...... 0XFFFF
End of Packet Identifier ...... 0XFFFF
Client ID ...... Maximum 0XFF (255 Decimal)
Length ...... Maximum 0XFF (255 Decimal)

### Packet Types - Assignment 1
DATA ...... 0XFFF1
ACK ...... 0XFFF2
REJECT ...... 0XFFF3

### Reject Sub Codes - Assignment 1 
REJECT out of sequence ...... 0XFFF4
REJECT length mismatch ...... 0XFFF5
REJECT end of packet missing ...... 0XFFF6 
REJECT duplicate packet ...... 0XFFF7 

### Data Packet Format - Assignment 1 (in bytes)
|Start of Packet Id(2)|Client ID(1)|DATA(2)|Seg No(1)|Len(1)|Payload(255)|End of Packet id(2)| 
|Start of Packet Id(2)|Client ID(1)|ACK(2)|Seg No(1)|End of Packet id(2)| 
|Start of Packet Id(2)|Client ID(1)|REJECT(2)|Reject sub code(2)|Seg No(1)|End of Packet id(2)| 

#--------------------------------------------------------------------------------------------------------------------------

## Programming Assignment 2 
One client requests identification from server using customized protocol on top of UDP protocol for access permission to the network. Server will verify the validity of the request and respond accordingly. 

Server opens and reads a file named "Verification_Database.txt", which contains the subscriber's Number, Technology and payment status (paid = 1, not paid = 0). 

Client will send 5 packets to server while server is running and client will send one more packet to server after server is closed. The client will start an ack_timer for the packet. If the ACK for the packet does not arrive before the timeout, the client will retransmit the packet and restart the ack_timer. The ack_timer will be reset for a total of 3 times. If no ACK was received from the server after sending the same packet 3 times, the client will generate "Server does not respond". 

### Technologies - Assignment 2 
2G ...... 02
3G ...... 03
4G ...... 04
5G ...... 05

### Primitives of Protocol - Assignment 2
Start of Packet Identifier ...... 0XFFFF
End of Packet Identifier ...... 0XFFFF
Client ID ...... Maximum 0XFF (255 Decimal)
Length ...... Maximum 0XFF (255 Decimal)
Source Subscriber No ...... Maximum 0XFFFFFFFF (4294967295 Decimal)

### Packet Types - Assignment 2
Acc_Per ...... 0XFFF8
Not Paid ...... 0XFFF9
Not Exist ...... 0XFFFA

### Data Packet Format - Assignment 2 (in Bytes)
|Start of Packet Id(2)|Client ID(1)|Acc_per(2)|Seg No(1)|Len(1)|Tech(1)|Subsrciber Num(4)|End of Packet Id(2)| 
|Start of Packet Id(2)|Client ID(1)|Not Paid(2)|Seg No(1)|Len(1)|Tech(1)|Subsrciber Num(4)|End of Packet id(2)| 
|Start of Packet Id(2)|Client ID(1)|Not Exist(2)|Seg No(1 byte)|Len(1)|Tech(1)|Subsrciber Num(4)|End of Packet id(2)| 

#--------------------------------------------------------------------------------------------------------------------------

##Installing and Running 
Clone/Download the project. Use ./server to run the server. Use ./client to run the client.  

## Built With C 